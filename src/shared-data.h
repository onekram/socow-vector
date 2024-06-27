#pragma once

#include <cstddef>
#include <utility>

template <typename T>
class shared_data {
public:
  using value_type = T;
  using pointer = T*;
  using reference = T&;

  template <typename U, std::size_t SMALL_SIZE>
  friend class socow_vector;

public:
  shared_data()
      : _data(nullptr)
      , _count(nullptr) {}

  shared_data(const shared_data& other)
      : _data(other._data)
      , _count(other._count) {
    ++*_count;
  }

  shared_data& operator=(const shared_data& other) noexcept {
    if (this != &other) {
      shared_data copy(other);
      swap(*this, copy);
    }
    return *this;
  }

  shared_data(shared_data&& other) noexcept
      : _data(other._data)
      , _count(other._count) {
    other._data = nullptr;
    other._count = nullptr;
  }

  shared_data& operator=(shared_data&& other) noexcept {
    if (this != &other) {
      shared_data empty;
      swap(*this, empty);
      swap(*this, other);
    }
    return *this;
  }

  ~shared_data() {
    if (_count != nullptr && !--*_count) {
      delete _data;
      delete _count;
    }
  }

  template <typename... Args>
  explicit shared_data(Args&&... args)
      : _data(new T(std::forward<Args>(args)...))
      , _count(new std::size_t(1)) {}

  reference operator*() const {
    return *_data;
  }

  pointer operator->() const {
    return _data;
  }

  std::size_t use_count() const {
    return *_count;
  }

  friend void swap(shared_data& lhs, shared_data& rhs) {
    using std::swap;
    swap(lhs._data, rhs._data);
    swap(lhs._count, rhs._count);
  }

private:
  pointer _data = nullptr;
  std::size_t* _count = nullptr;
};
