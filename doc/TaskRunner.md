# TaskRunner Implementation

## Overview

TaskRunner is a thread-safe task execution manager that allows you to run tasks asynchronously. It supports both single-execution tasks and repeated tasks with configurable intervals. Tasks can run a fixed number of times or infinitely until stopped.

## Architecture

### Core Components

```cpp
class TaskRunner {
public:
    using Task = std::function<void()>;

private:
    struct RepeatedTaskData { ... };

    std::mutex tasksMutex;
    std::vector<std::unique_ptr<std::thread>> singleTasks;
    std::vector<std::shared_ptr<RepeatedTaskData>> repeatedTasks;
    std::atomic<bool> shuttingDown;
};
```

**Key Design Elements:**

1. **Task Type Alias**: `std::function<void()>` allows any callable (lambda, function, functor)
2. **Two Task Categories**: Single-execution and repeated tasks managed separately
3. **Thread Safety**: Mutex protection for task collections
4. **Graceful Shutdown**: Atomic flag for coordinated shutdown

## Thread Safety

### Mutex Protection

The `tasksMutex` protects:
- Adding new tasks to collections
- Cleaning up finished tasks
- Stopping all tasks
- Waiting for completion

```cpp
void executeTask(Task task) {
    std::lock_guard<std::mutex> lock(tasksMutex);
    // Add task safely
}
```

### Atomic Shutdown Flag

```cpp
std::atomic<bool> shuttingDown;
```

- Allows multiple threads to check shutdown status without locking
- Used by repeated tasks to check if they should stop
- Set by `stopAll()` and checked in task loops

### Per-Task Stop Flags

```cpp
struct RepeatedTaskData {
    std::atomic<bool> shouldStop;
    // ...
};
```

- Each repeated task has its own atomic stop flag
- Allows selective task stopping (though current API stops all)
- Thread-safe without mutex overhead

## Task Types

### Single-Execution Tasks

Tasks that run once and automatically clean up.

**Characteristics:**
- Execute immediately in a new thread
- Automatically removed when finished
- Lightweight for fire-and-forget operations
- No explicit stop mechanism (completes naturally)

**Implementation:**

```cpp
void executeTask(Task task) {
    std::lock_guard<std::mutex> lock(tasksMutex);

    cleanupFinishedTasks();  // Remove old tasks first

    auto thread = std::make_unique<std::thread>([task = std::move(task)]() {
        task();
    });

    singleTasks.push_back(std::move(thread));
}
```

### Repeated Tasks

Tasks that run multiple times with a configurable interval.

**Characteristics:**
- Run in dedicated thread
- Configurable execution interval
- Can run fixed number of times or infinitely
- Can be stopped explicitly
- Shared ownership via `std::shared_ptr`

**Implementation:**

```cpp
void executeRepeatedTask(Task task, std::chrono::milliseconds interval, size_t count) {
    std::lock_guard<std::mutex> lock(tasksMutex);

    auto data = std::make_shared<RepeatedTaskData>(std::move(task), interval, count);
    data->thread = std::make_unique<std::thread>(&TaskRunner::runRepeatedTask, this, data);

    repeatedTasks.push_back(data);
}
```

## Repeated Task Execution Loop

### Main Loop Logic

```cpp
void runRepeatedTask(std::shared_ptr<RepeatedTaskData> data) {
    size_t executionCount = 0;
    bool infinite = (data->count == 0);

    while (!data->shouldStop && !shuttingDown) {
        if (!infinite && executionCount >= data->count) {
            break;
        }

        data->task();  // Execute the task
        executionCount++;

        if (!infinite && executionCount >= data->count) {
            break;
        }

        // Sleep with periodic stop checks
        auto sleepStart = std::chrono::steady_clock::now();
        while (!data->shouldStop && !shuttingDown) {
            auto elapsed = std::chrono::steady_clock::now() - sleepStart;
            if (elapsed >= data->interval) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
```

### Key Features:

1. **Dual Stop Conditions**: Checks both `shouldStop` and `shuttingDown`
2. **Count-Based or Infinite**: `count == 0` means run forever
3. **Responsive Sleep**: Checks for stop every 10ms during interval
4. **Early Exit**: Stops immediately when `shouldStop` or `shuttingDown` is set

### Why Periodic Sleep Checks?

Instead of:
```cpp
std::this_thread::sleep_for(data->interval);  // Can't interrupt
```

We use:
```cpp
while (!data->shouldStop && !shuttingDown) {
    auto elapsed = std::chrono::steady_clock::now() - sleepStart;
    if (elapsed >= data->interval) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

**Benefits:**
- Task can stop within 10ms instead of waiting full interval
- More responsive shutdown
- Better user experience for long intervals

## Memory Management

### RepeatedTaskData Structure

```cpp
struct RepeatedTaskData {
    Task task;
    std::chrono::milliseconds interval;
    size_t count;
    std::atomic<bool> shouldStop;
    std::unique_ptr<std::thread> thread;

    RepeatedTaskData(Task t, std::chrono::milliseconds i, size_t c)
        : task(std::move(t)), interval(i), count(c), shouldStop(false) {}
};
```

**Ownership Model:**
- `std::shared_ptr<RepeatedTaskData>`: Shared between TaskRunner and task thread
- `std::unique_ptr<std::thread>`: Owned by RepeatedTaskData
- Task thread keeps data alive via shared_ptr
- Data cleaned up when both TaskRunner releases it AND thread finishes

### Thread Ownership

**Single Tasks:**
```cpp
std::vector<std::unique_ptr<std::thread>> singleTasks;
```
- Unique ownership by TaskRunner
- Cleaned up when joined

**Repeated Tasks:**
```cpp
std::vector<std::shared_ptr<RepeatedTaskData>> repeatedTasks;
```
- Shared ownership allows task thread to keep data alive
- Thread inside RepeatedTaskData

## Lifecycle Management

### Construction

```cpp
TaskRunner::TaskRunner() : shuttingDown(false) {}
```

- Initializes atomic flag to false
- No threads created until tasks are submitted

### Destruction

```cpp
TaskRunner::~TaskRunner() {
    stopAll();
    waitForCompletion();
}
```

- Automatically stops all tasks
- Waits for all threads to finish
- Ensures clean shutdown even if user forgets

### Task Cleanup

```cpp
void cleanupFinishedTasks() {
    singleTasks.erase(
        std::remove_if(singleTasks.begin(), singleTasks.end(),
            [](const std::unique_ptr<std::thread>& t) {
                if (t->joinable()) {
                    return false;
                }
                return true;
            }),
        singleTasks.end()
    );
}
```

- Removes non-joinable threads (already finished)
- Called before adding new tasks
- Prevents unbounded memory growth

## API Methods

### executeTask

```cpp
void executeTask(Task task);
```

**Purpose:** Execute a task once in a separate thread

**Thread Safety:** Yes, fully thread-safe

**Parameters:**
- `task`: Any callable object with signature `void()`

**Behavior:**
- Cleans up old finished tasks
- Creates new thread
- Task executes immediately
- Automatic cleanup after completion

**Example:**
```cpp
runner.executeTask([]() {
    std::cout << "Task executed!" << std::endl;
});
```

### executeRepeatedTask

```cpp
void executeRepeatedTask(Task task, std::chrono::milliseconds interval, size_t count = 0);
```

**Purpose:** Execute a task repeatedly with a specified interval

**Thread Safety:** Yes, fully thread-safe

**Parameters:**
- `task`: Callable to execute repeatedly
- `interval`: Time to wait between executions
- `count`: Number of times to execute (0 = infinite)

**Behavior:**
- Creates dedicated thread for this task
- Executes immediately, then waits interval
- Stops after count executions (if count > 0)
- Can be stopped early with `stopAll()`

**Examples:**

Fixed count:
```cpp
runner.executeRepeatedTask(
    []() { std::cout << "Tick" << std::endl; },
    std::chrono::milliseconds(1000),
    5  // Execute 5 times
);
```

Infinite:
```cpp
runner.executeRepeatedTask(
    []() { std::cout << "Tick" << std::endl; },
    std::chrono::milliseconds(1000),
    0  // Run forever
);
```

### stopAll

```cpp
void stopAll();
```

**Purpose:** Signal all repeated tasks to stop

**Thread Safety:** Yes, fully thread-safe

**Behavior:**
- Sets global `shuttingDown` flag
- Sets `shouldStop` flag on all repeated tasks
- Non-blocking (returns immediately)
- Tasks stop within ~10ms of current interval

**Note:** Only affects repeated tasks. Single tasks run to completion.

**Example:**
```cpp
runner.stopAll();
runner.waitForCompletion();  // Wait for actual stop
```

### waitForCompletion

```cpp
void waitForCompletion();
```

**Purpose:** Block until all tasks complete

**Thread Safety:** Yes, fully thread-safe

**Behavior:**
- Joins all single task threads
- Joins all repeated task threads
- Clears all task collections
- Blocks until all threads finish

**Warning:** For infinite repeated tasks, call `stopAll()` first!

**Example:**
```cpp
// Start tasks
runner.executeTask(task1);
runner.executeRepeatedTask(task2, ms(100), 10);

// Wait for all to complete
runner.waitForCompletion();
```

## Usage Patterns

### Pattern 1: Fire-and-Forget Tasks

```cpp
TaskRunner runner;

// Queue multiple tasks
for (int i = 0; i < 10; ++i) {
    runner.executeTask([i]() {
        processData(i);
    });
}

// Wait for all to finish
runner.waitForCompletion();
```

### Pattern 2: Periodic Operations

```cpp
TaskRunner runner;

// Check for updates every second, 60 times (1 minute)
runner.executeRepeatedTask(
    []() {
        checkForUpdates();
    },
    std::chrono::seconds(1),
    60
);

runner.waitForCompletion();
```

### Pattern 3: Background Service

```cpp
TaskRunner runner;

// Start background service
runner.executeRepeatedTask(
    []() {
        processQueue();
    },
    std::chrono::milliseconds(100),
    0  // Infinite
);

// Do other work...
doMainWork();

// Shutdown when done
runner.stopAll();
runner.waitForCompletion();
```

### Pattern 4: Mixed Tasks

```cpp
TaskRunner runner;

// One-time initialization
runner.executeTask([]() {
    initializeResources();
});

// Periodic monitoring
runner.executeRepeatedTask(
    []() { monitorHealth(); },
    std::chrono::seconds(5),
    0
);

// One-time cleanup later
runner.executeTask([]() {
    cleanupTemporaryFiles();
});

// Eventually shutdown
runner.stopAll();
runner.waitForCompletion();
```

### Pattern 5: Captured State

```cpp
TaskRunner runner;
std::atomic<int> sharedCounter{0};
std::string message = "Hello";

runner.executeRepeatedTask(
    [&sharedCounter, message]() {
        std::cout << message << " " << sharedCounter++ << std::endl;
    },
    std::chrono::milliseconds(500),
    10
);

runner.waitForCompletion();
```

**Important:** Use atomic types or mutexes for shared state!

## Best Practices

### 1. Use Atomic Types for Shared State

```cpp
// WRONG
int counter = 0;
runner.executeTask([&counter]() { counter++; });  // Race condition!

// CORRECT
std::atomic<int> counter{0};
runner.executeTask([&counter]() { counter++; });  // Thread-safe
```

### 2. Call stopAll() Before Waiting on Infinite Tasks

```cpp
// WRONG - hangs forever
runner.executeRepeatedTask(task, ms(100), 0);
runner.waitForCompletion();  // Never returns!

// CORRECT
runner.executeRepeatedTask(task, ms(100), 0);
// ... do work ...
runner.stopAll();
runner.waitForCompletion();  // Returns after tasks stop
```

### 3. Keep Tasks Short

```cpp
// WRONG - blocks for 10 seconds before checking stop
runner.executeRepeatedTask([]() {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    doWork();
}, ms(100), 0);

// CORRECT - responsive to stop signals
runner.executeRepeatedTask([]() {
    doWork();  // Keep task body short
}, ms(100), 0);
```

### 4. Handle Exceptions in Tasks

```cpp
runner.executeTask([]() {
    try {
        riskyOperation();
    } catch (const std::exception& e) {
        std::cerr << "Task error: " << e.what() << std::endl;
    }
});
```

**Note:** Uncaught exceptions in tasks will terminate the program!

### 5. Automatic Cleanup with RAII

```cpp
void processData() {
    TaskRunner runner;

    runner.executeTask([]() { /* work */ });
    runner.executeRepeatedTask([]() { /* work */ }, ms(100), 10);

    // Automatic cleanup when runner goes out of scope
}  // Destructor calls stopAll() and waitForCompletion()
```

## Performance Considerations

### Thread Creation Overhead

- Each task creates a new thread (~few microseconds)
- For very frequent tasks, consider using a thread pool instead
- Single tasks: One thread per `executeTask()` call
- Repeated tasks: One dedicated thread per `executeRepeatedTask()` call

### Memory Usage

- Per single task: ~8KB (thread stack) + task closure size
- Per repeated task: ~8KB + RepeatedTaskData struct + task closure
- Finished single tasks cleaned up on next task submission
- Repeated tasks cleaned up in `waitForCompletion()`

### Timing Accuracy

- Repeated task interval: ±10ms accuracy (due to stop check frequency)
- For high-precision timing, consider using `std::chrono::high_resolution_clock`
- System scheduler may introduce additional jitter

### Scalability

**Good for:**
- Dozens of concurrent tasks
- Long-running background operations
- Periodic low-frequency tasks

**Consider alternatives for:**
- Thousands of concurrent tasks (use thread pool)
- High-frequency tasks (>100 Hz)
- CPU-intensive parallel computing (use std::async or thread pool)

## Common Pitfalls

### 1. Forgetting to Wait

```cpp
{
    TaskRunner runner;
    runner.executeTask(task);
}  // Task might not finish before destructor!
```

Better:
```cpp
{
    TaskRunner runner;
    runner.executeTask(task);
    runner.waitForCompletion();
}  // Guaranteed to finish
```

### 2. Infinite Tasks Without stopAll

```cpp
TaskRunner runner;
runner.executeRepeatedTask(task, ms(100), 0);
runner.waitForCompletion();  // HANGS FOREVER!
```

### 3. Data Races

```cpp
std::vector<int> data;
runner.executeTask([&data]() { data.push_back(1); });
runner.executeTask([&data]() { data.push_back(2); });  // RACE CONDITION!
```

Use mutex or atomic types!

### 4. Dangling References

```cpp
void bad() {
    std::string message = "Hello";
    runner.executeRepeatedTask([&message]() {  // Captures by reference
        std::cout << message << std::endl;
    }, ms(100), 0);
}  // message destroyed, but task still running!
```

Use capture by value or ensure lifetime:
```cpp
runner.executeRepeatedTask([message = std::string("Hello")]() {
    std::cout << message << std::endl;
}, ms(100), 0);
```

## Testing Considerations

### Synchronization Points

```cpp
TEST(TaskRunner, BasicExecution) {
    TaskRunner runner;
    std::atomic<bool> executed{false};

    runner.executeTask([&executed]() {
        executed = true;
    });

    runner.waitForCompletion();
    ASSERT_TRUE(executed);
}
```

### Counting Executions

```cpp
TEST(TaskRunner, RepeatedTask) {
    TaskRunner runner;
    std::atomic<int> count{0};

    runner.executeRepeatedTask(
        [&count]() { count++; },
        std::chrono::milliseconds(10),
        5
    );

    runner.waitForCompletion();
    ASSERT_EQ(count, 5);
}
```

### Testing Stop Functionality

```cpp
TEST(TaskRunner, StopInfiniteTask) {
    TaskRunner runner;
    std::atomic<int> count{0};

    runner.executeRepeatedTask(
        [&count]() { count++; },
        std::chrono::milliseconds(10),
        0  // Infinite
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    runner.stopAll();

    int countAtStop = count.load();
    runner.waitForCompletion();

    ASSERT_EQ(count, countAtStop);  // No more increments
    ASSERT_GT(count, 0);            // Executed at least once
}
```

## Technical Specifications

- **C++ Standard**: C++17 or later
- **Header Dependencies**: `<functional>`, `<thread>`, `<mutex>`, `<chrono>`, `<atomic>`, `<memory>`, `<queue>`, `<condition_variable>`
- **Library**: Requires linking against pthread (`-pthread`)
- **Thread Safety**: Full thread safety for all public methods
- **Exception Safety**: Basic guarantee (stops on exception in task)
- **Maximum Tasks**: Limited by system thread limits (~thousands)
