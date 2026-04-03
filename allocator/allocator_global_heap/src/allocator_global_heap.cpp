#include "../include/allocator_global_heap.h"
#include <cstddef>
#include <iostream>
#include <mutex>
#include <new>
// #define DEBUG_ALLOCATOR 1

allocator_global_heap::allocator_global_heap() {}

allocator_global_heap::~allocator_global_heap() {}

allocator_global_heap::allocator_global_heap(
    const allocator_global_heap &other) {}

allocator_global_heap &
allocator_global_heap::operator=(const allocator_global_heap &other) {
  return *this;
}

allocator_global_heap::allocator_global_heap(
    allocator_global_heap &&other) noexcept {}

allocator_global_heap &
allocator_global_heap::operator=(allocator_global_heap &&other) noexcept {
  return *this;
}

[[nodiscard]] void *allocator_global_heap::do_allocate_sm(size_t size) {
  void *ptr = ::operator new(size);
#ifdef DEBUG_ALLOCATOR
  std::cout << "[allocate] size=" << size << " ptr=" << ptr
            << " dump=" << get_dump(reinterpret_cast<char *>(ptr), size)
            << "\n";
#endif
  return ptr;
}

void allocator_global_heap::do_deallocate_sm(void *at) {
#ifdef DEBUG_ALLOCATOR
  std::cout << "[deallocate] ptr=" << at
            << " dump=" << get_dump(reinterpret_cast<char *>(at), 8) << "\n";
#endif
  ::operator delete(at);
}

bool allocator_global_heap::do_is_equal(
    const std::pmr::memory_resource &other) const noexcept {
  return dynamic_cast<const allocator_global_heap *>(&other) != nullptr;
}