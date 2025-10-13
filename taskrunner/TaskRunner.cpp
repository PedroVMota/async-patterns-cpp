#include "../include/TaskRunner.h"
#include <algorithm>

TaskRunner::TaskRunner() : shuttingDown(false) {}

TaskRunner::~TaskRunner() {
    stopAll();
    waitForCompletion();
}

void TaskRunner::executeTask(Task task) {
    std::lock_guard<std::mutex> lock(tasksMutex);

    // Clean up finished tasks before adding new one
    cleanupFinishedTasks();

    auto thread = std::make_unique<std::thread>([task = std::move(task)]() {
        task();
    });

    singleTasks.push_back(std::move(thread));
}

void TaskRunner::executeRepeatedTask(Task task, std::chrono::milliseconds interval, size_t count) {
    std::lock_guard<std::mutex> lock(tasksMutex);

    auto data = std::make_shared<RepeatedTaskData>(std::move(task), interval, count);

    data->thread = std::make_unique<std::thread>(&TaskRunner::runRepeatedTask, this, data);

    repeatedTasks.push_back(data);
}

void TaskRunner::runRepeatedTask(std::shared_ptr<RepeatedTaskData> data) {
    size_t executionCount = 0;
    bool infinite = (data->count == 0);

    while (!data->shouldStop && !shuttingDown) {
        if (!infinite && executionCount >= data->count) {
            break;
        }

        // Execute the task
        data->task();

        executionCount++;

        // Check if we should continue
        if (!infinite && executionCount >= data->count) {
            break;
        }

        // Sleep for the interval, checking periodically for stop signal
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

void TaskRunner::stopAll() {
    shuttingDown = true;

    std::lock_guard<std::mutex> lock(tasksMutex);

    // Stop all repeated tasks
    for (auto& data : repeatedTasks) {
        data->shouldStop = true;
    }
}

void TaskRunner::waitForCompletion() {
    std::unique_lock<std::mutex> lock(tasksMutex);

    // Wait for all single tasks
    for (auto& thread : singleTasks) {
        if (thread && thread->joinable()) {
            lock.unlock();
            thread->join();
            lock.lock();
        }
    }
    singleTasks.clear();

    // Wait for all repeated tasks
    for (auto& data : repeatedTasks) {
        if (data->thread && data->thread->joinable()) {
            lock.unlock();
            data->thread->join();
            lock.lock();
        }
    }
    repeatedTasks.clear();
}

void TaskRunner::cleanupFinishedTasks() {
    // Remove threads that have finished execution
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

    // Join and remove finished single tasks
    for (auto it = singleTasks.begin(); it != singleTasks.end();) {
        // Check if thread is still running (this is a simple check)
        // In a more sophisticated implementation, you might want to track task completion
        ++it;
    }
}
