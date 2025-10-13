#include "../include/TaskRunner.h"
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <string>

// Test 1: Execute single task with lambda
TEST(TaskRunnerTest, SingleTaskLambda) {
    TaskRunner runner;
    std::atomic<bool> taskExecuted{false};

    runner.executeTask([&taskExecuted]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        taskExecuted = true;
    });

    runner.waitForCompletion();

    EXPECT_TRUE(taskExecuted);
}

// Test 2: Execute single task with function
void myFunction(std::atomic<int>& counter) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    counter++;
}

TEST(TaskRunnerTest, SingleTaskFunction) {
    TaskRunner runner;
    std::atomic<int> counter{0};

    runner.executeTask([&counter]() {
        myFunction(counter);
    });

    runner.waitForCompletion();

    EXPECT_EQ(counter, 1);
}

// Test 3: Execute multiple single tasks
TEST(TaskRunnerTest, MultipleSingleTasks) {
    TaskRunner runner;
    std::atomic<int> counter{0};

    const int numTasks = 5;
    for (int i = 0; i < numTasks; ++i) {
        runner.executeTask([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            counter++;
        });
    }

    runner.waitForCompletion();

    EXPECT_EQ(counter, numTasks);
}

// Test 4: Repeated task with fixed count
TEST(TaskRunnerTest, RepeatedTaskFixedCount) {
    TaskRunner runner;
    std::atomic<int> counter{0};

    const size_t repeatCount = 5;
    runner.executeRepeatedTask(
        [&counter]() {
            counter++;
        },
        std::chrono::milliseconds(50),
        repeatCount
    );

    runner.waitForCompletion();

    EXPECT_EQ(counter, repeatCount);
}

// Test 5: Repeated task with stop
TEST(TaskRunnerTest, RepeatedTaskWithStop) {
    TaskRunner runner;
    std::atomic<int> counter{0};

    // Start an infinite task
    runner.executeRepeatedTask(
        [&counter]() {
            counter++;
        },
        std::chrono::milliseconds(50),
        0  // 0 means infinite
    );

    // Let it run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Stop all tasks
    runner.stopAll();
    int countAtStop = counter.load();

    // Wait for completion
    runner.waitForCompletion();

    // Counter should have stopped incrementing
    EXPECT_EQ(counter, countAtStop);
    EXPECT_GT(counter, 0);  // Should have executed at least once
}

// Test 6: Multiple repeated tasks
TEST(TaskRunnerTest, MultipleRepeatedTasks) {
    TaskRunner runner;
    std::atomic<int> counter1{0};
    std::atomic<int> counter2{0};

    runner.executeRepeatedTask(
        [&counter1]() { counter1++; },
        std::chrono::milliseconds(50),
        3
    );

    runner.executeRepeatedTask(
        [&counter2]() { counter2++; },
        std::chrono::milliseconds(50),
        5
    );

    runner.waitForCompletion();

    EXPECT_EQ(counter1, 3);
    EXPECT_EQ(counter2, 5);
}

// Test 7: Mixed single and repeated tasks
TEST(TaskRunnerTest, MixedTasks) {
    TaskRunner runner;
    std::atomic<int> singleCounter{0};
    std::atomic<int> repeatedCounter{0};

    // Execute single tasks
    for (int i = 0; i < 3; ++i) {
        runner.executeTask([&singleCounter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            singleCounter++;
        });
    }

    // Execute repeated task
    runner.executeRepeatedTask(
        [&repeatedCounter]() {
            repeatedCounter++;
        },
        std::chrono::milliseconds(40),
        4
    );

    runner.waitForCompletion();

    EXPECT_EQ(singleCounter, 3);
    EXPECT_EQ(repeatedCounter, 4);
}

// Test 8: Task with captured variables
TEST(TaskRunnerTest, TaskWithCapture) {
    TaskRunner runner;
    std::string result;
    std::atomic<bool> done{false};

    std::string message = "Hello from task!";

    runner.executeTask([&result, &done, message]() {
        result = message;
        done = true;
    });

    runner.waitForCompletion();

    EXPECT_TRUE(done);
    EXPECT_EQ(result, "Hello from task!");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
