#pragma once

#include <fstream>
#include <optional>
#include <string>

#include "ILogReader.h"
#include "spdlog/spdlog.h"

namespace UI {

class FileLogReader : public ILogReader {
 public:
  explicit FileLogReader(const std::string& filePath)
      : file_(filePath), filePath_(filePath) {
    if (!file_.is_open()) {
      error_ = std::format("Failed to open log file. path [{}]", filePath);
    }
  }

  std::optional<std::string> readNextLine() override {
    if (error_.has_value()) {
      return std::nullopt;
    }

    std::string line;
    if (std::getline(file_, line)) {
      return line;
    }

    if (file_.eof()) {
      file_.clear();        // Reset EOF flag
      return std::nullopt;  // Signal to wait/sleep
    }

    error_ = std::format("Error reading log file. path [{}]", filePath_);
    return std::nullopt;
  }

  bool hasError() const override { return error_.has_value(); }

  // TODO: magic string?
  std::string getError() const override { return error_.value_or("No error"); }

 private:
  std::ifstream file_;
  std::string filePath_;
  std::optional<std::string> error_;
};

}  // namespace UI