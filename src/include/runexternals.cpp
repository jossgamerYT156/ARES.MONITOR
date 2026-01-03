#pragma once
#include "sys/wait.h"

/**
 * Core Execution Logic
 * Wraps the standard fork/exec but tracks errors in our Anti-POSIX global state.
 */
void run_external(const std::string& binary, const std::vector<std::string>& args) {
    pid_t pid = fork();

    if (pid == 0) { // Child Process
        std::vector<char*> c_args;
        c_args.push_back(const_cast<char*>(binary.c_str()));
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execvp(binary.c_str(), c_args.data());
        
        // If execvp reaches here, the command wasn't found
        std::cerr << "[4083]:[BINARY_NOT_FOUND] " << binary << std::endl;
        exit(127); 
    } 
    
    if (pid > 0) { // Parent Process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            *global_err_ptr += 1;
            session_errors.push_back("External Operand '" + binary + "' failed with status " + std::to_string(WEXITSTATUS(status)));
        }
    }
}


// Handler for \@EXEC <command>, same as @. they do the same thing, just different prefixes.
void handle_exec(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        session_errors.push_back("[EXEC] : [Syntax Error] : [\\@EXEC :: requires a target binary]");
        return;
    }
    
    // Extract everything after \@EXEC
    std::vector<std::string> pass_through(args.begin() + 2, args.end());
    run_external(args[1], pass_through);
}
