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
      : _size(other.size()) {
    if (other.small_object()) {
      _data._static_data = other._data._static_data;
    } else {
      _data._dynamic_data = other._data._dynamic_data;
    }
  }

  socow_vector(socow_vector&& other) noexcept {
      swap(other);
  }

  // O(SMALL_SIZE) / O(1); strong / nothrow
  socow_vector& operator=(const socow_vector& other) {
    if (this != &other) {
      socow_vector copy(other);
      swap(copy);
    }
    return *this;
  }

  // ???
  ~socow_vector() {
    clear();
  }
  // Fields access

  std::size_t size() const {
    return small_object() ? _size : _data._dynamic_data->size();
  }

  bool empty() const {
    return size() == 0;
  }

  std::size_t capacity() const {
    return small_object() ? SMALL_SIZE : _data._dynamic_data->capacity();
  }

  // O(1) / O(size); nothrow / strong
  pointer data() {
    if (small_object()) {
      return _data._static_data.data();
    } else {
      if (_data._dynamic_data.use_count() > 1) {
        vector<T> vec = *_data._dynamic_data;
        _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      }
      return _data._dynamic_data->data();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_pointer data() const noexcept {
    if (small_object()) {
      return _data._static_data.data();
    } else {
      return _data._dynamic_data->data();
    }
  }
  // Operations

  // O(1) / O(1)*; nothrow / strong
  void push_back(const T& value) {
    value_type v = value;
    push_back(std::move(v));
  }

  // O(1) / O(1)*; nothrow / strong
  void push_back(T&& value) {
    if (full()) {
      vector<T> vec(_data._static_data, _size);
      std::destroy(begin(), end());
      ++_size;
      vec.push_back(std::move(value));
      _data._dynamic_data._data = nullptr;
      _data._dynamic_data._count = nullptr;
      _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
    } else if (small_object()) {
      new (_data._static_data.data() + _size) T(std::move(value));
      ++_size;
    } else {
      if (_data._dynamic_data.use_count() > 1) {
        vector<T> vec = *_data._dynamic_data;
        _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      }
      _data._dynamic_data->push_back(std::move(value));
    }
  }


  // 0(SMALL_SIZE) / 0(size); strong / strong
  void reserve(std::size_t new_capacity) {
    if (small_object() && size() + new_capacity > SMALL_SIZE) {
      vector<T> vec(_data._static_data, size());
      _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      _data._dynamic_data->reserve(new_capacity);
      _size = SMALL_SIZE + 1;
    } else if (!small_object()) {
      if (_data._dynamic_data.use_count() > 1) {
        vector<T> vec = *_data._dynamic_data;
        _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      }
      _data._dynamic_data->reserve(new_capacity);
    }
  }

  // 0(1) / 0(size); nothrow / strong
  void shrink_to_fit() {
    if (!small_object()) {
      _data._dynamic_data->shrink_to_fit();
    }
  }

  // O(1) / 0(N); nothrow / nothrow
  void clear() noexcept {
    if (small_object()) {
      std::destroy(begin(), end());
      _size = 0;
    } else {
      if (_data._dynamic_data.use_count() > 1) {
        vector<T> vec = *_data._dynamic_data;
        _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      }
      _data._dynamic_data->clear();
    }
  }

  // O(1) nothrow
  void swap(socow_vector& other) noexcept {
    using std::swap;
    if (small_object() && other.small_object()) {
      swap(_data._static_data, other._data._static_data);
    } else {
      if (small_object()) {
        vector<T> vec(_data._static_data, _size);
        _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      } else if (other.small_object()) {
        vector<T> vec(other._data._static_data, other._size);
        other._data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      }
      swap(_data._dynamic_data, other._data._dynamic_data);
    }
    swap(_size, other._size);
  }

  // Element access

  // O(1) / O(size); nothrow / strong
  reference operator[](std::size_t index) {
    assert(index < size());
    if (small_object()) {
      return _data._static_data[index];
    } else {
      if (_data._dynamic_data.use_count() > 1) {
        vector<T> vec = *_data._dynamic_data;
        _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      }
      return (*_data._dynamic_data)[index];
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_reference operator[](std::size_t index) const noexcept {
    assert(index < size());
    if (small_object()) {
      return _data._static_data[index];
    } else {
      return (*_data._dynamic_data)[index];
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
      return _data._static_data.begin();
    } else {
      if (_data._dynamic_data.use_count() > 1) {
        vector<T> vec = *_data._dynamic_data;
        _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      }
      return _data._dynamic_data->begin();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_iterator begin() const noexcept {
    if (small_object()) {
      return _data._static_data.begin();
    } else {
      return _data._dynamic_data->begin();
    }
  }

  // O(1) / O(size); nothrow / strong
  iterator end() {
    if (small_object()) {
      iterator begin = _data._static_data.begin();
      std::advance(begin, _size);
      return begin;
    } else {
      if (_data._dynamic_data.use_count() > 1) {
        vector<T> vec = *_data._dynamic_data;
        _data._dynamic_data = shared_data<vector<T>>(std::move(vec));
      }
      return _data._dynamic_data->end();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_iterator end() const noexcept {
    if (small_object()) {
      const_iterator begin = _data._static_data.begin();
      std::advance(begin, _size);
      return begin;
    } else {
      return _data._dynamic_data->end();
    }
  }

public:
  std::size_t _size;

  union small_data {
    small_data() {}
    ~small_data() {}

    shared_data<vector<T>> _dynamic_data;
    std::array<T, SMALL_SIZE> _static_data;
  };
  
  small_data _data;

  bool small_object() const {
    return _size <= SMALL_SIZE;
  }
  bool full() const {
    return _size == SMALL_SIZE;
  }
};
