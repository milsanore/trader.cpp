#pragma once

#include <deque>
#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>
#include <vector>

#include "../../utils/env.h"
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
  void start() const;

 private:
  // UI
  IScreen& screen_;
  ftxui::Component component_;
  float scroll_x = 0.1;
  float scroll_y = 1;
  /// @brief the columns in the trade box table, and their widths
  const std::array<std::pair<std::string, uint8_t>, 4> columns_ = {
      {{"Time", 26}, {"Level", 7}, {"Thread", 7}, {"Message", 7}}};
  ftxui::Elements header_;

  // log ring-buffer
  std::deque<std::string> log_ring_;
  static inline constexpr uint16_t MAX_LINES_ = 100;
  alignas(utils::Env::CACHE_LINE_SIZE) std::mutex log_ring_mutex_;

  // log-file watcher
  std::unique_ptr<ILogWatcher> log_watcher_;
  void on_log_lines(std::vector<std::string> lines);
};

}  // namespace ui
