# MemPool - Memory Pool Allocator

## Overview

`MemPool` is a simple memory allocator that tracks all allocations and allows bulk deallocation. It wraps standard C++ `new`/`delete` operations while maintaining a registry of allocated pointers.

## Features

- Track all memory allocations
- Verify if a pointer was allocated by the pool
- Deallocate individual allocations
- Bulk deallocate all tracked memory at once
- Query number of active allocations

## Quick Start

```cpp
#include "MemPool.h"

MemPool pool;

// Allocate memory
void* ptr = pool.allocate(1024);

// Use the memory
int* numbers = static_cast<int*>(ptr);
numbers[0] = 42;

// Check if allocated
if (pool.isAllocated(ptr)) {
    std::cout << "Memory is tracked\n";
}

// Deallocate
pool.deallocate(ptr);

// Or deallocate everything at once
pool.reset();
```

## API Reference

### Constructor / Destructor

```cpp
MemPool();  // Create empty pool
~MemPool(); // Destroy pool (does NOT free memory automatically)
```

**Important**: The destructor does NOT free allocated memory. You must call `reset()` or `deallocate()` manually.

### allocate()

```cpp
void* allocate(size_t size);
```

Allocates memory of specified size and tracks it.

**Parameters**:
- `size` - Number of bytes to allocate

**Returns**: Pointer to allocated memory

**Example**:
```cpp
void* buffer = pool.allocate(512);
```

### deallocate()

```cpp
void deallocate(void* ptr);
```

Deallocates a previously allocated pointer and removes it from tracking.

**Parameters**:
- `ptr` - Pointer to deallocate (safe to pass nullptr)

**Safety**:
- Deallocating nullptr is safe (no-op)
- Deallocating same pointer twice is safe (no-op)
- Deallocating untracked pointer is safe (no-op)

**Example**:
```cpp
pool.deallocate(ptr);
```

### reset()

```cpp
void reset();
```

Deallocates ALL tracked allocations and clears the pool.

**Use Cases**:
- End-of-frame cleanup in games
- Request completion in web servers
- Cleanup after batch processing

**Example**:
```cpp
// Allocate many objects
for (int i = 0; i < 100; ++i) {
    pool.allocate(64);
}

// Free all at once
pool.reset();
```

### isAllocated()

```cpp
bool isAllocated(void* ptr);
```

Checks if a pointer is currently tracked by the pool.

**Parameters**:
- `ptr` - Pointer to check

**Returns**: `true` if tracked, `false` otherwise

**Example**:
```cpp
if (pool.isAllocated(ptr)) {
    std::cout << "Pointer is valid\n";
}
```

### getSize()

```cpp
int getSize() const;
```

Returns the number of currently tracked allocations.

**Returns**: Number of active allocations

**Example**:
```cpp
std::cout << "Active allocations: " << pool.getSize() << "\n";
```

## Usage Examples

### Basic Usage

```cpp
MemPool pool;

// Allocate
int* data = static_cast<int*>(pool.allocate(10 * sizeof(int)));

// Use
for (int i = 0; i < 10; ++i) {
    data[i] = i;
}

// Cleanup
pool.deallocate(data);
```

### Bulk Cleanup

```cpp
MemPool pool;

// Many allocations
std::vector<void*> ptrs;
for (int i = 0; i < 100; ++i) {
    ptrs.push_back(pool.allocate(256));
}

// Use allocations...

// Free all at once instead of individually
pool.reset();
```

### Game Frame Allocator

```cpp
class GameEngine {
    MemPool framePool;

public:
    void update() {
        framePool.reset(); // Clear previous frame

        // Allocate temporary data for this frame
        auto* renderData = framePool.allocate(sizeof(RenderData));
        auto* physicsData = framePool.allocate(sizeof(PhysicsData));

        // Process frame...

        // Memory automatically reset next frame
    }
};
```

### Memory Leak Detection

```cpp
MemPool pool;

// Do work
void* ptr1 = pool.allocate(100);
void* ptr2 = pool.allocate(200);

pool.deallocate(ptr1);

// Check for leaks
if (pool.getSize() > 0) {
    std::cerr << "Leak: " << pool.getSize() << " allocations\n";
}

pool.reset(); // Cleanup
```

## Thread Safety

**Not thread-safe**. Do not use the same MemPool instance from multiple threads without external synchronization.

### Thread-Safe Patterns

**Option 1: Thread-local pools**
```cpp
thread_local MemPool pool; // Each thread has its own pool
```

**Option 2: External mutex**
```cpp
MemPool pool;
std::mutex poolMutex;

void allocate(size_t size) {
    std::lock_guard<std::mutex> lock(poolMutex);
    return pool.allocate(size);
}
```

## Performance

| Operation | Complexity |
|-----------|-----------|
| allocate() | O(log n) |
| deallocate() | O(log n) |
| reset() | O(n) |
| isAllocated() | O(n) |
| getSize() | O(1) |

**Memory Overhead**: ~32-48 bytes per allocation for tracking.

## Best Practices

**DO:**
- Call `reset()` before the pool is destroyed
- Use for temporary/scoped allocations
- Use for bulk deallocation scenarios

**DON'T:**
- Share pool across threads without synchronization
- Mix pool deallocation with standard `delete`
- Use pointer after calling `reset()`
- Rely on destructor to free memory

## Common Pitfalls

### Memory Leak (Missing reset)

```cpp
// BAD - memory leaked
{
    MemPool pool;
    pool.allocate(1000);
} // Destructor doesn't free memory

// GOOD
{
    MemPool pool;
    pool.allocate(1000);
    pool.reset(); // Explicit cleanup
}
```

### Use After Free

```cpp
// BAD
void* ptr = pool.allocate(100);
pool.reset();
*static_cast<int*>(ptr) = 42; // Use after free!

// GOOD
void* ptr = pool.allocate(100);
*static_cast<int*>(ptr) = 42;
pool.reset();
```

### Race Condition

```cpp
// BAD - not thread-safe
MemPool pool;
std::thread t1([&]() { pool.allocate(100); });
std::thread t2([&]() { pool.allocate(200); });

// GOOD - thread-local
thread_local MemPool pool;
std::thread t1([]() { pool.allocate(100); });
std::thread t2([]() { pool.allocate(200); });
```

## When to Use

**Good for:**
- Managing many allocations that can be freed together
- Temporary per-request or per-frame allocations
- Memory leak detection and debugging
- Scenarios requiring allocation tracking

**Not ideal for:**
- High-performance critical paths (has overhead)
- Thread-concurrent allocations
- When standard allocator is sufficient

## See Also

- [API Reference](API-Reference.md) - Complete API documentation
- [Examples](Examples.md) - More usage examples
- [Integration Guide](INTEGRATION.md) - How to integrate into your project
