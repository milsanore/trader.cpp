#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ui/log_box/ilog_watcher.h"

namespace ui {

class MockLogWatcher : public ILogWatcher {
  using Callback = std::function<void(std::vector<std::string>)>;

 public:
  explicit MockLogWatcher() = default;

  void set_callback([[maybe_unused]] Callback cb) override {}

  void start() override {};
};

}  // namespace ui
