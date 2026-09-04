
namespace ARES {
#pragma once
#include "std_glbl.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
  namespace RTE::EXTERNALS {
      extern void run_external(const std::string &cmd, const std::vector<std::string> &args);
    }
namespace MODULES {
namespace AEX {

  std::string parserV = "0.0.14-alpha"; // New AEX Preinit Feature, Bumped version.
  // BEFORE: 0.0.13-alpha
  // AFTER : 0.0.14-alpha
  std::string parser_rev = "2026-09-04"; // YY-MM-DD, i finally explained my fucking date format. apparently miracles happen.

// External tokenizer
    extern std::vector<std::string> smart_tokenize(const std::string &input);
    namespace RTE::EXTERNALS
    {
      extern void run_external(const std::string &cmd,const std::vector<std::string> &args);
    }

int CarryFlag = 0;
int ZeroFlag = 0;

// Still unused.
enum AexErrorCode {
  AEX_OK = 0,
  AEX_FILE_NOT_FOUND = 600,
  AEX_UNSUPPORTED_VERSION = 601,
  AEX_INVALID_HEADER = 602,
  AEX_LOOP_MEM_OVERFLOW = 603,
  AEX_MISSING_AEND = 604,
  AEX_MISSING_RET = 605
};

// ==============================
// Internal helpers
// ==============================

static void update_flags() {
  ZeroFlag = (last_error_code != 0) ? 1 : 0;
  CarryFlag = (last_error_code == 0) ? 1 : 0;
}

// Evaluate a condition token against !0 (nonzero/exists)
// token is either a file path or ".ZF"
static bool evaluate_condition(const std::string &token) {
  if (token == ".ZF")
    return ZeroFlag != 0;
  if (token == ".CF")
    return CarryFlag != 0;
  return IO::FileOperations::path_exists(token);
}

// Dispatch a tokenized line through the commands map
static void dispatch_line(const std::vector<std::string> &tokens) {
  if (tokens.empty())
    return;

  auto it = CORE::commands.find(tokens[0]);
  if (it != CORE::commands.end()) {
    it->second(tokens);
    MODULES::AEX::update_flags();
  } else if (!tokens[0].empty() && tokens[0][0] == '@') {
    ARES::RTE::EXTERNALS::run_external(tokens[0].substr(1), {tokens.begin() + 1, tokens.end()});
    MODULES::AEX::update_flags();
  } else {
    session_errors.push_back("[ARES-AEX]:[UNKNOWN_COMMAND] " + tokens[0]);
    last_error_code = 1;
    MODULES::AEX::update_flags();
  }
}

// Capture output of a command into a string
// Used by \#INTO
static std::string capture_command(const std::vector<std::string> &tokens) {
  // Redirect stdout temporarily
  std::streambuf *old_buf = std::cout.rdbuf();
  std::ostringstream capture;
  std::cout.rdbuf(capture.rdbuf());

  dispatch_line(tokens);

  std::cout.rdbuf(old_buf);
  return capture.str();
}

// ==============================
// Main AEX executor
// ==============================

void execute_Ares_Automation(const std::vector<std::string> &args) {
  if (args.size() != 2 && args.size() != 4) {
    session_errors.push_back(
        "[ARES-AEX]:[INVALID_ARGS] Usage: \\@AEX script.ares [TIMES N]");
    last_error_code = AEX_FILE_NOT_FOUND;
    return;
  }

  std::string script_path = args[1];
  int max_runs = 1;

  if (args.size() == 4) {
    if (args[2] != "TIMES") {
      session_errors.push_back("[ARES-AEX]:[INVALID_ARGS] Expected TIMES N");
      last_error_code = AEX_FILE_NOT_FOUND;
      return;
    }
    max_runs = std::stoi(args[3]);
    if (max_runs < 1 || max_runs > 5) {
      session_errors.push_back(
          "[ARES-AEX]:[LOOP_MEM_OVERFLOW] TIMES must be 1-5");
      last_error_code = AEX_LOOP_MEM_OVERFLOW;
      return;
    }
  }

  // ==============================
  // Load script
  // ==============================
  std::ifstream file(script_path);
  if (!file) {
    session_errors.push_back("[ARES-AEX]:[FILE_NOT_FOUND] " + script_path);
    last_error_code = AEX_FILE_NOT_FOUND;
    return;
  }

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    lines.push_back(line);
  }

  // ==============================
  // Header validation
  // ==============================
  if (lines.empty()) {
    session_errors.push_back("[ARES-AEX]:[INVALID_HEADER] Empty script");
    last_error_code = AEX_INVALID_HEADER;
    return;
  }

  auto header_tokens = smart_tokenize(lines[0]);
  if (header_tokens.size() < 2 || header_tokens[0] != "AVER") {
    session_errors.push_back(
        "[ARES-AEX]:[INVALID_HEADER] First line must be AVER <version>");
    last_error_code = AEX_INVALID_HEADER;
    return;
  }

  if (header_tokens[1] != parserV) {
    session_errors.push_back(
        "[ARES-AEX]:[UNSUPPORTED_VERSION] Script needs engine " +
        header_tokens[1] + " but running " + parserV);
    last_error_code = AEX_UNSUPPORTED_VERSION;
    return;
  }

  // ==============================
  // Execution loop
  // ==============================
  for (int run = 0; run < max_runs; ++run) {

    // Flat if/else state
    bool in_if_block = false;
    bool condition_met = false;
    bool block_done = false; // true once a branch has executed, skip rest

    for (size_t i = 0; i < lines.size(); ++i) {
      auto tokens = smart_tokenize(lines[i]);
      if (tokens.empty())
        continue;
      const std::string &cmd = tokens[0];
      // ---- Script control ----
      // For now, i don't do much with these.
      if (cmd == "AEND")
        break;
      if (cmd == "AMEM")
        continue;
      if (cmd == "AVER")
        continue;
      if (cmd == "\\@COMM")
        continue;

      // ---- Early return ----
      if (cmd == "\\@RET") {
        if (ZeroFlag != 0)
          return; // error occurred, bail out
        continue;
      }

      // ---- \#REPORT ERRORS ----
      if (cmd == "\\#REPORT") {
        if (tokens.size() >= 2 && tokens[1] == "ERRORS") {
          for (const auto &err : session_errors)
            std::cout << err << std::endl;
        }
        continue;
      }

      // ---- \#IF ----
      // Changes: Removed debug lines on if-else blocks checks. if you need them, simply add a output that references these vars yourself, sorry.
      if (cmd == "\\#IF") {
        if (tokens.size() < 3) {
          session_errors.push_back(
              "[ARES-AEX]:[INVALID_IF] Usage: \\#IF <path|.ZF|.CF> !0");
          last_error_code = 1;
          return;
        }
        // reset state completely for new block
        in_if_block = true;
        block_done = false;
        condition_met = evaluate_condition(tokens[1]);
        if (condition_met)
          block_done = true;
        continue;
      }

      // ---- \#ELSE IF ----
      if (cmd == "\\#ELIF" && tokens.size() >= 3 && tokens[1] == "IF") {
        if (!in_if_block)
          continue;
        if (block_done) {
          condition_met = false;
          continue;
        }
        condition_met = evaluate_condition(tokens[2]);
        continue;
      }

      // ---- \#ELSE ----
      if (cmd == "\\#ELSE") {
        if (!in_if_block)
          continue;
        if (block_done) {
          condition_met = false;
          continue;
        }
        condition_met = !condition_met;
        continue;
      }

      // ---- Skip if condition not met ----
      if (in_if_block && !condition_met)
        continue;

      // ---- If condition met, mark block done after executing ----
      if (in_if_block && condition_met)
        block_done = true;

      // ---- \#INTO "path" PUT <command tokens...> ----
      if (cmd == "\\#INTO") {
        // \#INTO "path" PUT \@CMD args...
        if (tokens.size() < 4 || tokens[2] != "PUT") {
          session_errors.push_back(
              "[ARES-AEX]:[INVALID_INTO] Usage: \\#INTO <path> PUT <command>");
          last_error_code = 1;
          update_flags();
          continue;
        }
        std::string out_path = tokens[1];
        std::vector<std::string> cmd_tokens(tokens.begin() + 3, tokens.end());

        std::string captured = capture_command(cmd_tokens);

        std::ofstream out(out_path);
        if (!out) {
          session_errors.push_back("[ARES-AEX]:[INTO_WRITE_FAILED] " +
                                   out_path);
          last_error_code = 1;
          update_flags();
          continue;
        }
        out << captured;
        last_error_code = 0;
        update_flags();
        continue;
      }

      // ---- \#BLOCK <VARNAME> LINES n ----
      // This, makes so we can define long content blocks before writing/appending to a file or output.
      // Kinda useful for long scripts.
      if (cmd == "\\#BLOCK") {
        // Expected: \#BLOCK <VARNAME> LINES <n>
        if (tokens.size() < 4 || tokens[2] != "LINES") {
          session_errors.push_back(
              "[ARES-AEX]:[INVALID_BLOCK] Usage: \\#BLOCK <VARNAME> LINES <n>");
          last_error_code = 1;
          update_flags();
          continue;
        }
        std::string block_var = tokens[1];
        int line_count = 0;
        try {
          line_count = std::stoi(tokens[3]);
        } catch (...) {
          session_errors.push_back(
              "[ARES-AEX]:[INVALID_BLOCK] LINES value must be an integer");
          last_error_code = 1;
          update_flags();
          continue;
        }
        if (line_count < 1) {
          session_errors.push_back(
              "[ARES-AEX]:[INVALID_BLOCK] LINES must be >= 1");
          last_error_code = 1;
          update_flags();
          continue;
        }
        std::string block_content;
        int collected = 0;
        ++i; // move past \#BLOCK line
        while (i < lines.size() && collected < line_count) {
          block_content += lines[i] + "\n";
          ++i;
          ++collected;
        }
        // expect \#BLOCKEND
        if (i < lines.size()) {
          auto bend_tokens = smart_tokenize(lines[i]);
          if (bend_tokens.empty() || bend_tokens[0] != "\\#BLOCKEND") {
            session_errors.push_back(
                "[ARES-AEX]:[INVALID_BLOCK] Expected \\#BLOCKEND after " +
                std::to_string(line_count) + " lines for block '" + block_var + "'");
            last_error_code = 1;
            update_flags();
            continue;
          }
          // consume \#BLOCKEND
        } else {
          session_errors.push_back(
              "[ARES-AEX]:[INVALID_BLOCK] Reached AEND without \\#BLOCKEND for block '" + block_var + "'");
          last_error_code = 1;
          update_flags();
          continue;
        }
        // store into internal_vars under the given name
        ARES::RTE::ENV::internal_vars[block_var] = block_content;
        last_error_code = 0;
        update_flags();
        continue;
      }

      // ---- \#BLOCKEND (standalone, outside \#BLOCK context) ----
      if (cmd == "\\#BLOCKEND")
        continue;

      // ---- Normal command dispatch ----
      dispatch_line(tokens);
    }
  }

  last_error_code = 0;
  update_flags();
}
}
}
} // namespace ARES