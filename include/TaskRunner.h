#ifndef TASKRUNNER_H
#define TASKRUNNER_H

#include <functional>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <memory>
#include <queue>
#include <condition_variable>

class TaskRunner {
public:
    using Task = std::function<void()>;

    TaskRunner();
    ~TaskRunner();

    // Delete copy and move
    TaskRunner(const TaskRunner&) = delete;
    TaskRunner& operator=(const TaskRunner&) = delete;
    TaskRunner(TaskRunner&&) = delete;
    TaskRunner& operator=(TaskRunner&&) = delete;

    // Execute a task once in a separate thread
    void executeTask(Task task);

    // Execute a task repeatedly with interval and count
    // If count is 0, runs infinitely
    void executeRepeatedTask(Task task, std::chrono::milliseconds interval, size_t count = 0);

    // Stop all repeated tasks
    void stopAll();

    // Wait for all tasks to complete
    void waitForCompletion();

private:
    struct RepeatedTaskData {
        Task task;
        std::chrono::milliseconds interval;
        size_t count;
        std::atomic<bool> shouldStop;
        std::unique_ptr<std::thread> thread;

        RepeatedTaskData(Task t, std::chrono::milliseconds i, size_t c)
            : task(std::move(t)), interval(i), count(c), shouldStop(false) {}
    };

    void runRepeatedTask(std::shared_ptr<RepeatedTaskData> data);
    void cleanupFinishedTasks();

    std::mutex tasksMutex;
    std::vector<std::unique_ptr<std::thread>> singleTasks;
    std::vector<std::shared_ptr<RepeatedTaskData>> repeatedTasks;
    std::atomic<bool> shuttingDown;
};

#endif // TASKRUNNER_H
