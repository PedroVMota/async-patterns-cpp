CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I./include
LDFLAGS = -pthread
AR = ar
ARFLAGS = rcs

# Installation directories
PREFIX = /usr/local
INSTALL_LIB_DIR = $(PREFIX)/lib
INSTALL_INCLUDE_DIR = $(PREFIX)/include

# Directories
INCLUDE_DIR = include
SRC_DIR = taskrunner
TEST_DIR = tests
LIB_DIR = lib
BUILD_DIR = build

# Source files
TASKRUNNER_SRC = $(SRC_DIR)/TaskRunner.cpp
TASKRUNNER_OBJ = $(BUILD_DIR)/TaskRunner.o

# Test files
SINGLETON_TEST = $(TEST_DIR)/SingletonTest.cpp
TASKRUNNER_TEST = $(TEST_DIR)/TaskRunnerTest.cpp

# Output
LIBRARY = $(LIB_DIR)/libtaskrunner.a
SINGLETON_TEST_BIN = $(BUILD_DIR)/singleton_test
TASKRUNNER_TEST_BIN = $(BUILD_DIR)/taskrunner_test

# Default target
all: library

# Build the static library
library: $(LIBRARY)
	@echo "TaskRunner library built successfully: $(LIBRARY)"
	@echo "Header files in: $(INCLUDE_DIR)"

$(LIBRARY): $(TASKRUNNER_OBJ) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(TASKRUNNER_OBJ): $(TASKRUNNER_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test executables
tests: $(SINGLETON_TEST_BIN) $(TASKRUNNER_TEST_BIN)
	@echo "Test executables built successfully"

$(SINGLETON_TEST_BIN): $(SINGLETON_TEST) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

$(TASKRUNNER_TEST_BIN): $(TASKRUNNER_TEST) $(LIBRARY) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ -L$(LIB_DIR) -ltaskrunner $(LDFLAGS)

# Run tests
run-tests: tests
	@echo "\n=== Running Singleton Tests ==="
	@$(SINGLETON_TEST_BIN)
	@echo "\n=== Running TaskRunner Tests ==="
	@$(TASKRUNNER_TEST_BIN)

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)

# Install library and headers system-wide (requires sudo)
install: all
	@echo "Installing TaskRunner library to $(PREFIX)..."
	install -d $(INSTALL_LIB_DIR)
	install -d $(INSTALL_INCLUDE_DIR)
	install -m 644 $(LIBRARY) $(INSTALL_LIB_DIR)/
	install -m 644 $(INCLUDE_DIR)/Singleton.h $(INSTALL_INCLUDE_DIR)/
	install -m 644 $(INCLUDE_DIR)/TaskRunner.h $(INSTALL_INCLUDE_DIR)/
	@echo "Installation complete!"
	@echo "Library installed to: $(INSTALL_LIB_DIR)/libtaskrunner.a"
	@echo "Headers installed to: $(INSTALL_INCLUDE_DIR)/"

# Uninstall library and headers from system
uninstall:
	@echo "Uninstalling TaskRunner library from $(PREFIX)..."
	rm -f $(INSTALL_LIB_DIR)/libtaskrunner.a
	rm -f $(INSTALL_INCLUDE_DIR)/Singleton.h
	rm -f $(INSTALL_INCLUDE_DIR)/TaskRunner.h
	@echo "Uninstallation complete!"

.PHONY: all library tests run-tests clean install uninstall
