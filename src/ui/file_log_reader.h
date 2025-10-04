#pragma once

#include <fstream>
#include <optional>
#include <string>

#include "ilog_reader.h"
#include "spdlog/spdlog.h"

namespace ui {

class FileLogReader : public ILogReader {
 public:
  explicit FileLogReader(const std::string& file_path)
      : file_(file_path), file_path_(file_path) {
    if (!file_.is_open()) {
      error_ = std::format("Failed to open log file. path [{}]", file_path);
    }
  }

  std::optional<std::string> read_next_line() override {
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

    error_ = std::format("Error reading log file. path [{}]", file_path_);
    return std::nullopt;
  }

  bool has_error() const override { return error_.has_value(); }

  // TODO: magic string?
  std::string get_error() const override { return error_.value_or("No error"); }

 private:
  std::ifstream file_;
  std::string file_path_;
  std::optional<std::string> error_;
};

}  // namespace ui
