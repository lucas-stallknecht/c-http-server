TARGET = server

SRC_DIR := src/
BUILD_DIR := build/

SRC := $(wildcard $(SRC_DIR)*.c)
OBJS := $(patsubst $(SRC_DIR)%.c,$(BUILD_DIR)%.o,$(SRC))

# Build target that depends on every .o
$(TARGET): $(OBJS)
	gcc $^ -o $@

# Create the build/ directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile the source files into object files
$(BUILD_DIR)%.o: $(SRC_DIR)%.c | $(BUILD_DIR)
	gcc -c $< -o $@
