#include "mutelemetry/mutelemetry_logger.h"

using namespace std;
using namespace mutelemetry_logger;
using namespace mutelemetry_tools;

bool MutelemetryLogger::init(const std::string &filename,
                             ConcQueue<SerializedDataPtr> *data_queue) {
  if (inited_) return false;

  bool res = false;
  filename_ = filename;
  file_ = ofstream(filename_, ios::binary);
  if (file_.is_open()) {
    assert(data_queue);
    data_queue_ = data_queue;
    res = true;
  }
  return res;
}

void MutelemetryLogger::run() {
  // TODO:
}
