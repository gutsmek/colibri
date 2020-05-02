#include "mutelemetry/mutelemetry.h"

#include <assert.h>
#include <iostream>
#include <memory>

#include <glog/logging.h>
#include <muconfig/muconfig.h>

using namespace std;
using namespace muconfig;
using namespace mutelemetry;

MuTelemetry MuTelemetry::instance_ = {};

bool MuTelemetry::read_config(MuTelemetry &inst, const string &file) {
  unique_ptr<MuConfig> cfg = MuConfig::createConfig(file);
  bool with_local_log = false;
  bool with_network = false;
  string log_dir = "";
  bool res = false;

  if (cfg != nullptr && cfg->isOk()) {
    const string s_with_network = "mutelemetry.with_network";
    auto o_with_network = cfg->getObject(s_with_network, TYPE::BOOL);
    if (o_with_network) {
      auto with_networkp = o_with_network->getValueSimple<bool>();
      if (with_networkp) with_network = *with_networkp;
    }

    const string s_with_local_log = "mutelemetry.with_local_log";
    auto o_with_local_log = cfg->getObject(s_with_local_log, TYPE::BOOL);
    if (o_with_local_log) {
      auto with_local_logp = o_with_local_log->getValueSimple<bool>();
      if (with_local_logp) with_local_log = *with_local_logp;
    }

    if (with_local_log) {
      const string s_log_dir = "mutelemetry.log_directory_path";
      auto o_log_dir = cfg->getObject(s_log_dir, TYPE::STRING);
      if (o_log_dir) {
        auto log_dirp = o_log_dir->getValueString();
        if (log_dirp) log_dir = *log_dirp;
      }
    }

    inst.with_network_ = with_network;
    inst.with_local_log_ = with_local_log;
    if (log_dir == "")
      inst.log_dir_ = "./";
    else
      inst.log_dir_ = log_dir;
    res = true;
  }

  return res;
}

bool MuTelemetry::init(fflow::RouteSystemPtr roster) {
  if (instance_.roster_) return false;
  instance_.start_timestamp_ = instance_.timestamp();
  if (instance_.read_config()) {
    if (instance_.with_network_ && roster != nullptr) {
      instance_.roster_ = roster;
      if (instance_.roster_ == nullptr) instance_.with_network_ = false;
    } else {
      LOG(INFO) << "MuTelemetry network is disabled\n";
    }

    if (instance_.with_local_log_) {
      LOG(INFO) << "MuTelemetry log directory: " << instance_.log_dir_ << endl;
    } else {
      LOG(INFO) << "MuTelemetry logging is disabled\n";
    }
  }

  if (!instance_.is_enabled()) LOG(INFO) << "MuTelemetry is disabled\n";

  return instance_.is_enabled();
}

bool MuTelemetry::store_data_intl(const vector<uint8_t> &data,
                                  const string &type_name,
                                  const string &annotation,
                                  uint64_t timestamp) {
  LOG(INFO) << "[" << this_thread::get_id() << "]:"
            << " Annotation=\'" << annotation << "\' Type=" << type_name;
  // TODO: here we pack both ADD message and DATA message
  return true;
}

bool MuTelemetry::store_message(const std::string &message,
                                mutelemetry_ulog::ULogLevel level) {
  uint64_t tstmp = timestamp();
  // TODO:
  return true;
}

bool MuTelemetry::register_info(const std::string &key,
                                const std::string &value) {
  // TODO:
  return true;
}

bool MuTelemetry::register_info_multi(const std::string &key,
                                      const std::string &value,
                                      bool is_continued) {
  // TODO:
  return true;
}

bool MuTelemetry::register_data(const std::string &format) {
  // TODO:
  return true;
}
