#pragma once
// Initialize Global Memory Counter
#include "std_glbl.hpp"
#include <ostream>
#include <sys/utsname.h>

#ifdef __APPLE__
// Include the FUCKING environment parsing, because my ass forgot to do this on apple hosts and compiling breaks without this.
#include "boost/process/v2/detail/environment_posix.hpp"
#endif
#include "get_self_path.cpp"
namespace ARES {
namespace CORE {
  /**
   * @brief Initializes the system by loading environment variables and setting up global state.
   * This function should be called at the very start of the program to ensure that all necessary
   * environment variables are loaded and the global error pointer is initialized.
   * It also sets the SHELL environment variable to the path of the current executable.
   * This is crucial for the correct functioning of the shell and its built-in commands.
   */
void init_system()
{
  for (char **env = environ; *env != nullptr; env++) {
    std::string entry(*env);
    size_t pos = entry.find('=');
    if (pos != std::string::npos) {
      ARES::RTE::ENV::internal_vars[entry.substr(0, pos)] = entry.substr(pos + 1);
    }
  }
  #ifdef __APPLE__
    std::string PLATFORM = "*NIX:macOS";
  #elif defined(__linux__)
    std::string PLATFORM = "*NIX:Linux GNU";
  #elif defined(__unix__)
    std::string PLATFORM = "SysV:*NIX";
  #else
    std::string PLATFORM = "Unknown";
  #endif
    // Override SHELL after loading environ, and bypass handle_env directly
    ARES::RTE::ENV::internal_vars["ARES_BUILDSRC"] = "Source Built on " + PLATFORM ;
    // Get RTE Operating System.
    struct utsname buffer;
    std::string OS; // We use this once.
    if(uname(&buffer) == 0) {
      OS = buffer.sysname;
    }

    ARES::RTE::ENV::internal_vars["ARES_VERSION"] = "ARES " + ARES_VERSION + "-" + OS + " " + BRANCH;
    ARES::RTE::ENV::internal_vars["ARES_RELEASE"] = RELEASE_DATE + " - " +ARES_RELEASE;
    setenv("SHELL", ARES::CORE::UTILS::get_self_path().c_str(), 1);
    global_err_ptr = (unsigned long long *)malloc(sizeof(unsigned long long));
    if (global_err_ptr)
      *global_err_ptr = 0;
}
// Logic to check for memory cap
void check_memory_integrity()
{
  if (*global_err_ptr < MEM_LIMIT)
    return;

  std::cerr << "[MEMORYCAP]:[E:MEMORYSAFE_EXIT]" << std::endl;
  free(global_err_ptr);
  exit(-1);
}

// Logic to punish NOPOSIX mistakes
void handle_syntax_punishment()
{
  noposix_error_counter += 2;
  *global_err_ptr += 2;

  ARES::CORE::check_memory_integrity();

  if (noposix_error_counter < 1024)
    return;

  std::cerr << "[NOPOSIX]:[UNHANDLEDUSER] - Too many mistakes. Returning to "
               "default shell."
            << std::endl;
  free(global_err_ptr);

  // Kick user to default shell (MacOS/Linux)
  const char *shell = getenv("SHELL");
  if (!shell)
    shell = "/bin/sh";
  execl(shell, shell, NULL);
}
} // namespace CORE


namespace MODULES::AEX {
// Improved Tokenizer to handle quoted strings: "Like This"
std::vector<std::string> smart_tokenize(const std::string &input)
{
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    char quote_char = 0;

    for (size_t i = 0; i < input.size(); i++) {
        char c = input[i];

        // Handle both single and double quotes
        if ((c == '"' || c == '\'') && !in_quotes) {
            in_quotes = true;
            quote_char = c;
            continue;
        }
        if (c == quote_char && in_quotes) {
            in_quotes = false;
            quote_char = 0;
            continue;
        }

        // Escape sequences inside quotes
        if (in_quotes && c == '\\' && i + 1 < input.size()) {
            char next = input[i + 1];
            if (next == '"' || next == '\'' || next == '\\') {
                current += next;
                i++;
                continue;
            }
            if (next == 'n') { current += '\n'; i++; continue; }
            if (next == 't') { current += '\t'; i++; continue; }
        }

        // Comments — ignore everything after #
        if (!in_quotes && c == '#' && current.empty()) break;

        if (c == ' ' && !in_quotes) {
            if (!current.empty()) tokens.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}
}
}