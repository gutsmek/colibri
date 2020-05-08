#include "mutelemetry/mutelemetry_logger.h"

#include <muqueue/erqperiodic.h>
#include <cassert>

#define TEST_PARSE_VALIDITY

using namespace std;
using namespace fflow;
using namespace mutelemetry_logger;
using namespace mutelemetry_tools;

MutelemetryLogger::MutelemetryLogger()
    : inited_(false), data_queue_(nullptr), filename_("") {
  for (size_t i = 1; i < mem_pool_.size(); ++i)
    pool_stacked_index_.push(&mem_pool_[i]);
  curr_idx_ = &mem_pool_[0];
}

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

void MutelemetryLogger::start_io_worker(DataBuffer *db) {
#if 0
    size_t sz = db->size();
    SerializedData result;
    for (size_t i = 0; i < sz; ++i) {
      SerializedDataPtr dp = (*db)[i];
      // TODO:
    }

    db->clear();
    pool_stacked_index_.push(db);
#endif
}

void MutelemetryLogger::run() {
  add_periodic<void>(([&](void) -> void {
                       while (!data_queue_->empty()) {
                         auto dp = data_queue_->pop();
                         assert(dp != nullptr);
                         bool is_full = !curr_idx_->add(dp);
                         if (is_full) {
                           start_io_worker(curr_idx_);
                           curr_idx_ = pool_stacked_index_.pop();
                           assert(curr_idx_->idx() == 0);
                           curr_idx_->add(dp);
                         }
                       }
                     }),
                     0.000001, 0.1);
}
