# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LDFLAGS = -lncurses  # Only needed if using ncurses library

# Project directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
EXEC = $(BIN_DIR)/main

# List of source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default target
all: $(EXEC)

# Link object files into the final executable
$(EXEC): $(OBJS)
	@mkdir -p $(BIN_DIR)  # Create bin directory if it doesn't exist
	$(CXX) $(OBJS) -o $(EXEC) $(LDFLAGS)

# Compile each source file into an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)  # Create obj directory if it doesn't exist
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# Clean up object files and executable
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run the program
run: $(EXEC)
	./$(EXEC)

# Phony targets
.PHONY: all clean run