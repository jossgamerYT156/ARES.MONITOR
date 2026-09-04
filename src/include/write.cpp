// general write function for Writing stuff to terminal or files.
// yes, that's a thing.
#pragma once
#include "modules/transcript/arescript.cpp"
#include "std_glbl.hpp"
#include <string>
#include <unistd.h>
namespace ARES {
// Inline %VAR interpolation — scans string for %WORD, resolves via find_var,
// substitutes in place. If var not found, leaves %WORD as-is. Whitespace or
// end of string terminates the var name. You wanted plain %VAR, people suffer.
// :3
namespace RTE::ENV {
static std::string interpolate_vars(const std::string &input) {
  std::string result;
  result.reserve(input.size());
  size_t i = 0;
  while (i < input.size()) {
    if (input[i] == '%' && i + 1 < input.size() &&
        (isalnum(input[i + 1]) || input[i + 1] == '_')) {
      // collect var name
      size_t start = i + 1;
      size_t j = start;
      while (j < input.size() && (isalnum(input[j]) || input[j] == '_'))
        ++j;
      std::string var_name = input.substr(start, j - start);
      std::string val = ARES::RTE::ENV::find_var(var_name);
      if (!val.empty())
        result += val;
      else
        result += input.substr(i, j - i); // leave %VARNAME as-is
      i = j;
    } else {
      result += input[i++];
    }
  }
  return result;
}
} // namespace RTE::ENV
namespace IO::FileOperations {
    void handle_write(const std::vector<std::string> &args) {
      if (args.size() < 2) {
        session_errors.push_back(
            "[WRITE]:[Syntax Error]:[NO CONTENT SPECIFIED : Cannot write NULL.]");
        last_error_code = 1;
        return;
      }

      std::string content = "";

      // Phase 1: Input
      if (args[1] == "FROM" && args.size() >= 3) {
        // Check if it's a variable reference
        if (args[2][0] == '%') {

          std::string val = ARES::RTE::ENV::find_var(args[2].substr(1).substr(-1)); // strip the %'s, we need that for files. this is a very weird bug.
          /**
           * For who doesn't know, there was a bug with \@WRITE FROM %SOMETHING/stuff.some, where somehow env would not expand. adding that -1, ensures we PROPERLY seek for environment variables.
           *
           * How does that makes sense? the fuck do i know? i didn't wrote C++. it just does. and now you can write from paths without much issue.
           * It is also a very weird bug that appears once every idfk how often.
           */
          if (val.empty()) {
            session_errors.push_back("[WRITE]:[Var Error: " + args[2] +
                                     " not found.]");
            last_error_code = 1;
            return;
          }
          content = val;
        } else {
          // Treat as file path
          std::ifstream src(args[2]);
          if (!src) {
            session_errors.push_back("[WRITE]:[File Read Error: " + args[2] + "]");
            last_error_code = 404;
            return;
          }
          content.assign((std::istreambuf_iterator<char>(src)),
                         std::istreambuf_iterator<char>());
        }
      } else {
        content = args[1];
      }
    
      // Phase 1.5: Inline %VAR interpolation
      content = ARES::RTE::ENV::interpolate_vars(content);
    
      // Phase 2: Arescript Processor
      for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "WITH" && i + 1 < args.size()) {
          content = ARES::MODULES::ARESCRIPT::apply_arescript(content, args[i + 1]);
        }
      }
    
      // Phase 3: Output
      bool saved = false;
      for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "TO" && i + 1 < args.size()) {
          std::ofstream dest(args[i + 1]);
          dest << content;
          saved = true;
          break;
        }
      }

      if (!saved)
        std::cout << content << std::endl;
    }
    void handle_append(const std::vector<std::string> &args) {
      if (args.size() < 2) {
        session_errors.push_back(
            "[APPEND]:[Syntax Error]:[NO CONTENT SPECIFIED : Cannot append NULL.]");
        last_error_code = 1;
        return;
      }

      std::string content = "";
    
      // Phase 1: Input
      if (args[1] == "FROM" && args.size() >= 3) {
        if (args[2][0] == '%') {
          std::string val = ARES::RTE::ENV::find_var(args[2].substr(1));
          if (val.empty()) {
            session_errors.push_back("[APPEND]:[Var Error: " + args[2] +
                                     " not found.]");
            last_error_code = 1;
            return;
          }
          content = val;
        } else {
          std::ifstream src(args[2]);
          if (!src) {
            session_errors.push_back("[APPEND]:[File Read Error: " + args[2] + "]");
            last_error_code = 404;
            return;
          }
          content.assign((std::istreambuf_iterator<char>(src)),
                         std::istreambuf_iterator<char>());
        }
      } else {
        content = args[1];
      }
    
      // Phase 1.5: Inline %VAR interpolation
      content = ARES::RTE::ENV::interpolate_vars(content);

      // Phase 2: Output (append mode)
      bool saved = false;
      for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "TO" && i + 1 < args.size()) {
          std::ofstream dest(args[i + 1], std::ios::app);
          if (!dest) {
            session_errors.push_back("[APPEND]:[File Write Error: " + args[i + 1] +
                                     "]");
            last_error_code = 1;
            return;
          }
          dest << content;
          saved = true;
          break;
        }
      }

      if (!saved)
        std::cout << content << std::endl;
    }
  } // namespace IO::FileOperations
} // namespace ARES