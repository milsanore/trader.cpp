#pragma once

#include <deque>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>
#include <thread>

#include "IScreen.h"

/*
tail the /logs/log file
*/

namespace UI {

class LogBox {
 public:
  LogBox(IScreen& screen, std::string logFilePath,
         std::function<void(std::stop_token)> task = {});
  static LogBox fromEnv(IScreen& screen);
  // Return the FTXUI component to plug into layout
  ftxui::Component GetComponent();
  /// if any exceptions occurred
  std::exception_ptr thread_exception;
  // start log reader worker thread
  void Start();

 private:
  IScreen& screen_;
  ftxui::Component component_;
  float scroll_x = 0.1;
  float scroll_y = 1;

  std::string logFilePath_;
  std::jthread worker_;
  std::function<void(std::stop_token)> workerTask_;
  static constexpr int MAX_LINES_ = 100;
  void tailLogFile(const std::stop_token& stoken);
  std::deque<std::string> logBuffer_;
  std::mutex buffer_mutex;
};

}  // namespace UI
