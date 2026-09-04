// Include for ALL definitions that are NOT inside the int main() context.
// The reason this exist? to shut up the warning about #pragma once.
#include "shell.hpp"
#include "include/std_glbl.hpp"
#include <algorithm>
#include <string>
namespace ARES {
  namespace IO {
    void clearTerminalContents(const std::vector<std::string>& args){
      printf ("\033[2J\033[3J\033[H");
    }
  }
}
// ovbious main entry point. we might or might not need this... just saying, i
// is very self-explanatory.
int main(int argc, char *argv[]) {
  /*
   Initialize all internal variables, environment, and GEPs...
   this used to run depending on the casecall, but, Shell Modifiers shouldn't really affect, we want these to be loaded regardless.
   */
  ARES::CORE::init_system();

  /*
     Some Shell Modifiers checking on start up, so we know if we need to do something differently than the default AIM(Always Interactive Mode) casecall.
     Not explicitly necessary for ARES, but, i thought it would be nice to have.
     Notes? Yes, i didn't made this a MemMap, because the modifiers will NOT expand more than NI-Mode, and QUIET.
     These are just flag-checking. if we had more complex modifiers, ARES would fail on its purpose. which is to be simple and easy on memory.
     Which heavy MemMaps are not.
  */
  if (argc >= 3 && std::string(argv[1]) == "\\C") {
        std::string cmd = argv[2];

        auto tokens = ARES::MODULES::AEX::smart_tokenize(cmd);
        ARES::RTE::ENV::expand_variables(tokens);

        if (ARES::CORE::commands.count(tokens[0])) {
            ARES::CORE::commands[tokens[0]](tokens);
        } else if (tokens[0][0] == '@') {
            ARES::RTE::EXTERNALS::run_external(tokens[0].substr(1),
                         {tokens.begin() + 1, tokens.end()});
        } else {
            ARES::CORE::handle_syntax_punishment();
        }

        return last_error_code;
    }
    // Deadbrain simple checking for the quiet flag.
  if (argc > 1 && argv[1] == std::string("\\QUIET")) {

  }
  // same, mute the header if there is a .QUIET File.
  else if (fs::exists(ARES::RTE::ENV::internal_vars["HOME"]+"/.QUIET")){

  }
  else {
    std::cout << ARES::CORE::HELP::HLPMSG << std::endl;
  }
  std::string line;
  bool condition_active = false; // are we inside an #IF block?
  bool condition_met = false;    // did the condition pass?
  // This is me being nice. and yes, it is hardcoded.
  // You get ONE way to personalize your environment before i drop you into the Interactive Prompt.
  // I know this is flaky. but, you shouldn't really worry about this, since all this does is run an init script if it exists. and is AEX, so, properly documented stuff.
  if (fs::exists(ARES::RTE::ENV::internal_vars["HOME"]+"/System/Programs/ARES/preinit.ares")){
    std::cout << "Running Pre-Init Automatization/Environment Setup" << std::endl ; // : " << ARES::RTE::ENV::internal_vars["HOME"]+"/System/Programs/ARES/preinit.ares" <<std::endl; // debug extras. not needed on normal execution.
    auto cli = "\\@AEX " + ARES::RTE::ENV::internal_vars["HOME"]+"/System/Programs/ARES/preinit.ares";
    auto tokens = ARES::MODULES::AEX::smart_tokenize(cli);
    if (ARES::CORE::commands.count(tokens[0])) {
      ARES::CORE::commands[tokens[0]](tokens);
    }
  }
  while (true) {
    std::cout << "ARES &> ";  // The prompt you see in the shell, this is hardcoded for now, but, yes, i am thinking about making it customizable in the future by using PlainASCIIStrings.
                              // No, we are not getting ANSI ECS in the prompt. that is NOT going to be a thing.

    // Redundant parsing :v
    if (!std::getline(std::cin, line))
      break;
    if (line.empty())
      continue;
    // Yes, there's command history, but is only stored in memory, and is not saved to disk. i was going to add a command to save it... but... why?
    push_history(line); // You know what you execute. you can just scroll up on the scrollback to see it. if you can't... work on your memory, bruh.

    // More redundant parsing... but, now this actually detects operands.
    auto tokens = ARES::MODULES::AEX::smart_tokenize(line);
    std::string cmd = tokens[0];

    // --- #IF / #ELSE BLOCK ---
    if (cmd == "\\#IF") {
      condition_active = true;
      if (line.find("RETURNS !0") != std::string::npos)
        condition_met = (last_error_code != 0);
      else
        condition_met = (last_error_code == 0);
      continue;
    }
    if (cmd == "\\#ELSE") {
      condition_met = !condition_met;
      continue;
    }
    if (cmd == "\\#ENDIF") {
      condition_active = false;
      condition_met = false;
      continue;
    }
    if (condition_active && !condition_met)
      continue; // skip line

    // --- VARIABLE ASSIGNMENT ---
    // Brief explanation?
    /**
     * If we have 2 tokens, but we lack \ or @, then we are going to assign a variable.
     * this is on PURPOSE to punish POSIX Users who are used to not namespacing their execution operands.
     * Not a bug, yes, this is supposed to be like this.
     *
     * If you want to run anything, you MUST use prefixes, and use the correct syntax. this is the WHOLE POINT of ARES.
     */
    if (tokens.size() == 2 && cmd[0] != '\\' && cmd[0] != '@') {
      ARES::RTE::ENV::internal_vars[cmd] = tokens[1];
      continue;
    }
    // Since i am nice, i expand all %VAR automatically. yes, this means you can use vars in your cli without weird bugs.
    // You're welcome.
    ARES::RTE::ENV::expand_variables(tokens);

    // Even more redundant parsing.
    // But we actually execute shit now.
    if (ARES::CORE::commands.count(tokens[0])) {
      ARES::CORE::commands[tokens[0]](tokens);
    } else if (tokens[0][0] == '@') {
      ARES::RTE::EXTERNALS::run_external(tokens[0].substr(1), {tokens.begin() + 1, tokens.end()});
    } else {
      ARES::CORE::handle_syntax_punishment();
    }
  }
  return 0;
}