#include "../include/Singleton.h"
#include <gtest/gtest.h>
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
TEST(SingletonTest, BasicSingletonFunctionality) {
    TestSingleton& instance1 = TestSingleton::getInstance();
    instance1.setValue(42);
    instance1.setName("first");

    TestSingleton& instance2 = TestSingleton::getInstance();

    // Both should reference the same object
    EXPECT_EQ(instance2.getValue(), 42);
    EXPECT_EQ(instance2.getName(), "first");

    // Modify through second reference
    instance2.setValue(100);
    EXPECT_EQ(instance1.getValue(), 100);
}

// Test 2: Thread safety
TEST(SingletonTest, ThreadSafety) {
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
        EXPECT_EQ(instances[i], firstInstance);
    }
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

TEST(SingletonTest, MultipleSingletonTypes) {
    TestSingleton& ts = TestSingleton::getInstance();
    ts.setValue(111);

    AnotherSingleton& as = AnotherSingleton::getInstance();
    as.setData(222);

    // Each should maintain its own state
    EXPECT_EQ(ts.getValue(), 111);
    EXPECT_EQ(as.getData(), 222);

    // Verify they are different instances
    EXPECT_NE(static_cast<void*>(&ts), static_cast<void*>(&as));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
