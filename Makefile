# Main Makefile - Coordinates testing and library building
#
# Flow:
# 1. Makefile.gtest - Downloads Google Test, compiles tests, runs tests
# 2. Makefile.lib   - Compiles library and handles installation
#
# Usage:
#   make              - Run tests, then build library
#   make install-local - Run tests, build library, install locally
#   make install      - Run tests, build library, install system-wide
#   make clean        - Clean all build artifacts
#   make clean-all    - Clean everything including Google Test

# Default target - runs tests then builds library
all: library

# Build the static library (requires tests to pass first)
library:
	@echo "=== Step 1: Building library for testing ==="
	@$(MAKE) -f Makefile.lib library
	@echo "\n=== Step 2: Running tests with Google Test ==="
	@$(MAKE) -f Makefile.gtest run-tests || (echo "\n✗ Tests failed! Removing library." && $(MAKE) -f Makefile.lib clean && exit 1)
	@echo "\n=== Step 3: All tests passed! Library is ready ==="
	@echo "\n=== Build complete! ==="
	@$(MAKE) -f Makefile.lib library

# Install library and headers locally (in build/install)
install-local:
	@echo "=== Step 1: Building library for testing ==="
	@$(MAKE) -f Makefile.lib library
	@echo "\n=== Step 2: Running tests with Google Test ==="
	@$(MAKE) -f Makefile.gtest run-tests || (echo "\n✗ Tests failed! Library will not be installed." && $(MAKE) -f Makefile.lib clean && exit 1)
	@echo "\n=== Step 3: Tests passed! Installing library locally ==="
	@$(MAKE) -f Makefile.lib install-local
	@echo "\n=== Local installation complete! ==="

# Install library and headers system-wide (requires sudo)
install:
	@echo "=== Step 1: Building library for testing ==="
	@$(MAKE) -f Makefile.lib library
	@echo "\n=== Step 2: Running tests with Google Test ==="
	@$(MAKE) -f Makefile.gtest run-tests || (echo "\n✗ Tests failed! Library will not be installed." && $(MAKE) -f Makefile.lib clean && exit 1)
	@echo "\n=== Step 3: Tests passed! Installing library system-wide ==="
	@$(MAKE) -f Makefile.lib install
	@echo "\n=== System-wide installation complete! ==="

# Uninstall library and headers from system
uninstall:
	@$(MAKE) -f Makefile.lib uninstall

# Clean build artifacts
clean:
	@echo "Cleaning test artifacts..."
	@$(MAKE) -f Makefile.gtest clean
	@echo "Cleaning library artifacts..."
	@$(MAKE) -f Makefile.lib clean
	@echo "Clean complete!"

# Clean everything including Google Test
clean-all:
	@echo "Cleaning all artifacts including Google Test..."
	@$(MAKE) -f Makefile.gtest clean-all
	@$(MAKE) -f Makefile.lib clean
	@echo "Deep clean complete!"

.PHONY: all library install-local install uninstall clean clean-all
