#pragma once

#include "element.h"
#include "socow-vector.h"

#include <catch2/catch_test_macros.hpp>

template class socow_vector<int, 3>;
template class socow_vector<element, 3>;
template class socow_vector<element, 10>;

template <typename... Ts>
class snapshot {
public:
  explicit snapshot(const Ts&... objects)
      : snapshots(objects...) {}

  snapshot(const snapshot&) = delete;

  void full_verify(const Ts&... objects) const {
    std::apply([&](const snapshot<Ts>&... snapshots) { (snapshots.full_verify(objects), ...); }, snapshots);
  }

  void verify(const Ts&... objects) const {
    std::apply([&](const snapshot<Ts>&... snapshots) { (snapshots.verify(objects), ...); }, snapshots);
  }

private:
  std::tuple<snapshot<Ts>...> snapshots;
};

template <>
class snapshot<element> {
public:
  explicit snapshot(const element& e)
      : value(e) {}

  snapshot(const snapshot&) = delete;

  void full_verify(const element& other) const {
    verify(other);
  }

  void verify(const element& other) const {
    REQUIRE(value == other);
  }

  element value;
};

template <std::size_t SMALL_SIZE>
class snapshot<socow_vector<element, SMALL_SIZE>> {
public:
  explicit snapshot(const socow_vector<element, SMALL_SIZE>& a)
      : capacity(a.capacity())
      , data(a.data())
      , elements(a.begin(), a.end()) {}

  snapshot(const snapshot&) = delete;

  void full_verify(const socow_vector<element, SMALL_SIZE>& other) const {
    REQUIRE(other.capacity() == capacity);
    REQUIRE(other.data() == data);
    verify(other);
  }

  void verify(const socow_vector<element, SMALL_SIZE>& other) const {
    REQUIRE(other.size() == elements.size());
    for (std::size_t i = 0; i < elements.size(); ++i) {
      CAPTURE(i);
      REQUIRE(other[i] == elements[i]);
    }
  }

  std::size_t capacity;
  const element* data;
  std::vector<element> elements;
};

template <std::size_t SMALL_SIZE>
bool is_static_storage(const socow_vector<element, SMALL_SIZE>& a) {
  if (a.capacity() != SMALL_SIZE) {
    return false;
  }
  const element* data = a.data();
  return std::less_equal<const void*>{}(&a, data) && std::greater<const void*>{}(&a + 1, data);
}

template <std::size_t SMALL_SIZE>
void assert_empty_storage(const socow_vector<element, SMALL_SIZE>& a) {
  REQUIRE(a.empty());
  REQUIRE(a.size() == 0);
  REQUIRE(is_static_storage(a));
}

template <std::size_t SMALL_SIZE>
void mass_push_back(socow_vector<element, SMALL_SIZE>& a, std::size_t size) {
  for (std::size_t i = 0; i < size; ++i) {
    a.push_back(2 * i + 1);
  }
}
