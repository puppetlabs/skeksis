#include "type.hpp"
#include "file_type.hpp"
#include <iostream>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "incorrect number of arguments" << std::endl;
    exit(1);
  }
  auto users = type_registry::instances(argv[1]);
  for(const auto& u : users) {
    std::cout << u->toJson() << std::endl << std::endl;
  };
}
