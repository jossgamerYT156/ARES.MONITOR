// general write function for Writing stuff to terminal or files.
// yes, that's a thing.

#pragma once
#include "modules/transcript/arescript.cpp"
#include "std_glbl.hpp"

void handle_write(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        session_errors.push_back("[WRITE]:[Syntax Error]:[NO CONTENT SPECIFIED : Cannot write NULL.]");
        last_error_code = 1;
        return;
    }

    std::string content = "";
    
    // Phase 1: Input
    if (args[1] == "FROM" && args.size() >= 3) {
        std::ifstream src(args[2]);
        if (!src) {
            session_errors.push_back("[WRITE]:[File Read Error: " + args[2] + "]");
            last_error_code = 404;
            return;
        }
        content.assign((std::istreambuf_iterator<char>(src)), std::istreambuf_iterator<char>());
    } else {
        content = args[1];
    }

    // Phase 2: Arescript Processor
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "WITH" && i + 1 < args.size()) {
            content = apply_arescript(content, args[i+1]);
        }
    }

    // Phase 3: Output
    bool saved = false;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "TO" && i + 1 < args.size()) {
            std::ofstream dest(args[i+1]);
            dest << content;
            saved = true;
            break;
        }
    }

    if (!saved) std::cout << content << std::endl;
}