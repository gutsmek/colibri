#pragma once

#include "mutelemetry/mutelemetry_tools.h"

#include <cassert>
#include <fstream>
#include <string>

#define TEST_PARSE_VALIDITY

namespace mutelemetry_logger {

class MutelemetryLogger {
 public:
  MutelemetryLogger() : inited_(false), data_queue_(nullptr), filename_("") {}
  MutelemetryLogger(const MutelemetryLogger &) = delete;
  MutelemetryLogger &operator=(const MutelemetryLogger &) = delete;

  ~MutelemetryLogger() {
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

  bool init(
      const std::string &,
      mutelemetry_tools::ConcQueue<mutelemetry_tools::SerializedDataPtr> *);
  void run();

 private:
  inline void flush() {
    if (inited_) file_.flush();
  }

 private:
  bool inited_;
  mutelemetry_tools::ConcQueue<mutelemetry_tools::SerializedDataPtr>
      *data_queue_;
  std::string filename_;
  std::ofstream file_;
};

}  // namespace mutelemetry_logger
