# C++17 Utilities Library

A collection of thread-safe C++17 utilities including a Singleton pattern implementation and a TaskRunner for asynchronous task execution.

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

## Directory Structure

```
singleton_library/
├── include/
│   ├── Singleton.h      # Singleton template class (header-only)
│   └── TaskRunner.h     # TaskRunner class header
├── taskrunner/
│   └── TaskRunner.cpp   # TaskRunner implementation
├── tests/
│   ├── SingletonTest.cpp    # Unit tests for Singleton
│   └── TaskRunnerTest.cpp   # Unit tests for TaskRunner
├── lib/                 # Static library output (created by make)
├── build/               # Build artifacts (created by make)
├── Makefile
└── README.md
```

## Building

Build the static library:

```bash
make
```

Build and run tests:

```bash
make run-tests
```

Build tests only:

```bash
make tests
```

Clean build artifacts:

```bash
make clean
```

## Usage

### Singleton Usage

To create a singleton class, inherit from `Singleton<YourClass>` and make `Singleton<YourClass>` a friend:

```cpp
#include "Singleton.h"
#include <string>

class MyClass : public Singleton<MyClass> {
    friend class Singleton<MyClass>;

private:
    // Constructor must be private
    MyClass() : data("default") {}

    std::string data;

public:
    void setData(const std::string& newData) {
        data = newData;
    }

    std::string getData() const {
        return data;
    }
};
```

Accessing the singleton instance:

```cpp
int main() {
    // Get the singleton instance
    MyClass& instance1 = MyClass::getInstance();
    instance1.setData("Hello, Singleton!");

    // Get the same instance again
    MyClass& instance2 = MyClass::getInstance();

    // Both references point to the same object
    std::cout << instance2.getData() << std::endl; // Outputs: Hello, Singleton!

    return 0;
}
```

### TaskRunner Usage

The TaskRunner allows you to execute tasks asynchronously in separate threads.

#### Execute a Single Task

```cpp
#include "TaskRunner.h"
#include <iostream>

int main() {
    TaskRunner runner;

    // Execute a lambda
    runner.executeTask([]() {
        std::cout << "Task executed in thread!" << std::endl;
    });

    // Execute a function
    auto myFunction = []() {
        std::cout << "Another task!" << std::endl;
    };
    runner.executeTask(myFunction);

    // Wait for all tasks to complete
    runner.waitForCompletion();

    return 0;
}
```

#### Execute a Repeated Task

```cpp
#include "TaskRunner.h"
#include <iostream>
#include <chrono>

int main() {
    TaskRunner runner;

    // Execute task 5 times with 100ms interval
    runner.executeRepeatedTask(
        []() {
            std::cout << "Repeated task execution" << std::endl;
        },
        std::chrono::milliseconds(100),
        5  // Execute 5 times
    );

    runner.waitForCompletion();

    return 0;
}
```

#### Execute an Infinite Task (with stop)

```cpp
#include "TaskRunner.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    TaskRunner runner;

    // Execute task infinitely with 200ms interval
    runner.executeRepeatedTask(
        []() {
            std::cout << "Infinite task running..." << std::endl;
        },
        std::chrono::milliseconds(200),
        0  // 0 means infinite
    );

    // Let it run for a while
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Stop all tasks
    runner.stopAll();
    runner.waitForCompletion();

    return 0;
}
```

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

## How It Works

### Singleton
1. Uses CRTP (Curiously Recurring Template Pattern)
2. The derived class is passed as a template parameter
3. `std::call_once` ensures thread-safe initialization
4. The constructor is protected, preventing direct instantiation
5. Copy and move operations are deleted
6. `getInstance()` provides the single access point

### TaskRunner
1. Maintains separate thread pools for single and repeated tasks
2. Single tasks are executed once and automatically cleaned up
3. Repeated tasks run in dedicated threads with configurable intervals
4. Uses atomic flags for thread-safe stop signals
5. Mutex protection for task collection management
6. Automatic task deletion after completion

## Thread Safety

Both implementations are fully thread-safe:

**Singleton:**
- Uses `std::call_once` with `std::once_flag` for initialization
- Guarantees single instance creation even in multi-threaded environments

**TaskRunner:**
- Thread-safe task submission with mutex protection
- Atomic flags for stopping repeated tasks
- Safe concurrent task execution
- No race conditions during task cleanup

## Running Tests

The library includes comprehensive unit tests for both components:

```bash
make run-tests
```

**Singleton Tests:**
- Basic singleton functionality
- Thread safety verification
- Multiple singleton types

**TaskRunner Tests:**
- Single task execution with lambdas
- Single task execution with functions
- Multiple concurrent tasks
- Repeated tasks with fixed count
- Infinite tasks with stop functionality
- Mixed single and repeated tasks
- Task capture and state management

## Requirements

- C++17 or later
- Standard library with `<memory>`, `<mutex>`, `<thread>`, and `<chrono>` support
- pthread library (automatically linked with `-pthread`)

## Project Organization

- **include/** - All header files (.h)
- **taskrunner/** - TaskRunner implementation (.cpp)
- **tests/** - Unit tests for each component
- **lib/** - Generated static library
- **build/** - Build artifacts

## License

Free to use for any purpose.
