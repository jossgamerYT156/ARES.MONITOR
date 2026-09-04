#pragma once
#include <string>
#include <unistd.h>
#include <limits.h>

#if defined(__APPLE__)
// Include so we can use _NSGetExecutablePath on macOS.
// Because of course Apple has to be different and special. -_-
#include <mach-o/dyld.h>
#endif
namespace ARES {
    namespace CORE {
        // There at some point will be more than just this. for now, this is simply to define stuff and not have weird bugs.

        namespace UTILS {
            /**
             * @brief Gets the absolute path of the currently running executable(ARES Monitor... expected.)
             * @return std::string Absolute path to the executable, or "ares" as a fallback if it fails.
             * @note This function uses the /proc/self/exe symlink, which is a Linux-specific feature. It may not work on non-Linux systems.
             */
            #if defined(__linux__)
            std::string get_self_path()
            {
                char buffer[PATH_MAX];
                ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
                if (len != -1) {
                    buffer[len] = '\0';
                    return std::string(buffer);
                }
                return "ares"; // fallback if something goes wrong
            }
            #elif defined(__APPLE__)
            std::string get_self_path()
            {
                char buffer[PATH_MAX];
                uint32_t size = sizeof(buffer);
                if (_NSGetExecutablePath(buffer, &size) == 0) {
                    return std::string(buffer);
                }
                return "ares";
            }
            #elif defined(__unix__)
            // For other UNIX-like systems, we can try using /proc/self/path/a.out
            std::string get_self_path()
            {
                char buffer[PATH_MAX];
                // a.out thing is more common on OpenBSD, so, good luck!.
                // If you are running NetBSD or FreeBSD or SystemV, i am sorry, i cannot cover up EVERY UNIX-Like system out there.
                // So, modify this to whatever fits your system if you wanna make it work... or just... use OpenBSD or Linux... they are more generic.
                // There's also VMs and Compatibility Layers... so, have fun with it.
                ssize_t len = readlink("/proc/self/path/a.out", buffer, sizeof(buffer) - 1);
                if (len != -1) {
                    buffer[len] = '\0';
                    return std::string(buffer);
                }
                return "ares"; // fallback if something goes wrong
            }
            #else
            // For unsupported platforms, return a generic name.
            std::string get_self_path()
            {
                return "ares"; // fallback for unsupported platforms
            }
            #endif
        }
    }
}