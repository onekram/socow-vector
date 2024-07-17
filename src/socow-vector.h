#pragma once

#include "shared-data.h"
#include "vector.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>

template <typename T, std::size_t SMALL_SIZE>
class socow_vector {
public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

public:
  socow_vector()
      : _size(0) {}

  // O(SMALL_SIZE) / O(1); strong / nothrow
  socow_vector(const socow_vector& other)
      : _size(other._size) {
    if (other.small_object()) {
      for (std::size_t i = 0; i != other.size(); ++i) {
        new (_static_data.data() + i) value_type(other[i]);
      }
    } else {
      _dynamic_data._data = nullptr;
      _dynamic_data._count = nullptr;
      _dynamic_data = other._dynamic_data;
    }
  }

  socow_vector(socow_vector&& other) noexcept
      : _size(0) {
    using std::swap;
    if (other.small_object()) {
      swap(_static_data, other._static_data);
    } else {
      _dynamic_data._data = nullptr;
      _dynamic_data._count = nullptr;
      swap(_dynamic_data, other._dynamic_data);
    }
    swap(_size, other._size);
  }

  // O(SMALL_SIZE) / O(1); strong / nothrow
  socow_vector& operator=(const socow_vector& other) {
    if (this != &other) {
      socow_vector copy(other);
      swap(copy);
    }
    return *this;
  }

  // O(SMALL_SIZE) / O(1); nothrow / nothrow
  socow_vector& operator=(socow_vector&& other) noexcept {
    if (this != &other) {
      swap(other);
    }
    return *this;
  }

  // ???
  ~socow_vector() {
    if (small_object()) {
      std::destroy(begin(), end());
    } else {
      _dynamic_data.~shared_data();
    }
  }

  // Fields access

  // O(1) / O(1); nothrow / nothrow
  std::size_t size() const noexcept {
    return small_object() ? _size : _dynamic_data->size();
  }

  // O(1) / O(1); nothrow / nothrow
  bool empty() const noexcept {
    return size() == 0;
  }

  // O(1) / O(1); nothrow / nothrow
  std::size_t capacity() const noexcept {
    return small_object() ? SMALL_SIZE : _dynamic_data->capacity();
  }

  // O(1) / O(size); nothrow / strong
  pointer data() {
    if (small_object()) {
      return _static_data.data();
    } else {
      unpin();
      return _dynamic_data->data();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_pointer data() const noexcept {
    if (small_object()) {
      return _static_data.data();
    } else {
      return _dynamic_data->data();
    }
  }

  // Operations

  // O(1) / O(1)*; strong / strong
  void push_back(const T& value) {
    value_type v = value;
    push_back(std::move(v));
  }

  // O(1) / O(1)*; nothrow / strong
  void push_back(T&& value) {
    if (small_object() && !full()) {
      new (_static_data.data() + _size) value_type(std::move(value));
      ++_size;
    } else {
      if (full()) {
        change_storage();
      }
      unpin();
      _dynamic_data->push_back(std::move(value));
    }
  }

  // O(1) / O(1); nothrow / strong
  void pop_back() {
    if (small_object()) {
      data()[_size - 1].~value_type();
      --_size;
    } else {
      unpin();
      _dynamic_data->pop_back();
    }
  }

  // O(1) / O(1)*; strong / strong
  iterator insert(const_iterator pos, const T& value) {
    value_type v = value;
    return insert(pos, std::move(v));
  }

  // O(1) / O(1)*; nothrow / strong
  iterator insert(const_iterator pos, T&& value) {
    std::size_t idx = pos - begin();
    if (small_object() && !full()) {
      new (_static_data.data() + _size) T(std::move(value));
      ++_size;
      iterator it = end() - 1;
      while (it != begin() + idx) {
        std::iter_swap(it - 1, it);
        --it;
      }
      return begin() + idx;
    }
    if (full()) {
      change_storage();
    }
    unpin();
    return _dynamic_data->insert(begin() + idx, std::move(value));
  }

  // O(SMALL_SIZE) / O(N); nothrow / nothrow
  iterator erase(const_iterator pos) noexcept {
    return erase(pos, pos + 1);
  }

  // O(SMALL_SIZE) / O(N); nothrow / nothrow
  iterator erase(const_iterator first, const_iterator last) noexcept {
    if (first == last) {
      return begin() + (last - begin());
    }

    if (small_object()) {
      size_t idx = first - begin();
      iterator left = begin() + (first - begin());
      iterator right = begin() + (last - begin());

      while (right != end()) {
        std::iter_swap(left, right);
        ++right;
        ++left;
      }
      for (; left != end(); ++left) {
        left->~value_type();
      }

      _size -= last - first;
      return begin() + idx;
    }
    unpin();
    return _dynamic_data->erase(first, last);
  }

  // 0(SMALL_SIZE) / 0(size); strong / strong
  void reserve(std::size_t new_capacity) {
    if (small_object() && size() + new_capacity > SMALL_SIZE) {
      vector<T> vec(_static_data, size());
      _dynamic_data._data = nullptr;
      _dynamic_data._count = nullptr;
      _dynamic_data = shared_data<vector<T>>(std::move(vec));
      _dynamic_data->reserve(new_capacity);
      _size = SMALL_SIZE + 1;
    } else if (!small_object()) {
      unpin();
      _dynamic_data->reserve(new_capacity);
    }
  }

  // 0(1) / 0(size); nothrow / strong
  void shrink_to_fit() {
    if (!small_object()) {
      _dynamic_data->shrink_to_fit();
    }
  }

  // O(1) / 0(N); nothrow / nothrow
  void clear() noexcept {
    if (small_object()) {
      std::destroy(begin(), end());
      _size = 0;
    } else {
      unpin();
      _dynamic_data->clear();
    }
  }

  friend void swap(socow_vector& lhs, socow_vector& rhs) noexcept {
    lhs.swap(rhs);
  }

  // O(1) nothrow
  void swap(socow_vector& other) noexcept {
    using std::swap;
    if (small_object() && other.small_object()) {
      swap(_static_data, other._static_data);
    } else {
      if (small_object()) {
        change_storage();
      }
      if (other.small_object()) {
        other.change_storage();
      }
      swap(_dynamic_data, other._dynamic_data);
    }
    swap(_size, other._size);
  }

  // Element access

  // O(1) / O(size); nothrow / strong
  reference operator[](std::size_t index) {
    assert(index < size());
    if (small_object()) {
      return _static_data[index];
    } else {
      unpin();
      return (*_dynamic_data)[index];
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_reference operator[](std::size_t index) const noexcept {
    assert(index < size());
    if (small_object()) {
      return _static_data[index];
    } else {
      return (*_dynamic_data)[index];
    }
  }

  // O(1) / O(size); nothrow / strong
  reference front() {
    return operator[](0);
  }

  // O(1) / O(1); nothrow / nothrow
  const_reference front() const noexcept {
    return operator[](0);
  }

  // O(1) / O(size); nothrow / strong
  reference back() {
    return operator[](size() - 1);
  }

  // O(1) / O(1); nothrow / nothrow
  const_reference back() const noexcept {
    return operator[](size() - 1);
  }

  // Iterator

  // O(1) / O(size); nothrow / strong
  iterator begin() {
    if (small_object()) {
      return _static_data.begin();
    } else {
      unpin();
      return _dynamic_data->begin();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_iterator begin() const noexcept {
    if (small_object()) {
      return _static_data.begin();
    } else {
      return _dynamic_data->begin();
    }
  }

  // O(1) / O(size); nothrow / strong
  iterator end() {
    if (small_object()) {
      iterator begin = _static_data.begin();
      return begin + _size;
    } else {
      unpin();
      return _dynamic_data->end();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_iterator end() const noexcept {
    if (small_object()) {
      const_iterator begin = _static_data.begin();
      std::advance(begin, _size);
      return begin;
    } else {
      return _dynamic_data->end();
    }
  }

public:
  std::size_t _size;

  union {
    shared_data<vector<T>> _dynamic_data;
    std::array<T, SMALL_SIZE> _static_data;
  };

  bool small_object() const noexcept {
    return _size <= SMALL_SIZE;
  }

  bool full() const noexcept {
    return _size == SMALL_SIZE;
  }

  // O(SMALL_SIZE) / 0(1); strong / nothrow
  void change_storage() {
    vector<T> vec(_static_data, _size);
    std::destroy(begin(), end());
    _size = SMALL_SIZE + 1;
    _dynamic_data._data = nullptr;
    _dynamic_data._count = nullptr;
    _dynamic_data = shared_data<vector<T>>(std::move(vec));
  }

  // O(1) / 0(size); nothrow / strong
  void unpin() {
    if (!small_object() && _dynamic_data.use_count() > 1) {
      _dynamic_data = shared_data<vector<T>>(*_dynamic_data);
    }
  }
};
