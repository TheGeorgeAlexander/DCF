# Compiler and compiler flags
CXX = clang++
FLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Werror -O3 -Iinclude

# Source and build directories
SRC = test
BUILD = build

# Find all source files in the source directory and subdirectories
SRC_FILES = $(shell find $(SRC) -type f -name '*.cpp')

# Create a list of object files from the source files
OBJ_FILES = $(patsubst $(SRC)/%, $(BUILD)/%, $(SRC_FILES:.cpp=.o))

# Target
TARGET = dcf-test



$(TARGET): $(OBJ_FILES)
	$(CXX) $(FLAGS) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(FLAGS) -c -o $@ $<


.PHONY: rebuild
rebuild:
	@$(MAKE) clean $(TARGET)


.PHONY: clean
clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)
