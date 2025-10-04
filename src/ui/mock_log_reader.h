#pragma once

#include <queue>

#include "ilog_reader.h"

namespace ui {

class MockLogReader : public ILogReader {
 public:
  void add_line(const std::string& line) { lines_.push(line); }

  void set_error(const std::string& message) { error_ = message; }

  std::optional<std::string> read_next_line() override {
    if (error_.has_value()) return std::nullopt;
    if (lines_.empty()) return std::nullopt;

    std::string line = lines_.front();
    lines_.pop();
    return line;
  }

  bool has_error() const override { return error_.has_value(); }

  std::string get_error() const override { return error_.value_or("No error"); }

 private:
  std::queue<std::string> lines_;
  std::optional<std::string> error_;
};

}  // namespace ui
