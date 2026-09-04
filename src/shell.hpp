#ifndef SHELL_HPP
#define SHELL_HPP

// external modules . builtin commands.
#include "core_sys.cpp" // Core system functions
#include "env.cpp"      // Include environment variable operations
#include "fs_ops.cpp"   // Include filesystem operations
#include "help_sys.cpp" // help system, kinda important if you ask me... oh, or you might know this system better than me, in that case, delete it, i dare you mfkr, fs_ops.cpp depends on this.  -- Lilly Aizawa
#include "runexternals.cpp" // for @ and \@EXEC. yes, i made it a different file, DEAL WITH IT.
#include "std_glbl.hpp"
#include "system_ctrl.cpp"

// halt is different, you can delete it and make it UNABLE TO EXIT THE SHELL...
// have fun, masochist.
#include "hlt.cpp"

// .ares scripting support.
#include "modules/AEX/ares.cpp" // ARES Automation Execution eXtension

// Filesystem bullshit.
#include <filesystem>
namespace fs = std::filesystem;

// write is a builtin command, so include it here. this isn't super critical,
// you can delete or move it, but i don't recommend it because you'll break
// stuff. like... a lot of stuff.
#include "write.cpp"
#include <string>
#include <unordered_map>
// later the ARES Monitor will have ARES_AUTOMATE to auto-handle stuff, but for
// now, and it'll require a LOT of builtins to work, so, maybe leave them
// alone... for the sake of their and your own sanity.
namespace ARES::CORE
{
  extern std::unordered_map<std::string, CommandFunc> commands;
}
// MARK - \@REPORT Logic. for if-else block reproting of errors into the
// public-error counter.
void handle_report(const std::vector<std::string> &args) {
  if (args.size() < 2)
    return;

  // Shift tokens to run the command inside the report
  std::vector<std::string> sub_tokens(args.begin() + 1, args.end());

  if (ARES::CORE::commands.count(sub_tokens[0])) {
    ARES::CORE::commands[sub_tokens[0]](sub_tokens);
    // last_error_code is set by external runs or handle_syntax_punishment
    // You can now check this value for your IF-ELSE logic.
  }
}
#pragma once
#include "getErrors.cpp"

#endif