#pragma once

#include <deque>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <mutex>
#include <thread>

#include "ILogReader.h"
#include "IScreen.h"

namespace ui {

class LogBox {
 public:
  LogBox(IScreen &screen, std::unique_ptr<ILogReader> logReader,
         std::function<void(std::stop_token)> task = {});
  static std::unique_ptr<LogBox> fromEnv(IScreen &screen);
  // Return the FTXUI component to plug into layout
  ftxui::Component GetComponent();
  /// if any exceptions occurred
  std::exception_ptr thread_exception;
  // start log reader worker thread
  void Start();

  // LogBox has a worker thread and a handle to the log file.
  // 1. Delete copy constructor and copy assignment
  LogBox(const LogBox &) = delete;
  LogBox &operator=(const LogBox &) = delete;
  // 2. Declare move and move-assignment constructors
  LogBox(LogBox &&) noexcept;
  LogBox &operator=(LogBox &&) noexcept;

 private:
  IScreen &screen_;
  ftxui::Component component_;
  float scroll_x = 0.1;
  float scroll_y = 1;

  std::unique_ptr<ILogReader> logReader_;
  std::jthread worker_;
  std::function<void(std::stop_token)> workerTask_;
  void tailLogFile(const std::stop_token &stoken);
  static constexpr int MAX_LINES_ = 100;
  std::deque<std::string> logBuffer_;
  std::mutex buffer_mutex;
};

}  // namespace ui
