#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ui/log_box/ilog_watcher.h"

namespace ui {

class MockLogWatcher : public ILogWatcher {
  using Callback = std::function<void(std::vector<std::string>)>;

 public:
  explicit MockLogWatcher() {}

  void set_callback(Callback cb) override {}

  void start() override {};
};

}  // namespace ui
