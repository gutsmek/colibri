#pragma once

#include "mutelemetry/mutelemetry_tools.h"

#include <array>
#include <cassert>
#include <fstream>
#include <memory>
#include <string>

namespace mutelemetry_logger {

class MutelemetryLogger {
  class DataBuffer {
   public:
    DataBuffer() : idx_(0) {}
    DataBuffer(const DataBuffer &) = delete;
    DataBuffer &operator=(const DataBuffer &) = delete;
    virtual ~DataBuffer() = default;

   public:
    bool add(mutelemetry_tools::SerializedDataPtr dp) {
      if (idx_ == buffer_.size()) return false;
      buffer_[idx_++] = dp;
      return true;
    }

    mutelemetry_tools::SerializedDataPtr operator[](size_t idx) {
      if (idx >= buffer_.size()) return nullptr;
      return buffer_[idx];
    }

    inline size_t size() const { return buffer_.size(); }
    inline size_t idx() const { return idx_; }
    inline void clear() { idx_ = 0; }

   private:
    static const size_t max_size_ = 64;
    size_t idx_;
    std::array<mutelemetry_tools::SerializedDataPtr, max_size_> buffer_;
  };

 public:
  MutelemetryLogger();
  MutelemetryLogger(const MutelemetryLogger &) = delete;
  MutelemetryLogger &operator=(const MutelemetryLogger &) = delete;

  virtual ~MutelemetryLogger() {
    if (inited_) {
      if (file_.is_open()) {
        flush();
        file_.close();
      }
      inited_ = false;
    }
  }

 public:
  const inline std::string &get_logname() const { return filename_; }

  void run();
  bool init(
      const std::string &,
      mutelemetry_tools::ConcQueue<mutelemetry_tools::SerializedDataPtr> *);

 private:
  inline void flush() {
    if (inited_) file_.flush();
  }

  void start_io_worker(DataBuffer *db);

 private:
  bool inited_;
  mutelemetry_tools::ConcQueue<mutelemetry_tools::SerializedDataPtr>
      *data_queue_;
  std::string filename_;
  std::ofstream file_;
  std::array<DataBuffer, 4> mem_pool_;
  mutelemetry_tools::ConcStack<DataBuffer *> pool_stacked_index_;
  DataBuffer *curr_idx_;
};

}  // namespace mutelemetry_logger
