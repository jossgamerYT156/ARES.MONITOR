.PHONY: all macos linux-cross linux-native clean build install

# --- OS Detection ---
## We at LDS have decided that Microsoft's Windows does NOT deserve our interest, if you wish to compile for said arch.
## You may create your own fork and compile it with your own Compile rules.
# - Lilly Aizawa (LDS LLC's CEO)
ifeq ($(OS),Windows_NT)
    OS_TYPE := Windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OS_TYPE := Linux
    else ifeq ($(UNAME_S),Darwin)
        OS_TYPE := macOS
    else
        OS_TYPE := Unknown ## fallback for anything that isn't Linux/macOS. sorry BSD, you will need to work this one out.
    endif
endif

# --- Tools and Flags ---
## For now we just support:
## clang++ (c++ Compiler toolchain), zig c++ (macos-linux cross-compiler.)

ZIGCXX      = zig c++
CXX         = clang++
ZIGCFLAGS   = -I./src/include -target x86_64-linux-musl
CFLAGS      = -I./src/include -std=c++17
SOURCES     := $(wildcard src/*.cpp)
# --- Output Paths for binaries, only for Linux and MacOS x86_64 ---
## even though it can indeed emmit arm64 binaries using clang++ on macOS, if ran on an Apple Silicon Apple Computer.

LINUXCOUT   = bin/ARES.MON.LINUX_x64 # Linux Output
COUT        = bin/ARES.MON.MACOS_x64 # macOS X86_64 output(arm64 output if applicable.)

# --- Build Targets ---
all: build

build:
ifeq ($(OS_TYPE),Linux)
	@$(MAKE) linux-native
else ifeq ($(OS_TYPE),macOS)
	@$(MAKE) macos
else ifeq ($(OS_TYPE), Windows)
	@echo "LDS LLC(c) does not distribute for Microsoft(R) Windows NT(c/r)."
	@echo "We refuse to provide binaries for any MS Product, now or ever."
	@echo "Thanks. - LDS LLC(c)"
	@exit 1
else
	@echo "Unsupported OS: $(OS_TYPE)"
	@exit 126
endif

$(COUT):build

$(LINUXCOUT):build

macos: $(SOURCES)
	@echo "Compiling macOS binary..."
	@$(CXX) $(CFLAGS) $^ -o $(COUT)

linux-native: $(SOURCES)
	@echo "Compiling Linux binary..."
	@$(CXX) $(CFLAGS) $^ -o $(LINUXCOUT)

linux-cross: $(SOURCES)
	@echo "Cross-compiling for Linux via Zig..."
	@$(ZIGCXX) $(ZIGCFLAGS) $^ -o $(LINUXCOUT)

# --- Install Rules ---
# Using a generic install that branches based on detected OS, just in case people wanna actually use ARES more than just for hobby reasons.
install:
ifeq ($(OS_TYPE),macOS)
	@$(MAKE) macinstall
else ifeq ($(OS_TYPE),Linux)
	@$(MAKE) linstall
endif

macinstall: $(COUT)
	@echo "Installing for macOS..."
	@bash ./Helpers/macinstall.sh

linstall: $(LINUXCOUT)
	@echo "Installing for Linux..."
	@bash ./Helpers/install.sh

clean:
	@bash ./Helpers/clean.sh