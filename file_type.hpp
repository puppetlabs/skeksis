#pragma once
#include "type.hpp"

#include <string>

namespace file {
struct mode_tag {
  static const char* name() { return "mode"; }
};
struct name_tag {
  static const char* name() { return "name"; }
};
using name = param<std::string, name_tag>;
struct mode : public param<int, mode_tag> {
    bool isValid() const {
      bool large_enough = value > 0;
      bool small_enough = value <= 07777;
      return large_enough && small_enough;
    };
  };

  using type = ::type<name, mode>;
}
