#include <gtest/gtest.h>
#include "../include/MemPool.h"

// Test fixture for MemPool tests
class MemPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
    }

    void TearDown() override {
        // Cleanup code after each test
    }
};

// Test: MemPool can be instantiated
// ===== Basic Functionality Tests =====

// Test 1: Can instantiate the memory pool
TEST_F(MemPoolTest, CanInstantiate) {
    MemPool pool;
    EXPECT_TRUE(true);
}

// Test 2: Allocate returns a valid pointer
TEST_F(MemPoolTest, AllocateReturnsPointer) {
    MemPool pool;
    void* ptr = pool.allocate(64);
    EXPECT_NE(ptr, nullptr);
    EXPECT_TRUE(pool.isAllocated(ptr));
}

// Test 3: Deallocate does not crash
TEST_F(MemPoolTest, DeallocateDoesNotCrash) {
    MemPool pool;
    void* ptr = pool.allocate(64);
    EXPECT_NO_THROW(pool.deallocate(ptr));
}

// Test 4: Reset does not crash
TEST_F(MemPoolTest, ResetDoesNotCrash) {
    MemPool pool;
    EXPECT_NO_THROW(pool.reset());
}

// ===== Multiple Allocation Tests =====

// Test 5: Multiple allocations return different pointers
TEST_F(MemPoolTest, MultipleAllocationsReturnDifferentPointers) {
    MemPool pool;
    void* ptr1 = pool.allocate(64);
    void* ptr2 = pool.allocate(64);
    EXPECT_NE(ptr1, ptr2);
}

// Test 6: Can allocate many small blocks
TEST_F(MemPoolTest, CanAllocateManySmallBlocks) {
    MemPool pool;
    std::vector<void*> ptrs;
    for (int i = 0; i < 100; ++i) {
        void* ptr = pool.allocate(16);
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
}

// Test 7: Can allocate various sizes
TEST_F(MemPoolTest, CanAllocateVariousSizes) {
    MemPool pool;
    void* ptr1 = pool.allocate(8);
    void* ptr2 = pool.allocate(64);
    void* ptr3 = pool.allocate(256);
    void* ptr4 = pool.allocate(1024);
    
    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);
    EXPECT_NE(ptr4, nullptr);
}

// ===== Edge Case Tests =====

// Test 8: Allocate zero bytes
TEST_F(MemPoolTest, AllocateZeroBytes) {
    MemPool pool;
    void* ptr = pool.allocate(0);
    // Implementation-dependent: could return nullptr or a valid pointer
    EXPECT_NO_THROW(pool.deallocate(ptr));
}

// Test 9: Allocate very large block
TEST_F(MemPoolTest, AllocateLargeBlock) {
    MemPool pool;
    void* ptr = pool.allocate(1024 * 1024); // 1MB
    EXPECT_NE(ptr, nullptr);
}

// Test 10: Deallocate nullptr should not crash
TEST_F(MemPoolTest, DeallocateNullptrDoesNotCrash) {
    MemPool pool;
    EXPECT_NO_THROW(pool.deallocate(nullptr));
}

// ===== Memory Write/Read Tests =====

// Test 11: Can write and read from allocated memory
TEST_F(MemPoolTest, CanWriteAndReadMemory) {
    MemPool pool;
    int* ptr = static_cast<int*>(pool.allocate(sizeof(int)));
    ASSERT_NE(ptr, nullptr);
    *ptr = 42;
    EXPECT_EQ(*ptr, 42);
}

// Test 12: Multiple allocations have independent memory
TEST_F(MemPoolTest, AllocationsAreIndependent) {
    MemPool pool;
    int* ptr1 = static_cast<int*>(pool.allocate(sizeof(int)));
    int* ptr2 = static_cast<int*>(pool.allocate(sizeof(int)));
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    
    *ptr1 = 100;
    *ptr2 = 200;
    
    EXPECT_EQ(*ptr1, 100);
    EXPECT_EQ(*ptr2, 200);
}

// ===== Reset Functionality Tests =====

// Test 13: Reset invalidates previous allocations
TEST_F(MemPoolTest, ResetInvalidatesPreviousAllocations) {
    MemPool pool;
    void* ptr1 = pool.allocate(64);
    EXPECT_NE(ptr1, nullptr);
    
    pool.reset();
    
    // After reset, we should be able to allocate again
    void* ptr2 = pool.allocate(64);
    EXPECT_NE(ptr2, nullptr);
}

// Test 14: Can allocate after reset
TEST_F(MemPoolTest, CanAllocateAfterReset) {
    MemPool pool;
    pool.allocate(128);
    pool.allocate(256);
    
    pool.reset();
    
    void* ptr = pool.allocate(64);
    EXPECT_NE(ptr, nullptr);
}

// Test 15: Multiple resets do not crash
TEST_F(MemPoolTest, MultipleResetsDoNotCrash) {
    MemPool pool;
    EXPECT_NO_THROW(pool.reset());
    EXPECT_NO_THROW(pool.reset());
    EXPECT_NO_THROW(pool.reset());
}

// ===== Allocation/Deallocation Patterns =====

// Test 16: Allocate, deallocate, then allocate again
TEST_F(MemPoolTest, AllocateDeallocateAllocatePattern) {
    MemPool pool;
    void* ptr1 = pool.allocate(64);
    EXPECT_NE(ptr1, nullptr);
    
    pool.deallocate(ptr1);
    
    void* ptr2 = pool.allocate(64);
    EXPECT_NE(ptr2, nullptr);
}

// Test 17: Deallocate in reverse order
TEST_F(MemPoolTest, DeallocateInReverseOrder) {
    MemPool pool;
    void* ptr1 = pool.allocate(64);
    void* ptr2 = pool.allocate(64);
    void* ptr3 = pool.allocate(64);
    
    EXPECT_NO_THROW(pool.deallocate(ptr3));
    EXPECT_NO_THROW(pool.deallocate(ptr2));
    EXPECT_NO_THROW(pool.deallocate(ptr1));
}

// Test 18: Deallocate in random order
TEST_F(MemPoolTest, DeallocateInRandomOrder) {
    MemPool pool;
    void* ptr1 = pool.allocate(64);
    void* ptr2 = pool.allocate(64);
    void* ptr3 = pool.allocate(64);
    
    EXPECT_NO_THROW(pool.deallocate(ptr2));
    EXPECT_NO_THROW(pool.deallocate(ptr1));
    EXPECT_NO_THROW(pool.deallocate(ptr3));
}

// ===== Alignment Tests =====

// Test 19: Allocated memory is properly aligned
TEST_F(MemPoolTest, AllocatedMemoryIsAligned) {
    MemPool pool;
    void* ptr = pool.allocate(64);
    ASSERT_NE(ptr, nullptr);
    
    // Check if pointer is aligned to at least 8 bytes
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    EXPECT_EQ(addr % 8, 0);
}

// Test 20: Large allocations are aligned
TEST_F(MemPoolTest, LargeAllocationsAreAligned) {
    MemPool pool;
    void* ptr = pool.allocate(1024);
    ASSERT_NE(ptr, nullptr);
    
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    EXPECT_EQ(addr % 8, 0);
}

// ===== Stress Tests =====

// Test 21: Stress test with many allocations
TEST_F(MemPoolTest, StressTestManyAllocations) {
    MemPool pool;
    std::vector<void*> ptrs;
    
    for (int i = 0; i < 1000; ++i) {
        void* ptr = pool.allocate(32);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    // Deallocate all
    for (void* ptr : ptrs) {
        EXPECT_NO_THROW(pool.deallocate(ptr));
    }
}

// Test 22: Alternating allocate and deallocate
TEST_F(MemPoolTest, AlternatingAllocateAndDeallocate) {
    MemPool pool;
    
    for (int i = 0; i < 50; ++i) {
        void* ptr = pool.allocate(64);
        EXPECT_NE(ptr, nullptr);
        pool.deallocate(ptr);
    }
}

// ===== Bonus Tests =====

// Test 23: Double deallocate should not crash (if implementation supports it)
TEST_F(MemPoolTest, DoubleDeallocateDoesNotCrash) {
    MemPool pool;
    void* ptr = pool.allocate(64);
    pool.deallocate(ptr);
    // Note: This behavior is implementation-dependent
    EXPECT_NO_THROW(pool.deallocate(ptr));
}

// Test 24: Reset then deallocate should not crash
TEST_F(MemPoolTest, ResetThenDeallocateDoesNotCrash) {
    MemPool pool;
    void* ptr = pool.allocate(64);
    pool.reset();
    // Deallocating after reset - implementation dependent
    EXPECT_NO_THROW(pool.deallocate(ptr));
}

// Test 25: Can write large data structure
TEST_F(MemPoolTest, CanWriteLargeDataStructure) {
    MemPool pool;
    struct LargeStruct {
        int data[256];
    };
    
    LargeStruct* ptr = static_cast<LargeStruct*>(pool.allocate(sizeof(LargeStruct)));
    ASSERT_NE(ptr, nullptr);
    
    for (int i = 0; i < 256; ++i) {
        ptr->data[i] = i;
    }
    
    for (int i = 0; i < 256; ++i) {
        EXPECT_EQ(ptr->data[i], i);
    }
}
