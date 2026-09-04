.PHONY: all macos linux-cross linux-native clean build install

# --- OS Detection ---
## We at LDS have decided that Microsoft's Windows does NOT deserve our interest, if you wish to compile for said arch.
## You may create your own fork and compile it with your own Compile rules.
# - Lilly Aizawa (LDS 's CEO)
ifeq ($(OS),Windows_NT)
    OS_TYPE := Windows ## Windbloat is still banned from existing anywhere near THIS Project.
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        ifeq ($(ANDROID_ROOT), /system) ## If the ANDROID_ROOT Environment variable is set, we are on Android... we might will have issues building this if we use so.
			OS_TYPE := Android
		else ifeq ($(UNAME_S),FreeBSD)
			OS_TYPE := FreeBSD
		else ifeq ($(UNAME_S),OpenBSD)
			OS_TYPE := OpenBSD
		else ifeq ($(UNAME_S),NetBSD)
			OS_TYPE := NetBSD
		else
			OS_TYPE := Linux
		endif
    else ifeq ($(UNAME_S),Darwin)
        OS_TYPE := macOS
    else
        OS_TYPE := Unknown ## fallback for anything that isn't Linux/macOS. sorry BSD, you will need to work this one out.
    endif
endif

# --- Tools and Flags ---
## For now we just support:
## clang++ (c++ Compiler toolchain), zig c++ (macos-linux cross-compiler.)

## For Android, we need the Android NDK Installed in the .NDK Directory. i KNOW this is annoying. but, it is what it is.
## If you already have an NDK Installed, simply replace these lines against the ones you already have.
NDK_CROSS   = aarch64-linux-android21-clang++
NDK_HOME    = .NDK/android-ndk-r27d
ANDROID_CXX = $(NDK_HOME)/toolchains/llvm/prebuilt/linux-x86_64/bin/$(NDK_CROSS)
## Specific OpenSSL For Android, you NEED to cross-compile this yourself.
# You may find OpenSSL's source code @ https://github.com/openssl/openssl/
OPENSSL_ANDROID_PATH = .NDK/openssl/build
ANDROID_FLAGS = -I$(OPENSSL_ANDROID_PATH)/include -L$(OPENSSL_ANDROID_PATH)/lib -lssl -lcrypto

## Legacy code stuff that i really don't need to change... yet.
ZIGCXX      = zig c++
CXX         = clang++
INCLUDES 	=  -I./src/include -I./src/include/* -I./src/include/modules/transcript/
BOOSTINCLUDES = -I/usr/local/Cellar/boost/1.90.0_1/include 
ZIGCFLAGS_LINUX = -target x86_64-linux-musl
CFLAGS      = -std=c++17
SOURCES     := $(wildcard src/*.cpp)
# --- Output Paths for binaries, only for Linux and MacOS x86_64 ---
## even though it can indeed emmit arm64 binaries using clang++ on macOS, if ran on an Apple Silicon Apple Computer.

LINUXCOUT    = bin/ARES.MON.LINUX_x64 # Linux Output
ANDRCOUT	 = bin/ARES.MON.ANDROID_ARM64
COUT         = bin/ARES.MON.MACOS_x64 # macOS X86_64 output(arm64 output if applicable.)

# --- Build Targets ---
all: build

## Target to build it all supported types, for all supported OSes(Except Windows, obviously.)
allTargets: macos android linux-native linux-cross cross-android

build:
ifeq ($(OS_TYPE),Linux)
	@$(MAKE) linux-native
else ifeq ($(OS_TYPE),macOS)
	@$(MAKE) macos
else ifeq ($(OS_TYPE), Windows)
	@printf "\e[31mLDS LLC(c) does not distribute for Microsoft(R) Windows NT(c)/r).\e[0m\n"
	@printf "\e[31mWe refuse to provide binaries for any MS Product, now or ever.\e[0m\n"
	@printf "\e[32mThanks. - LDS (c)\e[0m\n"
	@exit 1
else ifeq ($(OS_TYPE), Android)
	@printf "\e[31mAndroid is not supported as a build platform,\e[0m,"
	@echo " But you can cross-compile for Android using the 'linux-native' target directly if you have the necessary libraries through PKG on Termux, or the 'cross-android' target if you have an available NDK."
	@exit 1
else
	@printf "Unsupported OS: \e[31m$(OS_TYPE)\e[0m\n"
	@echo "This build system so far only supports Linux and macOS as build platforms, with the ability to cross-compile against Android and Linux through Zig and the Android NDK."
	@echo "If you wish to add support for your OS, please incrementally add for so, create a Fork, and submit to PR Review."
	@exit 126
endif

$(COUT):build

$(LINUXCOUT):build

macos: $(SOURCES)
	@echo "Compiling macOS binary..."
	@$(CXX) $(CFLAGS) $(INCLUDES) $(BOOSTINCLUDES) $^ -o $(COUT)

linux-native: $(SOURCES)
	@echo "Compiling Linux binary..."
	@$(CXX) $(CFLAGS) $(INCLUDES) -lssl -lcrypto $^ -o $(LINUXCOUT)

linux-cross: $(SOURCES)
	@echo "Cross-compiling for Linux via Zig..."
	@$(ZIGCXX) $(ZIGCFLAGS_LINUX) $(INCLUDES) $^ -o $(LINUXCOUT)

## This method, checks for an available NDK, if it exists, it uses the toolchain to cross-compile for Android using Static Linking, if this is NOT found, you get an error code.
## Ensure you have an available NDK at the expected location, and that you have the correct permissions to execute the toolchain.
## This method in specific, was made JUST... because i wanted THIS on my phone.
cross-android: $(SOURCES)
	@echo "Cross-compiling for Android via Android's NDK..."
	@bash Helpers/check-android-ndk.sh
	@$(ANDROID_CXX) $(CFLAGS) $(INCLUDES) $(ANDROID_FLAGS) $^ -stdlib=libc++ -static-libstdc++ -o $(ANDRCOUT)

## Simply a pointer, because, i am NOT making a method to compile this within Termux.
## The dependency hell and static-link bs is WAY too much of an annoyance for me to deal or care for.
## So, this is someone else's task and problem to figure out HOW to compile within an android device without dealing with the static-dynamic linking issues.
android: cross-android

# --- Install Rules ---
# Using a generic install that branches based on detected OS, just in case people wanna actually use ARES more than just for hobby reasons.
##
## Added installation functionality and options... for Linux.
## I don't use macOS Enough to care if the install process is a bit more simple and dull.
##
install:
ifeq ($(OS_TYPE),macOS)
	@$(MAKE) macinstall
else ifeq ($(OS_TYPE),Linux)
	@echo "Linux accepts for Unprivileged and Privileged installs, if you wish to install one of these or in a custom directory, format as follows:"
	@echo "- For Custom Path Installation: 'INSTPATH=/path/to/install/directory make install'"
	@echo "- For a Safe Installation (under ~/.local/bin): 'SAFE_INSTALL=true make install'"
	@echo "- For a Privileged Installation (under /usr/local/bin, may require su su and constant acknowledgment of changes): 'PRIVILEGED_INSTALL=true make install'"
	@echo "Or if you wish to simply install with the default settings, just run 'make install'. This will attempt to install under $(HOME)/System/Public/Programs, if not found, it will proceed to try and unstall under /usr/local/bin, using su, which will ask for your password on each step."
	@$(MAKE) linstall
endif

macinstall: $(COUT)
	@echo "Installing for macOS..."
	@bash ./Helpers/macinstall.sh

## I did a few changes on this one to not just go straight into copying files... but, to ask the user to CONFIRM the action.
## JIC you mistyped.
linstall: $(LINUXCOUT)
	@echo "Installing for Linux..."
	@echo -n "This process will write changes to disk. Proceed? (y/n): " && read yN; \
	case "$$yN" in \
		[yYsS]*) echo "Proceeding..." ;; \
		*) echo "Installation aborted." ; exit 1 ;; \
	esac; \
	if [ -n "$(INSTPATH)" ]; then \
		bash ./Helpers/install.sh --prefix "$(INSTPATH)"; \
	elif [ -n "$(SAFE_INSTALL)" ]; then \
		bash ./Helpers/install.sh --safe-lds; \
	elif [ -n "$(PRIVILEGED_INSTALL)" ]; then \
		bash ./Helpers/install.sh --privileged; \
	else \
		bash ./Helpers/install.sh; \
	fi

clean:
	@bash ./Helpers/clean.sh