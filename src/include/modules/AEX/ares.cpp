#pragma once
#include "std_glbl.hpp"
#include "env.cpp"

int CarryFlag = 0;
int ZeroFlag = 0;

enum AexErrorCode {
    AEX_OK = 0,

    AEX_FILE_NOT_FOUND = 600,
    AEX_UNSUPPORTED_VERSION = 601,
    AEX_INVALID_HEADER = 602,
    AEX_LOOP_MEM_OVERFLOW = 603,
    AEX_MISSING_AEND = 604,
    AEX_MISSING_RET = 605
};


void execute_Ares_Automation(const std::vector<std::string>& args) {
    if (args.size() != 2 && args.size() != 4) {
        session_errors.push_back(
            "[ARES-AEX]:[INVALID_ARGS] Usage: \\@AEX script.ares [TIMES N]"
        );
        last_error_code = 600;
        return;
    }

    std::string script_path = args[1];
    int max_runs = 1;

    if (args.size() == 4) {
        if (args[2] != "TIMES") {
            session_errors.push_back(
                "[ARES-AEX]:[INVALID_ARGS]::[USE:TIMES N]"
            );
            last_error_code = 600;
            return;
        }
        max_runs = std::stoi(args[3]);
        if (max_runs < 1 || max_runs > 5) {
            session_errors.push_back(
                "[ARES-AEX]:[LOOP_MEM_OVERFLOW]"
            );
            last_error_code = 603;
            return;
        }
    }

    std::ifstream file(script_path);
    if (!file) {
        session_errors.push_back(
            "[ARES-AEX]:[FILE_NOT_FOUND] " + script_path
        );
        last_error_code = 600;
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    // ==============================
    // Header validation
    // ==============================
    if (lines.size() < 3 || lines[0].find("AVER") != 0) {
        session_errors.push_back(
            "[ARES-AEX]:[INVALID_HEADER]"
        );
        last_error_code = 602;
        return;
    }

    auto version = smart_tokenize(lines[0])[1];
    if (version != ARES_VERSION) {
        session_errors.push_back(
            "[ARES-AEX]:[UNSUPPORTED_ARES_SCRIPT] " + version
        );
        last_error_code = 601;
        return;
    }

    int loop_counter = 0;

    // ==============================
    // Execution loop
    // ==============================
    for (int run = 0; run < max_runs; ++run) {
        for (size_t i = 0; i < lines.size(); ++i) {
            auto tokens = smart_tokenize(lines[i]);
            if (tokens.empty()) continue;

            if (tokens[0] == "AEND") {
                loop_counter++;
                if (loop_counter > 5) {
                    session_errors.push_back(
                        "[ARES-AEX]:[FATAL]:[LOOP_MEM_OVERFLOW]"
                    );
                    last_error_code = 603;
                    return;
                }
                break;
            }

            if (tokens[0] == "\\@COMM") continue;

            if (tokens[0] == "\\@RET") {
                if (CarryFlag != 0 || ZeroFlag != 0)
                    return;
                else
                    break;
            }

            // Dispatch built-in command
            auto it = commands.find(tokens[0]);
            if (it != commands.end()) {
                it->second(tokens);
            }
        }
    }

    last_error_code = 0;
}
