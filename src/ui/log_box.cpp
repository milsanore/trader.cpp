#include "log_box.h"

#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>
#include <thread>

#include "../core/env.h"
#include "./../utils/threading.h"
#include "file_log_reader.h"
#include "ilog_reader.h"
#include "iscreen.h"
#include "mock_log_reader.h"
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

namespace ui {

LogBox::LogBox(IScreen& screen, std::unique_ptr<ILogReader> log_reader,
               std::function<void(std::stop_token)> task)
    : screen_(screen), log_reader_(std::move(log_reader)), worker_task_(std::move(task)) {
  if (!worker_task_) {
    worker_task_ = ([this](const std::stop_token& stoken) {
      utils::Threading::set_thread_name("tradercppuiLOG");
      spdlog::info("starting polling log file on background thread");
      tail_log_file(stoken);
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
      snapshot = log_buffer_;  // safe copy
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
std::unique_ptr<LogBox> LogBox::from_env(IScreen& screen) {
  const std::string log_path = core::Env::get_env_or_throw("LOG_PATH");
  std::unique_ptr<ILogReader> log_reader = std::make_unique<FileLogReader>(log_path);
  return std::make_unique<LogBox>(screen, std::move(log_reader));
}

// move constructor
LogBox::LogBox(LogBox&& other) noexcept
    : screen_(other.screen_),
      component_(std::move(other.component_)),
      scroll_x(other.scroll_x),
      scroll_y(other.scroll_y),
      log_reader_(std::move(other.log_reader_)),
      worker_(std::move(other.worker_)),
      worker_task_(std::move(other.worker_task_)),
      log_buffer_(std::move(other.log_buffer_)) {
  // mutex is not copied/moved, a new instance is created
}

// move-assignment constructor
LogBox& LogBox::operator=(LogBox&& other) noexcept {
  if (this != &other) {
    screen_ = other.screen_;
    component_ = std::move(other.component_);
    scroll_x = other.scroll_x;
    scroll_y = other.scroll_y;
    log_reader_ = std::move(other.log_reader_);
    worker_ = std::move(other.worker_);
    worker_task_ = std::move(other.worker_task_);
    log_buffer_ = std::move(other.log_buffer_);
    // mutex is not copied/moved, a new instance is created
  }
  return *this;
}

Component LogBox::get_component() { return component_; }

void LogBox::start() { worker_ = std::jthread(worker_task_); }

void LogBox::tail_log_file(const std::stop_token& stoken) {
  try {
    if (!log_reader_ || log_reader_->has_error()) {
      throw std::runtime_error("Log reader init error: " + (log_reader_
                                                                ? log_reader_->get_error()
                                                                : "null logReader"));
    }

    while (!stoken.stop_requested()) {
      auto line_opt = log_reader_->read_next_line();
      if (line_opt.has_value()) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if (log_buffer_.size() >= MAX_LINES_) {
          log_buffer_.pop_front();
        }
        log_buffer_.push_back(line_opt.value());
        screen_.post_event(ftxui::Event::Custom);
      } else {
        if (log_reader_->has_error()) {
          throw std::runtime_error("Log read error: " + log_reader_->get_error());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    }
  } catch (const std::exception& e) {
    spdlog::error("error in log worker thread, error [{}]", e.what());
    thread_exception = std::current_exception();
  }
}

}  // namespace ui
