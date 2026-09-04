#include "arescript.hpp"
#include "std_glbl.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace ARES {
// External tokenizer from core_sys, we reuse this one a LOT.
    extern std::vector<std::string> ARES::MODULES::AEX::smart_tokenize(const std::string &input);

    namespace MODULES::ARESCRIPT {
    struct AresRule {
        std::string pattern;
        std::string replacement;
    };
    /**
     * @brief absolutely massive function that applies the ARESCRIPT Formatting Rules using an .arescript rulebook specified.
     * This is mostly used by `\@WRITE` when using the "WITH" argument.
     *
     * I did this at the begining of 2026, and i cannot read it anymore, so... can't really explain how it works. it just does.
     */
    std::string apply_arescript(std::string data, const std::string &script_path)
        {

            std::cout << "[ARESCRIPT:FORMATTER]:[V:" << arescript_version
                      << "]:[REV:" << arescript_rev << "]" << std::endl;
        
            // ==============================
            // Phase 0: Open Script
            // ==============================
            std::ifstream file(script_path);
            if (!file) {
                session_errors.push_back("[ARESCRIPT]:[CODE:100]:[FILE_NOT_FOUND] " + script_path);
                last_error_code = 100;
                *global_err_ptr += 1;
                return data;
            }

            // ==============================
            // Phase 1: Script State
            // ==============================
            std::vector<AresRule> rules;
            int group_size = 1;
            bool is_strict = false;
            std::string wrap_open, wrap_close;
            std::string local_section_name = "";
            std::string local_assign_token = " ";
            WDelimMode wdelim_mode = WDelimMode::NONE;
            std::string wdelim_wrap;
            std::string line;
        
            // ==============================
            // Phase 2: Parse Script
            // ==============================
            while (std::getline(file, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                auto tokens = ARES::MODULES::AEX::smart_tokenize(line);
                if (tokens.empty()) continue;
            
                const std::string &cmd = tokens[0];
            
                if (cmd == "STRICT") {
                    is_strict = true;
                
                } else if (cmd == "GROUP") {
                    if (tokens.size() != 2 || std::stoi(tokens[1]) <= 0) {
                        session_errors.push_back("[ARESCRIPT]:[CODE:102]:[INVALID_GROUP] GROUP requires integer > 0");
                        last_error_code = 102;
                        *global_err_ptr += 2;
                        return data;
                    }
                    group_size = std::stoi(tokens[1]);
                
                } else if (cmd == "WRAP") {
                    if (tokens.size() != 3) {
                        session_errors.push_back("[ARESCRIPT]:[CODE:103]:[INVALID_WRAP] WRAP requires open and close");
                        last_error_code = 103;
                        *global_err_ptr += 2;
                        return data;
                    }
                    wrap_open = tokens[1];
                    wrap_close = tokens[2];
                
                } else if (cmd == "SECTION") {
                    if (tokens.size() != 2) {
                        session_errors.push_back("[ARESCRIPT]:[CODE:106]:[INVALID_SECTION] SECTION requires a name");
                        last_error_code = 106;
                        *global_err_ptr += 1;
                        return data;
                    }
                    local_section_name = tokens[1];
                
                } else if (cmd == "ASSIGN") {
                    if (tokens.size() != 2) {
                        session_errors.push_back("[ARESCRIPT]:[CODE:107]:[INVALID_ASSIGN] ASSIGN requires one token");
                        last_error_code = 107;
                        *global_err_ptr += 1;
                        return data;
                    }
                    local_assign_token = tokens[1];
                
                } else if (cmd == "DELIM") {
                    if (tokens.size() != 2) {
                        session_errors.push_back("[ARESCRIPT]:[CODE:104]:[INVALID_DELIM] DELIM requires one value");
                        last_error_code = 104;
                        *global_err_ptr += 1;
                        return data;
                    }
                    rules.push_back({"DELIM", tokens[1]});
                
                } else if (cmd == "WDELIM") {
                    if (tokens.size() < 2) {
                        session_errors.push_back("[ARESCRIPT]:[CODE:105]:[INVALID_WDELIM] Expected WDELIM QUOTE <SINGLE|DOUBLE|QPERCENT> or WDELIM WRAP <token> or WDELIM KEY_BARE_VALUE_QUOTED");
                        last_error_code = 105;
                        *global_err_ptr += 1;
                        return data;
                    }
                    if (tokens[1] == "KEY_BARE_VALUE_QUOTED") {
                        wdelim_mode = WDelimMode::KEY_BARE_VALUE_QUOTED;
                    } else if (tokens[1] == "QUOTE") {
                        if (tokens.size() < 3) {
                            session_errors.push_back("[ARESCRIPT]:[CODE:105]:[INVALID_WDELIM_QUOTE] Use SINGLE, DOUBLE or QPERCENT");
                            last_error_code = 105;
                            *global_err_ptr += 1;
                            return data;
                        }
                        if (tokens[2] == "SINGLE")       wdelim_mode = WDelimMode::QUOTE_SINGLE;
                        else if (tokens[2] == "DOUBLE")  wdelim_mode = WDelimMode::QUOTE_DOUBLE;
                        else if (tokens[2] == "QPERCENT") wdelim_mode = WDelimMode::QPERCENT;
                        else {
                            session_errors.push_back("[ARESCRIPT]:[CODE:105]:[INVALID_WDELIM_QUOTE] Use SINGLE, DOUBLE or QPERCENT");
                            last_error_code = 105;
                            *global_err_ptr += 1;
                            return data;
                        }
                    } else if (tokens[1] == "WRAP") {
                        if (tokens.size() < 3) {
                            session_errors.push_back("[ARESCRIPT]:[CODE:105]:[INVALID_WDELIM_WRAP] WRAP requires one token");
                            last_error_code = 105;
                            *global_err_ptr += 1;
                            return data;
                        }
                        wdelim_mode = WDelimMode::WRAP;
                        wdelim_wrap = tokens[2];
                    } else {
                        session_errors.push_back("[ARESCRIPT]:[CODE:105]:[UNKNOWN_WDELIM_MODE] \"" + tokens[1] + "\"");
                        last_error_code = 105;
                        *global_err_ptr += 1;
                        return data;
                    }
                
                } else if (cmd == "$.") {
                    if (tokens.size() != 2) {
                        session_errors.push_back("[ARESCRIPT]:[CODE:301]:[INVALID_VARIABLE_RULE] $. requires value");
                        last_error_code = 301;
                        *global_err_ptr += 1;
                        return data;
                    }
                    rules.push_back({"$.", tokens[1]});
                
                } else if (tokens.size() >= 2) {
                    rules.push_back({tokens[0], tokens[1]});
                
                } else {
                    session_errors.push_back("[ARESCRIPT]:[CODE:101]:[UNKNOWN_DIRECTIVE] " + cmd);
                    last_error_code = 101;
                    *global_err_ptr += 1;
                    return data;
                }
            } // end Phase 2
        
            // ==============================
            // Phase 3: Line-based Transform
            // ==============================
            std::stringstream output;
        
            if (!wrap_open.empty()) output << wrap_open << "\n";
            if (!local_section_name.empty()) output << "[" << local_section_name << "]\n";
        
            std::stringstream input_stream(data);
            std::string input_line;
            bool first_entry = true;
        
            while (std::getline(input_stream, input_line))
            {
                if (input_line.empty()) continue;
            
                auto words = ARES::MODULES::AEX::smart_tokenize(input_line);
                if (words.empty()) continue;
            
                if (is_strict && (words.size() % group_size != 0)) {
                    session_errors.push_back(
                        "[ARESCRIPT]:[CODE:200]:[STRICT_GROUP_MISMATCH_LINE] " +
                        std::to_string(words.size()) + " % " + std::to_string(group_size));
                    last_error_code = 200;
                    *global_err_ptr += 5;
                    return data;
                }
            
                for (size_t i = 0; i < words.size(); i += group_size) {
                    std::string key = words[i];
                    std::string value = (group_size > 1 && i + 1 < words.size()) ? words[i + 1] : "";
                
                    // Apply wdelim
                    if (wdelim_mode == WDelimMode::QUOTE_SINGLE) {
                        key = "'" + key + "'";
                        if (!value.empty()) value = "'" + value + "'";
                    } else if (wdelim_mode == WDelimMode::QUOTE_DOUBLE) {
                        key = "\"" + key + "\"";
                        if (!value.empty()) value = "\"" + value + "\"";
                    } else if (wdelim_mode == WDelimMode::QPERCENT) {
                        key = "%" + key + "%";
                        if (!value.empty()) value = "%" + value + "%";
                    } else if (wdelim_mode == WDelimMode::WRAP) {
                        key = wdelim_wrap + key + wdelim_wrap;
                        if (!value.empty()) value = wdelim_wrap + value + wdelim_wrap;
                    } else if (wdelim_mode == WDelimMode::KEY_BARE_VALUE_QUOTED) {
                        if (!value.empty()) value = "\"" + value + "\"";
                    }
                
                    // Apply rules
                    for (const auto &rule : rules) {
                        if (rule.pattern == "$.") {
                            if (ARES::RTE::ENV::internal_vars.count(key)) key = ARES::RTE::ENV::internal_vars[key];
                            else if (is_strict) {
                                session_errors.push_back("[ARESCRIPT]:[CODE:301]:[VARIABLE_UNDEFINED] " + key);
                                last_error_code = 301;
                                *global_err_ptr += 2;
                                return data;
                            }
                        } else if (rule.pattern != "DELIM") {
                            size_t p = key.find(rule.pattern);
                            if (p != std::string::npos)
                                key.replace(p, rule.pattern.size(), rule.replacement);
                        }
                    }
                
                    std::string chunk = value.empty() ? key : key + " " + local_assign_token + " " + value;
                
                    // Apply DELIM to chunk
                    for (const auto &rule : rules) {
                        if (rule.pattern == "DELIM") {
                            size_t p = chunk.find(" ");
                            if (p == std::string::npos) {
                                session_errors.push_back("[ARESCRIPT]:[CODE:300]:[DELIM_FAILED]");
                                last_error_code = 300;
                                *global_err_ptr += 1;
                                if (is_strict) return data;
                            } else {
                                chunk.replace(p, 1, rule.replacement);
                            }
                        }
                    }
                
                    if (!first_entry) output << "\n";
                    first_entry = false;
                    output << chunk;
                }
            }
        
            output << "\n";
            if (!wrap_close.empty()) output << wrap_close;
        
            last_error_code = 0;
            return output.str();
        } // end apply_arescript
    }
}