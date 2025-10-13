# Integration Guide

This guide explains how to integrate the C++17 utilities library into your project using two different methods: system-wide installation or local dependency.

## Table of Contents

- [System-Wide Installation](#system-wide-installation)
- [Local Dependency](#local-dependency)
- [Usage Examples](#usage-examples)
- [Troubleshooting](#troubleshooting)

---

## System-Wide Installation

Installing the library system-wide makes it available to all projects on your system, similar to other system libraries.

### Installation Steps

1. **Build and install the library** (requires sudo/administrator privileges):

```bash
cd /path/to/singleton_library
make
sudo make install
```

This will install:
- Library: `/usr/local/lib/libtaskrunner.a`
- Headers: `/usr/local/include/Singleton.h` and `/usr/local/include/TaskRunner.h`

2. **Verify installation**:

```bash
ls -l /usr/local/lib/libtaskrunner.a
ls -l /usr/local/include/Singleton.h
ls -l /usr/local/include/TaskRunner.h
```

### Using System-Wide Installation in Your Project

#### For Singleton (Header-Only)

```bash
g++ -std=c++17 -o myapp main.cpp -pthread
```

Example `main.cpp`:
```cpp
#include <Singleton.h>
#include <iostream>

class MyService : public Singleton<MyService> {
    friend class Singleton<MyService>;
private:
    MyService() { std::cout << "Service initialized\n"; }
};

int main() {
    MyService::getInstance();
    return 0;
}
```

#### For TaskRunner

```bash
g++ -std=c++17 -o myapp main.cpp -ltaskrunner -pthread
```

Example `main.cpp`:
```cpp
#include <TaskRunner.h>
#include <iostream>

int main() {
    TaskRunner runner;
    runner.executeTask([]() {
        std::cout << "Task executed!\n";
    });
    runner.waitForCompletion();
    return 0;
}
```

#### For Both Components

```bash
g++ -std=c++17 -o myapp main.cpp -ltaskrunner -pthread
```

### Using with CMake (System-Wide)

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the library
find_library(TASKRUNNER_LIB taskrunner)

# Add your executable
add_executable(myapp main.cpp)

# Link libraries
target_link_libraries(myapp
    ${TASKRUNNER_LIB}
    pthread
)
```

### Using with Makefile (System-Wide)

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -pthread -ltaskrunner

myapp: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f myapp
```

### Uninstalling

To remove the system-wide installation:

```bash
cd /path/to/singleton_library
sudo make uninstall
```

---

## Local Dependency

Using the library as a local dependency keeps it contained within your project, which is useful for:
- Version control
- Project portability
- Avoiding system-wide modifications
- Different versions per project

### Method 1: Git Submodule (Recommended)

1. **Add as submodule**:

```bash
cd /path/to/your/project
git submodule add <repository-url> lib/cpp-utils
git submodule update --init --recursive
```

2. **Build the library**:

```bash
cd lib/cpp-utils
make
cd ../..
```

3. **Compile your project**:

```bash
g++ -std=c++17 -I./lib/cpp-utils/include \
    -o myapp main.cpp \
    -L./lib/cpp-utils/lib -ltaskrunner -pthread
```

### Method 2: Direct Copy

1. **Copy the library into your project**:

```bash
cp -r /path/to/singleton_library /path/to/your/project/lib/cpp-utils
```

2. **Build the library**:

```bash
cd lib/cpp-utils
make
cd ../..
```

3. **Compile your project** (same as Method 1)

### Using with CMake (Local Dependency)

Create a `CMakeLists.txt` in your project root:

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the library directory
set(CPP_UTILS_DIR ${CMAKE_SOURCE_DIR}/lib/cpp-utils)

# Include directories
include_directories(${CPP_UTILS_DIR}/include)

# Link directories
link_directories(${CPP_UTILS_DIR}/lib)

# Add your executable
add_executable(myapp main.cpp)

# Link libraries
target_link_libraries(myapp
    taskrunner
    pthread
)

# Optional: Add custom target to build the library
add_custom_target(build_cpp_utils
    COMMAND make
    WORKING_DIRECTORY ${CPP_UTILS_DIR}
    COMMENT "Building cpp-utils library"
)
add_dependencies(myapp build_cpp_utils)
```

Build your project:

```bash
mkdir build
cd build
cmake ..
make
```

### Using with Makefile (Local Dependency)

Create a `Makefile` in your project root:

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -pthread

# Library paths
CPP_UTILS_DIR = lib/cpp-utils
CPP_UTILS_INCLUDE = $(CPP_UTILS_DIR)/include
CPP_UTILS_LIB = $(CPP_UTILS_DIR)/lib

# Your project
SRC = main.cpp
TARGET = myapp

all: cpp_utils $(TARGET)

# Build the cpp-utils library
cpp_utils:
	cd $(CPP_UTILS_DIR) && $(MAKE)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -I$(CPP_UTILS_INCLUDE) \
		-o $@ $< -L$(CPP_UTILS_LIB) -ltaskrunner $(LDFLAGS)

clean:
	rm -f $(TARGET)
	cd $(CPP_UTILS_DIR) && $(MAKE) clean

.PHONY: all cpp_utils clean
```

Build your project:

```bash
make
```

### Project Structure Example

```
your-project/
├── lib/
│   └── cpp-utils/              # The library (submodule or copy)
│       ├── include/
│       │   ├── Singleton.h
│       │   └── TaskRunner.h
│       ├── lib/
│       │   └── libtaskrunner.a
│       └── Makefile
├── src/
│   └── main.cpp
├── CMakeLists.txt              # or Makefile
└── README.md
```

---

## Usage Examples

### Example 1: Singleton Only

```cpp
#include <Singleton.h>
#include <iostream>
#include <string>

class Config : public Singleton<Config> {
    friend class Singleton<Config>;
private:
    Config() : appName("MyApp"), version("1.0.0") {}
    std::string appName;
    std::string version;

public:
    std::string getAppName() const { return appName; }
    std::string getVersion() const { return version; }
    void setAppName(const std::string& name) { appName = name; }
};

int main() {
    Config::getInstance().setAppName("SuperApp");
    std::cout << "App: " << Config::getInstance().getAppName() << "\n";
    std::cout << "Version: " << Config::getInstance().getVersion() << "\n";
    return 0;
}
```

Compile (system-wide):
```bash
g++ -std=c++17 -o app main.cpp -pthread
```

Compile (local):
```bash
g++ -std=c++17 -I./lib/cpp-utils/include -o app main.cpp -pthread
```

### Example 2: TaskRunner Only

```cpp
#include <TaskRunner.h>
#include <iostream>
#include <chrono>

int main() {
    TaskRunner runner;

    // Single task
    runner.executeTask([]() {
        std::cout << "Processing data...\n";
    });

    // Repeated task (5 times, 500ms interval)
    runner.executeRepeatedTask(
        []() {
            std::cout << "Heartbeat\n";
        },
        std::chrono::milliseconds(500),
        5
    );

    runner.waitForCompletion();
    return 0;
}
```

Compile (system-wide):
```bash
g++ -std=c++17 -o app main.cpp -ltaskrunner -pthread
```

Compile (local):
```bash
g++ -std=c++17 -I./lib/cpp-utils/include -o app main.cpp \
    -L./lib/cpp-utils/lib -ltaskrunner -pthread
```

### Example 3: Combined Usage

```cpp
#include <Singleton.h>
#include <TaskRunner.h>
#include <iostream>
#include <chrono>

class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>;
private:
    Logger() { std::cout << "[Logger] Initialized\n"; }
public:
    void log(const std::string& msg) {
        std::cout << "[LOG] " << msg << "\n";
    }
};

int main() {
    TaskRunner runner;

    runner.executeTask([]() {
        Logger::getInstance().log("Task started");
    });

    runner.executeRepeatedTask(
        []() {
            Logger::getInstance().log("Periodic check");
        },
        std::chrono::seconds(1),
        3
    );

    runner.waitForCompletion();
    Logger::getInstance().log("All tasks completed");

    return 0;
}
```

Compile (system-wide):
```bash
g++ -std=c++17 -o app main.cpp -ltaskrunner -pthread
```

Compile (local):
```bash
g++ -std=c++17 -I./lib/cpp-utils/include -o app main.cpp \
    -L./lib/cpp-utils/lib -ltaskrunner -pthread
```

---

## Troubleshooting

### Common Issues and Solutions

#### 1. "undefined reference to" errors

**Problem**: Linker cannot find the library.

**Solution**:
- System-wide: Make sure `-ltaskrunner` is in your link flags
- Local: Verify the library path with `-L./lib/cpp-utils/lib`
- Ensure the library is built: `make` in the library directory

#### 2. "No such file or directory" for headers

**Problem**: Compiler cannot find header files.

**Solution**:
- System-wide: Headers should be in `/usr/local/include`
- Local: Add `-I./lib/cpp-utils/include` to your compile flags
- Verify installation: Check if headers exist in the include directory

#### 3. Library not found at runtime (rare for static library)

**Problem**: This is uncommon with static libraries but can occur with dynamic linking.

**Solution**:
```bash
# Check library location
ls -l /usr/local/lib/libtaskrunner.a

# Update library cache (Linux)
sudo ldconfig
```

#### 4. Permission denied during installation

**Problem**: Cannot write to `/usr/local`

**Solution**:
```bash
# Use sudo
sudo make install

# Or install to a custom location
make install PREFIX=$HOME/.local
```

Then compile with:
```bash
g++ -std=c++17 -I$HOME/.local/include -o app main.cpp \
    -L$HOME/.local/lib -ltaskrunner -pthread
```

#### 5. Different C++ standard errors

**Problem**: Compiler errors about C++17 features.

**Solution**:
- Ensure `-std=c++17` is in your compile flags
- Update your compiler if it's too old (GCC 7+, Clang 5+, MSVC 2017+)

#### 6. Thread-related errors

**Problem**: "undefined reference to pthread_create"

**Solution**:
- Add `-pthread` to your compile and link flags
- This is required for both Singleton and TaskRunner

### Verification Commands

Check system-wide installation:
```bash
ls -l /usr/local/lib/libtaskrunner.a
ls -l /usr/local/include/Singleton.h
ls -l /usr/local/include/TaskRunner.h
```

Check local installation:
```bash
ls -l lib/cpp-utils/lib/libtaskrunner.a
ls -l lib/cpp-utils/include/Singleton.h
ls -l lib/cpp-utils/include/TaskRunner.h
```

Test compilation (minimal):
```bash
# System-wide
g++ -std=c++17 -o test test.cpp -ltaskrunner -pthread

# Local
g++ -std=c++17 -I./lib/cpp-utils/include -o test test.cpp \
    -L./lib/cpp-utils/lib -ltaskrunner -pthread
```

### Getting Help

If you encounter issues not covered here:

1. Check that the library is built: `make` in the library directory
2. Verify your compiler supports C++17: `g++ --version`
3. Check compilation flags: ensure `-std=c++17` and `-pthread` are present
4. Review the library's test suite for working examples
5. Consult the main README.md for usage examples
