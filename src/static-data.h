#pragma once

#include <cstddef>

template <typename T, std::size_t SIZE>
class static_data {
public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

public:
};