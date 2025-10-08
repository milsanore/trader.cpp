#pragma once

namespace ui {

class ILogWatcher {
 public:
  virtual ~ILogWatcher() = default;
  virtual void set_callback(std::function<void(std::vector<std::string>)> cb) = 0;
  virtual void start() = 0;
};

}  // namespace ui
