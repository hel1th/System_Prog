#include "../include/allocator_sorted_list.h"
#include "allocator_test_utils.h"
#include <cstddef>
#include <cstring>
#include <memory_resource>
#include <mutex>
#include <new>
#include <not_implemented.h>
#include <vector>

allocator_sorted_list::~allocator_sorted_list() {
  get_mutex().~mutex();
  get_parent()->deallocate(_trusted_memory, allocator_metadata_size +
                                                block_metadata_size +
                                                get_space_size());
  _trusted_memory = nullptr;
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept {
  _trusted_memory = other._trusted_memory;
  other._trusted_memory = nullptr;
}

allocator_sorted_list &
allocator_sorted_list::operator=(allocator_sorted_list &&other) noexcept {
  if (*this == other) {
    return *this;
  }

  get_mutex().~mutex();
  get_parent()->deallocate(_trusted_memory, allocator_metadata_size +
                                                block_metadata_size +
                                                get_space_size());

  _trusted_memory = other._trusted_memory;
  other._trusted_memory = nullptr;

  return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size, std::pmr::memory_resource *parent_allocator,
    allocator_with_fit_mode::fit_mode allocate_fit_mode) {
  if (parent_allocator == nullptr) {
    parent_allocator = std::pmr::get_default_resource();
  }

  size_t totalSize = allocator_metadata_size + block_metadata_size + space_size;
  _trusted_memory = parent_allocator->allocate(totalSize);

  auto ptr = reinterpret_cast<char *>(_trusted_memory);

  *reinterpret_cast<std::pmr::memory_resource **>(ptr) = parent_allocator;
  ptr += sizeof(std::pmr::memory_resource *);

  *reinterpret_cast<fit_mode *>(ptr) = allocate_fit_mode;
  ptr += sizeof(fit_mode);

  *reinterpret_cast<size_t *>(ptr) = space_size;
  ptr += sizeof(size_t);

  new (ptr) std::mutex();
  ptr += sizeof(std::mutex);

  void *first_free = ptr + sizeof(void *);
  *reinterpret_cast<void **>(ptr) = first_free;
  ptr += sizeof(void *);

  // first (and only) free block header
  *reinterpret_cast<void **>(ptr) = nullptr; // next free = nullptr
  ptr += sizeof(void *);

  *reinterpret_cast<size_t *>(ptr) = space_size; // usable size
}

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(size_t size) {
  std::lock_guard<std::mutex> lock(get_mutex());

  void *prev = nullptr;
  void *chosen = nullptr;
  void *chosen_prev = nullptr;

  void *curr = get_first_free();

  while (curr != nullptr) {
    size_t curr_size = read_block_size(curr);

    if (curr_size >= size) {
      switch (get_fit_mode()) {
      case fit_mode::first_fit:
        chosen = curr;
        chosen_prev = prev;
        curr = nullptr; // stop immediately
        break;

      case fit_mode::the_best_fit:
        // smallest block that still fits
        if (chosen == nullptr || curr_size < read_block_size(chosen)) {
          chosen_prev = prev;
          chosen = curr;
        }
        break;

      case fit_mode::the_worst_fit:
        // largest block
        if (chosen == nullptr || curr_size > read_block_size(chosen)) {
          chosen_prev = prev;
          chosen = curr;
        }
        break;
      }
    }

    if (curr != nullptr) {
      prev = curr;
      curr = read_block_next(curr);
    }
  }

  if (chosen == nullptr)
    throw std::bad_alloc();

  size_t chosen_size = read_block_size(chosen);
  void *remainder = nullptr;

  if (chosen_size >= size + block_metadata_size + 1) {
    remainder = reinterpret_cast<char *>(chosen) + block_metadata_size + size;

    *reinterpret_cast<void **>(remainder) = read_block_next(chosen);
    *reinterpret_cast<size_t *>(reinterpret_cast<char *>(remainder) +
                                sizeof(void *)) =
        chosen_size - size - block_metadata_size;
  }

  void *next = remainder != nullptr ? remainder : read_block_next(chosen);

  if (chosen_prev == nullptr) {
    set_first_free(next);
  } else {
    *reinterpret_cast<void **>(chosen_prev) = next;
  }

  *reinterpret_cast<size_t *>(reinterpret_cast<char *>(chosen) +
                              sizeof(void *)) = size;

  return reinterpret_cast<char *>(chosen) + block_metadata_size;
}

allocator_sorted_list::allocator_sorted_list(
    const allocator_sorted_list &other) {
  size_t totalSize =
      allocator_metadata_size + block_metadata_size + other.get_space_size();
  _trusted_memory = other.get_parent()->allocate(totalSize);
  std::memcpy(_trusted_memory, other._trusted_memory, totalSize);
  new (&get_mutex()) std::mutex();
}

allocator_sorted_list &
allocator_sorted_list::operator=(const allocator_sorted_list &other) {
  if (*this == other)
    return *this;

  get_mutex().~mutex();
  get_parent()->deallocate(_trusted_memory, allocator_metadata_size +
                                                block_metadata_size +
                                                get_space_size());

  size_t totalSize =
      allocator_metadata_size + block_metadata_size + other.get_space_size();
  _trusted_memory = other.get_parent()->allocate(totalSize);
  std::memcpy(_trusted_memory, other._trusted_memory, totalSize);
  new (&get_mutex()) std::mutex();

  return *this;
}

bool allocator_sorted_list::do_is_equal(
    const std::pmr::memory_resource &other) const noexcept {
  auto *o = dynamic_cast<const allocator_sorted_list *>(&other);
  return o != nullptr && o->_trusted_memory == _trusted_memory;
}

void allocator_sorted_list::do_deallocate_sm(void *at) {
  std::lock_guard<std::mutex> lock(get_mutex());

  void *memStart =
      reinterpret_cast<char *>(_trusted_memory) + allocator_metadata_size;
  void *memEnd = reinterpret_cast<char *>(memStart) + get_space_size();

  void *block = reinterpret_cast<char *>(at) - block_metadata_size;

  if (block < memStart || block >= memEnd) {
    throw std::invalid_argument("block does not belong to this allocator");
  }

  void *prev = nullptr;
  void *curr = get_first_free();

  while (curr != nullptr && curr < block) {
    prev = curr;
    curr = read_block_next(curr);
  }

  *reinterpret_cast<void **>(block) = curr;

  if (prev == nullptr) {
    set_first_free(block);
  } else {
    *reinterpret_cast<void **>(prev) = block;
  }

  void *block_end = reinterpret_cast<char *>(block) + block_metadata_size +
                    read_block_size(block);

  if (curr != nullptr && block_end == curr) {
    *reinterpret_cast<void **>(block) = read_block_next(curr);
    *reinterpret_cast<size_t *>(reinterpret_cast<char *>(block) +
                                sizeof(void *)) =
        read_block_size(block) + block_metadata_size + read_block_size(curr);
  }

  if (prev != nullptr) {
    void *prev_end = reinterpret_cast<char *>(prev) + block_metadata_size +
                     read_block_size(prev);

    if (prev_end == block) {
      *reinterpret_cast<void **>(prev) = read_block_next(block);
      *reinterpret_cast<size_t *>(reinterpret_cast<char *>(prev) +
                                  sizeof(void *)) =
          read_block_size(prev) + block_metadata_size + read_block_size(block);
    }
  }
}

inline void
allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
  auto *ptr = reinterpret_cast<char *>(_trusted_memory) +
              sizeof(std::pmr::memory_resource *);
  *reinterpret_cast<fit_mode *>(ptr) = mode;
}

std::vector<allocator_test_utils::block_info>
allocator_sorted_list::get_blocks_info() const noexcept {
  std::lock_guard<std::mutex> lock(get_mutex());
  return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info>
allocator_sorted_list::get_blocks_info_inner() const {
  std::vector<allocator_test_utils::block_info> infoVec;

  for (auto it = begin(); it != end(); ++it) {
    infoVec.push_back({it.size(), it.occupied()});
  }

  return infoVec;
}

allocator_sorted_list::sorted_free_iterator
allocator_sorted_list::free_begin() const noexcept {
  return sorted_free_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_free_iterator
allocator_sorted_list::free_end() const noexcept {
  return sorted_free_iterator();
}

allocator_sorted_list::sorted_iterator
allocator_sorted_list::begin() const noexcept {
  return sorted_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_iterator
allocator_sorted_list::end() const noexcept {
  return sorted_iterator();
}

bool allocator_sorted_list::sorted_free_iterator::operator==(
    const allocator_sorted_list::sorted_free_iterator &other) const noexcept {
  return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
    const allocator_sorted_list::sorted_free_iterator &other) const noexcept {
  return _free_ptr != other._free_ptr;
}

allocator_sorted_list::sorted_free_iterator &
allocator_sorted_list::sorted_free_iterator::operator++() & noexcept {
  _free_ptr = read_block_next(_free_ptr);
  return *this;
}

allocator_sorted_list::sorted_free_iterator
allocator_sorted_list::sorted_free_iterator::operator++(int n) {
  auto tmp = *this;
  ++(*this);
  return tmp;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept {
  return read_block_size(_free_ptr);
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept {
  return _free_ptr;
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator()
    : _free_ptr(nullptr) {}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(
    void *trusted) {
  _free_ptr = read_first_free(trusted);
}

bool allocator_sorted_list::sorted_iterator::operator==(
    const allocator_sorted_list::sorted_iterator &other) const noexcept {
  return _current_ptr == other._current_ptr;
}

bool allocator_sorted_list::sorted_iterator::operator!=(
    const allocator_sorted_list::sorted_iterator &other) const noexcept {
  return _current_ptr != other._current_ptr;
}

allocator_sorted_list::sorted_iterator &
allocator_sorted_list::sorted_iterator::operator++() & noexcept {
  if (_current_ptr == _free_ptr && _free_ptr != nullptr)
    _free_ptr = read_block_next(_free_ptr);

  _current_ptr =
      reinterpret_cast<char *>(_current_ptr) + block_metadata_size + size();

  void *memory_end = reinterpret_cast<char *>(_trusted_memory) +
                     allocator_metadata_size + read_space_size(_trusted_memory);

  if (_current_ptr >= memory_end)
    _current_ptr = nullptr;

  return *this;
}

allocator_sorted_list::sorted_iterator
allocator_sorted_list::sorted_iterator::operator++(int n) {
  auto tmp = *this;
  ++(*this);
  return tmp;
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept {
  return read_block_size(_current_ptr);
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept {
  return _current_ptr;
}

allocator_sorted_list::sorted_iterator::sorted_iterator()
    : _free_ptr(nullptr), _current_ptr(nullptr), _trusted_memory(nullptr) {}

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted)
    : _trusted_memory(trusted) {
  _free_ptr = read_first_free(trusted);
  _current_ptr = reinterpret_cast<char *>(trusted) + allocator_metadata_size;
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept {
  return _current_ptr != _free_ptr;
}

size_t allocator_sorted_list::get_space_size() const noexcept {
  return read_space_size(_trusted_memory);
}

void *allocator_sorted_list::get_first_free() const noexcept {
  return read_first_free(_trusted_memory);
}

std::pmr::memory_resource *allocator_sorted_list::get_parent() const noexcept {
  return *reinterpret_cast<std::pmr::memory_resource **>(_trusted_memory);
}

allocator_sorted_list::fit_mode
allocator_sorted_list::get_fit_mode() const noexcept {
  auto *ptr = reinterpret_cast<char *>(_trusted_memory) +
              sizeof(std::pmr::memory_resource *);
  return *reinterpret_cast<fit_mode *>(ptr);
}

std::mutex &allocator_sorted_list::get_mutex() const noexcept {
  auto *ptr = reinterpret_cast<char *>(_trusted_memory) +
              sizeof(std::pmr::memory_resource *) + sizeof(fit_mode) +
              sizeof(size_t);
  return *reinterpret_cast<std::mutex *>(ptr);
}

void allocator_sorted_list::set_first_free(void *ptr) noexcept {
  auto *p = reinterpret_cast<char *>(_trusted_memory) +
            sizeof(std::pmr::memory_resource *) + sizeof(fit_mode) +
            sizeof(size_t) + sizeof(std::mutex);
  *reinterpret_cast<void **>(p) = ptr;
}

size_t allocator_sorted_list::read_space_size(void *trusted) noexcept {
  return *reinterpret_cast<size_t *>(reinterpret_cast<char *>(trusted) +
                                     sizeof(std::pmr::memory_resource *) +
                                     sizeof(fit_mode));
}

void *allocator_sorted_list::read_first_free(void *trusted) noexcept {
  return *reinterpret_cast<void **>(
      reinterpret_cast<char *>(trusted) + sizeof(std::pmr::memory_resource *) +
      sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex));
}

void *allocator_sorted_list::read_block_next(void *block) noexcept {
  return *reinterpret_cast<void **>(block);
}

size_t allocator_sorted_list::read_block_size(void *block) noexcept {
  return *reinterpret_cast<size_t *>(reinterpret_cast<char *>(block) +
                                     sizeof(void *));
}