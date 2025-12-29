// external modules . builtin commands.
#include "help_sys.cpp" // help system, kinda important if you ask me... oh, or you might know this system better than me, in that case, delete it, i dare you mfkr, fs_ops.cpp depends on this.  -- Lilly Aizawa
#include "fs_ops.cpp" // Include filesystem operations
#include "env.cpp"    // Include environment variable operations
#include "std_glbl.hpp"
#include "system_ctrl.cpp"
#include "runexternals.cpp" // for @ and \@EXEC. yes, i made it a different file, DEAL WITH IT.
#include "core_sys.cpp" // Core system functions

// halt is different, you can delete it and make it UNABLE TO EXIT THE SHELL... have fun, masochist.
#include "hlt.cpp"

// .ares scripting support.
#include "modules/AEX/ares.cpp" // ARES Automation Execution eXtension

// write is a builtin command, so include it here. this isn't super critical, you can delete or move it, but i don't recommend it because you'll break stuff. like... a lot of stuff.
#include "write.cpp"
// later the ARES Monitor will have ARES_AUTOMATE to auto-handle stuff, but for now, and it'll require a LOT of builtins to work, so, maybe leave them alone... for the sake of their and your own sanity.

// MARK - \@REPORT Logic. for if-else block reproting of errors into the public-error counter.
void handle_report(const std::vector<std::string>& args) {
    if (args.size() < 2) return;

    // Shift tokens to run the command inside the report
    std::vector<std::string> sub_tokens(args.begin() + 1, args.end());
    
    if (commands.count(sub_tokens[0])) {
        commands[sub_tokens[0]](sub_tokens);
        // last_error_code is set by external runs or handle_syntax_punishment 
        // You can now check this value for your IF-ELSE logic.
    }
}

// ovbious main entry point. we might or might not need this... just saying, i is very self-explanatory.
int main() {
    init_system();
    std::cout << HLPMSG << std::endl; 
    std::string line;
    bool skip_logic = false;

    while (true) {
        std::cout << "ARES &> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        auto tokens = smart_tokenize(line);
        std::string cmd = tokens[0];

        // --- #IF / #ELSE BLOCK ---
        if (cmd == "\\#IF") {
            if (line.find("RETURNS !0") != std::string::npos) skip_logic = (last_error_code == 0);
            else skip_logic = (last_error_code != 0);
            continue;
        }
        if (cmd == "\\#ELSE") {
            skip_logic = !skip_logic;
            continue;
        }
        if (skip_logic) {
            skip_logic = false;
            continue;
        }

        // --- VARIABLE ASSIGNMENT ---
        if (tokens.size() == 2 && cmd[0] != '\\' && cmd[0] != '@') {
            internal_vars[cmd] = tokens[1];
            continue;
        }

        expand_variables(tokens);

        if (commands.count(tokens[0])) {
            commands[tokens[0]](tokens);
        } else if (tokens[0][0] == '@') {
            run_external(tokens[0].substr(1), {tokens.begin() + 1, tokens.end()});
        } else {
            handle_syntax_punishment();
        }
    }
    return 0;
}