#include "socow-vector.h"
#include "test-utils.h"

#include <catch2/catch_test_macros.hpp>

#include <iterator>
#include <ranges>
#include <type_traits>

TEST_CASE("Member types") {
  STATIC_REQUIRE(std::is_same_v<socow_vector<element, 3>::value_type, element>);
  STATIC_REQUIRE(std::is_same_v<socow_vector<element, 3>::reference, element&>);
  STATIC_REQUIRE(std::is_same_v<socow_vector<element, 3>::const_reference, const element&>);
  STATIC_REQUIRE(std::is_same_v<socow_vector<element, 3>::pointer, element*>);
  STATIC_REQUIRE(std::is_same_v<socow_vector<element, 3>::const_pointer, const element*>);
  STATIC_REQUIRE(std::is_same_v<socow_vector<element, 3>::iterator, element*>);
  STATIC_REQUIRE(std::is_same_v<socow_vector<element, 3>::const_iterator, const element*>);
}

TEST_CASE("Vector is contiguous") {
  STATIC_REQUIRE(std::contiguous_iterator<socow_vector<element, 3>::iterator>);
  STATIC_REQUIRE(std::contiguous_iterator<socow_vector<element, 3>::const_iterator>);

  STATIC_REQUIRE(std::ranges::contiguous_range<socow_vector<element, 3>>);
}
