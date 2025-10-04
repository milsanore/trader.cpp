#include "LogBox.h"

#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>
#include <thread>

#include "../core/Env.h"
#include "./../utils/Threading.h"
#include "FileLogReader.h"
#include "ILogReader.h"
#include "IScreen.h"
#include "MockLogReader.h"
#include "spdlog/spdlog.h"

using ftxui::bold;
using ftxui::border;
using ftxui::Color;
using ftxui::Component;
using ftxui::dim;
using ftxui::Direction;
using ftxui::Elements;
using ftxui::flex;
using ftxui::focusPositionRelative;
using ftxui::frame;
using ftxui::Renderer;
using ftxui::SliderOption;
using ftxui::text;
using ftxui::vbox;

namespace UI {

LogBox::LogBox(IScreen& screen, std::unique_ptr<ILogReader> logReader,
               std::function<void(std::stop_token)> task)
    : screen_(screen), logReader_(std::move(logReader)), workerTask_(std::move(task)) {
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

  component_ = ftxui::Container::Vertical({
      ftxui::Container::Horizontal({
          content,
          scrollbar_y,
      }) | flex,
      ftxui::Container::Horizontal({
          scrollbar_x,
          Renderer([] { return text(L"x"); }),
      }),
  });
}

// static function
std::unique_ptr<LogBox> LogBox::fromEnv(IScreen& screen) {
  const std::string logPath = Core::Env::getEnvOrThrow("LOG_PATH");
  std::unique_ptr<ILogReader> logReader = std::make_unique<FileLogReader>(logPath);
  return std::make_unique<LogBox>(screen, std::move(logReader));
}

// move constructor
LogBox::LogBox(LogBox&& other) noexcept
    : screen_(other.screen_),
      component_(std::move(other.component_)),
      scroll_x(other.scroll_x),
      scroll_y(other.scroll_y),
      logReader_(std::move(other.logReader_)),
      worker_(std::move(other.worker_)),
      workerTask_(std::move(other.workerTask_)),
      logBuffer_(std::move(other.logBuffer_)) {
  // mutex is not copied/moved, a new instance is created
}

// move-assignment constructor
LogBox& LogBox::operator=(LogBox&& other) noexcept {
  if (this != &other) {
    screen_ = other.screen_;
    component_ = std::move(other.component_);
    scroll_x = other.scroll_x;
    scroll_y = other.scroll_y;
    logReader_ = std::move(other.logReader_);
    worker_ = std::move(other.worker_);
    workerTask_ = std::move(other.workerTask_);
    logBuffer_ = std::move(other.logBuffer_);
    // mutex is not copied/moved, a new instance is created
  }
  return *this;
}

Component LogBox::GetComponent() { return component_; }

void LogBox::Start() { worker_ = std::jthread(workerTask_); }

void LogBox::tailLogFile(const std::stop_token& stoken) {
  try {
    if (!logReader_ || logReader_->hasError()) {
      throw std::runtime_error("Log reader init error: " +
                               (logReader_ ? logReader_->getError() : "null logReader"));
    }

    while (!stoken.stop_requested()) {
      auto lineOpt = logReader_->readNextLine();
      if (lineOpt.has_value()) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if (logBuffer_.size() >= MAX_LINES_) {
          logBuffer_.pop_front();
        }
        logBuffer_.push_back(lineOpt.value());
        screen_.PostEvent(ftxui::Event::Custom);
      } else {
        if (logReader_->hasError()) {
          throw std::runtime_error("Log read error: " + logReader_->getError());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    }
  } catch (const std::exception& e) {
    spdlog::error("error in log worker thread, error [{}]", e.what());
    thread_exception = std::current_exception();
  }
}

}  // namespace UI
