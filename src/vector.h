#pragma once

#include <algorithm>
#include <cstddef>
#include <new>

template <typename T>
class vector {
public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

public:
  // O(1) nothrow
  vector() noexcept
      : _capacity(0)
      , _size(0)
      , _data(nullptr) {}

  // O(N) strong
  vector(const vector& other)
      : _capacity(other.size())
      , _size(other.size())
      , _data(nullptr) {
    if (_capacity > 0) {
      auto tmp = create_tmp(other, other.capacity());
      _data = tmp;
    }
  }

  // O(1) strong
  vector(vector&& other) noexcept
      : _capacity(other.size())
      , _size(other.size())
      , _data(other.data()) {
    other._capacity = 0;
    other._size = 0;
    other._data = nullptr;
  }

  // O(N) strong
  template <typename Array>
  vector(const Array& other, std::size_t new_capacity)
      : _capacity(other.size())
      , _size(other.size())
      , _data(nullptr) {
    if (_capacity > 0) {
      auto tmp = create_tmp(other, new_capacity);
      _data = tmp;
    }
  }

  // O(N) strong
  template <typename Array>
  vector(Array&& other, std::size_t new_size)
      : _capacity(new_size)
      , _size(new_size)
      , _data(nullptr) {
    if (_capacity > 0) {
      auto tmp = create_tmp(std::forward<Array>(other), new_size);
      _data = tmp;
    }
  }

  // O(N) strong
  vector& operator=(const vector& other) {
    if (this == &other) {
      return *this;
    }
    vector(other).swap(*this);
    return *this;
  }

  // O(1) strong
  vector& operator=(vector&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    swap(other);
    return *this;
  }

  // O(N) nothrow
  ~vector() noexcept {
    data_clear(data(), size());
  }

  // O(1) nothrow
  reference operator[](size_t index) {
    return *(_data + index);
  }

  // O(1) nothrow
  const_reference operator[](size_t index) const {
    return *(_data + index);
  }

  // O(1) nothrow
  pointer data() noexcept {
    return _data;
  }

  // O(1) nothrow
  const_pointer data() const noexcept {
    return _data;
  }

  // O(1) nothrow
  size_t size() const noexcept {
    return _size;
  }

  // O(1) nothrow
  reference front() {
    return *(data());
  }

  // O(1) nothrow
  const_reference front() const {
    return *(data());
  }

  // O(1) nothrow
  reference back() {
    return *(data() + size() - 1);
  }

  // O(1) nothrow
  const_reference back() const {
    return *(data() + size() - 1);
  }

  // O(1)* strong
  void push_back(const T& value) {
    value_type v = value;
    push_back(v);
  }

  // O(1)* strong
  void push_back(T&& value) {
    if (_size == _capacity) {
      auto tmp = create_tmp(std::move(*this), _capacity * 2 + 1);
      try {
        new (tmp + size()) value_type(std::move(value));
      } catch (...) {
        data_clear(tmp, size());
        throw;
      }
      data_clear(data(), size());
      _data = tmp;
      _capacity = _capacity * 2 + 1;
      ++_size;
    } else {
      new (data() + size()) value_type(std::move(value));
      ++_size;
    }
  }

  // O(1) nothrow
  void pop_back() noexcept {
    data()[size() - 1].~value_type();
    --_size;
  }

  // O(1) nothrow
  bool empty() const noexcept {
    return size() == 0;
  }

  // O(1) nothrow
  size_t capacity() const noexcept {
    return _capacity;
  }

  // O(N) strong
  void reserve(size_t new_capacity) {
    if (new_capacity > capacity()) {
      change_capacity(new_capacity);
    }
  }

  // O(N) strong
  void shrink_to_fit() {
    if (size() < capacity()) {
      change_capacity(size());
    }
  }

  // O(N) nothrow
  void clear() noexcept {
    for (size_t j = _size; j > 0; --j) {
      _data[j - 1].~T();
    }
    _size = 0;
  }

  // O(1) nothrow
  void swap(vector& other) noexcept {
    std::swap(_capacity, other._capacity);
    std::swap(_size, other._size);
    std::swap(_data, other._data);
  }

  // O(1) nothrow
  iterator begin() noexcept {
    return data();
  }

  // O(1) nothrow
  iterator end() noexcept {
    return data() + size();
  }

  // O(1) nothrow
  const_iterator begin() const noexcept {
    return data();
  }

  // O(1) nothrow
  const_iterator end() const noexcept {
    return data() + size();
  }

  // O(N) strong
  iterator insert(const_iterator pos, const T& value) {
    value_type v = value;
    return insert(pos, std::move(v));
  }

  // O(N) strong
  iterator insert(const_iterator pos, T&& value) {
    size_t idx = pos - begin();

    push_back(std::move(value));

    iterator it = end() - 1;
    while (it != begin() + idx) {
      std::iter_swap(it - 1, it);
      --it;
    }
    return begin() + idx;
  }

  // O(N) nothrow(swap)
  iterator erase(const_iterator pos) noexcept {
    return erase(pos, pos + 1);
  }

  // O(N) nothrow(swap)
  iterator erase(const_iterator first, const_iterator last) noexcept {
    if (first == last) {
      return begin() + (last - begin());
    }
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

private:
  size_t _capacity;
  size_t _size;
  pointer _data;

  // O(N) nothrow
  static void data_clear(pointer memory, size_t idx) noexcept {
    for (size_t j = idx; j > 0; --j) {
      memory[j - 1].~value_type();
    }
    operator delete(memory);
  }

  // O(N) strong
  template <typename Array>
  static pointer create_tmp(const Array& source, size_t new_capacity) {
    auto tmp = static_cast<pointer>(operator new(sizeof(value_type) * new_capacity));
    size_t i = 0;
    try {
      for (auto it = source.begin(); it != source.end() && i < new_capacity; ++i, ++it) {
        new (tmp + i) value_type(*it);
      }
    } catch (...) {
      data_clear(tmp, i);
      throw;
    }
    return tmp;
  }

  // O(N) strong
  template <typename Array>
  static pointer create_tmp(Array&& source, size_t new_capacity) {
    auto tmp = static_cast<pointer>(operator new(sizeof(value_type) * new_capacity));
    size_t i = 0;
    for (auto it = source.begin(); it != source.end() && i < new_capacity; ++i, ++it) {
      new (tmp + i) value_type(std::move(*it));
    }
    return tmp;
  }

  // O(N) strong
  void change_capacity(size_t new_capacity) {
    auto tmp = create_tmp(*this, new_capacity);
    data_clear(data(), size());
    _data = tmp;
    _capacity = new_capacity;
  }
};
