#include "user_type.hpp"
#include "file_type.hpp"
#include "provider.hpp"

#include <pwd.h>

class user_posix {
public:
  static std::vector<type_ptr> instances() {
    std::vector<type_ptr> result;
    while(auto passwd = getpwent()) {
      auto instance = std::unique_ptr<user::type>(new user::type);
      instance->param(user::name()).set(passwd->pw_name);
      instance->param(user::uid()).set(passwd->pw_uid);
      instance->param(user::gid()).set(passwd->pw_gid);
      result.push_back(std::move(instance));
    };
    endpwent();
    return result;
  };
};


REGISTER_PROVIDER(suser, user_posix)
