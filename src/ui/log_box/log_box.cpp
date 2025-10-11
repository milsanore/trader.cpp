#include "log_box.h"

#include <efsw/efsw.hpp>
#include <format>
#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <mutex>
#include <thread>

#include "../../utils/env.h"
#include "../app/iscreen.h"
#include "./../../utils/threading.h"
#include "ilog_watcher.h"
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

LogBox::LogBox(IScreen& screen, std::unique_ptr<ILogWatcher> watcher)
    : screen_(screen), log_watcher_(std::move(watcher)) {
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
  option_y.increment = 0.1f;
  option_y.direction = Direction::Down;
  option_y.color_active = Color::Yellow;
  option_y.color_inactive = Color::YellowLight;
  auto scrollbar_y = Slider(option_y);

  auto content = Renderer([this](bool focused) {
    std::deque<std::string> buffer_copy;
    {
      std::lock_guard<std::mutex> lock(log_ring_mutex_);
      buffer_copy = log_ring_;  // safe copy
    }

    Elements lines;
    for (const auto& line : buffer_copy) {
      lines.push_back(text(line));
    }

    return vbox(std::move(lines)) | (focused ? bold : dim) |
           focusPositionRelative(scroll_x, scroll_y) | frame | flex | border;
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
  // parse watch path
  const std::string log_path = utils::Env::get_env_or_throw("LOG_PATH");
  size_t pos = log_path.find_last_of("/\\");  // handle both forward and backward slashes
  std::string directory;
  std::string filename;
  if (pos != std::string::npos) {
    directory = log_path.substr(0, pos + 1);  // include the slash
    filename = log_path.substr(pos + 1);
  } else {
    // No slash found
    throw std::runtime_error(std::format("invalid log file path, path [{}]", log_path));
  }

  auto watcher = std::make_unique<LogFileWatcher>(directory, filename);
  auto log_box = std::make_unique<LogBox>(screen, std::move(watcher));
  log_box->log_watcher_->set_callback(
      [box_ptr = log_box.get()](std::vector<std::string> lines) {
        box_ptr->on_log_lines(std::move(lines));
      });

  return log_box;
}

Component LogBox::get_component() {
  return component_;
}

void LogBox::start() {
  log_watcher_->start();
}

// runs on log watcher thread ( @ref ui::LogFileWatcher::THREAD_NAME_ )
void LogBox::on_log_lines(std::vector<std::string> lines) {
  std::lock_guard<std::mutex> lock(log_ring_mutex_);
  for (std::string& line : lines) {
    if (log_ring_.size() >= MAX_LINES_) {
      log_ring_.pop_front();
    }
    log_ring_.push_back(std::move(line));
  }
  screen_.post_event(ftxui::Event::Custom);
};

}  // namespace ui
