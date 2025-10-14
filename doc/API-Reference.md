# API Reference

Complete API documentation for the C++17 Utilities Library.

## Table of Contents

1. [Singleton](#singleton)
2. [TaskRunner](#taskrunner)
3. [MemPool](#mempool)

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

## MemPool

### Class Definition

```cpp
class MemPool
```

**Header:** `<MemPool.h>`

**Link:** `-ltaskrunner -pthread`

**Description:** Memory pool allocator with tracking capabilities.

### Public Methods

#### Constructor

```cpp
MemPool()
```

**Description:** Creates a new empty memory pool

**Thread Safety:** N/A

**Example:**
```cpp
MemPool pool;
```

#### Destructor

```cpp
~MemPool()
```

**Description:** Destroys the memory pool

**Warning:** Does NOT automatically free allocated memory. Call `reset()` or `deallocate()` manually.

**Thread Safety:** No

#### allocate

```cpp
void* allocate(size_t size)
```

**Description:** Allocates memory and tracks it in the pool

**Parameters:**
- `size` - Number of bytes to allocate

**Returns:** Pointer to allocated memory

**Thread Safety:** No

**Time Complexity:** O(log n) where n = number of tracked allocations

**Example:**
```cpp
void* ptr = pool.allocate(1024);
```

**Notes:**
- Memory is allocated using `new char[size]`
- Pointer is automatically tracked
- Memory is aligned for any object type

#### deallocate

```cpp
void deallocate(void* ptr)
```

**Description:** Deallocates a previously allocated pointer

**Parameters:**
- `ptr` - Pointer to deallocate (can be nullptr)

**Thread Safety:** No

**Time Complexity:** O(log n)

**Example:**
```cpp
pool.deallocate(ptr);
```

**Notes:**
- Safe to call with nullptr (no-op)
- Safe to call twice on same pointer (no-op)
- Safe to call with untracked pointer (no-op)
- Only deallocates if pointer is tracked by this pool

#### reset

```cpp
void reset()
```

**Description:** Deallocates all tracked memory and clears the pool

**Thread Safety:** No

**Time Complexity:** O(n) where n = number of tracked allocations

**Example:**
```cpp
pool.reset(); // Free all allocations
```

**Notes:**
- All tracked pointers are deallocated
- Pool size becomes 0
- Should be called before destructor to avoid leaks

#### isAllocated

```cpp
bool isAllocated(void* ptr)
```

**Description:** Check if a pointer is tracked by this pool

**Parameters:**
- `ptr` - Pointer to check

**Returns:** `true` if pointer is tracked, `false` otherwise

**Thread Safety:** No

**Time Complexity:** O(n)

**Example:**
```cpp
if (pool.isAllocated(ptr)) {
    std::cout << "Pointer is valid\n";
}
```

**Notes:**
- Returns `false` after pointer is deallocated
- Returns `false` for pointers not allocated by this pool

#### getSize

```cpp
int getSize() const
```

**Description:** Returns the number of currently tracked allocations

**Returns:** Number of active allocations

**Thread Safety:** No

**Time Complexity:** O(1)

**Example:**
```cpp
std::cout << "Active: " << pool.getSize() << "\n";
```

### Deleted Methods

```cpp
MemPool(const MemPool&) = delete;
MemPool& operator=(const MemPool&) = delete;
MemPool(MemPool&&) = delete;
MemPool& operator=(MemPool&&) = delete;
```

**Description:** Copy and move operations are explicitly deleted

**Rationale:** Memory pool manages tracked allocations that cannot be safely copied or moved

### Private Members

```cpp
std::set<uintptr_t> _serialized_memory
```

**Description:** Stores integer representations of tracked pointers for O(log n) lookup

### Usage Example

```cpp
MemPool pool;

// Allocate
int* data = static_cast<int*>(pool.allocate(10 * sizeof(int)));

// Use
for (int i = 0; i < 10; ++i) {
    data[i] = i;
}

// Verify
if (pool.isAllocated(data)) {
    std::cout << "Memory tracked\n";
}

// Cleanup
pool.deallocate(data);
// or
pool.reset(); // Cleanup all
```

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
| `MemPool::allocate()` | ✗ No | Use thread-local or external mutex |
| `MemPool::deallocate()` | ✗ No | Use thread-local or external mutex |
| `MemPool::reset()` | ✗ No | Use thread-local or external mutex |
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

### MemPool

| Operation | Time | Space |
|-----------|------|-------|
| `allocate()` | O(log n) | ~32-48 bytes overhead per allocation |
| `deallocate()` | O(log n) | O(1) |
| `isAllocated()` | O(n) | O(1) |
| `reset()` | O(n) | O(1) |
| `getSize()` | O(1) | O(1) |

Where n = number of tracked allocations

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

### MemPool

**Headers:**
- `<cstdint>`
- `<iostream>`
- `<set>`

**Libraries:**
- libtaskrunner.a (static library, includes MemPool)

---

## Compilation Flags

### For Singleton (Header-Only)

```bash
g++ -std=c++17 -pthread your_code.cpp -o your_program
```

### For TaskRunner or MemPool (With Library)

```bash
g++ -std=c++17 -I./include your_code.cpp \
    -L./lib -ltaskrunner -pthread -o your_program
```

**Note:** The `libtaskrunner.a` library includes both TaskRunner and MemPool implementations.

### Recommended Warnings

```bash
g++ -std=c++17 -Wall -Wextra -pthread ...
```
