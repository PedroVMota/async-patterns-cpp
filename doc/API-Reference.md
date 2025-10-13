# API Reference

Complete API documentation for the C++17 Utilities Library.

## Table of Contents

1. [Singleton](#singleton)
2. [TaskRunner](#taskrunner)

---

## Singleton

### Template Class Definition

```cpp
template <typename T>
class Singleton
```

**Header:** `<Singleton.h>`

**Description:** Thread-safe singleton base class using CRTP pattern.

### Public Methods

#### getInstance

```cpp
static T& getInstance()
```

**Description:** Returns a reference to the single instance of type `T`.

**Thread Safety:** Yes - uses `std::call_once` for thread-safe initialization

**Returns:** Reference to the singleton instance

**Time Complexity:** O(1) after first initialization

**Example:**
```cpp
MyClass& instance = MyClass::getInstance();
```

**Notes:**
- First call initializes the instance
- Subsequent calls return the same instance
- Instance created via private constructor
- Never returns null

### Protected Methods

#### Constructor

```cpp
protected:
    Singleton() = default
```

**Description:** Default protected constructor

**Access:** Protected - only accessible to derived classes

#### Destructor

```cpp
protected:
    virtual ~Singleton() = default
```

**Description:** Virtual destructor for proper cleanup

**Access:** Protected

### Deleted Methods

```cpp
Singleton(const Singleton&) = delete;
Singleton& operator=(const Singleton&) = delete;
Singleton(Singleton&&) = delete;
Singleton& operator=(Singleton&&) = delete;
```

**Description:** Copy and move operations are explicitly deleted

**Rationale:** Prevents copying or moving singleton instances

### Static Members

```cpp
static std::unique_ptr<T> instance;
static std::once_flag initFlag;
```

**Description:** Static members for instance storage and initialization

**Note:** Automatically initialized per template instantiation

### Usage Requirements

To use Singleton, a derived class must:

1. Inherit from `Singleton<DerivedClass>`
2. Declare `Singleton<DerivedClass>` as a friend
3. Have a private constructor

**Example:**
```cpp
class MyClass : public Singleton<MyClass> {
    friend class Singleton<MyClass>;
private:
    MyClass() { /* initialization */ }
public:
    void doSomething() { }
};
```

---

## TaskRunner

### Class Definition

```cpp
class TaskRunner
```

**Header:** `<TaskRunner.h>`

**Link:** `-ltaskrunner -pthread`

**Description:** Thread-safe asynchronous task execution manager.

### Type Aliases

#### Task

```cpp
using Task = std::function<void()>
```

**Description:** Type alias for callable tasks

**Requirements:** Any callable with signature `void()`

**Examples:**
- Lambda: `[]() { /* code */ }`
- Function: `void myFunc() { }`
- Functor: `struct F { void operator()() { } }`

### Public Methods

#### Constructor

```cpp
TaskRunner()
```

**Description:** Creates a new TaskRunner instance

**Thread Safety:** N/A

**Exceptions:** None

**Example:**
```cpp
TaskRunner runner;
```

#### Destructor

```cpp
~TaskRunner()
```

**Description:** Destroys TaskRunner, stopping and waiting for all tasks

**Behavior:**
- Calls `stopAll()`
- Calls `waitForCompletion()`
- Blocks until all tasks finish

**Thread Safety:** Yes

**Example:**
```cpp
{
    TaskRunner runner;
    runner.executeTask(task);
}  // Automatic cleanup
```

#### executeTask

```cpp
void executeTask(Task task)
```

**Description:** Execute a task once in a separate thread

**Parameters:**
- `task` - Callable object to execute

**Thread Safety:** Yes

**Behavior:**
- Creates new thread immediately
- Task executes asynchronously
- Automatic cleanup after completion

**Time Complexity:** O(1) amortized (may cleanup old tasks)

**Example:**
```cpp
runner.executeTask([]() {
    std::cout << "Hello from task!" << std::endl;
});
```

**Notes:**
- Task must be copyable or movable
- Exceptions in task will terminate program
- Thread created immediately (not pooled)

#### executeRepeatedTask

```cpp
void executeRepeatedTask(
    Task task,
    std::chrono::milliseconds interval,
    size_t count = 0
)
```

**Description:** Execute a task repeatedly with specified interval

**Parameters:**
- `task` - Callable object to execute
- `interval` - Time to wait between executions
- `count` - Number of times to execute (0 = infinite)

**Thread Safety:** Yes

**Behavior:**
- Creates dedicated thread for this task
- Executes immediately, then waits interval
- Stops after count executions (if count > 0)
- Checks for stop signal every 10ms during interval

**Time Complexity:** O(1)

**Example:**
```cpp
// Fixed count
runner.executeRepeatedTask(
    []() { std::cout << "Tick" << std::endl; },
    std::chrono::milliseconds(1000),
    5
);

// Infinite
runner.executeRepeatedTask(
    []() { std::cout << "Tick" << std::endl; },
    std::chrono::milliseconds(1000),
    0
);
```

**Notes:**
- Interval accuracy: ±10ms
- Stop response time: ~10ms
- Use `stopAll()` to stop infinite tasks

#### stopAll

```cpp
void stopAll()
```

**Description:** Signal all repeated tasks to stop

**Thread Safety:** Yes

**Behavior:**
- Sets shutdown flag
- Signals all repeated tasks to stop
- Returns immediately (non-blocking)
- Tasks stop within ~10ms

**Time Complexity:** O(n) where n = number of repeated tasks

**Example:**
```cpp
runner.executeRepeatedTask(task, ms(100), 0);
// ... later ...
runner.stopAll();
runner.waitForCompletion();
```

**Notes:**
- Does not affect single-execution tasks
- Does not wait for tasks to actually stop
- Safe to call multiple times

#### waitForCompletion

```cpp
void waitForCompletion()
```

**Description:** Block until all tasks complete

**Thread Safety:** Yes

**Behavior:**
- Joins all single task threads
- Joins all repeated task threads
- Clears all task collections
- Blocks until all threads finish

**Time Complexity:** O(n) where n = total number of tasks

**Example:**
```cpp
runner.executeTask(task1);
runner.executeRepeatedTask(task2, ms(100), 10);
runner.waitForCompletion();  // Blocks until both finish
```

**Notes:**
- Required before accessing task results
- Automatically called by destructor
- For infinite tasks, call `stopAll()` first
- Safe to call multiple times

### Deleted Methods

```cpp
TaskRunner(const TaskRunner&) = delete;
TaskRunner& operator=(const TaskRunner&) = delete;
TaskRunner(TaskRunner&&) = delete;
TaskRunner& operator=(TaskRunner&&) = delete;
```

**Description:** Copy and move operations are explicitly deleted

**Rationale:** TaskRunner manages threads which cannot be safely copied or moved

### Private Methods

#### runRepeatedTask

```cpp
void runRepeatedTask(std::shared_ptr<RepeatedTaskData> data)
```

**Description:** Internal method that executes repeated task loop

**Access:** Private

**Note:** Called by repeated task threads, not directly by users

#### cleanupFinishedTasks

```cpp
void cleanupFinishedTasks()
```

**Description:** Removes finished single tasks from collection

**Access:** Private

**Note:** Called automatically by `executeTask()`

### Private Members

#### tasksMutex

```cpp
std::mutex tasksMutex
```

**Description:** Protects task collections from concurrent access

#### singleTasks

```cpp
std::vector<std::unique_ptr<std::thread>> singleTasks
```

**Description:** Collection of single-execution task threads

#### repeatedTasks

```cpp
std::vector<std::shared_ptr<RepeatedTaskData>> repeatedTasks
```

**Description:** Collection of repeated task data

#### shuttingDown

```cpp
std::atomic<bool> shuttingDown
```

**Description:** Flag indicating TaskRunner is shutting down

### RepeatedTaskData Structure

```cpp
struct RepeatedTaskData {
    Task task;
    std::chrono::milliseconds interval;
    size_t count;
    std::atomic<bool> shouldStop;
    std::unique_ptr<std::thread> thread;

    RepeatedTaskData(Task t, std::chrono::milliseconds i, size_t c);
}
```

**Description:** Internal structure holding repeated task information

**Access:** Private

**Members:**
- `task` - The callable to execute
- `interval` - Time between executions
- `count` - Number of times to execute (0 = infinite)
- `shouldStop` - Atomic flag for stopping this task
- `thread` - Thread executing this task

---

## Common Types

### Duration Types

The library uses `std::chrono` duration types:

```cpp
#include <chrono>

using namespace std::chrono;

milliseconds ms(100);           // 100 milliseconds
seconds sec(5);                 // 5 seconds
minutes min(2);                 // 2 minutes
```

**Supported by:** `TaskRunner::executeRepeatedTask()`

**Example:**
```cpp
runner.executeRepeatedTask(task, std::chrono::milliseconds(500), 10);
runner.executeRepeatedTask(task, std::chrono::seconds(1), 10);
```

---

## Error Handling

### Singleton

**Compilation Errors:**

1. **Missing friend declaration:**
   ```cpp
   error: 'MyClass::MyClass()' is private
   ```
   **Solution:** Add `friend class Singleton<MyClass>;`

2. **Public constructor:**
   ```cpp
   // Compiles but defeats purpose
   ```
   **Solution:** Make constructor private

**Runtime:**
- No runtime errors (initialization is guaranteed)

### TaskRunner

**Compilation Errors:**

1. **Non-callable task:**
   ```cpp
   error: no matching function for call to 'std::function<void()>::function(int)'
   ```
   **Solution:** Task must have signature `void()`

**Runtime:**

1. **Uncaught exception in task:**
   ```cpp
   std::terminate called after throwing an instance of 'std::exception'
   ```
   **Solution:** Catch exceptions within tasks:
   ```cpp
   runner.executeTask([]() {
       try {
           riskyOperation();
       } catch (...) {
           handleError();
       }
   });
   ```

2. **Deadlock in waitForCompletion:**
   - Symptom: Program hangs
   - Cause: Infinite repeated task without `stopAll()`
   - Solution: Call `stopAll()` before `waitForCompletion()`

---

## Thread Safety Summary

| Method | Thread Safety | Notes |
|--------|---------------|-------|
| `Singleton::getInstance()` | ✓ Yes | Uses `std::call_once` |
| `TaskRunner::executeTask()` | ✓ Yes | Mutex protected |
| `TaskRunner::executeRepeatedTask()` | ✓ Yes | Mutex protected |
| `TaskRunner::stopAll()` | ✓ Yes | Atomic operations |
| `TaskRunner::waitForCompletion()` | ✓ Yes | Mutex protected |
| Task execution | ⚠ User responsibility | Protect shared state |

---

## Performance Characteristics

### Singleton

| Operation | Time | Space |
|-----------|------|-------|
| First `getInstance()` | O(1) + construction | O(1) |
| Subsequent `getInstance()` | O(1) | O(1) |
| Memory per type | - | sizeof(T) + pointer |

### TaskRunner

| Operation | Time | Space |
|-----------|------|-------|
| `executeTask()` | O(1) amortized | O(1) + thread stack |
| `executeRepeatedTask()` | O(1) | O(1) + thread stack |
| `stopAll()` | O(n) | O(1) |
| `waitForCompletion()` | O(n) | O(1) |

Where n = number of tasks

**Thread Overhead:**
- Per thread: ~8KB stack (platform dependent)
- Thread creation: ~few microseconds

---

## Version Information

- **Library Version:** 1.0
- **C++ Standard:** C++17 minimum
- **Tested Platforms:** Linux, macOS
- **Compiler Support:** GCC 7+, Clang 5+, MSVC 2017+

---

## Dependencies

### Singleton

**Headers:**
- `<memory>`
- `<mutex>`

**Libraries:** None (header-only)

### TaskRunner

**Headers:**
- `<functional>`
- `<thread>`
- `<mutex>`
- `<chrono>`
- `<atomic>`
- `<memory>`
- `<queue>`
- `<condition_variable>`

**Libraries:**
- pthread (automatic with `-pthread`)
- libtaskrunner.a (static library)

---

## Compilation Flags

### For Singleton (Header-Only)

```bash
g++ -std=c++17 -pthread your_code.cpp -o your_program
```

### For TaskRunner (With Library)

```bash
g++ -std=c++17 -I./include your_code.cpp \
    -L./lib -ltaskrunner -pthread -o your_program
```

### Recommended Warnings

```bash
g++ -std=c++17 -Wall -Wextra -pthread ...
```
