# C++17 Utilities Library

A collection of thread-safe C++17 utilities including a Singleton pattern implementation and a TaskRunner for asynchronous task execution.

## Quick Links

- [Detailed Documentation](#documentation)
- [Quick Start](#quick-start)
- [Building](#building)
- [Usage Examples](#usage)

## Features

### Singleton
- Thread-safe singleton implementation using `std::call_once`
- Header-only (no compilation required)
- CRTP (Curiously Recurring Template Pattern) design
- Prevents copying and moving
- Lazy initialization

### TaskRunner
- Thread-safe task execution
- Support for functions and lambdas
- Single task execution in separate threads
- Repeated task execution with configurable interval and count
- Infinite task execution with stop capability
- Automatic task cleanup after execution

## Documentation

Comprehensive documentation is available in the `doc/` folder:

- **[Singleton.md](doc/Singleton.md)** - Deep dive into Singleton pattern implementation
  - Design patterns and architecture
  - Thread safety mechanisms
  - Memory management details
  - Best practices and common pitfalls

- **[TaskRunner.md](doc/TaskRunner.md)** - Complete TaskRunner guide
  - Architecture and core components
  - Task execution lifecycle
  - Thread management and safety
  - Performance considerations

- **[API-Reference.md](doc/API-Reference.md)** - Full API documentation
  - Complete method signatures
  - Parameters and return values
  - Thread safety guarantees
  - Error handling

- **[Examples.md](doc/Examples.md)** - Practical usage examples
  - Real-world use cases
  - Design patterns
  - Combined usage scenarios
  - Complete working code

## Quick Start

### Singleton

```cpp
#include "Singleton.h"

class MyClass : public Singleton<MyClass> {
    friend class Singleton<MyClass>;
private:
    MyClass() { /* initialization */ }
public:
    void doSomething() { /* ... */ }
};

// Usage
MyClass::getInstance().doSomething();
```

### TaskRunner

```cpp
#include "TaskRunner.h"

TaskRunner runner;

// Single task
runner.executeTask([]() {
    std::cout << "Task executed!" << std::endl;
});

// Repeated task (5 times, every 100ms)
runner.executeRepeatedTask(
    []() { std::cout << "Tick" << std::endl; },
    std::chrono::milliseconds(100),
    5
);

runner.waitForCompletion();
```

## Directory Structure

```
singleton_library/
├── include/
│   ├── Singleton.h      # Singleton template class (header-only)
│   └── TaskRunner.h     # TaskRunner class header
├── taskrunner/
│   └── TaskRunner.cpp   # TaskRunner implementation
├── tests/
│   ├── SingletonTest.cpp    # Unit tests for Singleton (Google Test)
│   └── TaskRunnerTest.cpp   # Unit tests for TaskRunner (Google Test)
├── scripts/
│   └── setup-google-unit-test.sh  # Google Test setup script
├── doc/                 # Detailed documentation
│   ├── Singleton.md     # Singleton pattern deep dive
│   ├── TaskRunner.md    # TaskRunner comprehensive guide
│   ├── API-Reference.md # Complete API documentation
│   └── Examples.md      # Practical usage examples
├── .deps/               # Dependencies (created by build, not in git)
│   └── googletest/      # Google Test framework (auto-downloaded)
├── lib/                 # Static library output (created by make)
├── build/               # Build artifacts (created by make)
├── Makefile             # Main build orchestration
├── Makefile.gtest       # Google Test build and test execution
├── Makefile.lib         # Library compilation and installation
└── README.md
```

## Building

The project uses a modular Makefile system with Google Test for testing. The build process automatically:
1. Builds the library
2. Downloads and sets up Google Test (if not already present)
3. Runs all tests
4. Only completes if all tests pass

### Build Commands

Build the library (runs tests first):

```bash
make
```

Install locally (in `build/install/`):

```bash
make install-local
```

Install system-wide (requires sudo):

```bash
sudo make install
```

Clean build artifacts:

```bash
make clean
```

Clean everything including Google Test:

```bash
make clean-all
```

## Usage

For complete usage examples, see [Examples.md](doc/Examples.md).

### Basic Singleton Usage

```cpp
#include "Singleton.h"

class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>;
private:
    Logger() { /* initialize */ }
public:
    void log(const std::string& msg) {
        std::cout << "[LOG] " << msg << std::endl;
    }
};

// Usage anywhere in your code
Logger::getInstance().log("Application started");
```

### Basic TaskRunner Usage

```cpp
#include "TaskRunner.h"

TaskRunner runner;

// Single task
runner.executeTask([]() {
    processData();
});

// Repeated task (10 times, every 500ms)
runner.executeRepeatedTask(
    []() { checkStatus(); },
    std::chrono::milliseconds(500),
    10
);

// Wait for completion
runner.waitForCompletion();
```

For more examples including:
- Logger, Config, and Database Connection singletons
- Parallel file processing
- Background services
- Event systems
- And much more...

See the [Examples.md](doc/Examples.md) documentation.

### Compiling Your Project

For Singleton only (header-only):

```bash
g++ -std=c++17 -I./singleton_library/include your_main.cpp -o your_program -pthread
```

For TaskRunner (requires linking):

```bash
g++ -std=c++17 -I./singleton_library/include your_main.cpp \
    -L./singleton_library/lib -ltaskrunner -o your_program -pthread
```

## Key Concepts

### Singleton Pattern
- **CRTP Design**: Uses Curiously Recurring Template Pattern for type-safe inheritance
- **Thread-Safe**: `std::call_once` guarantees safe initialization in multi-threaded environments
- **Lazy Initialization**: Instance created only when first accessed
- **Non-Copyable**: Copy and move operations explicitly deleted

For detailed information, see [Singleton.md](doc/Singleton.md).

### TaskRunner Architecture
- **Dual Task Types**: Single-execution and repeated tasks managed separately
- **Thread Per Task**: Each task runs in its own thread for true parallelism
- **Responsive Stopping**: Repeated tasks can be stopped with ~10ms response time
- **Automatic Cleanup**: RAII design ensures proper resource cleanup

For detailed information, see [TaskRunner.md](doc/TaskRunner.md).

## Thread Safety

Both components are fully thread-safe. See [API-Reference.md](doc/API-Reference.md#thread-safety-summary) for detailed thread safety guarantees.

**Important**: While the library infrastructure is thread-safe, you must protect shared state accessed within tasks using mutexes or atomic types.

## Testing

The project uses **Google Test** framework for comprehensive unit testing. Tests are automatically run during the build process.

Run tests manually:

```bash
make -f Makefile.gtest run-tests
```

**Test Framework:**
- Uses Google Test (downloaded automatically on first build)
- Located in `.deps/googletest/` (not committed to repo)
- All tests must pass before library is built

**Test Coverage:**
- **Singleton Tests:** Basic functionality, thread safety, multiple singleton types
- **TaskRunner Tests:** Single tasks, repeated tasks, task stopping, mixed scenarios

See test files in `tests/` for examples of proper usage with Google Test.

## Requirements

- **C++ Standard**: C++17 or later
- **Compiler**: GCC 7+, Clang 5+, MSVC 2017+
- **Platform**: Linux, macOS, Windows (with pthread support)
- **Build Tools**: make, cmake (for Google Test)
- **Dependencies**:
  - Standard library
  - pthread
  - Google Test (downloaded automatically during build)
  - git (for downloading Google Test)

## Contributing

This is a simple utility library. Feel free to use, modify, and extend it for your needs.

## Additional Resources

- [Singleton Deep Dive](doc/Singleton.md) - Design patterns, thread safety, best practices
- [TaskRunner Guide](doc/TaskRunner.md) - Architecture, lifecycle, performance
- [API Reference](doc/API-Reference.md) - Complete method documentation
- [Examples](doc/Examples.md) - Real-world usage patterns

## License

Free to use for any purpose.
