#include "socow-vector.h"
#include "test-utils.h"

#include <catch2/catch_test_macros.hpp>

#include <utility>

TEST_CASE("Default constructor") {
  element::no_new_intances_guard ig;

  socow_vector<element, 3> a;
  assert_empty_storage(a);
  REQUIRE(ig.check_no_new_instances());
}

TEST_CASE("Push back") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;
  socow_vector<element, 3> a;

  for (std::size_t i = 0; i < N; ++i) {
    a.push_back(2 * i + 1);
  }

  REQUIRE(a.size() == N);
  REQUIRE(a.capacity() >= N);

  for (std::size_t i = 0; i < N; ++i) {
    CAPTURE(i);
    REQUIRE(a[i] == 2 * i + 1);
  }
}

TEST_CASE("Push back xvalue") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;
  socow_vector<element, 3> a;

  for (std::size_t i = 0; i < N; ++i) {
    element x = 2 * i + 1;
    a.push_back(std::move(x));
    REQUIRE(x == -1);
  }

  REQUIRE(a.size() == N);
  REQUIRE(a.capacity() >= N);

  for (std::size_t i = 0; i < N; ++i) {
    CAPTURE(i);
    REQUIRE(a[i] == 2 * i + 1);
  }
}

TEST_CASE("Push back from self") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;
  socow_vector<element, 3> a;

  a.push_back(42);
  for (std::size_t i = 1; i < N; ++i) {
    a.push_back(a[i - 1]);
  }

  REQUIRE(a.size() == N);
  REQUIRE(a.capacity() >= N);

  for (std::size_t i = 0; i < N; ++i) {
    CAPTURE(i);
    REQUIRE(a[i] == 42);
  }
}

TEST_CASE("Subscript") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50, K = 10;

  socow_vector<element, 3> a;
  mass_push_back(a, N);

  for (std::size_t i = 0; i < K; ++i) {
    a[i] = 4 * i + 1;
  }

  SECTION("non-const") {
    for (std::size_t i = 0; i < K; ++i) {
      CAPTURE(i);
      REQUIRE(a[i] == 4 * i + 1);
    }
    for (std::size_t i = K; i < N; ++i) {
      CAPTURE(i);
      REQUIRE(a[i] == 2 * i + 1);
    }
  }
  SECTION("const") {
    for (std::size_t i = 0; i < K; ++i) {
      CAPTURE(i);
      REQUIRE(std::as_const(a)[i] == 4 * i + 1);
    }
    for (std::size_t i = K; i < N; ++i) {
      CAPTURE(i);
      REQUIRE(std::as_const(a)[i] == 2 * i + 1);
    }
  }
}

TEST_CASE("Access data") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);

  SECTION("non-const") {
    element* data = a.data();
    REQUIRE(data == &a[0]);

    for (std::size_t i = 0; i < N; ++i) {
      CAPTURE(i);
      REQUIRE(data[i] == 2 * i + 1);
    }
  }
  SECTION("const") {
    const element* cdata = std::as_const(a).data();
    REQUIRE(cdata == &std::as_const(a)[0]);

    for (std::size_t i = 0; i < N; ++i) {
      CAPTURE(i);
      REQUIRE(cdata[i] == 2 * i + 1);
    }
  }
}

TEST_CASE("Access front/back element") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);

  SECTION("non-const") {
    REQUIRE(a.front() == 1);
    REQUIRE(&a.front() == &a[0]);

    REQUIRE(a.back() == 2 * N - 1);
    REQUIRE(&a.back() == &a[N - 1]);
  }

  SECTION("const") {
    const socow_vector<element, 3>& ca = a;

    REQUIRE(ca.front() == 1);
    REQUIRE(&ca.front() == &ca[0]);

    REQUIRE(ca.back() == 2 * N - 1);
    REQUIRE(&ca.back() == &ca[N - 1]);
  }
}

TEST_CASE("Reserve capacity") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 10, M = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);

  a.reserve(M);
  snapshot s(a);

  for (std::size_t i = N; i < M; ++i) {
    a.push_back(2 * i + 1);
  }

  REQUIRE(a.size() == M);
  REQUIRE(a.data() == s.data);
  REQUIRE(a.capacity() == s.capacity);
}

TEST_CASE("Shrink capacity") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50, M = 10;

  socow_vector<element, 3> a;
  a.reserve(N);
  mass_push_back(a, M);

  REQUIRE(a.size() == M);
  REQUIRE(a.capacity() == N);

  snapshot s(a);

  a.shrink_to_fit();

  REQUIRE(a.capacity() == M);
  s.verify(a);
}

TEST_CASE("Clear") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);

  {
    snapshot s(a);

    a.clear();

    REQUIRE(a.empty());
    REQUIRE(a.size() == 0);
    REQUIRE(a.capacity() == s.capacity);
    REQUIRE(a.data() == s.data);
  }

  REQUIRE(ig.check_no_new_instances());
}

TEST_CASE("Swap") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 4, M = 7;

  socow_vector<element, 3> a;
  for (std::size_t i = 1; i <= N; ++i) {
    a.push_back(i);
  }

  socow_vector<element, 3> b;
  for (std::size_t i = N; i <= N + M; ++i) {
    b.push_back(i);
  }

  snapshot s(a, b);
  a.swap(b);
  s.full_verify(b, a);
}

TEST_CASE("Copy constructor") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);

  socow_vector<element, 3> b = a;
  REQUIRE(b.size() == a.size());

  for (std::size_t i = 0; i < N; ++i) {
    CAPTURE(i);
    REQUIRE(b[i] == 2 * i + 1);
  }
}

TEST_CASE("Copy assignment") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50, K = 10;

  socow_vector<element, 3> a;
  mass_push_back(a, N);
  snapshot s(a);

  socow_vector<element, 3> b;
  mass_push_back(b, K);

  a = a;
  b = a;

  s.verify(a);
  s.verify(b);
}

TEST_CASE("Move constructor") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);

  snapshot s(a);

  socow_vector<element, 3> b = std::move(a);

  s.full_verify(b);
  assert_empty_storage(a);
}

TEST_CASE("Move assignment") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);
  snapshot s(a);

  static constexpr std::size_t K = 50;

  socow_vector<element, 3> b;
  mass_push_back(b, K);

  a = std::move(a);
  b = std::move(a);

  s.verify(b);
}

TEST_CASE("Pop back") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);

  {
    snapshot s(a);

    for (std::size_t i = N; i > 0; --i) {
      CAPTURE(i);
      REQUIRE(a.back() == 2 * (i - 1) + 1);
      REQUIRE(a.size() == i);
      a.pop_back();
    }

    REQUIRE(a.empty());
    REQUIRE(a.size() == 0);
    REQUIRE(a.capacity() == s.capacity);
    REQUIRE(a.data() == s.data);
  }

  REQUIRE(ig.check_no_new_instances());
}

TEST_CASE("Insert") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50, K = 10;

  socow_vector<element, 3> a;
  for (std::size_t i = 0; i < N; ++i) {
    a.push_back(2 * i + 1);
  }

  socow_vector<element, 3>::const_iterator it = a.insert(a.begin() + K, 42);

  REQUIRE(it == a.begin() + K);
  REQUIRE(*it == 42);

  REQUIRE(a.size() == N + 1);
  REQUIRE(a.capacity() >= N + 1);

  for (std::size_t i = 0; i < K; ++i) {
    CAPTURE(i);
    REQUIRE(a[i] == 2 * i + 1);
  }

  REQUIRE(a[K] == 42);

  for (std::size_t i = K; i < N; ++i) {
    CAPTURE(i);
    REQUIRE(a[i + 1] == 2 * i + 1);
  }
}

TEST_CASE("Erase single") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50, POS = 10;

  socow_vector<element, 3> a;
  mass_push_back(a, N);
  snapshot s(a);

  auto it = a.erase(a.begin() + POS);
  REQUIRE(it == a.begin() + POS);
  REQUIRE(a.size() == N - 1);
  REQUIRE(a.capacity() == s.capacity);
  REQUIRE(a.data() == s.data);

  for (size_t i = 0; i < POS; ++i) {
    CAPTURE(i);
    REQUIRE(a[i] == 2 * i + 1);
  }
  for (size_t i = POS; i < a.size(); ++i) {
    CAPTURE(i);
    REQUIRE(a[i] == 2 * (i + 1) + 1);
  }
}

TEST_CASE("Erase range") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50, K = 10;

  socow_vector<element, 3> a;
  mass_push_back(a, N);
  snapshot s(a);

  auto it = a.erase(a.begin() + K, a.end() - K);
  REQUIRE(it == a.begin() + K);
  REQUIRE(a.size() == 2 * K);
  REQUIRE(a.capacity() == s.capacity);
  REQUIRE(a.data() == s.data);

  for (std::size_t i = 0; i < K; ++i) {
    CAPTURE(i);
    REQUIRE(a[i] == 2 * i + 1);
  }
  for (std::size_t i = K; i < a.size(); ++i) {
    CAPTURE(i);
    REQUIRE(a[i] == 2 * (i + (N - 2 * K)) + 1);
  }
}

TEST_CASE("Range-based for") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  for (std::size_t i = 0; i < N; ++i) {
    a.push_back(42);
  }

  std::size_t i = 0;
  for (element& e : a) {
    e = i * 2 + 1;
    ++i;
  }

  i = 0;
  for (const element& e : a) {
    CAPTURE(i);
    REQUIRE(e == i * 2 + 1);
    ++i;
  }
}

TEST_CASE("Small Object Optimization") {
  element::no_new_intances_guard ig;

  socow_vector<element, 3> a;

  a.push_back(42);
  a.push_back(43);
  a.push_back(44);
  REQUIRE(is_static_storage(a));

  a.push_back(45);
  REQUIRE_FALSE(is_static_storage(a));
}

TEST_CASE("Copy-on-write") {
  element::no_new_intances_guard ig;

  static constexpr std::size_t N = 50;

  socow_vector<element, 3> a;
  mass_push_back(a, N);
  snapshot s(a);

  socow_vector<element, 3> b = a;
  socow_vector<element, 3> c = a;

  s.full_verify(b);
  s.full_verify(c);

  b[0] = 42;

  REQUIRE(std::as_const(a[0]) == 1);
  REQUIRE(std::as_const(b[0]) == 42);
  REQUIRE(std::as_const(c[0]) == 1);
}
