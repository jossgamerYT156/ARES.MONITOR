#pragma once

// Logic for @ENV <VARNAME> <Value>
void handle_env(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        session_errors.push_back("[ENV]:[Syntax Error]:: [requires NAME and VALUE.]");
        return;
    }

    // setenv(name, value, overwrite_flag)
    // 1 means overwrite if it exists
    if (setenv(args[1].c_str(), args[2].c_str(), 1) != 0) {
        session_errors.push_back("[ENV]: [Could not set environment variable]");
        *global_err_ptr += 1;
    } else {
        std::cout << "[ENV]: Set " << args[1] << " to " << args[2] << std::endl;
    }
}

void handle_replace(const std::vector<std::string>& args) {
    // Check for @REPLACE <Word> <File> WITH <NewWord> (args size 5)
    if (args.size() < 5 || args[3] != "WITH") {
        session_errors.push_back("[REPLACE] : [Syntax Error: @REPLACE <Find> <File> WITH <Replace>]");
        return;
    }

    std::string find_word = args[1];
    std::string file_path = args[2];
    std::string replace_word = args[4];

    std::ifstream in(file_path);
    if (!in) {
        session_errors.push_back("[REPLACE]:[Error]:[Cannot open entry: \"" + file_path +"\" ]");
        return;
    }

    // Read whole file
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    size_t pos = content.find(find_word);
    
    // @REPLACE (First occurrence only)
    if (pos != std::string::npos) {
        content.replace(pos, find_word.length(), replace_word);
    }

    std::ofstream out(file_path);
    out << content;
}

// this one is the same as @\ENV, but better, because it doesn't need the @\ prefix.

void expand_variables(std::vector<std::string>& tokens) {
    for (auto& token : tokens) {
        // Look for tokens starting with $
        if (token.size() > 1 && token[0] == '$') {
            std::string var_name = token.substr(1);
            if (internal_vars.count(var_name)) {
                token = internal_vars[var_name]; // Replace token with the value
            }
        }
    }
}
// it also stores in memory, so it only applies to current session..