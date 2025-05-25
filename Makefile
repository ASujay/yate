# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# SDL2 flags via pkg-config
SDL_CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_ttf)
SDL_LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_ttf)

# Directories
SRC_DIR = ./src
BUILD_DIR = ./build
BIN_DIR = ./bin

# Source file 
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Executable
TARGET = $(BIN_DIR)/test

# Build target
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(SDL_LDFLAGS)

# Build object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	mkdir -p $@

# Create bin directory
$(BIN_DIR):
	mkdir -p $@

# Main target
all: $(BIN_DIR) $(TARGET)

# Clean up
clean:
	rm -r $(BIN_DIR) $(BUILD_DIR)

run:
	./bin/test

.PHONY: all clean run
