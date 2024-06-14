#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>

#include "vector.h"

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
  socow_vector() : _size(0), _buffer(nullptr) {}

  // O(SMALL_SIZE) / O(1); strong / nothrow
  socow_vector(const socow_vector& other)
      : _size(other.size()), _buffer(nullptr) {
    if (other.is_small_object()) {
      _data = other._data;
//      std::copy(std::begin(other._data), std::end(other._data), std::begin(_data))
    } else {
      _buffer = other._buffer;
    }
  }

  socow_vector(socow_vector&& other) noexcept
      : _size(other._size)
      , _buffer(other._buffer) {

    std::swap(_data, other._data);
    other._buffer = nullptr;
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
    if (is_small_object()) {
      for(iterator it = begin(); it != end(); ++it) {
        it->~value_type();
      }
    }
  }

  // Fields access

  std::size_t size() const {
    return is_small_object() ? _size : _buffer->size();
  }

  bool empty() const {
    return size() == 0;
  }

  std::size_t capacity() const {
    return is_small_object() ? SMALL_SIZE : _buffer->capacity();
  }

  // O(1) / O(size); nothrow / strong
  pointer data() {
    if (is_small_object()) {
      return _data.data();
    } else {
      if (_buffer.use_count() > 1) {
        vector<T> vec = *_buffer;
        _buffer = std::make_shared<vector<T>>(vec);
      }
      return _buffer->data();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_pointer data() const noexcept {
    if (is_small_object()) {
      return _data.data();
    } else {
      return _buffer->data();
    }
  }

  void destroy_small() {
    for(std::size_t i = 0; i < SMALL_SIZE; ++i) {
      _data[i].~value_type();
    }
  }

  // Operations

  // O(1) / O(1)*; nothrow / strong
  void push_back(const T& value) {
    if (is_full()) {
      vector<T> vec(_data, _size);
//      destroy_small();
      ++_size;
      vec.push_back(value);
      _buffer = std::make_shared<vector<T>>(std::move(vec));
    } else if (is_small_object()) {
      _data[_size++] = value;
    } else {
      if (_buffer.use_count() > 1) {
        vector<T> vec = *_buffer;
        _buffer = std::make_shared<vector<T>>(vec);
      }
      _buffer->push_back(value);
    }
  }

  // 0(SMALL_SIZE) / 0(size); strong / strong
  void reserve(std::size_t new_capacity) {
    if (is_small_object() && _size + new_capacity > SMALL_SIZE) {
      vector<T> vec(_data, _size);
      _buffer = std::make_shared<vector<T>>(vec);
      _buffer->reserve(new_capacity);
    } else if (!is_small_object()) {
      if (_buffer.use_count() > 1) {
        vector<T> vec = *_buffer;
        _buffer = std::make_shared<vector<T>>(vec);
      }
      _buffer->reserve(new_capacity);
    }
  }

  // 0(1) / 0(size); nothrow / strong
  void shrink_to_fit() {
    if (!is_small_object()) {
      _buffer->shrink_to_fit();
    }
  }

  // O(1) / 0(N); nothrow / nothrow
  void clear() noexcept {
    if (!is_small_object()) {
      if (_buffer.use_count() > 1) {
        vector<T> vec = *_buffer;
        _buffer = std::make_shared<vector<T>>(vec);
      }
      _buffer->clear();
    }
    _size = 0;
  }

  // O(1) nothrow
  void swap(socow_vector& other) noexcept {
    std::swap(_size, other._size);
    std::swap(_buffer, other._buffer);
    std::swap(_data, other._data);
  }

  // Element access

  // O(1) / O(size); nothrow / strong
  reference operator[](std::size_t index) {
    assert(index < size());
    if (is_small_object()) {
      return _data[index];
    } else {
      if (_buffer.use_count() > 1) {
        vector<T> vec = *_buffer;
        _buffer = std::make_shared<vector<T>>(vec);
      }
      return (*_buffer)[index];
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_reference operator[](std::size_t index) const noexcept {
    assert(index < size());
    if (is_small_object()) {
      return _data[index];
    } else {
      return (*_buffer)[index];
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
    if (is_small_object()) {
      return _data.begin();
    } else {
      if (_buffer.use_count() > 1) {
        vector<T> vec = *_buffer;
        _buffer = std::make_shared<vector<T>>(vec);
      }
      return _buffer->begin();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_iterator begin() const noexcept {
    if (is_small_object()) {
      return _data.begin();
    } else {
      return _buffer->begin();
    }
  }

  // O(1) / O(size); nothrow / strong
  iterator end() {
    if (is_small_object()) {
      iterator begin = _data.begin();
      std::advance(begin, _size);
      return begin;
    } else {
      if (_buffer.use_count() > 1) {
        vector<T> vec = *_buffer;
        _buffer = std::make_shared<vector<T>>(vec);
      }
      return _buffer->end();
    }
  }

  // O(1) / O(1); nothrow / nothrow
  const_iterator end() const noexcept {
    if (is_small_object()) {
      const_iterator begin = _data.begin();
      std::advance(begin, _size);
      return begin;
    } else {
      return _buffer->end();
    }
  }

public:
  std::size_t _size;

  union {
    std::shared_ptr<vector<T>> _buffer;
    std::array<T, SMALL_SIZE> _data;
//    T _data[SMALL_SIZE];
  };

  bool is_small_object() const {
    return _size <= SMALL_SIZE;
  }
  bool is_full() const {
    return _size == SMALL_SIZE;
  }
};
