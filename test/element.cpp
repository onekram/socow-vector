#include "element.h"

#include <catch2/catch_test_macros.hpp>

#include <utility>

element::element(int data)
    : value(data) {
  add_instance();
}

element::element(const element& other)
    : value(-1) {
  other.assert_exists();
  add_instance();
  value = other.value;
}

element::element(element&& other) noexcept
    : value(-1) {
  other.assert_exists();
  add_instance();
  value = std::exchange(other.value, -1);
}

element::~element() {
  delete_instance();
}

element& element::operator=(const element& other) {
  assert_exists();
  other.assert_exists();

  value = other.value;
  return *this;
}

element& element::operator=(element&& other) noexcept {
  assert_exists();
  other.assert_exists();

  value = std::exchange(other.value, -1);
  return *this;
}

bool operator==(const element& lhs, const element& rhs) {
  lhs.assert_exists();
  rhs.assert_exists();

  return lhs.value == rhs.value;
}

bool operator!=(const element& lhs, const element& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const element& e) {
  e.assert_exists();
  out << e.value;
  return out;
}

void element::add_instance() {
  auto p = instances.insert(this);
  if (!p.second) {
    // clang-format off
    FAIL(
        "A new object is created at the address "
        << static_cast<void*>(this)
        << " while the previous object at this address was not destroyed"
    );
    // clang-format on
  }
}

void element::delete_instance() {
  std::size_t erased = instances.erase(this);
  if (erased != 1) {
    FAIL("Attempt of destroying non-existing object at address " << static_cast<void*>(this));
  }
}

void element::assert_exists() const {
  bool exists = instances.find(this) != instances.end();
  if (!exists) {
    FAIL("Accessing a non-existing object at address " << static_cast<const void*>(this));
  }
}

element::no_new_intances_guard::no_new_intances_guard()
    : old_instances(instances) {}

element::no_new_intances_guard::~no_new_intances_guard() noexcept(false) {
  if (std::uncaught_exceptions() == 0) {
    bool no_new_instances = check_no_new_instances();
    instances = old_instances;
    REQUIRE(no_new_instances);
  } else {
    instances = old_instances;
  }
}

bool element::no_new_intances_guard::check_no_new_instances() const noexcept {
  return old_instances == instances;
}
