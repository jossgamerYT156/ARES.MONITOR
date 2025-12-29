#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "std_glbl.hpp"

std::string arescript_version = "0.0.323-alpha";
std::string arescript_rev = "2026-01-01";

// Error code definitions for Arescript pre-processing.
enum AresErrorCode
{
    ARES_OK = 0,

    // Script errors (100–199)
    ARES_FILE_NOT_FOUND = 100,
    ARES_INVALID_DIRECTIVE = 101,
    ARES_INVALID_GROUP_VALUE = 102,
    ARES_WRAP_MISSING_ARGS = 103,
    ARES_UNKNOWN_RULE = 104,

    // Structural errors (200–299)
    ARES_STRICT_GROUP_MISMATCH = 200,
    ARES_EMPTY_INPUT = 201,

    // Transformation errors (300–399)
    ARES_DELIM_NO_SPACE = 300,
    ARES_VARIABLE_UNDEFINED = 301,

    // Internal failures (900+)
    ARES_INTERNAL_ERROR = 900
};

// Initial state
bool seen_group = false;
bool seen_wrap = false;
bool seen_delim = false;
bool seen_wdelim = false;
// All false.

// External tokenizer from core_sys
extern std::vector<std::string> smart_tokenize(const std::string &input);

struct AresRule
{
    std::string pattern;
    std::string replacement;
};

/**
 * apply_arescript
 * Strict implementation of the Arescript Translation API.
 * Tracks structural errors via global_err_ptr.
 */
std::string apply_arescript(std::string data, const std::string &script_path)
{

    std::cout << "[ARESCRIPT:FORMATTER]:[V:" << arescript_version << "]:[REV:" << arescript_rev << "]"<< std::endl;
    // ==============================
    // Phase 0: Open Script
    // ==============================
    std::ifstream file(script_path);
    if (!file)
    {
        session_errors.push_back(
            "[ARESCRIPT]:[CODE:100]:[FILE_NOT_FOUND] " + script_path);
        last_error_code = 100;
        *global_err_ptr += 1;
        return data;
    }

    // ==============================
    // Phase 1: Script State
    // ==============================
    struct AresRule
    {
        std::string pattern, replacement;
    };
    std::vector<AresRule> rules;

    int group_size = 1;
    bool is_strict = false;

    std::string wrap_open, wrap_close;

    enum class WDelimMode
    {
        NONE,
        WRAP,
        QUOTE_SINGLE,
        QUOTE_DOUBLE,
        QPERCENT
    };
    WDelimMode wdelim_mode = WDelimMode::NONE;
    std::string wdelim_wrap;

    std::string line;

    // ==============================
    // Phase 2: Parse Script
    // ==============================
    while (std::getline(file, line))
    {
        auto tokens = smart_tokenize(line);
        if (tokens.empty())
            continue;

        const std::string &cmd = tokens[0];

        if (cmd == "STRICT")
        {
            is_strict = true;
        }

        else if (cmd == "GROUP")
        {
            if (tokens.size() != 2 || std::stoi(tokens[1]) <= 0)
            {
                session_errors.push_back(
                    "[ARESCRIPT]:[CODE:102]:[INVALID_GROUP] GROUP requires integer > 0");
                last_error_code = 102;
                *global_err_ptr += 2;
                return data;
            }
            group_size = std::stoi(tokens[1]);
        }

        else if (cmd == "WRAP")
        {
            if (tokens.size() != 3)
            {
                session_errors.push_back(
                    "[ARESCRIPT]:[CODE:103]:[INVALID_WRAP] WRAP requires open and close");
                last_error_code = 103;
                *global_err_ptr += 2;
                return data;
            }
            wrap_open = tokens[1];
            wrap_close = tokens[2];
        }

        else if (cmd == "DELIM")
        {
            if (tokens.size() != 2)
            {
                session_errors.push_back(
                    "[ARESCRIPT]:[CODE:104]:[INVALID_DELIM] DELIM requires one value");
                last_error_code = 104;
                *global_err_ptr += 1;
                return data;
            }
            rules.push_back({"DELIM", tokens[1]});
        }

        else if (cmd == "WDELIM")
        {
            if (tokens.size() < 3)
            {
                session_errors.push_back(
                    "[ARESCRIPT]:[CODE:105]:[INVALID_WDELIM] Expected WDELIM QUOTE <SINGLE|DOUBLE> or WDELIM WRAP <token>");
                last_error_code = 105;
                *global_err_ptr += 1;
                return data;
            }

            if (tokens[1] == "QUOTE")
            {
                if (tokens[2] == "SINGLE")
                {
                    wdelim_mode = WDelimMode::QUOTE_SINGLE;
                }
                else if (tokens[2] == "DOUBLE")
                {
                    wdelim_mode = WDelimMode::QUOTE_DOUBLE;
                }
                else if (tokens[2] == "QPERCENT"){
                    wdelim_mode = WDelimMode::QPERCENT;
                }
                else
                {
                    session_errors.push_back(
                        "[ARESCRIPT]:[CODE:105]:[INVALID_WDELIM_QUOTE] Use SINGLE, DOUBLE or QPERCENT");
                    last_error_code = 105;
                    *global_err_ptr += 1;
                    return data;
                }
            }

            else if (tokens[1] == "WRAP")
            {
                if (tokens.size() != 3)
                {
                    session_errors.push_back(
                        "[ARESCRIPT]:[CODE:105]:[INVALID_WDELIM_WRAP] \"WRAP\" requires one token");
                    last_error_code = 105;
                    *global_err_ptr += 1;
                    return data;
                }
                wdelim_mode = WDelimMode::WRAP;
                wdelim_wrap = tokens[2];
            }

            else
            {
                session_errors.push_back(
                    "[ARESCRIPT]:[CODE:105]:[UNKNOWN_WDELIM_MODE] \"" + tokens[1] + "\"");
                last_error_code = 105;
                *global_err_ptr += 1;
                return data;
            }
        }

        else if (cmd == "$.")
        {
            if (tokens.size() != 2)
            {
                session_errors.push_back(
                    "[ARESCRIPT]:[CODE:301]:[INVALID_VARIABLE_RULE] $. requires value");
                last_error_code = 301;
                *global_err_ptr += 1;
                return data;
            }
            rules.push_back({"$.", tokens[1]});
        }

        else if (tokens.size() >= 2)
        {
            rules.push_back({tokens[0], tokens[1]});
        }

        else
        {
            session_errors.push_back(
                "[ARESCRIPT]:[CODE:101]:[UNKNOWN_DIRECTIVE] " + cmd);
            last_error_code = 101;
            *global_err_ptr += 1;
            return data;
        }
    }

    // ==============================
    // Phase 3: Line-based Transform
    // ==============================
    std::stringstream output;
    if (!wrap_open.empty())
        output << wrap_open << "\n";

    std::stringstream input_stream(data);
    std::string input_line;
    bool first_entry = true;

    while (std::getline(input_stream, input_line))
    {
        if (input_line.empty())
            continue;

        auto words = smart_tokenize(input_line);

        if (words.empty())
            continue;

        if (is_strict && (words.size() % group_size != 0))
        {
            session_errors.push_back(
                "[ARESCRIPT]:[CODE:200]:[STRICT_GROUP_MISMATCH_LINE] " +
                std::to_string(words.size()) + " % " +
                std::to_string(group_size));
            last_error_code = 200;
            *global_err_ptr += 5;
            return data;
        }

        for (size_t i = 0; i < words.size(); i += group_size)
        {
            std::string chunk;

            for (int g = 0; g < group_size && i + g < words.size(); ++g)
            {
                std::string current = words[i + g];

                // Word-level delimiter
                if (wdelim_mode == WDelimMode::QUOTE_SINGLE)
                    current = "'" + current + "'";
                else if (wdelim_mode == WDelimMode::QUOTE_DOUBLE)
                    current = "\"" + current + "\"";
                else if (wdelim_mode == WDelimMode::QPERCENT)
                    current = "%" + current + "%";
                else if (wdelim_mode == WDelimMode::WRAP)
                    current = wdelim_wrap + current + wdelim_wrap;

                // Apply rules
                for (const auto &rule : rules)
                {
                    if (rule.pattern == "$.")
                    {
                        if (internal_vars.count(current))
                            current = internal_vars[current];
                        else if (is_strict)
                        {
                            session_errors.push_back(
                                "[ARESCRIPT]:[CODE:301]:[VARIABLE_UNDEFINED] " + current);
                            last_error_code = 301;
                            *global_err_ptr += 2;
                            return data;
                        }
                    }
                    else
                    {
                        size_t p = current.find(rule.pattern);
                        if (p != std::string::npos)
                            current.replace(p, rule.pattern.size(), rule.replacement);
                    }
                }

                if (!chunk.empty())
                    chunk += " ";
                chunk += current;
            }

            // Apply DELIM to chunk
            for (const auto &rule : rules)
            {
                if (rule.pattern == "DELIM")
                {
                    size_t p = chunk.find(" ");
                    if (p == std::string::npos)
                    {
                        session_errors.push_back(
                            "[ARESCRIPT]:[CODE:300]:[DELIM_FAILED]");
                        last_error_code = 300;
                        *global_err_ptr += 1;
                        if (is_strict)
                            return data;
                    }
                    else
                    {
                        chunk.replace(p, 1, rule.replacement);
                    }
                }
            }

            if (!first_entry)
                output << ",\n";
            first_entry = false;

            output << "  " << chunk;
        }
    }

    output << "\n";
    if (!wrap_close.empty())
        output << wrap_close;

    last_error_code = 0;
    return output.str();
}