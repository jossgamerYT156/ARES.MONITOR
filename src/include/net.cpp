#pragma once
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef __linux__
// Linux Only Headers.
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

extern std::vector<std::string> session_errors;
extern int last_error_code;

/**
 * @brief LDS Network Utilities for ARES.MONITOR
 * @note  Command implementations are Linux-only. Requires -lssl -lcrypto.
 *        The namespace is always visible to all targets so the shell.cpp
 *        command map can reference it without ifdef guards at the call site.
 */
namespace ARES {
    extern bool has_flag(const std::vector<std::string> &args, const std::string &flag);
namespace MODULES::NETWORK {
using namespace ARES;
// this kinda doesn't work on macOS Yet... and you kinda don't wanna remove this ifdef... just saying...
#ifdef __linux__

struct ParsedURL {
  std::string scheme;
  std::string host;
  std::string port;
  std::string path;
  bool tls;
};

struct HTTPResponse {
  int status_code = 0;
  std::string status_text;
  std::string content_type;
  std::string body;
  size_t body_size = 0; // decoded byte count
  long long ping_ms = 0;
  long long response_ms = 0;
  long long dl_ms = 0;
};

static bool parse_url(const std::string &url, ParsedURL &out) {
  out = {};
  size_t scheme_end = url.find("://");
  if (scheme_end == std::string::npos)
    return false;
  out.scheme = url.substr(0, scheme_end);
  out.tls = (out.scheme == "https");
  std::string rest = url.substr(scheme_end + 3);
  size_t path_start = rest.find('/');
  std::string hostport =
      (path_start == std::string::npos) ? rest : rest.substr(0, path_start);
  out.path = (path_start == std::string::npos) ? "/" : rest.substr(path_start);
  size_t colon = hostport.rfind(':');
  if (colon != std::string::npos) {
    out.host = hostport.substr(0, colon);
    out.port = hostport.substr(colon + 1);
  } else {
    out.host = hostport;
    out.port = out.tls ? "443" : "80";
  }
  return !out.host.empty();
}

static int connect_to(const std::string &host, const std::string &port) {
  struct addrinfo hints{}, *res = nullptr;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0)
    return -1;
  int fd = -1;
  for (auto *p = res; p; p = p->ai_next) {
    fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd < 0)
      continue;
    if (connect(fd, p->ai_addr, p->ai_addrlen) == 0)
      break;
    close(fd);
    fd = -1;
  }
  freeaddrinfo(res);
  return fd;
}

static std::string build_request(const std::string &method,
                                 const ParsedURL &url,
                                 const std::string &mime_type,
                                 const std::string &body) {
  std::ostringstream req;
  req << method << " " << url.path << " HTTP/1.1\r\n";
  req << "Host: " << url.host << "\r\n";
  req << "User-Agent: ARES.MONITOR/NET\r\n";
  req << "Connection: close\r\n";
  if (!body.empty()) {
    req << "Content-Type: "
        << (mime_type.empty() ? "application/octet-stream" : mime_type)
        << "\r\n";
    req << "Content-Length: " << body.size() << "\r\n";
  }
  req << "\r\n" << body;
  return req.str();
}

// ── Chunked transfer decode ──────────────────────────────────────────────────
// Strips Transfer-Encoding: chunked framing. Returns clean body.
// If body is not chunked or is malformed, returns input unchanged.

static std::string decode_chunked(const std::string &raw) {
  std::string out;
  size_t pos = 0;
  while (pos < raw.size()) {
    size_t crlf = raw.find("\r\n", pos);
    if (crlf == std::string::npos)
      break;
    std::string size_str = raw.substr(pos, crlf - pos);
    size_t semi = size_str.find(';');
    if (semi != std::string::npos)
      size_str = size_str.substr(0, semi);
    size_t chunk_size = 0;
    try {
      chunk_size = std::stoul(size_str, nullptr, 16);
    } catch (...) {
      break;
    }
    if (chunk_size == 0)
      break;
    pos = crlf + 2;
    if (pos + chunk_size > raw.size())
      break;
    out.append(raw, pos, chunk_size);
    pos += chunk_size + 2;
  }
  return out.empty() ? raw : out;
}

// ── Response parser ──────────────────────────────────────────────────────────

static HTTPResponse parse_response(const std::string &raw, long long ping_ms,
                                   long long response_ms, long long dl_ms) {
  HTTPResponse r;
  r.ping_ms = ping_ms;
  r.response_ms = response_ms;
  r.dl_ms = dl_ms;

  size_t header_end = raw.find("\r\n\r\n");
  std::string headers =
      (header_end != std::string::npos) ? raw.substr(0, header_end) : raw;
  r.body = (header_end != std::string::npos) ? raw.substr(header_end + 4) : "";

  std::istringstream ss(headers);
  std::string status_line;
  std::getline(ss, status_line);
  size_t sp1 = status_line.find(' ');
  if (sp1 != std::string::npos) {
    size_t sp2 = status_line.find(' ', sp1 + 1);
    r.status_code = std::stoi(status_line.substr(
        sp1 + 1,
        (sp2 != std::string::npos) ? sp2 - sp1 - 1 : std::string::npos));
    r.status_text =
        (sp2 != std::string::npos) ? status_line.substr(sp2 + 1) : "";
    if (!r.status_text.empty() && r.status_text.back() == '\r')
      r.status_text.pop_back();
  }

  bool chunked = false;
  std::string line;
  while (std::getline(ss, line)) {
    std::string lower = line;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.substr(0, 13) == "content-type:") {
      r.content_type = line.substr(13);
      while (!r.content_type.empty() && r.content_type.front() == ' ')
        r.content_type.erase(r.content_type.begin());
      if (!r.content_type.empty() && r.content_type.back() == '\r')
        r.content_type.pop_back();
    }
    if (lower.find("transfer-encoding:") != std::string::npos &&
        lower.find("chunked") != std::string::npos)
      chunked = true;
  }

  if (chunked)
    r.body = decode_chunked(r.body);
  r.body_size = r.body.size();
  return r;
}

// ── Core send/receive (plain + TLS) ─────────────────────────────────────────

static bool do_request(const ParsedURL &url, const std::string &method,
                       const std::string &mime_type, const std::string &body,
                       bool follow_redirect, bool verbose,
                       HTTPResponse &out_response, std::string &out_error,
                       int hop = 0) {

  if (hop > 6) {
    out_error = "TMR";
    return false;
  }

  auto t0 = std::chrono::steady_clock::now();
  int fd = connect_to(url.host, url.port);
  if (fd < 0) {
    out_error = "CONNECTION FAILED";
    return false;
  }

  auto t1 = std::chrono::steady_clock::now();
  long long ping_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  std::string request_str = build_request(method, url, mime_type, body);

  if (verbose) {
    printf("[NET:VERBOSE] Connecting to %s:%s%s\n", url.host.c_str(),
           url.port.c_str(), url.path.c_str());
    printf("[NET:VERBOSE] Sending:\n%s\n", request_str.c_str());
  }

  std::string raw_response;

  if (url.tls) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
      close(fd);
      out_error = "TLS CONTEXT FAILED";
      return false;
    }
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    SSL_set_tlsext_host_name(ssl, url.host.c_str());
    if (SSL_connect(ssl) != 1) {
      SSL_free(ssl);
      SSL_CTX_free(ctx);
      close(fd);
      out_error = "TLS HANDSHAKE FAILED";
      return false;
    }
    if (verbose)
      printf("[NET:VERBOSE] TLS handshake OK\n");
    SSL_write(ssl, request_str.c_str(), (int)request_str.size());
    auto t2 = std::chrono::steady_clock::now();
    char buf[4096];
    int n;
    bool first = true;
    long long response_ms = 0, dl_ms = 0;
    while ((n = SSL_read(ssl, buf, sizeof(buf))) > 0) {
      if (first) {
        response_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - t2)
                          .count();
        first = false;
      }
      raw_response.append(buf, n);
    }
    dl_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t2)
                .count() -
            response_ms;
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(fd);
    out_response = parse_response(raw_response, ping_ms, response_ms, dl_ms);
  } else {
    send(fd, request_str.c_str(), request_str.size(), 0);
    auto t2 = std::chrono::steady_clock::now();
    char buf[4096];
    ssize_t n;
    bool first = true;
    long long response_ms = 0, dl_ms = 0;
    while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) {
      if (first) {
        response_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - t2)
                          .count();
        first = false;
      }
      raw_response.append(buf, n);
    }
    dl_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t2)
                .count() -
            response_ms;
    close(fd);
    out_response = parse_response(raw_response, ping_ms, response_ms, dl_ms);
  }

  if (follow_redirect &&
      (out_response.status_code == 301 || out_response.status_code == 302 ||
       out_response.status_code == 303 || out_response.status_code == 307 ||
       out_response.status_code == 308)) {
    size_t loc = raw_response.find("Location:");
    if (loc == std::string::npos)
      loc = raw_response.find("location:");
    if (loc != std::string::npos) {
      size_t start = raw_response.find(' ', loc) + 1;
      size_t end = raw_response.find("\r\n", start);
      std::string new_url = raw_response.substr(start, end - start);
      // Always notify on redirect — this is part of REDIRECT semantics, not
      // just VERBOSE
      printf("[NET:REDIRECT] %s -> %s (hop %d)\n", url.host.c_str(),
             new_url.c_str(), hop + 1);
      if (verbose)
        printf("[NET:VERBOSE] Following redirect (hop %d/6)\n", hop + 1);
      ParsedURL redirect_url;
      if (!parse_url(new_url, redirect_url)) {
        out_error = "REDIRECT URL MALFORMED";
        return false;
      }
      return do_request(redirect_url, method, mime_type, body, follow_redirect,
                        verbose, out_response, out_error, hop + 1);
    }
  }
  return true;
}

// ── Output printer ───────────────────────────────────────────────────────────

// mime_base strips charset and params: "text/html; charset=utf-8" ->
// "text/html"
static std::string mime_base(const std::string &mime) {
  size_t semi = mime.find(';');
  std::string base = (semi != std::string::npos) ? mime.substr(0, semi) : mime;
  while (!base.empty() && base.back() == ' ')
    base.pop_back();
  return base;
}

// expected_mime: empty = accept any. Non-empty = must prefix-match response
// MIME.
static void print_response(const HTTPResponse &r,
                           const std::string &expected_mime = "") {
  printf("Status: %d %s\n", r.status_code, r.status_text.c_str());
  printf("____________________\n");

  bool mime_mismatch = false;
  if (!expected_mime.empty()) {
    std::string actual_base = mime_base(r.content_type);
    if (actual_base != expected_mime)
      mime_mismatch = true;
  }

  if (mime_mismatch) {
    printf("W: \"Malformed response: server MIME does not match expected "
           "type\"\n");
  } else {
    printf("Body:\n\"\"\"\n%s\n\"\"\"\n", r.body.c_str());
  }

  printf("____________________\n");
  printf("Stats:\n");
  printf("  Ping Time:     %lldms\n", r.ping_ms);
  printf("  Response Time: %lldms\n", r.response_ms);
  printf("  DL Time:       %lldms\n", r.dl_ms);
  printf("  Size:          %zu bytes\n", r.body_size);
  if (mime_mismatch)
    printf("  MIME:          %s - EXPECTED: %s\n",
           r.content_type.empty() ? "unknown" : r.content_type.c_str(),
           expected_mime.c_str());
  else
    printf("  MIME:          %s\n",
           r.content_type.empty() ? "unknown" : r.content_type.c_str());
}

// ── Argument helpers ─────────────────────────────────────────────────────────

static std::string get_flag_value(const std::vector<std::string> &args,
                                  const std::string &flag) {
  for (size_t i = 0; i + 1 < args.size(); ++i)
    if (args[i] == flag)
      return args[i + 1];
  return "";
}
// ── \@REQUEST ────────────────────────────────────────────────────────────────
//
//   \@REQUEST HTTP "<url>" [\EXPECT "<code>"] [\WITH-DATA "<mime/type>"]
//   [VERBOSE]
//
void handle_networkRequest(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    session_errors.push_back(
        "[NET:'\\@REQUEST' MISSING ARGS]:['\\@REQUEST':\"Expected HTTP <url>\" "
        "- \"SYNTAX\"]");
    last_error_code = 1;
    return;
  }
  std::string url_str = args[2];
  std::string expect = get_flag_value(args, "EXPECT");
  std::string mime = get_flag_value(args, "WITH-DATA");
  bool verbose = has_flag(args, "VERBOSE");

  ParsedURL url;
  if (!parse_url(url_str, url)) {
    session_errors.push_back("[NET:'\\@REQUEST' INVALID URL]:['\\@REQUEST' '" +
                             url_str +
                             "':\"URL is malformed\" - \"MALFORMED\"]");
    last_error_code = 1;
    return;
  }
  if (verbose)
    printf("[NET:VERBOSE] \\@REQUEST -> %s\n", url_str.c_str());

  HTTPResponse response;
  std::string error;
  if (!do_request(url, "GET", "", "", false, verbose, response, error)) {
    session_errors.push_back("[NET:'\\@REQUEST' " + error +
                             "]:['\\@REQUEST' '" + url_str +
                             "':\"Request failed\" - \"NETWORK\"]");
    last_error_code = 1;
    return;
  }
  print_response(response, mime);

  if (!expect.empty()) {
    std::string code_str = std::to_string(response.status_code);
    if (code_str != expect) {
      session_errors.push_back(
          "[NET:'\\@REQUEST' UNEXPECTED STATUS]:['\\@REQUEST' '" + url_str +
          "':\"Expected " + expect + " got " + code_str + "\" - \"" + code_str +
          "\"]");
      last_error_code = 1;
    }
  }
}

// ── \@FETCH ──────────────────────────────────────────────────────────────────
//
//   \@FETCH FROM "<url>" TO "<path>" [VERBOSE]
//
void handle_networkContentFetch(const std::vector<std::string> &args) {
  std::string url_str = get_flag_value(args, "FROM");
  std::string to_path = get_flag_value(args, "TO");
  bool verbose = has_flag(args, "VERBOSE");

  if (url_str.empty()) {
    session_errors.push_back("[NET:'\\@FETCH' MISSING FROM]:['\\@FETCH':\"No "
                             "URL specified after FROM\" - \"SYNTAX\"]");
    last_error_code = 1;
    return;
  }
  if (to_path.empty()) {
    session_errors.push_back(
        "[NET:'\\@FETCH' MISSING TO]:['\\@FETCH':\"No destination path "
        "specified after TO\" - \"SYNTAX\"]");
    last_error_code = 1;
    return;
  }

  ParsedURL url;
  if (!parse_url(url_str, url)) {
    session_errors.push_back("[NET:'\\@FETCH' INVALID URL]:['\\@FETCH' '" +
                             url_str +
                             "':\"URL is malformed\" - \"MALFORMED\"]");
    last_error_code = 1;
    return;
  }
  if (verbose)
    printf("[NET:VERBOSE] \\@FETCH -> %s -> %s\n", url_str.c_str(),
           to_path.c_str());

  HTTPResponse response;
  std::string error;
  if (!do_request(url, "GET", "", "", true, verbose, response, error)) {
    session_errors.push_back("[NET:'\\@FETCH' " + error + "]:['\\@FETCH' '" +
                             url_str + "':\"Fetch failed\" - \"NETWORK\"]");
    last_error_code = 1;
    return;
  }
  if (response.status_code < 200 || response.status_code >= 300) {
    session_errors.push_back("[NET:'\\@FETCH' BAD STATUS]:['\\@FETCH' '" +
                             url_str + "':\"Server returned " +
                             std::to_string(response.status_code) + "\" - \"" +
                             std::to_string(response.status_code) + "\"]");
    last_error_code = 1;
    return;
  }

  std::ofstream out(to_path, std::ios::binary);
  if (!out.is_open()) {
    session_errors.push_back("[NET:'\\@FETCH' WRITE FAILED]:['" + to_path +
                             "':\"Cannot open file for writing\" - \"ERRNO\"]");
    last_error_code = 1;
    return;
  }
  out.write(response.body.data(), (std::streamsize)response.body.size());
  out.close();
  if (verbose)
    printf("[NET:VERBOSE] Written %zu bytes to %s\n", response.body_size,
           to_path.c_str());
}

// ── \@REACH ──────────────────────────────────────────────────────────────────
//
//   \@REACH TO "<url>" <GET|POST|PUT|PATCH|DELETE> [FROM "<data|@file>"] [INTO
//   "<file>"] [REDIRECT] [DISCARD] [VERBOSE]
//
//   FROM: request body — raw string, or @/path/to/file to read from disk
//   INTO: write response body to this file
//   DISCARD: explicitly ignore response body
//
void handle_networkPing(const std::vector<std::string> &args) {
  std::string url_str = get_flag_value(args, "TO");
  std::string from_val = get_flag_value(args, "FROM");
  std::string into_val = get_flag_value(args, "INTO");
  bool redirect = has_flag(args, "REDIRECT");
  bool discard = has_flag(args, "DISCARD");
  bool verbose = has_flag(args, "VERBOSE");

  static const std::vector<std::string> known_flags = {
      "TO", "FROM", "INTO", "REDIRECT", "DISCARD", "VERBOSE"};
  static const std::vector<std::string> verbs = {"GET", "POST", "PUT", "PATCH",
                                                 "DELETE"};

  std::string method = "GET";
  bool skip_next = false;
  for (size_t i = 0; i < args.size(); ++i) {
    if (skip_next) {
      skip_next = false;
      continue;
    }
    if (std::find(known_flags.begin(), known_flags.end(), args[i]) !=
        known_flags.end()) {
      if (args[i] != "REDIRECT" && args[i] != "DISCARD" && args[i] != "VERBOSE")
        skip_next = true;
      continue;
    }
    if (std::find(verbs.begin(), verbs.end(), args[i]) != verbs.end()) {
      method = args[i];
      break;
    }
  }

  if (url_str.empty()) {
    session_errors.push_back("[NET:'\\@REACH' MISSING TO]:['\\@REACH':\"No URL "
                             "specified after TO\" - \"SYNTAX\"]");
    last_error_code = 1;
    return;
  }

  ParsedURL url;
  if (!parse_url(url_str, url)) {
    session_errors.push_back("[NET:'\\@REACH' INVALID URL]:['\\@REACH' '" +
                             url_str +
                             "':\"URL is malformed\" - \"MALFORMED\"]");
    last_error_code = 1;
    return;
  }

  std::string request_body;
  if (!from_val.empty()) {
    if (from_val.front() == '@') {
      std::string fpath = from_val.substr(1);
      std::ifstream fin(fpath, std::ios::binary);
      if (!fin.is_open()) {
        session_errors.push_back(
            "[NET:'\\@REACH' FROM FILE NOT FOUND]:['" + fpath +
            "':\"Cannot open request body file\" - \"ERRNO\"]");
        last_error_code = 1;
        return;
      }
      request_body = std::string(std::istreambuf_iterator<char>(fin),
                                 std::istreambuf_iterator<char>());
    } else {
      request_body = from_val;
    }
  }

  if (verbose)
    printf("[NET:VERBOSE] \\@REACH -> %s %s%s%s\n", method.c_str(),
           url_str.c_str(), into_val.empty() ? "" : " -> INTO ",
           into_val.c_str());

  HTTPResponse response;
  std::string error;
  if (!do_request(url, method, "", request_body, redirect, verbose, response,
                  error)) {
    session_errors.push_back("[NET:'\\@REACH' " + error + "]:['\\@REACH' '" +
                             url_str + "':\"Request failed\" - \"NETWORK\"]");
    last_error_code = 1;
    return;
  }

  if (discard) {
    printf("Status: %d %s\n", response.status_code,
           response.status_text.c_str());
    printf("____________________\n");
    printf("Stats:\n");
    printf("  Ping Time:     %lldms\n", response.ping_ms);
    printf("  Response Time: %lldms\n", response.response_ms);
    printf("  DL Time:       %lldms\n", response.dl_ms);
    printf("  Size:          %zu bytes\n", response.body_size);
    printf("  MIME:          %s\n", response.content_type.empty()
                                        ? "unknown"
                                        : response.content_type.c_str());
    return;
  }

  if (!into_val.empty()) {
    std::ofstream out(into_val, std::ios::binary);
    if (!out.is_open()) {
      session_errors.push_back(
          "[NET:'\\@REACH' WRITE FAILED]:['" + into_val +
          "':\"Cannot open file for writing\" - \"ERRNO\"]");
      last_error_code = 1;
      return;
    }
    out.write(response.body.data(), (std::streamsize)response.body.size());
    out.close();
    if (verbose)
      printf("[NET:VERBOSE] Response written to %s (%zu bytes)\n",
             into_val.c_str(), response.body_size);
  } else {
    print_response(response);
  }
}

#endif // __linux__

} // namespace MODULES::NETWORK
} // namespace ARES