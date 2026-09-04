#pragma once
#include <fstream>
#include <functional>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#ifndef STD_GLBL_HPP
#pragma once
#define STD_GLBL_HPP

// Version information.
const std::string ARES_VERSION = "0.0.14-alpha";
const std::string ARES_RELEASE = "R2";
const std::string RELEASE_DATE = "2026-05-17";
const std::string BRANCH = "STABLE";

#include <termios.h>
inline struct termios orig_termios;

inline void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON); // no echo, read char by char
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

inline void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

inline int history_index = -1;
inline std::vector<std::string> command_history;
inline void push_history(const std::string &cmd) {
  if (!cmd.empty()) {
    command_history.push_back(cmd);
    history_index = command_history.size(); // reset to end
  }
}
inline std::string get_previous_command() {
  if (command_history.empty())
    return "";
  if (history_index > 0)
    history_index--;
  return command_history[history_index];
}

// Global error tracking
inline std::vector<std::string> session_errors;

inline int noposix_error_counter = 0;
inline unsigned long long *global_err_ptr = nullptr;
inline int last_error_code = 0; // initialize last error code, for future use.

const unsigned long long MEM_LIMIT =
    0xFFFFFF; // Memory cap for Anti-POSIX error tracking, about 16MB...
              // because, i know you'll try to HEAP_OVERFLOW it.

// Note: \@EXEC is added here, while prefix '@' is handled in main, why? fuck if
// i know, it's how i made it work
typedef std::function<void(const std::vector<std::string> &)> CommandFunc;
namespace ARES{
    namespace MODULES {
        namespace AEX {
            extern std::vector<std::string> smart_tokenize(const std::string &input);
            extern void execute_Ares_Automation(const std::vector<std::string> &args);
        }
    }
    namespace SYSTEM
    {
        namespace CTRL {
            extern void handle_kill(const std::vector<std::string> &args);
        }
    }
    namespace IO
    {
        // IO::CLI
        extern void clearTerminalContents(const std::vector<std::string> &args);
        // IO::FileOperations
        extern void handle_replace(const std::vector<std::string> &args);
        namespace FileOperations {

            // Directory Listing and workspace
            extern void handle_cwd(const std::vector<std::string> &args);
            extern void handle_ldc(const std::vector<std::string> &args);
            // Extern for appending to file.
            extern void handle_append(const std::vector<std::string> &args);
            // Writing contents to file.
            extern void handle_write(const std::vector<std::string> &args);
            // File and directories' logic
            extern void handle_delete(const std::vector<std::string> &args);
            extern void handle_create(const std::vector<std::string> &args);
        }
    }
    namespace RTE
    {
        namespace ENV {
            // Error handler // WIP.
            extern void handle_last_err(const std::vector<std::string> &args);
            // Store variables in memory:
            extern void getErrors(const std::vector<std::string> &args);
            inline std::unordered_map<std::string, std::string> internal_vars;
            extern void handle_env(const std::vector<std::string> &args);
            extern void getVariables(const std::vector<std::string> &args);

            extern void handle_cel(const std::vector<std::string> &args);
        }
        namespace EXTERNALS {
            extern void handle_exec(const std::vector<std::string> &args);
            // Redundant definition.
            extern void run_external(const std::string &binary, const std::vector<std::string> &args);
        }
    }
    namespace CORE
    {
        // Core Prototypes for built-in commands that are part of the Central
        // functionality of ARES.
        extern void handle_halt(const std::vector<std::string> &args);

        namespace HELP {
// Help Message Constant, for in case you MESS UP THE SYNTAX.
// Read the docks ffs.
const std::string HLPMSG =
    "Available commands:\n"
    "  \\@WRITE <content> [TO <file>]   - Write content to console or file\n"
    "  \\@WRITE FROM <file> [TO <file>] - Read from file and optionally write "
    "to another file\n"
    "  \\@HLT                           - Terminate the session and report "
    "errors\n"
    "  @<Binary or Binary Path>         - Executes a system binary using the ARES runtime.\n"
    "  \\*?                             - Display session errors log\n"
    "  \\!?                             - Display last error code from an ARES operand. (limited)\n"
    "  \\%? [PRETTY]                    - Display environment variables, with optional pretty print or display specific env values.\n"
    "  @/path/to/ares \\C \"<cmd>\"     - Runs ARES in One-Shot mode (escaping can change depending on Terminal Emulator.\n"
    "  \\QUIET                          - Supresses this message on load.\n"
    "built-in system shell(POSIX Shell Operand)\n"
    "-- For more information, use \\@HELP ALL --\n";
// READ THE DOCS!!
            extern void handle_help(const std::vector<std::string> &args);
        }
        // Duspatcher map, that's it, this has all the commands so DON'T TOUCH IT!.
#pragma once
// NAMESPACE: ARES -> Core -> Core Functions and definitions for ARES' Function
// REFACTOR: STD-CMDMAP.

// CORE -> Command Map -> EXEC : Maps command strings to their handler
// functions, naming is explicit of what these do.
std::unordered_map<std::string, CommandFunc> commands = {
    {"\\@WRITE", ARES::IO::FileOperations::handle_write},
    {"\\@APPEND", ARES::IO::FileOperations::handle_append},
    {"\\@HALT",
     ARES::CORE::handle_halt},
                        // Explcit namespace scoping to avoid ambiguity. since
                        // this is defined IN the core Namespace, we must specify
                        // it. If you want HLT to do something else, scope to
                        // your NS::SNS::handler(const std::vector<std::string>
                        // &args) and change the mapping here.
    {"\\@HLT", ARES::CORE::handle_halt},
    {"\\@EXEC", ARES::RTE::EXTERNALS::handle_exec},
    {"\\@CREATE", ARES::IO::FileOperations::handle_create},
    {"\\@DELETE", ARES::IO::FileOperations::handle_delete},
    {"\\@ENV", ARES::RTE::ENV::handle_env},
    {"\\@CWD", ARES::IO::FileOperations::handle_cwd},
    {"\\@LDC", ARES::IO::FileOperations::handle_ldc},
    {"\\@REPLACE", ARES::IO::handle_replace},
    {"\\@KILL", ARES::SYSTEM::CTRL::handle_kill},
    {"\\@HELP", ARES::CORE::HELP::handle_help},
    {"\\@KILALL",
     ARES::SYSTEM::CTRL::handle_kill}, // Mapping to same handler, because i am not rewriting this
                   // fucking logic, suck it up. - Lilly Aizawa.
    {"\\@AEX", ARES::MODULES::AEX::execute_Ares_Automation},
    {"\\@CTC", ARES::IO::clearTerminalContents},
    {"\\!?", ARES::RTE::ENV::handle_last_err},
    {"\\*?", ARES::RTE::ENV::getErrors},
    {"\\%?", ARES::RTE::ENV::getVariables},
    // Aliases for stuff we might use a lot.
    {"\\@%?", ARES::RTE::ENV::getVariables},
    {"\\@*?", ARES::RTE::ENV::getErrors},
    {"\\@!?", ARES::RTE::ENV::handle_last_err},
    // CEL is for "Clear Errors Log". very self explanatory.
    // This command clears all previous errors from the session errors log. so you don't end up with 4GB of Errors in your memory if you keep fucking up and not reading the damn docs.
    // READ THE DOCS!!. -- Liliana Aizawa @ LDS Softworks LLC.
    {"\\@CEL", ARES::RTE::ENV::handle_cel}
    };
    }
} // namespace ARES

#endif