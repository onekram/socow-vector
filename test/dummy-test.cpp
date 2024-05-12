#include "solution.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

TEST_CASE("ABI") {
  REQUIRE_THROWS_AS(throwing_func(), std::logic_error);
}
