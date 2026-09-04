#include "std_glbl.hpp"
#include <iostream>
#include <ostream>

namespace ARES::RTE::ENV {
/**
 * @brief Retrieves and displays the current session errors stored in the session error log. This function is designed to provide users with a way to review any errors that have been recorded during the current session. It checks if there are any errors in the log and prints them in a readable format. If no errors are present, it informs the user accordingly.
 * @param args Unused parameter, included for consistency with command handler signatures. This function does not require any arguments to operate.
 * @return void
 */
void getErrors(const std::vector<std::string> &args) {
  if (session_errors.empty()) {
    std::cout << "No Errors have been reported." << std::endl;
    return;
  }

  for (size_t i = 0; i < session_errors.size(); i++) {
    std::cout << "[E]: '" << session_errors[i] << "'" << std::endl;
  }
}
/**
 * @brief Clears the session error log, resetting the error tracking state.
 * This function is useful for starting a new session or clearing old errors after they have been reviewed. It resets the error counter and clears the error vector.
 * @param args Unused parameter, included for consistency with command handler signatures. This function does not require any arguments to operate.
 * @return void
 */
void handle_cel(const std::vector<std::string> &args) {
  session_errors.clear();
  noposix_error_counter = 0;
  std::cout << "[CEL]: Session error log cleared.\n";
}
} // namespace ARES::RTE::ENV