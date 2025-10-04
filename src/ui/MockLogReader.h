#pragma once

#include <queue>

#include "ILogReader.h"

namespace ui {

class MockLogReader : public ILogReader {
 public:
  void AddLine(const std::string& line) { lines_.push(line); }

  void SetError(const std::string& message) { error_ = message; }

  std::optional<std::string> readNextLine() override {
    if (error_.has_value()) return std::nullopt;
    if (lines_.empty()) return std::nullopt;

    std::string line = lines_.front();
    lines_.pop();
    return line;
  }

  bool hasError() const override { return error_.has_value(); }

  std::string getError() const override { return error_.value_or("No error"); }

 private:
  std::queue<std::string> lines_;
  std::optional<std::string> error_;
};

}  // namespace ui
