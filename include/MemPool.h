#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <cstdint>
#include <iostream>
#include <set>



class MemPool {
public:
    MemPool();
    ~MemPool();

    // Delete copy and move
    MemPool(const MemPool&) = delete;
    MemPool& operator=(const MemPool&) = delete;
    MemPool(MemPool&&) = delete;
    MemPool& operator=(MemPool&&) = delete;

    // Memory pool operations (to be implemented)
    void* allocate(size_t size);
    void deallocate(void* ptr);
    void reset();

    int getSize() const;
    bool isAllocated(void *);

private:
    std::set<uintptr_t> _serialized_memory;

    uintptr_t _serializedToInt(void **, bool);
    void *_serializedTpPtr(uintptr_t);
};

#endif // MEMPOOL_H
