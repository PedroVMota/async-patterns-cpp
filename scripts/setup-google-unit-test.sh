#!/bin/bash

# Ensure the script is executed with a PROJECT_ROOT argument
PROJECT_ROOT=$1
FOLDER_DEPS=".deps"

echo "Current directory: $PWD"

# Create the dependencies folder if it doesn't exist
mkdir -p "$FOLDER_DEPS"

# List files in the current directory (for debugging purposes)
ls -la

# Check if the googletest folder exists, if not, clone it
if [ ! -d "$FOLDER_DEPS/googletest" ]; then
  echo "Cloning googletest..."
  git clone https://github.com/google/googletest.git "$FOLDER_DEPS/googletest"
else
  echo "googletest already exists"
fi
