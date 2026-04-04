#include "../include/allocator_sorted_list.h"
#include <cstddef>
#include <memory_resource>
#include <mutex>
#include <not_implemented.h>

allocator_sorted_list::~allocator_sorted_list() {
  throw not_implemented("allocator_sorted_list::~allocator_sorted_list()",
                        "your code should be here...");
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept {
  throw not_implemented("allocator_sorted_list::allocator_sorted_list("
                        "allocator_sorted_list &&) noexcept",
                        "your code should be here...");
}

allocator_sorted_list &
allocator_sorted_list::operator=(allocator_sorted_list &&other) noexcept {
  throw not_implemented(
      "allocator_sorted_list "
      "&allocator_sorted_list::operator=(allocator_sorted_list &&) noexcept",
      "your code should be here...");
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

  *reinterpret_cast<void **>(ptr) = nullptr;
  ptr += sizeof(void *);

  *reinterpret_cast<size_t *>(ptr) = space_size;
}

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(size_t size) {
  throw not_implemented(
      "[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(size_t)",
      "your code should be here...");
}

allocator_sorted_list::allocator_sorted_list(
    const allocator_sorted_list &other) {
  throw not_implemented("allocator_sorted_list::allocator_sorted_list(const "
                        "allocator_sorted_list &other)",
                        "your code should be here...");
}

allocator_sorted_list &
allocator_sorted_list::operator=(const allocator_sorted_list &other) {
  throw not_implemented(
      "allocator_sorted_list &allocator_sorted_list::operator=(const "
      "allocator_sorted_list &other)",
      "your code should be here...");
}

bool allocator_sorted_list::do_is_equal(
    const std::pmr::memory_resource &other) const noexcept {
  auto *o = dynamic_cast<const allocator_sorted_list *>(&other);
  return o != nullptr && o->_trusted_memory == _trusted_memory;
}

void allocator_sorted_list::do_deallocate_sm(void *at) {
  throw not_implemented("void allocator_sorted_list::do_deallocate_sm(void *)",
                        "your code should be here...");
}

inline void
allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
  
  auto *ptr = reinterpret_cast<char *>(_trusted_memory) +
              sizeof(std::pmr::memory_resource *);

  *reinterpret_cast<fit_mode *>(ptr) = mode;
}

std::vector<allocator_test_utils::block_info>
allocator_sorted_list::get_blocks_info() const noexcept {
  throw not_implemented(
      "std::vector<allocator_test_utils::block_info> "
      "allocator_sorted_list::get_blocks_info() const noexcept",
      "your code should be here...");
}

std::vector<allocator_test_utils::block_info>
allocator_sorted_list::get_blocks_info_inner() const {
  throw not_implemented("std::vector<allocator_test_utils::block_info> "
                        "allocator_sorted_list::get_blocks_info_inner() const",
                        "your code should be here...");
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

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator() {
  throw not_implemented(
      "allocator_sorted_list::sorted_free_iterator::sorted_free_iterator()",
      "your code should be here...");
}

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