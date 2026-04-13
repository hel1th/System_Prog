#include "../include/allocator_boundary_tags.h"
#include "allocator_boundary_tags.h"
#include <cstddef>
#include <cstring>
#include <memory_resource>
#include <mutex>
#include <not_implemented.h>

// метаданныe аллокатора
//  memory_resource*
//  fit_mode
//  size_t
//  mutex
//  void*

static std::pmr::memory_resource *&get_parent(void *mem) noexcept {
  return *reinterpret_cast<std::pmr::memory_resource **>(mem);
}

static allocator_with_fit_mode::fit_mode &get_fit_mode(void *mem) noexcept {
  auto *p = reinterpret_cast<std::byte *>(mem);
  return *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(
      p + sizeof(std::pmr::memory_resource *));
}

static size_t &get_pool_size(void *mem) noexcept {
  auto *p = reinterpret_cast<std::byte *>(mem);
  return *reinterpret_cast<size_t *>(p + sizeof(std::pmr::memory_resource *) +
                                     sizeof(allocator_with_fit_mode::fit_mode));
}

static std::mutex &get_mutex(void *mem) noexcept {
  auto *p = reinterpret_cast<std::byte *>(mem);
  return *reinterpret_cast<std::mutex *>(
      p + sizeof(std::pmr::memory_resource *) +
      sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));
}

static void *&get_first_block(void *mem) noexcept {
  auto *p = reinterpret_cast<std::byte *>(mem);
  return *reinterpret_cast<void **>(p + sizeof(std::pmr::memory_resource *) +
                                    sizeof(allocator_with_fit_mode::fit_mode) +
                                    sizeof(size_t) + sizeof(std::mutex));
}

// заголовок занятого блока
// [void* back | void* forward | size_t size | void* parent_mem]

static void *&get_block_back(void *block) noexcept {
  return *reinterpret_cast<void **>(block);
}

static void *&get_block_forward(void *block) noexcept {
  auto *p = reinterpret_cast<std::byte *>(block);
  return *reinterpret_cast<void **>(p + sizeof(void *));
}

static size_t &get_block_size(void *block) noexcept {
  auto *p = reinterpret_cast<std::byte *>(block);
  return *reinterpret_cast<size_t *>(p + 2 * sizeof(void *));
}

static void *&get_block_parent(void *block) noexcept {
  auto *p = reinterpret_cast<std::byte *>(block);
  return *reinterpret_cast<void **>(p + 2 * sizeof(void *) + sizeof(size_t));
}

static constexpr size_t occ_meta = sizeof(void *) * 3 + sizeof(size_t);

static void *block_user_ptr(void *block) noexcept {
  return reinterpret_cast<char *>(block) + occ_meta;
}

static void *user_ptr_to_block(void *user_ptr) noexcept {
  return reinterpret_cast<char *>(user_ptr) - occ_meta;
}

allocator_boundary_tags::~allocator_boundary_tags() {
  get_mutex(_trusted_memory).~mutex();
  get_parent(_trusted_memory)
      ->deallocate(_trusted_memory,
                   allocator_metadata_size + get_pool_size(_trusted_memory));
  _trusted_memory = nullptr;
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept {
  _trusted_memory = other._trusted_memory;
  other._trusted_memory = nullptr;
}

allocator_boundary_tags &
allocator_boundary_tags::operator=(allocator_boundary_tags &&other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (_trusted_memory != nullptr) {
    get_mutex(_trusted_memory).~mutex();
    get_parent(_trusted_memory)
        ->deallocate(_trusted_memory,
                     allocator_metadata_size + get_pool_size(_trusted_memory));
  }

  _trusted_memory = other._trusted_memory;
  other._trusted_memory = nullptr;

  return *this;
}

/** If parent_allocator* == nullptr you should use
 * std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size, std::pmr::memory_resource *parent_allocator,
    allocator_with_fit_mode::fit_mode allocate_fit_mode) {
  if (parent_allocator == nullptr)
    parent_allocator = std::pmr::get_default_resource();

  auto totalSize = allocator_metadata_size + space_size;
  _trusted_memory = parent_allocator->allocate(totalSize);

  get_parent(_trusted_memory) = parent_allocator;
  get_fit_mode(_trusted_memory) = allocate_fit_mode;
  get_pool_size(_trusted_memory) = space_size;
  new (&get_mutex(_trusted_memory)) std::mutex();
  get_first_block(_trusted_memory) = nullptr;
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(size_t size) {
  throw not_implemented(
      "[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(size_t)",
      "your code should be here...");
}

void allocator_boundary_tags::do_deallocate_sm(void *at) {
  throw not_implemented(
      "void allocator_boundary_tags::do_deallocate_sm(void *)",
      "your code should be here...");
}

inline void
allocator_boundary_tags::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
  throw not_implemented("inline void "
                        "allocator_boundary_tags::set_fit_mode(allocator_with_"
                        "fit_mode::fit_mode)",
                        "your code should be here...");
}

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info() const {
  throw not_implemented("std::vector<allocator_test_utils::block_info> "
                        "allocator_boundary_tags::get_blocks_info() const",
                        "your code should be here...");
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::begin() const noexcept {
  return allocator_boundary_tags::boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::end() const noexcept {
  return allocator_boundary_tags::boundary_iterator();
}

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info_inner() const {
  throw not_implemented(
      "std::vector<allocator_test_utils::block_info> "
      "allocator_boundary_tags::get_blocks_info_inner() const",
      "your code should be here...");
}

allocator_boundary_tags::allocator_boundary_tags(
    const allocator_boundary_tags &other) {
  auto totalSize =
      allocator_metadata_size + get_pool_size(other._trusted_memory);
  _trusted_memory = get_parent(other._trusted_memory)->allocate(totalSize);
  std::memcpy(_trusted_memory, other._trusted_memory, totalSize);
  new (&get_mutex(_trusted_memory)) std::mutex();
}

allocator_boundary_tags &
allocator_boundary_tags::operator=(const allocator_boundary_tags &other) {
  if (this == &other) {
    return *this;
  }
  auto totalSize =
      allocator_metadata_size + get_pool_size(other._trusted_memory);

  get_mutex(_trusted_memory).~mutex();
  get_parent(_trusted_memory)
      ->deallocate(_trusted_memory,
                   allocator_metadata_size + get_pool_size(_trusted_memory));

  _trusted_memory = get_parent(other._trusted_memory)->allocate(totalSize);
  std::memcpy(_trusted_memory, other._trusted_memory, totalSize);
  new (&get_mutex(_trusted_memory)) std::mutex();

  return *this;
}

bool allocator_boundary_tags::do_is_equal(
    const std::pmr::memory_resource &other) const noexcept {
  throw not_implemented("bool allocator_boundary_tags::do_is_equal(const "
                        "std::pmr::memory_resource &other) const noexcept",
                        "your code should be here...");
}

bool allocator_boundary_tags::boundary_iterator::operator==(
    const allocator_boundary_tags::boundary_iterator &other) const noexcept {
  return _occupied_ptr == other._occupied_ptr && _occupied == other._occupied;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
    const allocator_boundary_tags::boundary_iterator &other) const noexcept {
  return _occupied_ptr != other._occupied_ptr || _occupied != other._occupied;
}

allocator_boundary_tags::boundary_iterator &
allocator_boundary_tags::boundary_iterator::operator++() & noexcept {
  throw not_implemented(
      "allocator_boundary_tags::boundary_iterator "
      "&allocator_boundary_tags::boundary_iterator::operator++() & noexcept",
      "your code should be here...");
}

allocator_boundary_tags::boundary_iterator &
allocator_boundary_tags::boundary_iterator::operator--() & noexcept {
  throw not_implemented(
      "allocator_boundary_tags::boundary_iterator "
      "&allocator_boundary_tags::boundary_iterator::operator--() & noexcept",
      "your code should be here...");
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator++(int n) {
  throw not_implemented(
      "allocator_boundary_tags::boundary_iterator "
      "allocator_boundary_tags::boundary_iterator::operator++(int n)",
      "your code should be here...");
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator--(int n) {
  throw not_implemented(
      "allocator_boundary_tags::boundary_iterator "
      "allocator_boundary_tags::boundary_iterator::operator--(int n)",
      "your code should be here...");
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept {
  throw not_implemented(
      "size_t allocator_boundary_tags::boundary_iterator::size() const "
      "noexcept",
      "your code should be here...");
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept {
  return _occupied;
}

void *allocator_boundary_tags::boundary_iterator::operator*() const noexcept {
  if (_occupied) {
    return block_user_ptr(_occupied_ptr);
  }
  return nullptr;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr) {}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted) {
  void *first = get_first_block(_trusted_memory);
  void *pool_start =
      reinterpret_cast<std::byte *>(trusted) + allocator_metadata_size;

  _occupied_ptr = first;
  _occupied = (first != nullptr && first == pool_start);
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept {
  return _occupied_ptr;
}
