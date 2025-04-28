CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
INCLUDES = -Iinclude -I.

# Source files
SRC_DIR = src
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(SRC_FILES:.cpp=.o)

# Test files
TEST_DIR = test
TEST_SRC = $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS = $(TEST_SRC:.cpp=)

# Main executable
MAIN = signal_os

# Default target
all: $(MAIN) tests

# Compile main program
$(MAIN): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile tests
tests: $(TEST_BINS)

$(TEST_DIR)/%: $(TEST_DIR)/%.cpp $(filter-out $(SRC_DIR)/main.o, $(OBJ_FILES))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Compile object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# Clean up
clean:
	rm -f $(OBJ_FILES) $(MAIN) $(TEST_BINS)

# Run the main program
run: $(MAIN)
	./$(MAIN)

# Run a specific test
run_test_%: $(TEST_DIR)/test_%
	./$(TEST_DIR)/test_$*

# Run all tests
run_tests: $(TEST_BINS)
	for test in $(TEST_BINS); do \
		echo "Running $$test"; \
		./$$test; \
	done

.PHONY: all tests clean run run_tests
