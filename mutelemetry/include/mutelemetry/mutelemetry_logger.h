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
    DataBuffer() : idx_(0), data_size_(0) {}
    DataBuffer(const DataBuffer &) = delete;
    DataBuffer &operator=(const DataBuffer &) = delete;
    virtual ~DataBuffer() = default;

   public:
    bool add(mutelemetry_tools::SerializedDataPtr dp) {
      if (idx_ == max_size_) return false;
      size_t data_size = dp->size();
      if (data_size + data_size_ >= max_data_size_) return false;
      data_size_ += data_size;
      buffer_[idx_++] = dp;
      return true;
    }

    mutelemetry_tools::SerializedDataPtr operator[](size_t idx) {
      if (idx >= idx_) return nullptr;
      return buffer_[idx];
    }

    inline bool has_data() const { return idx_ > 0; }
    inline size_t size() const { return data_size_; }
    inline size_t idx() const { return idx_; }
    inline void clear() { data_size_ = idx_ = 0; }

   public:
    static const size_t max_size_ = 64;
    static const size_t max_data_size_ = 8096;

   private:
    size_t data_size_;
    size_t idx_;
    std::array<mutelemetry_tools::SerializedDataPtr, max_size_> buffer_;
  };

 public:
  MutelemetryLogger();
  MutelemetryLogger(const MutelemetryLogger &) = delete;
  MutelemetryLogger &operator=(const MutelemetryLogger &) = delete;

  virtual ~MutelemetryLogger() { release(); }

 public:
  const inline std::string &get_logname() const { return filename_; }

  void run();
  bool init(
      const std::string &,
      mutelemetry_tools::ConcQueue<mutelemetry_tools::SerializedDataPtr> *);
  void release(bool on_file_error = false);

 private:
  inline void flush() {
    if (inited_) file_.flush();
  }

  void start_io_worker(DataBuffer *, bool do_flush = false);

 private:
  bool inited_;
  mutelemetry_tools::ConcQueue<mutelemetry_tools::SerializedDataPtr>
      *data_queue_;
  std::string filename_;
  std::ofstream file_;
  std::array<DataBuffer, 8> mem_pool_;
  mutelemetry_tools::ConcStack<DataBuffer *> pool_stacked_index_;
  DataBuffer *curr_idx_;
};

}  // namespace mutelemetry_logger
