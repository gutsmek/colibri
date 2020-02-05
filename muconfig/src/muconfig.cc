#include "muconfig/muconfig.h"
#include "cpptoml.h"

using namespace std;
using namespace cpptoml;
using namespace muconfig;

std::shared_ptr<std::string> MuConfObject::getValueString() const {
  std::shared_ptr<string> objp = {};
  if (t_ == TYPE::STRING) {
    objp = make_shared<string>();
    objp->assign(v_.begin(), v_.end());
  }
  return objp;
}

class MuConfigToml : public MuConfig {
 public:
  MuConfigToml() {
    name_ = "muconfig.toml";
    l_ = LANG::TOML;
    parse();
  }

  MuConfigToml(const string &name) {
    name_ = name;
    l_ = LANG::TOML;
    parse();
  }

  virtual ~MuConfigToml() {}

 public:
  bool isOk() override { return static_cast<bool>(config_); }

  void print() override {
    MuConfig::print();
    cout << (*config_) << endl << endl;
  }

  bool parse() override {
    if (!isOk()) {
      cout << "Parse " << name_ << endl;
      try {
        config_ = parse_file(name_);
      } catch (const parse_exception &e) {
        cout << "Failed to parse " << name_ << ": " << e.what() << endl;
      }
    }
    return isOk();
  }

  std::unique_ptr<MuConfObject> getObject(const string &key,
                                          TYPE type) override;

 private:
  template <typename T>
  option<T> getOption(const string &key) {
    option<T> value = {};
    try {
      // will return an empty value for non-existant key
      value = config_->get_qualified_as<T>(key);
    } catch (const parse_exception &e) {
      cout << "Get Value for Key=<" << key << "> failed: " << e.what() << endl;
    }
    return value;
  }

 private:
  shared_ptr<table> config_;
};

std::unique_ptr<MuConfObject> MuConfigToml::getObject(const string &key,
                                                      TYPE type) {
  unique_ptr<MuConfObject> objp = {};

  switch (type) {
    case TYPE::BOOL: {
      option<bool> o = getOption<bool>(key);
      if ((bool)o) {
        objp = make_unique<MuConfObject>();
        objp->v_.push_back(static_cast<uint8_t>(*o));
        objp->t_ = type;
      }
    } break;

    case TYPE::INT: {
      option<int> o = getOption<int>(key);
      if ((bool)o) {
        objp = make_unique<MuConfObject>();
        objp->v_.reserve(sizeof(int));
        memcpy(objp->v_.data(), &(*o), sizeof(int));
        objp->t_ = type;
      }
    } break;

    case TYPE::DOUBLE: {
      option<double> o = getOption<double>(key);
      if ((bool)o) {
        objp = make_unique<MuConfObject>();
        objp->v_.reserve(sizeof(double));
        memcpy(objp->v_.data(), &(*o), sizeof(double));
        objp->t_ = type;
      }
    } break;

    case TYPE::STRING: {
      option<string> o = getOption<string>(key);
      if ((bool)o) {
        objp = make_unique<MuConfObject>();
        objp->v_.assign(o->begin(), o->end());
        objp->t_ = type;
      }
    } break;

    default:
      break;
  }

  return objp;
}

unique_ptr<MuConfig> MuConfig::createConfig(const string &filename, LANG lang) {
  unique_ptr<MuConfig> cfgp = {};

  switch (lang) {
    case LANG::TOML: {
      if (filename == "")
        cfgp = make_unique<MuConfigToml>();
      else
        cfgp = make_unique<MuConfigToml>(filename);
    } break;

    case LANG::JSON:
    case LANG::YAML:
    default:
      break;
  }

  return cfgp;
}
