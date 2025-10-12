#pragma once

#include <deque>
#include <fstream>
#include <ftxui/component/component.hpp>
#include <mutex>
#include <vector>

#include "../app/iscreen.h"
#include "ilog_watcher.h"
#include "log_file_watcher.h"

namespace ui {

class LogBox {
 public:
  LogBox(IScreen& screen, std::unique_ptr<ILogWatcher> watcher);
  static std::unique_ptr<LogBox> from_env(IScreen& screen);
  // Return the FTXUI component to plug into layout
  ftxui::Component get_component();
  /// if any exceptions occurred
  std::exception_ptr thread_exception;
  // start log watcher
  void start();

 private:
  // UI
  IScreen& screen_;
  ftxui::Component component_;
  float scroll_x = 0.1;
  float scroll_y = 1;

  // log ring-buffer
  std::deque<std::string> log_ring_;
  static constexpr uint16_t MAX_LINES_ = 100;
  std::mutex log_ring_mutex_;

  // log-file watcher
  std::unique_ptr<ILogWatcher> log_watcher_;
  void on_log_lines(std::vector<std::string> lines);
};

}  // namespace ui
