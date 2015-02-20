#include "type.hpp"

std::map<std::string, type_registry::factory_ptr> type_registry::factories;

template<>
void toJson<std::string>(const std::string& str, rapidjson::Document& document, const char* key) {
  document.AddMember(key, str.c_str(), document.GetAllocator());
}
