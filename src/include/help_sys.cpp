#pragma once
#include "std_glbl.hpp"
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#pragma once
#include "ABI_HLP.cpp"

// NAMESPACE: ARES -> CORE -> HELP : Help system for ARES Monitor.
namespace ARES::CORE::HELP {
// Database of Anti-POSIX commands
const std::unordered_map<std::string, std::string> HELP_DB = {
    {"\\@WRITE", "Usage: \\@WRITE <\"text\"> [TO <\"file\">] or FROM "
                 "<\"file\"> [TO <\"file\">]"},
    {"\\@APPEND",
     "Usage: \\@APPEND <\"text\"> TO <\"file\"> or FROM <%VAR>|<\"file\"> TO "
     "<\"file\"> - Appends content to an existing file."},
    {"\\@HLT",
     "Usage: \\@HLT - Terminates session and prints the session error log."},
    {"\\@CWD", "Usage: \\@CWD <\"path\"> - Changes current working directory "
               "using fs::path."},
    {"\\@LDC", "Usage: \\@LDC [\"path\"] - Lists directory contents with "
               "[F]ile/[D]irectory tags."},
    {"\\@CREATE", "Usage: \\@CREATE [FILE <\"name\"> [WITH <\"content\">] | "
                  "DIR <\"name\"> | DIR STRUCTURE <\"path\">]"},
    {"\\@DELETE", "Usage: \\@DELETE [FILE <\"name\"> | DIR [RECURSIVELY] "
                  "<\"name\"> | FILE WITHEXT <\".ext\">]"},
    {"\\@EXEC", "Usage: \\@EXEC <\"binary\"> [args...] - Executes an external "
                "POSIX binary."},
    {"\\@AEX", "Usage: \\@AEX <\"script.ares\"> <TIMES <n>> - Executes an ARES "
               "(A)utomation (EX)ecution script with built-in syntax. made for "
               "Non-redundant testing."},
    {"\\@AEX_TIMES",
     "Executes an automation script a certain number of times (up to around 5 "
     "times, this limit can change eventually.)"},
    {"\\@KILL", "Usage: \\@KILL [PID <id> | <\"name\">] - Forcefully "
                "terminates a process."},
    {"\\@ENV",
     "Usage: \\@ENV <NAME> <VALUE> - Sets a session environment variable."},
    {"\\@!?",
     "Usage: \\@!? - Displays the exit code of the last executed process."},
    {"\\@REPLACE",
     "Usage: \\@REPLACE <Word> <File> WITH <NewWord> - Replaces first "
     "occurrence of the specified Word inside the specified File."},
    {"\\@HELP",
     "Usage: \\@HELP [ALL | <COMMAND_NAME>] - Displays help information."},
    {"\\ABI", ABI_HLP},
    {"\\*?", "Usage: \\*? | Returns all the errors of the current session"},
    {"\\%?", "Usage \\%?  |[PRETTY]|[EnvVar] | Returns all environment "
             "variables or the contents of an existing given variable"},
    {"\\@CTC", "Usage \\@CTC    | Clears the terminal's contents"},
    {"\\VERSION",
     "ARES MONITOR VERSION:" + ARES_VERSION + "\nRelease:" + ARES_RELEASE +
         "\nAnti-POSIX System Interface Version: LDS_APOSI STD0.0.1\n\n"
         "\tCopyright (c) 2025 Lilly Aizawa and LDS LLC. All rights reserved."},
    {"\\APOSI",
     "Anti-POSIX System Interface - A command line standard for the ARES "
     "Monitor with strong Syntax Types.\n\n"
     "\t\tAPOSI-ARES: Commands are prefixed with '\\@'.\n"
     "\t\tAPOSI-EXTERANL: Commands are prefixed with '@' or preceded by "
     "'\\@EXEC '\n\n"
     "Refer to the documentation at "
     "\e]8;;https://softworks.aizawallc.org/APOSI/\e\\LDS APOSI "
     "Documentation\e]8;;\e\\\n"
     "for detailed information about this standard.\n"
     "\n\n\tCopyright (c) 2025 Lilly Aizawa and LDS LLC. All rights reserved."},
    {"\\ARES",
     "ARES Monitor - An purposefully non-POSIX compliant Monitor for macOS and "
     "Linux developed by Lilly Aizawa under the LDS Softworks LLC brand.\n\n"
     "Version: " +
         ARES_VERSION + "-" + ARES_RELEASE +
         "\n"
         "Ares Monitor implements a VM/Firmware-Like environment with strict "
         "command syntax, minimal error handling, external binary execution "
         "capabilities and memory safe features to prevent user's abuse.\n\n"
         "for on-system information about the APOSI standard, run \\@HELP "
         "APOSI\nFor cloud information about the APOSI standard, visit "
         "\e]8;;https://softworks.aizawallc.org/APOSI/?Topic=Standard%2FAPOSI "
         "Standard\e\\LDS APOSI Standard\e]8;;\e\\\n"
         "For more information about the ARES Monitor software, visit "
         "\e]8;;https://softworks.aizawallc.org/APOSI/?Topic=ARES%2FARES "
         "Quickstart\e\\APOSI:ARES Monitor Quick-Start Information\e]8;;\e\\\n"
         "\n\n\tCopyright (c) 2025 Lilly Aizawa and LDS Softworks LLC. All "
         "rights reserved."},
    {"\\C", "Shell Modifier || Usage: @/path/to/ares \\C <ARES_CMD> - Executes "
            "a string of ARES commands directly from the shell without "
            "entering Interactive Mode."
            "\n\n\tThis can be used for Quick Testing, Scripting, or One-Off "
            "commands without the need of a full Interactive Session."},
    {"\\QUIET",
     "Shell Modifier || Usage: @/path/to/ares \\QUIET - Suppresses the initial "
     "help message when launching the ARES Monitor from the shell. This allows "
     "for a cleaner startup when the user is already familiar with the "
     "available commands or when the help message is not needed. "
     "(Alternatively, you can mute it by using the file \"$HOME/.QUIET\". "
     "Simply by existing, the header will be skipped on boot.)"},
    {"\\@CEL", "Usage: \\@CEL | Clears the session error log"}};

void handle_help(const std::vector<std::string> &args) {
  // Case 1: @HELP ALL
  if (args.size() > 1 && args[1] == "ALL") {
    std::cout
        << "\n--- [Anti-POSIX System Interface(APOSI) COMMAND REFERENCE] ---\n";
    for (const auto &[cmd, desc] : ARES::CORE::HELP::HELP_DB) {
      std::cout << std::left << std::setw(12) << cmd << " | " << desc << "\n";
    }
    std::cout
        << "--------------------------------------------------------------\n"
        << std::endl;
    return;
  }

  // Case 2: @HELP <CMD>
  if (args.size() > 1) {
    if (ARES::CORE::HELP::HELP_DB.count(args[1])) {
      std::cout << args[1] << " : " << ARES::CORE::HELP::HELP_DB.at(args[1])
                << std::endl;
    } else {
      std::cout << "[HELP]:[4083]:[UNKNOWN_COMMAND_HELP] " << args[1]
                << std::endl;
    }
    return;
  }

  // Default: Just @HELP (Show brief instruction)
  std::cout << "Usage: \\@HELP [ALL | <COMMAND_NAME>]\nExample: \\@HELP \\@AEX"
            << std::endl;
}
} // namespace ARES::CORE::HELP