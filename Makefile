.PHONY: all clean

CXX = clang++
CFLAGS = -I./src/include -std=c++17
COUT = bin/ARES.MON

SOURCES = src/*.cpp

all: $(SOURCES)
	@echo "Sources: $<\nCompiling '$<'\nOutput: $(COUT)"
	@$(CXX) $(CFLAGS) $< -o $(COUT)

clean:
	@bash ./Helpers/clean.sh