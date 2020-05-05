/* ----------------------------------------------------------------------------
 *
 * Copyright 2020, EDEL LLC
 * All Rights Reserved
 *
 * See LICENSE for the license information
 *
 * ULog Telemetry - storage and streaming
 * -------------------------------------------------------------------------- */

#pragma once

#include <muroute/subsystem.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

#include "mutelemetry_ulog.h"

namespace mutelemetry {

using SerializerFunc = std::function<std::vector<uint8_t>()>;

class Serializable {
 public:
  virtual std::vector<uint8_t> serialize() = 0;
};

class MuTelemetry {
  static MuTelemetry instance_;

  template <typename T>
  class ID {
    std::atomic<T> id_cntr_;
    std::unordered_map<std::string, T> ids_;

   public:
    ID() : id_cntr_(0) {}

    ID(const ID<T> &id) {
      id_cntr_ = id.id_cntr_.load();
      ids_ = id.ids_;
    }

    ID<T> &operator=(const ID<T> &that) {
      if (&that == this) return *this;
      id_cntr_ = that.id_cntr_.load();
      ids_ = that.ids_;
      return *this;
    }

    inline T get_id(const std::string &name) {
      if (ids_.find(name) == ids_.end()) ids_[name] = id_cntr_++;
      return ids_[name];
    }
  };

  // TODO: change to lock free
  template <typename T>
  class ConcQueue {
   public:
    ConcQueue() = default;
    virtual ~ConcQueue() = default;

    template <typename... Args>
    void push(Args &&... args) {
      addData_protected([&] { queue_.emplace(std::forward<Args>(args)...); });
    }

    T pop(void) noexcept {
      std::unique_lock<std::mutex> lock{mutex_};
      while (queue_.empty()) {
        condNewData_.wait(lock);
      }
      auto elem = std::move(queue_.front());
      queue_.pop();
      return elem;
    }

    size_t size(void) const { return queue_.size(); }
    bool empty(void) const { return queue_.empty(); }

   private:
    template <class F>
    void addData_protected(F &&fct) {
      std::unique_lock<std::mutex> lock{mutex_};
      fct();
      lock.unlock();
      condNewData_.notify_one();
    }

    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condNewData_;
  };

  bool with_network_ = false;
  bool with_local_log_ = false;
  fflow::RouteSystemPtr roster_ = nullptr;
  std::string log_dir_ = "";
  std::string log_file_ = "";
  uint64_t start_timestamp_ = 0;

  ID<uint16_t> msg_id_;
  std::unordered_map<uint16_t, ID<uint8_t>> multi_id_;

  using SerializedData = std::vector<uint8_t>;
  using SerializedDataPtr = std::shared_ptr<SerializedData>;
  ConcQueue<SerializedDataPtr> log_queue_;
  ConcQueue<SerializedDataPtr> net_queue_;

 private:
  MuTelemetry() {}
  MuTelemetry(const MuTelemetry &) = delete;
  MuTelemetry &operator=(const MuTelemetry &) = delete;

 private:
  inline uint16_t get_msg_id(const std::string &msg_name) {
    return msg_id_.get_id(msg_name);
  }

  inline uint8_t get_multi_id(uint16_t msg_id, const std::string &inst_name) {
    if (multi_id_.find(msg_id) == multi_id_.end())
      multi_id_[msg_id] = ID<uint8_t>{};
    return multi_id_[msg_id].get_id(inst_name);
  }

  inline uint8_t get_multi_id(const std::string &msg_name,
                              const std::string &inst_name) {
    return get_multi_id(get_msg_id(msg_name), inst_name);
  }

  inline std::pair<uint16_t, uint8_t> get_ids(const std::string &msg_name,
                                              const std::string &inst_name) {
    uint16_t msg_id = get_msg_id(msg_name);
    return {msg_id, get_multi_id(msg_id, inst_name)};
  }

  inline uint64_t timestamp() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
  }

  bool read_config(MuTelemetry &inst = instance_, const std::string &file = "");
  bool create_header_and_flags();

  inline void to_io(const SerializedDataPtr dp) {
    if (is_log_enabled()) log_queue_.push(dp);
    if (is_net_enabled()) net_queue_.push(dp);
  }

  bool store_data_intl(const std::vector<uint8_t> &, const std::string &,
                       const std::string &, uint64_t);

 public:
  static bool init(fflow::RouteSystemPtr roster = nullptr);
  static inline MuTelemetry &getInstance() { return instance_; }

 public:
  inline bool is_log_enabled() const { return with_local_log_; }
  inline bool is_net_enabled() const { return with_network_; }
  inline bool is_enabled() const { return with_network_ || with_local_log_; }

  inline const std::string &get_logname() const { return log_file_; }
  inline const std::string &get_logdir() const { return log_dir_; }

  bool register_param(const std::string &, int32_t);
  bool register_param(const std::string &, float);
  bool register_info(const std::string &, const std::string &);
  bool register_info_multi(const std::string &, const std::string &, bool);

  // example of format string: "DataTypeName;float a;int[3] b;bool c;"
  bool register_data_format(const std::string &);

  // v1: using Serializable interface
  bool store_data(std::shared_ptr<Serializable> s, const std::string &type_name,
                  const std::string &annotation = "") {
    std::vector<uint8_t> data = s->serialize();
    return store_data_intl(data, type_name, annotation, timestamp());
  }

  // v2: using lambda which provides serialization
  bool store_data(const SerializerFunc &s, const std::string &type_name,
                  const std::string &annotation = "") {
    std::vector<uint8_t> data = s();
    return store_data_intl(data, type_name, annotation, timestamp());
  }

  // v3: using data, which was already serialized by the caller
  bool store_data(const std::vector<uint8_t> &data,
                  const std::string &type_name,
                  const std::string &annotation = "") {
    return store_data_intl(data, type_name, annotation, timestamp());
  }

  bool store_message(const std::string &, mutelemetry_ulog::ULogLevel);
};

}  // namespace mutelemetry
