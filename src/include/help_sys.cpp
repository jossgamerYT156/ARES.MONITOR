#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include "std_glbl.hpp"

// Database of Anti-POSIX commands
const std::unordered_map<std::string, std::string> HELP_DB = {
    {"\\@WRITE",    "Usage: \\@WRITE <\"text\"> [TO <\"file\">] or FROM <\"file\"> [TO <\"file\">]"},
    {"\\@HALT",     "Usage: \\@HALT - Terminates session and prints the session error log."},
    {"\\@CWD",      "Usage: \\@CWD <\"path\"> - Changes current working directory using fs::path."},
    {"\\@LDC",      "Usage: \\@LDC [\"path\"] - Lists directory contents with [F]ile/[D]irectory tags."},
    {"\\@CREATE",   "Usage: \\@CREATE [FILE <\"name\"> [WITH <\"content\">] | DIR <\"name\"> | DIR STRUCTURE <\"path\">]"},
    {"\\@DELETE",   "Usage: \\@DELETE [FILE <\"name\"> | DIR [RECURSIVELY] <\"name\"> | FILE WITHEXT <\".ext\">]"},
    {"\\@EXEC",     "Usage: \\@EXEC <\"binary\"> [args...] - Executes an external POSIX binary."},
    {"\\@AEX",      "Usage: \\@AEX <\"script.ares\"> - Executes an ARES (A)utomation (EX)ecution script with built-in syntax. made for Non-redundant testing."},
    {"\\@KILL",     "Usage: \\@KILL [PID <id> | <\"name\">] - Forcefully terminates a process."},
    {"\\@ENV",      "Usage: \\@ENV <NAME> <VALUE> - Sets a session environment variable."},
    {"\\@!?",       "Usage: \\@!? - Displays the exit code of the last executed process."},
    {"\\@REPLACE",  "Usage: \\@REPLACE <Word> <File> WITH <NewWord> - Replaces first occurrence of the specified Word inside the specified File."},
    {"\\@HELP",     "Usage: \\@HELP [ALL | <COMMAND_NAME>] - Displays help information."},
    {"\\APOSI",       "Anti-POSIX System Interface - A command line standard for the ARES Monitor with strong Syntax Types.\n\n"
                    "\t\tAPOSI-ARES: Commands are prefixed with '\\@'.\n"
                    "\t\tAPOSI-EXTERANL: Commands are prefixed with '@' or preceded by '\\@EXEC '\n\n"
                    "Refer to the documentation at https://aizawallc.org/APOSI for detailed information about this standard.\n"
                    "\n\n\tCopyright (c) 2025 Lilly Aizawa and LDS LLC. All rights reserved."},
    {"\\ARES",        "ARES Monitor - An purposefully non-POSIX compliant Monitor for macOS and Linux developed by Lilly Aizawa under the LDS LLC brand.\n\n"
                    "Version: 0.0.12-alpha\n"
                    "Ares Monitor implements a VM/Firmware-Like environment with strict command syntax, minimal error handling, external binary execution capabilities and memory safe features to prevent user's abuse.\n\n"
                    "for on-system information about the APOSI standard, run \\@HELP APOSI\nFor cloud information about the APOSI standard, visit https://aizawallc.org/APOSI\n"
                    "For more information about the ARES Monitor software, visit https://aizawallc.org/APOSI/ARES-Monitor\n"
                    "\n\n\tCopyright (c) 2025 Lilly Aizawa and LDS LLC. All rights reserved."},
    {"\\VERSION",      "ARES MONITOR VERSION:" + ARES_VERSION +
                    "\nAnti-POSIX System Interface Version: LDS_APOSI STD0.0.1\n\n"
                    "\tCopyright (c) 2025 Lilly Aizawa and LDS LLC. All rights reserved."}
};

void handle_help(const std::vector<std::string>& args) {
    // Case 1: @HELP ALL
    if (args.size() > 1 && args[1] == "ALL") {
        std::cout << "\n--- [Anti-POSIX System Interface(APOSI) COMMAND REFERENCE] ---\n";
        for (const auto& [cmd, desc] : HELP_DB) {
            std::cout << std::left << std::setw(12) << cmd << " | " << desc << "\n";
        }
        std::cout << "--------------------------------------------------------------\n";
        return;
    }

    // Case 2: @HELP <CMD>
    if (args.size() > 1) {
        if (HELP_DB.count(args[1])) {
            std::cout << args[1] << " : " << HELP_DB.at(args[1]) << std::endl;
        } else {
            std::cout << "[HELP]:[4083]:[UNKNOWN_COMMAND_HELP] " << args[1] << std::endl;
        }
        return;
    }

    // Default: Just @HELP (Show brief instruction)
    std::cout << "Usage: \\@HELP [ALL | <COMMAND_NAME>]\nExample: \\@HELP \\@AEX" << std::endl;
}