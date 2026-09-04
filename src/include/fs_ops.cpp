
// for FS stuff.
#include "std_glbl.hpp"
#include <filesystem>
#include <iomanip> // For pretty alignment
#include <string>
#include <vector>
#include <iostream>

namespace fs = std::filesystem;

namespace ARES
{
    namespace IO
    {
    namespace FileOperations
    {
      // MARK: - Create File/Directory Logic
      // Logic for @CREATE
      void handle_create(const std::vector<std::string> &args)
      {
        if (args.size() < 3)
          return;
        // @CREATE FILE <Path> [WITH <Contents>]
        if (args[1] == "FILE") {
          std::ofstream file(args[2]);
          if (args.size() >= 5 && args[3] == "WITH") {
            file << args[4];
          }
          return;
        }

        // @CREATE DIR <Path>
        if (args[1] == "DIR" && args.size() == 3) {
          try {
            fs::create_directory(args[2]);
          } catch (fs::filesystem_error e) {
            std::string emsg= "[\\@CREATE DIR \\ E] [" + std::string(e.what()) + "]";
            session_errors.push_back(emsg);
          }
          return;
        }

        // @CREATE DIR STRUCTURE <Path>
        if (args[1] == "DIR" && args[2] == "STRUCTURE" && args.size() >= 4) {
          try {fs::create_directories(args[3]);}
          catch (fs::filesystem_error e) {
            std::string emsg= "[\\@CREATE DIR STRUCTURE \\ E] [" + std::string(e.what()) + "]";
            session_errors.push_back(emsg);
          }
          return;
        }
      }

      // MARK: - Delete File/Directory Logic
      // Logic for @DELETE FILE <Path> | @DELETE DIR <Path> | @DELETE DIR RECURSIVELY
      // <Path>
      void handle_delete(const std::vector<std::string> &args)
      {
        if (args.size() < 3) {
          session_errors.push_back(
              "[DELETE]:[Syntax Error]:[Missing TYPE and PATH Sub-Operators]");
          return;
        }

        std::string type = args[1];

        // Case: @DELETE FILE <Path>
        if (type == "FILE") {
          if (!fs::exists(args[2])) {
            std::cerr << "[40433]:[FILE_NOT_FOUND] " << args[2] << std::endl;
            return;
          }
          fs::remove(args[2]);
          return;

        // Case: @DELETE DIR <Path> or @DELETE DIR RECURSIVELY <Path>
        if (type == "DIR") {
          // Subcase: RECURSIVELY
          if (args[2] == "RECURSIVELY" && args.size() >= 4) {
            fs::remove_all(args[3]);
            return;
          }
          // Standard DIR delete (must be empty)
          fs::remove(args[2]);
          return;
        }
      }
      }
    // NS:: IO::FileOperations::*
  static bool path_exists(const std::string &path)
  {
    std::ifstream f(path);
    //std::cout << "[DEBUG path_exists] '" << path << "' = " << f.good()
    //          << std::endl;
    return f.good();
  }

  // MARK: - Change Directory Logic
  // Logic for \\@CWD <path>
  void handle_cwd(const std::vector<std::string> &args)
  {
    // Syntax Check
    if (args.size() < 2) {
      session_errors.push_back(
          "[CWD] : [Syntax Error]:[Missing PATH Sub-Operator]");
      ARES::CORE::HELP::handle_help({"", "\\@CWD"});
      *global_err_ptr += 1;
      return;
    }

    fs::path new_path = args[1];
  
    // Validation using C++17 Filesystem
    if (!fs::exists(new_path)) {
      std::cerr << "[4083]:[PATH_NOT_FOUND] " << args[1] << std::endl;
      session_errors.push_back("[CWD]:[No Such Entry]:[\"" + args[1] + "\"]");
      *global_err_ptr += 1;
      return;
    }

    if (!fs::is_directory(new_path)) {
      std::cerr << "[4043]:[NOT_A_DIRECTORY] " << args[1] << std::endl;
      session_errors.push_back("[CWD]:[FILE ENTRY]:[\"" + args[1] + "\"]");
      *global_err_ptr += 1;
      return;
    }

    // Perform the move
    try {
      fs::current_path(new_path);
    } catch (const fs::filesystem_error &e) {
      std::cerr << "[4053]:[ACCESS_DENIED]" << std::endl;
      session_errors.push_back(
          "[CWD]:[SECINT]:[SystemMessage:" + std::string(e.what()) + "]");
      *global_err_ptr += 1;
    }
  }

    // MARK: - List Directory Contents Logic
    // Logic for @LDC [Path]
    void handle_ldc(const std::vector<std::string> &args)
    {
      // Explicitly cast args[1] to fs::path to resolve compiler ambiguity
      fs::path target_path =
          (args.size() > 1) ? fs::path(args[1]) : fs::current_path();
    
      if (!fs::exists(target_path)) {
        std::cerr << "[4043]:[PATH_NOT_FOUND] " << target_path << std::endl;
        session_errors.push_back("[LDC]: [Target entry does NOT exist]");
        return;
      }
    
      std::cout << "[LISTING]: " << fs::absolute(target_path) << "\n";
      std::cout << "------------------------------------------\n";
    
      try {
        for (const auto &entry : fs::directory_iterator(target_path)) {
          std::string type_label = entry.is_directory() ? "[D] " : "[F] ";
          std::string size_info = "<DIR>";
        
          // ONLY call file_size if it is NOT a directory
          if (!entry.is_directory()) {
            const long long KB = 1024;
            const long long MB = KB * 1024;
            const long long GB = MB * 1024;
            const long long TB = GB * 1024;
          
            std::string size_measure = " B";
            double final_size = static_cast<double>(entry.file_size());
          
            if (entry.file_size() >= TB) {
              final_size /= TB;
              size_measure = " TB";
            } else if (entry.file_size() >= GB) {
              final_size /= GB;
              size_measure = " GB";
            } else if (entry.file_size() >= MB) {
              final_size /= MB;
              size_measure = " MB";
            } else if (entry.file_size() >= KB) {
              final_size /= KB;
              size_measure = " KB";
            }
          
            // Clean string formatting
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << final_size << size_measure;
            size_info = ss.str();
          }

          std::cout << type_label << std::left << std::setw(20)
                    << entry.path().filename().string() << " | " << size_info
                    << "\n";
        }
      } catch (const std::exception &e) {
        session_errors.push_back("[LDC]:[Could Not Read Entry]:[SystemMessage:" +
                                 std::string(e.what()) + "]");
      }
      std::cout << "------------------------------------------" << std::endl;
    }
  }
}
}