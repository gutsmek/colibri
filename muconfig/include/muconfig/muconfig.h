#ifndef MUCONFIG_H
#define MUCONFIG_H

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace muconfig {

enum class TYPE : uint8_t {
  UNDEF,
  BOOL = 1,
  INT,
  DOUBLE,
  STRING,
  TABLE,
  ARRAY,
  TABLE_ARRAY, /* NI */
  TIME,
  DATE,
  DATETIME,
  OFFSET /* NI */
};

enum class LANG : uint8_t {
  UNKNOWN = 0,
  JSON,  // NI
  TOML,
  YAML,  // NI
};

struct MuConfObject {
  TYPE t_;
  std::vector<uint8_t> v_;

  MuConfObject() : t_(TYPE::UNDEF) { v_.clear(); }

  std::shared_ptr<std::string> getValueString() const;

  template <typename T>
  std::shared_ptr<T> getValueSimple() const {
    std::shared_ptr<T> objp = {};

    switch (t_) {
      case TYPE::BOOL: {
        if (typeid(T) != typeid(bool)) break;
        objp = std::make_shared<T>();
        *objp = static_cast<bool>(v_[0]);
      } break;

      case TYPE::INT: {
        if (typeid(T) != typeid(int)) break;
        objp = std::make_shared<T>();
        std::memcpy(objp.get(), v_.data(), sizeof(int));
      } break;

      case TYPE::DOUBLE: {
        if (typeid(T) != typeid(double)) break;
        objp = std::make_shared<T>();
        std::memcpy(objp.get(), v_.data(), sizeof(double));
      } break;

      default:
        break;
    }

    return objp;
  }
};

class MuConfig {
 public:
  MuConfig() : name_("Unknown"), l_(LANG::UNKNOWN) {}
  virtual ~MuConfig() {}

 public:
  virtual bool isOk() { return false; }
  virtual bool parse() { return false; }
  virtual const std::string &getName() const { return name_; }
  virtual LANG getLang() const { return l_; }
  virtual void print() { std::cout << "\n===== " << getName() << " =====\n"; }

  virtual std::unique_ptr<MuConfObject> getObject(const std::string &key,
                                                  TYPE type) = 0;

 public:
  static std::unique_ptr<MuConfig> createConfig(
      const std::string &filename = "", LANG lang = LANG::TOML);

 protected:
  std::string name_;
  LANG l_;
};

}  // namespace muconfig

#endif  // MUCONFIG_H
