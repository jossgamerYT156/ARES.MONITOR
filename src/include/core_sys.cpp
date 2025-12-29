#pragma once
// Initialize Global Memory Counter
void init_system() {
    global_err_ptr = (unsigned long long*)malloc(sizeof(unsigned long long));
    if (global_err_ptr) *global_err_ptr = 0;
}

// Logic to check for memory cap
void check_memory_integrity() {
    if (*global_err_ptr < MEM_LIMIT) return;
    
    std::cerr << "[MEMORYCAP]:[E:MEMORYSAFE_EXIT]" << std::endl;
    free(global_err_ptr);
    exit(-1);
}

// Logic to punish NOPOSIX mistakes
void handle_syntax_punishment() {
    noposix_error_counter += 2;
    *global_err_ptr += 2;
    
    check_memory_integrity();

    if (noposix_error_counter < 1024) return;

    std::cerr << "[NOPOSIX]:[UNHANDLEDUSER] - Too many mistakes. Returning to default shell." << std::endl;
    free(global_err_ptr);
    
    // Kick user to default shell (MacOS/Linux)
    const char* shell = getenv("SHELL");
    if (!shell) shell = "/bin/sh";
    execl(shell, shell, NULL);
}

// Improved Tokenizer to handle quoted strings: "Like This"
std::vector<std::string> smart_tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;

    for (char c : input) {
        if (c == '\"') {
            in_quotes = !in_quotes;
            continue; 
        }
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