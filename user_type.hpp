#pragma once
#include "type.hpp"

#include <string>

namespace user {

  PARAM(name, std::string);
  PARAM(uid, int);
  PARAM(gid, int);

  using type = ::type<name, uid, gid>;
}
