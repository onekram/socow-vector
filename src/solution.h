#pragma once

#include <stdexcept>

inline void throwing_func() {
  throw std::logic_error("some exception");
}
