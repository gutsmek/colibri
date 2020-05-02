#include <assert.h>
#include <future>
#include <sstream>
#include <thread>
#include <utility>

#include <mutelemetry/mutelemetry.h>

#include "test_types.h"

using namespace std;
using namespace mutelemetry;
using namespace mutelemetry_ulog;

class DataType0Serializable : public Serializable, public DataType0 {
 public:
  vector<uint8_t> serialize() override {
    stringstream ss;
    ss << name() << " serialization ";  //<< fixed << setprecision(4) << *this;
    LOG(INFO) << ss.str();
    // DataType0 has its own ULog-compliant serialization method, let's reuse it
    return DataType0::serialize();
  }
};

class DataType1Serializable : public Serializable, public DataType1 {
 public:
  vector<uint8_t> serialize() override {
    std::vector<uint8_t> serialized;
    stringstream ss;
    ss << name() << " serialization ";  //<< fixed << setprecision(4) << *this;
    LOG(INFO) << ss.str();
    // TODO: add serialization here, since DataType1 doesn't have its own
    LOG(INFO) << name() << " serialization finished";
    return serialized;
  }
};

std::vector<uint8_t> DataType4_serialize(const DataType4 &d) {
  std::vector<uint8_t> serialized;
  stringstream ss;
  ss << TOSTR(DataType4)
     << " serialization ";  //<< fixed << setprecision(4) << d;
  LOG(INFO) << ss.str();
  // TODO: add serialization here
  LOG(INFO) << TOSTR(DataType4) << " external serialization finished";
  return serialized;
}

using RResult = pair<bool, thread::id>;
using RResultFutures = vector<future<RResult>>;

struct ThreadInternals {
  chrono::milliseconds delay_;
  int n_repeats_;
  ThreadInternals() = default;
  ThreadInternals(chrono::milliseconds delay, int n_repeats)
      : delay_(delay), n_repeats_(n_repeats) {}
};

auto generator = [](int type) {
  bool ret = true;
  int iteration = 0;
  RResult result{false, this_thread::get_id()};
  static const vector<ThreadInternals> params = {
      {chrono::milliseconds(20), 100},  // 0
      {chrono::milliseconds(50), 80},   // 1
      {chrono::milliseconds(100), 50},  // 2
      {chrono::milliseconds(150), 20},  // 3
      {chrono::milliseconds(500), 20},  // 4
  };

  LOG(INFO) << "Starting Thread [" << get<1>(result) << "] for type " << type
            << endl;

  try {
    int n_repeats = params[type].n_repeats_;
    while (n_repeats-- > 0 && ret) {
      switch (type) {
        case 0: {
          ret = MuTelemetry::getInstance().store_data(
              shared_ptr<Serializable>{new DataType0Serializable},
              DataType0::name(), "d01");
          if (ret) {
            ret = MuTelemetry::getInstance().store_data(
                shared_ptr<Serializable>{new DataType0Serializable},
                DataType0::name(), "d02");
          }
        } break;

        case 1:
          ret = MuTelemetry::getInstance().store_data(
              shared_ptr<Serializable>{new DataType1Serializable},
              DataType1::name(), "d1");
          if (ret) {
            ret = MuTelemetry::getInstance().store_data(
                shared_ptr<Serializable>{new DataType0Serializable},
                DataType0::name(), "d03");
          }
          break;

        case 2: {
          DataType2 d2;
          SerializerFunc s = [&d2]() {
            std::vector<uint8_t> serialized;
            stringstream ss;
            ss << DataType2::name() << " serialization ";
            //<< fixed << setprecision(4) << d2;
            LOG(INFO) << ss.str();
            // TODO: add serialization here
            LOG(INFO) << DataType2::name()
                      << " functional serialization finished" << endl;
            return serialized;
          };
          ret = MuTelemetry::getInstance().store_data(s, DataType2::name(),
                                                      TOSTR(d2));
        } break;

        case 3: {
          DataType2 d3;
          SerializerFunc s = [&d3]() {
            std::vector<uint8_t> serialized;
            stringstream ss;
            ss << DataType3::name() << " serialization ";
            //<< fixed << setprecision(4) << d3;
            LOG(INFO) << ss.str();
            // TODO: add serialization here
            LOG(INFO) << DataType3::name()
                      << " functional serialization finished" << endl;
            return serialized;
          };
          ret = MuTelemetry::getInstance().store_data(s, DataType2::name(),
                                                      TOSTR(d3));
        } break;

        case 4: {
          DataType4 d4;
          std::vector<uint8_t> d4_serialized = DataType4_serialize(d4);
          ret = MuTelemetry::getInstance().store_data(
              d4_serialized, TOSTR(DataType4), TOSTR(d4));
        } break;

        default:
          ret = false;
          break;
      }

      stringstream ss;
      ss << "Thread [" << get<1>(result) << "]: iteration: " << iteration++
         << " result: " << ret;
      MuTelemetry::getInstance().store_message(ss.str(), ULogLevel::Info);

      this_thread::sleep_for(chrono::milliseconds(params[type].delay_));
    }
  } catch (...) {
    stringstream ss;
    ret = false;
    ss << "Thread [" << get<1>(result) << "]: exception caught";
    MuTelemetry::getInstance().store_message(ss.str(), ULogLevel::Err);
  }

  get<0>(result) = ret;
  LOG(INFO) << "Thread [" << get<1>(result) << "] exited" << endl;
  return result;
};

int main(int argc, char **argv) {
  if (argc != 2) return 1;

  // google::InitGoogleLogging(argv[0]);
  MuTelemetry::init();

  MuTelemetry &mt = MuTelemetry::getInstance();

  if (!mt.is_enabled()) return 1;

  mt.register_info("company", "EDEL LLC");
  mt.register_info("sys_name", "RBPi4");
  mt.register_info("replay", mt.get_logname());
  mt.register_data(DataType0::name() + string(":") + DataType0::fields());
  mt.register_data(DataType1::name() + string(":") + DataType1::fields());
  mt.register_data(DataType2::name() + string(":") + DataType2::fields());
  mt.register_data(DataType3::name() + string(":") + DataType3::fields());
  mt.register_data("DataType4:DataType3[3] array;");

  const int n_threads = atoi(argv[1]);
  RResultFutures futures(n_threads);

  for (int i = 0; i < n_threads; ++i)
    futures[i] = std::async(launch::async, generator, i % 5);

  for (auto &f : futures) {
    RResult result = f.get();
    if (!get<0>(result))
      LOG(INFO) << "Thread [" << get<1>(result) << "] failed" << endl;
  }

  return 0;
}
