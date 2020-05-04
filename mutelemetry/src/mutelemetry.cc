#include "mutelemetry/mutelemetry.h"

#include <assert.h>
#include <iostream>
#include <memory>

#include <glog/logging.h>
#include <muconfig/muconfig.h>

using namespace std;
using namespace muconfig;
using namespace mutelemetry;
using namespace mutelemetry_ulog;

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

bool MuTelemetry::store_message(const string &message,
                                mutelemetry_ulog::ULogLevel level) {
  uint64_t tstmp = timestamp();
  // TODO:
  return true;
}

bool MuTelemetry::create_header_and_flags() {
  if (!is_enabled()) return true;

  ULogFileHeader fh(start_timestamp_);
  SerializedData fh_buffer(sizeof(fh));
  fh.write_to(fh_buffer.data());
  SerializedDataPtr fhp = make_shared<SerializedData>(move(fh_buffer));

  ULogMessageB mB = {};
  mB.h_.size_ = sizeof(mB) - sizeof(mB.h_);
  mB.h_.type_ = static_cast<uint8_t>(ULogMessageType::B);
  SerializedData mb_buffer(sizeof(mB));
  memcpy(mb_buffer.data(), reinterpret_cast<uint8_t *>(&mB), sizeof(mB));
  SerializedDataPtr mbp = make_shared<SerializedData>(move(mb_buffer));

  if (is_log_enabled()) {
    log_queue_.push(fhp);
    log_queue_.push(mbp);
  }

  if (is_net_enabled()) {
    net_queue_.push(fhp);
    net_queue_.push(mbp);
  }

  return true;
}

bool MuTelemetry::register_param(const string &key, int32_t value) {
  if (!is_enabled()) return true;

  bool res = true;
  ULogMessageP mP = {};
  mP.h_.type_ = static_cast<uint8_t>(ULogMessageType::P);
  size_t len = sizeof(value);
  mP.key_len_ = snprintf(mP.key_, sizeof(mP.key_), "%s", key.c_str());
  size_t msg_size = sizeof(mP) - sizeof(mP.key_) + mP.key_len_;

  if (len < (sizeof(mP) - msg_size)) {
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&mP);
    memcpy(&buffer[msg_size], &value, len);
    msg_size += len;
    mP.h_.size_ = msg_size - sizeof(mP.h_);
    SerializedData mp_buffer(buffer, buffer + msg_size);
    SerializedDataPtr mpp = make_shared<SerializedData>(move(mp_buffer));
    if (is_log_enabled()) log_queue_.push(mpp);
    if (is_net_enabled()) net_queue_.push(mpp);
  } else
    res = false;

  return res;
}

bool MuTelemetry::register_param(const string &key, float value) {
  if (!is_enabled()) return true;

  bool res = true;
  ULogMessageP mP = {};
  mP.h_.type_ = static_cast<uint8_t>(ULogMessageType::P);
  size_t len = sizeof(value);
  mP.key_len_ = snprintf(mP.key_, sizeof(mP.key_), "%s", key.c_str());
  size_t msg_size = sizeof(mP) - sizeof(mP.key_) + mP.key_len_;

  if (len < (sizeof(mP) - msg_size)) {
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&mP);
    memcpy(&buffer[msg_size], &value, len);
    msg_size += len;
    mP.h_.size_ = msg_size - sizeof(mP.h_);
    SerializedData mp_buffer(buffer, buffer + msg_size);
    SerializedDataPtr mpp = make_shared<SerializedData>(move(mp_buffer));
    if (is_log_enabled()) log_queue_.push(mpp);
    if (is_net_enabled()) net_queue_.push(mpp);
  } else
    res = false;

  return res;
}

bool MuTelemetry::register_info(const string &key, const std::string &value) {
  if (!is_enabled()) return true;

  bool res = true;
  ULogMessageI mI = {};
  mI.h_.type_ = static_cast<uint8_t>(ULogMessageType::I);
  size_t len = value.length();
  mI.key_len_ =
      snprintf(mI.key_, sizeof(mI.key_), "char[%zu] %s", len, key.c_str());
  size_t msg_size = sizeof(mI) - sizeof(mI.key_) + mI.key_len_;

  if (len < (sizeof(mI) - msg_size)) {
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&mI);
    memcpy(&buffer[msg_size], value.c_str(), len);
    msg_size += len;
    mI.h_.size_ = msg_size - sizeof(mI.h_);
    SerializedData mi_buffer(buffer, buffer + msg_size);
    SerializedDataPtr mip = make_shared<SerializedData>(move(mi_buffer));
    if (is_log_enabled()) log_queue_.push(mip);
    if (is_net_enabled()) net_queue_.push(mip);
  } else
    res = false;

  return res;
}

bool MuTelemetry::register_info_multi(const string &key, const string &value,
                                      bool is_continued) {
  if (!is_enabled()) return true;

  bool res = true;
  ULogMessageM mM = {};
  mM.h_.type_ = static_cast<uint8_t>(ULogMessageType::M);
  size_t len = value.length();
  mM.is_continued_ = is_continued;
  mM.key_len_ =
      snprintf(mM.key_, sizeof(mM.key_), "char[%zu] %s", len, key.c_str());
  size_t msg_size = sizeof(mM) - sizeof(mM.key_) + mM.key_len_;

  if (len < (sizeof(mM) - msg_size)) {
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&mM);
    memcpy(&buffer[msg_size], value.c_str(), len);
    msg_size += len;
    mM.h_.size_ = msg_size - sizeof(mM.h_);
    SerializedData mm_buffer(buffer, buffer + msg_size);
    SerializedDataPtr mmp = make_shared<SerializedData>(move(mm_buffer));
    if (is_log_enabled()) log_queue_.push(mmp);
    if (is_net_enabled()) net_queue_.push(mmp);
  } else
    res = false;

  return res;
}

bool MuTelemetry::register_data_format(const string &format) {
  // TODO:
  return true;
}
