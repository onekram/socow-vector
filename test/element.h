#pragma once

#include <ostream>
#include <set>

struct element {
  struct no_new_intances_guard;

  element() = delete;
  element(int data);
  element(const element& other);
  element(element&& other) noexcept;
  ~element();

  element& operator=(const element& other);
  element& operator=(element&& other) noexcept;

  friend bool operator==(const element& lhs, const element& rhs);
  friend bool operator!=(const element& lhs, const element& rhs);

  friend std::ostream& operator<<(std::ostream& out, const element& e);

private:
  void add_instance();
  void delete_instance();
  void assert_exists() const;

private:
  int value;

  inline static std::set<const element*> instances;
};

struct element::no_new_intances_guard {
  no_new_intances_guard();

  no_new_intances_guard(const no_new_intances_guard&) = delete;
  no_new_intances_guard& operator=(const no_new_intances_guard&) = delete;

  ~no_new_intances_guard() noexcept(false);

  bool check_no_new_instances() const noexcept;

private:
  std::set<const element*> old_instances;
};
