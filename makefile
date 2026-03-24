# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0) -lpthread

# Target executable name
TARGET = debian-fufus

# Source and Object files
SRC_DIR = src
OBJ_DIR = obj
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/usb.c $(SRC_DIR)/gui.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default rule
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Install (optional, moves to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

.PHONY: all clean install
