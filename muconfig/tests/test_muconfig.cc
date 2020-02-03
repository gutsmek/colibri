#include <muconfig/muconfig.h>
#include <iostream>
#include <memory>
#include <string>

using namespace std;
using namespace muconfig;

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " filename" << endl;
    return 1;
  }

  unique_ptr<MuConfig> cfg = MuConfig::createConfig(argv[1]);
  if (cfg == nullptr || !cfg->isOk()) return 1;

  cfg->print();

  const string s_local = "muroute.telemetry.with_local_log";
  auto local = cfg->getObject(s_local, TYPE::BOOL);
  if (local) {
    std::shared_ptr<bool> v1 = local->getValueSimple<bool>();
    if (v1) cout << s_local << "=" << *v1 << endl;
  }

  const string s_network = "muroute.telemetry.with_network";
  auto network = cfg->getObject(s_network, TYPE::BOOL);
  if (network) {
    auto v2 = network->getValueSimple<bool>();
    if (v2) cout << s_network << "=" << *v2 << endl;
  }

  const string s_tport = "muroute.telemetry.telemetry_port";
  auto tport = cfg->getObject(s_tport, TYPE::INT);
  if (tport) {
    std::shared_ptr<int> v3 = tport->getValueSimple<int>();
    if (v3) cout << s_tport << "=" << *v3 << endl;
  }

  const string s_threshold = "muflow.sparse_mem.detector_threshold";
  auto threshold = cfg->getObject(s_threshold, TYPE::DOUBLE);
  if (threshold) {
    auto v4 = threshold->getValueSimple<double>();
    if (v4) cout << s_threshold << "=" << *v4 << endl;
  }

  const string s_ni = "muroute.network_interface";
  auto ni = cfg->getObject(s_ni, TYPE::STRING);
  if (ni) {
    auto v5 = ni->getValueString();
    if (v5) cout << s_ni << "=" << *v5 << endl;
  }

  unique_ptr<MuConfig> cfg2 = MuConfig::createConfig(cfg->getName());
  if (cfg2 == nullptr || !cfg2->isOk()) return 1;

  const string s_log_dir = "muroute.telemetry.log_directory_path";
  auto log_dir = cfg2->getObject(s_log_dir, TYPE::STRING);
  if (log_dir) {
    auto v6 = log_dir->getValueString();
    if (v6) cout << s_log_dir << "=" << *v6 << endl;
  }

  const string s_cap = "muqueue.scheduler.queue_capacity";
  auto cap = cfg2->getObject(s_cap, TYPE::INT);
  if (cap) {
    auto v7 = cap->getValueSimple<int>();
    if (v7) cout << s_cap << "=" << *v7 << endl;
  }

  return 0;
}
