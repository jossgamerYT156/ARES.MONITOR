#pragma once
#include <signal.h>
#include "std_glbl.hpp"

// Logic for \@KILL PID <id> | \@KILL <name> | \@KILALL <name>
void handle_kill(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        session_errors.push_back("[\\@KILL] :: [Syntax Error : Missing PID or Process Name]");
        return;
    }

    if (args[1] == "PID" && args.size() >= 3) {
        pid_t target = std::stoi(args[2]);
        if (kill(target, SIGKILL) != 0) {
            std::cerr << "[4083]:[PROCESS_NOT_FOUND] " << target << std::endl;
            *global_err_ptr += 1;
        }
        return;
    }

    // Pass name-based kills to run_external which we'll handle in shell.cpp
    // (Assuming run_external is declared in shell.cpp)
}

void handle_last_err(const std::vector<std::string>& args) {
    std::cout << "[LAST_EXIT_CODE]: " << last_error_code << std::endl;
}