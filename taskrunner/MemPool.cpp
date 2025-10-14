#include "../include/MemPool.h"
#include <cstdint>

MemPool::MemPool() {}
MemPool::~MemPool() {}

int MemPool::getSize() const { return this->_serialized_memory.size(); }

bool MemPool::isAllocated(void *ptr) {

  uintptr_t val = this->_serializedToInt(&ptr, false);
  for (auto it : this->_serialized_memory) {
    if (it == val) {
      return true;
    }
  }
  return false;
}

void *MemPool::allocate(size_t size) {
  (void)size;

  void *ptr = new char[size];

  this->_serializedToInt(&ptr, true);
  // Allocate memory from the pool
  // TODO: Implement memory pool allocation logic
  return ptr;
}

void MemPool::deallocate(void *ptr) {
  if (ptr == nullptr)
    return;

  uintptr_t val = reinterpret_cast<uintptr_t>(ptr);
  auto it = _serialized_memory.find(val);

  if (it != _serialized_memory.end()) {
    _serialized_memory.erase(it);
    delete[] static_cast<char *>(ptr);
  }
}

void MemPool::reset() {
  for (uintptr_t val : _serialized_memory) {
    void *ptr = reinterpret_cast<void *>(val);
    delete[] static_cast<char *>(ptr);
  }
  _serialized_memory.clear();
}

uintptr_t MemPool::_serializedToInt(void **ptr, bool add) {
  uintptr_t val = reinterpret_cast<uintptr_t>(*ptr);
  if (add) {
    _serialized_memory.insert(val);
  }
  return val;
}
void *MemPool::_serializedTpPtr(uintptr_t ptrint) {
  return reinterpret_cast<void *>(ptrint);
}
