#include "../include/Singleton.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <string>

// Test class that uses Singleton
class TestSingleton : public Singleton<TestSingleton> {
    friend class Singleton<TestSingleton>;

private:
    TestSingleton() : value(0), name("default") {}

    int value;
    std::string name;

public:
    void setValue(int v) { value = v; }
    int getValue() const { return value; }

    void setName(const std::string& n) { name = n; }
    std::string getName() const { return name; }
};

// Test 1: Basic singleton functionality
void testBasicSingleton() {
    std::cout << "Test 1: Basic Singleton Functionality" << std::endl;

    TestSingleton& instance1 = TestSingleton::getInstance();
    instance1.setValue(42);
    instance1.setName("first");

    TestSingleton& instance2 = TestSingleton::getInstance();

    // Both should reference the same object
    assert(instance2.getValue() == 42);
    assert(instance2.getName() == "first");

    // Modify through second reference
    instance2.setValue(100);
    assert(instance1.getValue() == 100);

    std::cout << "  ✓ Same instance retrieved multiple times" << std::endl;
    std::cout << "  ✓ State is shared between references" << std::endl;
}

// Test 2: Thread safety
void testThreadSafety() {
    std::cout << "\nTest 2: Thread Safety" << std::endl;

    const int numThreads = 10;
    std::vector<std::thread> threads;
    std::vector<TestSingleton*> instances(numThreads);

    // Create multiple threads that all try to get the instance
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&instances, i]() {
            instances[i] = &TestSingleton::getInstance();
        });
    }

    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }

    // All instances should be the same
    TestSingleton* firstInstance = instances[0];
    for (int i = 1; i < numThreads; ++i) {
        assert(instances[i] == firstInstance);
    }

    std::cout << "  ✓ All threads received the same instance" << std::endl;
}

// Test 3: Multiple singleton types
class AnotherSingleton : public Singleton<AnotherSingleton> {
    friend class Singleton<AnotherSingleton>;

private:
    AnotherSingleton() : data(999) {}
    int data;

public:
    void setData(int d) { data = d; }
    int getData() const { return data; }
};

void testMultipleSingletonTypes() {
    std::cout << "\nTest 3: Multiple Singleton Types" << std::endl;

    TestSingleton& ts = TestSingleton::getInstance();
    ts.setValue(111);

    AnotherSingleton& as = AnotherSingleton::getInstance();
    as.setData(222);

    // Each should maintain its own state
    assert(ts.getValue() == 111);
    assert(as.getData() == 222);

    // Verify they are different instances
    assert(static_cast<void*>(&ts) != static_cast<void*>(&as));

    std::cout << "  ✓ Different singleton types maintain separate instances" << std::endl;
}

int main() {
    std::cout << "=== Running Singleton Unit Tests ===" << std::endl;
    std::cout << std::endl;

    try {
        testBasicSingleton();
        testThreadSafety();
        testMultipleSingletonTypes();

        std::cout << "\n=== All Singleton Tests Passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
