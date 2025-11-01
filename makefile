# Compiler, flags and dirs 
CC := gcc 
CFLAGS := -Wall -Wextra -std=c99 -Iinclude
SRC_DIR := src 
BUILD_DIR := build 
TARGET := $(BUILD_DIR)/void

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Default rule
all: $(TARGET)

# Link object files 
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Compile each source file to object
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	rm $(BUILD_DIR)/*
