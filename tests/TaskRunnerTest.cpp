#include "../include/TaskRunner.h"
#include <iostream>
#include <cassert>
#include <atomic>
#include <chrono>
#include <string>

// Test 1: Execute single task with lambda
void testSingleTaskLambda() {
    std::cout << "Test 1: Single Task with Lambda" << std::endl;

    TaskRunner runner;
    std::atomic<bool> taskExecuted{false};

    runner.executeTask([&taskExecuted]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        taskExecuted = true;
    });

    runner.waitForCompletion();

    assert(taskExecuted == true);
    std::cout << "  ✓ Lambda task executed successfully" << std::endl;
}

// Test 2: Execute single task with function
void myFunction(std::atomic<int>& counter) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    counter++;
}

void testSingleTaskFunction() {
    std::cout << "\nTest 2: Single Task with Function" << std::endl;

    TaskRunner runner;
    std::atomic<int> counter{0};

    runner.executeTask([&counter]() {
        myFunction(counter);
    });

    runner.waitForCompletion();

    assert(counter == 1);
    std::cout << "  ✓ Function task executed successfully" << std::endl;
}

// Test 3: Execute multiple single tasks
void testMultipleSingleTasks() {
    std::cout << "\nTest 3: Multiple Single Tasks" << std::endl;

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

    assert(counter == numTasks);
    std::cout << "  ✓ All " << numTasks << " tasks executed successfully" << std::endl;
}

// Test 4: Repeated task with fixed count
void testRepeatedTaskFixedCount() {
    std::cout << "\nTest 4: Repeated Task with Fixed Count" << std::endl;

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

    assert(counter == repeatCount);
    std::cout << "  ✓ Task repeated exactly " << repeatCount << " times" << std::endl;
}

// Test 5: Repeated task with stop
void testRepeatedTaskWithStop() {
    std::cout << "\nTest 5: Repeated Task with Stop" << std::endl;

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
    assert(counter == countAtStop);
    assert(counter > 0);  // Should have executed at least once

    std::cout << "  ✓ Infinite task stopped correctly after " << counter << " executions" << std::endl;
}

// Test 6: Multiple repeated tasks
void testMultipleRepeatedTasks() {
    std::cout << "\nTest 6: Multiple Repeated Tasks" << std::endl;

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

    assert(counter1 == 3);
    assert(counter2 == 5);
    std::cout << "  ✓ Multiple repeated tasks executed correctly" << std::endl;
}

// Test 7: Mixed single and repeated tasks
void testMixedTasks() {
    std::cout << "\nTest 7: Mixed Single and Repeated Tasks" << std::endl;

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

    assert(singleCounter == 3);
    assert(repeatedCounter == 4);
    std::cout << "  ✓ Mixed tasks executed correctly" << std::endl;
}

// Test 8: Task with captured variables
void testTaskWithCapture() {
    std::cout << "\nTest 8: Task with Captured Variables" << std::endl;

    TaskRunner runner;
    std::string result;
    std::atomic<bool> done{false};

    std::string message = "Hello from task!";

    runner.executeTask([&result, &done, message]() {
        result = message;
        done = true;
    });

    runner.waitForCompletion();

    assert(done == true);
    assert(result == "Hello from task!");
    std::cout << "  ✓ Task with captured variables executed correctly" << std::endl;
}

int main() {
    std::cout << "=== Running TaskRunner Unit Tests ===" << std::endl;
    std::cout << std::endl;

    try {
        testSingleTaskLambda();
        testSingleTaskFunction();
        testMultipleSingleTasks();
        testRepeatedTaskFixedCount();
        testRepeatedTaskWithStop();
        testMultipleRepeatedTasks();
        testMixedTasks();
        testTaskWithCapture();

        std::cout << "\n=== All TaskRunner Tests Passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
