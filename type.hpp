#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

template <typename T>
void toJson(const T& v, rapidjson::Document& document, const char* key) {
  document.AddMember(key, v, document.GetAllocator());
}

template<>
void toJson<std::string>(const std::string& str, rapidjson::Document& document, const char* key);

// Define the "parameter" class. Right now this just has getters and
// setters. It will eventually do validation, munging,
// (de)serialization, etc. If you have special needs for validation,
// etc you can simply subclass this and shadow methods. It's all
// staticly resolved by the compiler, so there is no need for these to
// be virtual.
template<typename data_type, typename name_tag> class param {
public:
  using type = data_type;
  type value;
  const char* name() const { return name_tag::name(); }
  const data_type& get() const {
    return value;
  };

  // There's no reason this can't be templatized to support perfect
  // forwarding, I'm just lazy.
  void set(type v) {
    value = v;
  };

  bool isValid() const {
    return true;
  };
};

// The base type, with no parameters. This should have base virtual
// methods for dealing with (de)serialization and general interfacing
// with the outside world, but right now it's empty.
template<typename... parameters> class type;
template<> class type<> {
public:
  void param() const {};

  virtual bool isValid() const {
    return true;
  }

  virtual void encodeParameters(rapidjson::Document&) const {};

  std::string toJson() const {
    rapidjson::Document document;
    document.SetObject();
    encodeParameters(document);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return std::string(buffer.GetString());
  }
};

// This is the meat of this mess. This is a recursive template that
// creates accessors for each parameter of the type. In the future it
// will also define (de)serialization methods through a similar
// recursive process.
template<typename parameter, typename... rest>
class type<parameter, rest...> : public type<rest...> {
public:
  using type<rest...>::param;

  const parameter& param(parameter) const {
    return value;
  }

  parameter& param(parameter) {
    return value;
  };

  virtual bool isValid() const {
    return value.isValid() && type<rest...>::isValid();
  }

  virtual void encodeParameters(rapidjson::Document& document) const {
    toJson<typename parameter::type>(value.get(), document, value.name());
    type<rest...>::encodeParameters(document);
  };
  static int obj;
private:
  parameter value;
};

template <typename parameter, typename... rest>
int type<parameter, rest...>::obj = 0;

using type_ptr = std::unique_ptr<type<>>;

class type_registry {
public:
  class factory_base {
  public:
    virtual type_ptr create() const =0;
    virtual std::vector<type_ptr> instances() const =0;
  };

  using factory_ptr = std::unique_ptr<factory_base>;

  static void register_factory(std::string name, factory_ptr factory) {
    factories[name] = std::move(factory);
  }

  static type_ptr create(std::string name) {
    return factories[name]->create();
  }

  static std::vector<type_ptr> instances(std::string name) {
    if(factories[name])
      return factories[name]->instances();
    else
      return std::vector<type_ptr> {} ;
  }

  static std::vector<type_ptr> instances() {
    std::vector<type_ptr> result;
    for(auto& kv : factories) {
      auto instances = kv.second->instances();
      std::move(instances.begin(), instances.end(), std::back_inserter(result));
    }
    return result;
  }

private:
  static std::map<std::string, factory_ptr> factories;
};

template<typename type>
class provider_registry {
public:
  class factory_base {
  public:
    virtual std::vector<type_ptr> instances() const =0;
  };

  using factory_ptr = std::unique_ptr<factory_base>;

  static void register_factory(factory_ptr factory) {
    factories.push_back(std::move(factory));
  };

  static std::vector<type_ptr> instances() {
    std::vector<type_ptr> result;
    for(auto& factory : factories) {
      auto instances = factory->instances();
      std::move(instances.begin(), instances.end(), std::back_inserter(result));
    }
    return result;
  };

private:
  static std::vector<factory_ptr> factories;
};

template <typename T>
std::vector<typename provider_registry<T>::factory_ptr> provider_registry<T>::factories;

template <typename T>
class register_type {
public:
  class factory : public type_registry::factory_base{
    type_ptr create() const {
      return std::unique_ptr<T>(new T);
    }

    std::vector<type_ptr> instances() const {
      return provider_registry<T>::instances();
    }
  };

  register_type<T>(std::string name) {
    type_registry::register_factory(name, std::unique_ptr<factory>(new factory));
  };
};

template <typename type, typename T>
class register_provider {
public:
  class factory : public provider_registry<type>::factory_base {
    std::vector<type_ptr> instances() const {
      return T::instances();
    }
  };

  register_provider<type, T>() {
    provider_registry<type>::register_factory(std::unique_ptr<factory>(new factory));
  };
};

#define PARAM(_name, type) struct _name##_tag { static const char* name() { return #_name; } }; using _name = param<type, _name##_tag>;

#define REGISTER_TYPE(name) static register_type<name::type> register_##name(#name);
#define REGISTER_PROVIDER(name, klass) static register_provider<name::type, klass> register_##name##_##klass;
