#include "mutelemetry/mutelemetry_logger.h"

#include <glog/logging.h>
#include <muqueue/erqperiodic.h>
#include <cassert>
#include <thread>

//#define TEST_PARSE_VALIDITY

#ifdef TEST_PARSE_VALIDITY
#include "mutelemetry/mutelemetry_parse.h"
using namespace mutelemetry_parse;
#endif

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

  filename_ = filename;
  file_ = ofstream(filename_, ios::binary);
  if (file_.is_open()) {
    assert(data_queue);
    data_queue_ = data_queue;
    inited_ = true;
  }

  return inited_;
}

void MutelemetryLogger::release(bool on_file_error) {
  if (inited_) {
    if (file_.is_open()) {
      if (!on_file_error && curr_idx_->has_data()) {
        start_io_worker(curr_idx_, true);
        curr_idx_ = nullptr;
      }
      file_.close();
    }
    inited_ = false;
  }
}

void MutelemetryLogger::start_io_worker(DataBuffer *dbp, bool do_flush) {
  fflow::post_function<void>([dbp, do_flush, this](void) -> void {
    DataBuffer &db = *dbp;
    SerializedData result;
    size_t idx = db.idx();
    for (size_t i = 0; i < idx; ++i) {
      SerializedDataPtr dp = db[i];
      result.insert(result.end(), dp->begin(), dp->end());
    }

    assert(result.size() == db.size());
    file_.write(reinterpret_cast<const char *>(result.data()), result.size());
    if (file_.bad()) {
      release(true);
      LOG(ERROR) << "Error writing to " << filename_ << endl;
    } else if (do_flush)
      this->flush();

    // this_thread::sleep_for(chrono::milliseconds(500));

    dbp->clear();
    pool_stacked_index_.push(dbp);
  });
}

void MutelemetryLogger::run() {
  add_periodic<void>(([&](void) -> void {
                       while (!data_queue_->empty() && inited_) {
                         auto dp = data_queue_->pop();
                         assert(dp != nullptr);
                         assert(dp->size() <= DataBuffer::max_data_size_);
#ifdef TEST_PARSE_VALIDITY
                         bool parse_res =
                             MutelemetryParser::getInstance().parse(dp->data());
                         assert(parse_res);
#endif
                         bool is_full = !curr_idx_->add(dp);
                         if (is_full) {
                           start_io_worker(curr_idx_);
                           curr_idx_ = pool_stacked_index_.pop();
                           assert(curr_idx_->idx() == 0);
                           curr_idx_->add(dp);
                         }
                       }

#if 0
                       // TODO: add timeout to start worker
                       if (curr_idx_->has_data()) {
                         start_io_worker(curr_idx_);
                         curr_idx_ = pool_stacked_index_.pop();
                       }
#endif
                     }),
                     0.000001, 0.1);
}
