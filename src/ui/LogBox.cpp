#include "LogBox.h"

#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>
#include <sstream>
#include <thread>

#include "../core/Env.h"
#include "./../utils/Threading.h"
#include "IScreen.h"
#include "spdlog/spdlog.h"

using namespace ftxui;

namespace UI {

LogBox::LogBox(IScreen& screen, std::string logFilePath,
               std::function<void(std::stop_token)> task)
    : screen_(screen), workerTask_(std::move(task)), logFilePath_(logFilePath) {
  if (!workerTask_) {
    workerTask_ = ([this](const std::stop_token& stoken) {
      Utils::Threading::set_thread_name("tradercppuiLOG");
      spdlog::info("starting polling log file on background thread");
      tailLogFile(stoken);
    });
  }

  SliderOption<float> option_x;
  option_x.value = &scroll_x;
  option_x.min = 0.f;
  option_x.max = 1.f;
  option_x.increment = 0.1f;
  option_x.direction = Direction::Right;
  option_x.color_active = Color::Blue;
  option_x.color_inactive = Color::BlueLight;
  auto scrollbar_x = Slider(option_x);

  SliderOption<float> option_y;
  option_y.value = &scroll_y;
  option_y.min = 0.f;
  option_y.max = 1.f;
  option_y.increment = 0.3f;
  option_y.direction = Direction::Down;
  option_y.color_active = Color::Yellow;
  option_y.color_inactive = Color::YellowLight;
  auto scrollbar_y = Slider(option_y);

  auto content = Renderer([this](bool focused) {
    std::deque<std::string> snapshot;
    {
      std::lock_guard<std::mutex> lock(buffer_mutex);
      snapshot = logBuffer_;  // safe copy
    }

    Elements lines;
    for (const auto& line : snapshot) {
      lines.push_back(text(line));
    }

    return vbox(std::move(lines)) | border | (focused ? bold : dim) |
           focusPositionRelative(scroll_x, scroll_y) | frame | flex;
  });

  component_ = Container::Vertical({
      Container::Horizontal({
          content,
          scrollbar_y,
      }) | flex,
      Container::Horizontal({
          scrollbar_x,
          Renderer([] { return text(L"x"); }),
      }),
  });
}

// static function
LogBox LogBox::fromEnv(IScreen& screen) {
  const std::string logPath = Core::Env::getEnvOrThrow("LOG_PATH");
  return LogBox(screen, logPath);
}

Component LogBox::GetComponent() { return component_; }

void LogBox::Start() { worker_ = std::jthread(workerTask_); }

void LogBox::tailLogFile(const std::stop_token& stoken) {
  try {
    std::ifstream file(logFilePath_);
    if (!file.is_open()) {
      throw std::runtime_error(
          std::format("failed to open log file for reading. path [{}]", logFilePath_));
    }

    std::string line;
    while (!stoken.stop_requested()) {
      if (std::getline(file, line)) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if (logBuffer_.size() >= MAX_LINES_) {
          logBuffer_.pop_front();
        }
        logBuffer_.push_back(line);
        screen_.PostEvent(ftxui::Event::Custom);
      } else {
        if (file.eof()) {
          file.clear();  // Clear EOF flag
          spdlog::info("sleeping between log file polls");
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } else {
          throw std::runtime_error(
              std::format("log file read error. path [{}]", logFilePath_));
          break;
        }
      }
    }
    file.close();
  } catch (const std::exception& e) {
    spdlog::error("error in log worker thread, error [{}]", e.what());
    thread_exception = std::current_exception();
  }
}

}  // namespace UI
