
// Logic for @HALT
// void handle_halt(const std::vector<std::string>& args) {

//     int i = 0;

//     std::cout << "\n[HALTING MONITOR...]" << std::endl;
    
//     if (session_errors.empty()) {
//         std::cout << "Clean execution. No errors reported." << std::endl;
//     } else {
//         std::cout << "The following errors occurred during this session:" << std::endl;
//         for (const auto& err : session_errors) {
//             std::cout << " - [ERR]: " << err << std::endl;
//         }
//     }
//     exit(0);
// }

// Logic for @HALT
void handle_halt(const std::vector<std::string>& args) {

    std::cout << "\n[HALTING MONITOR...]" << std::endl;
    
    if (session_errors.empty()) {
        std::cout << "No error reported." << std::endl;
    } else {
        std::cout << "The following errors occurred during this session:" << std::endl;

        for (size_t i = 0; i < session_errors.size(); ++i) {
            std::cout << "\033[31m - [ERR]: "  << session_errors[i] << "\033[0m" << std::endl;
        }
    }

    exit(0);
}
