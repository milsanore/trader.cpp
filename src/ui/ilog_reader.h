#pragma once

namespace ui {

class ILogReader {
 public:
  virtual ~ILogReader() = default;

  /// Tries to read the next line from the log source.
  /// Returns std::nullopt if EOF is reached (but no error).
  virtual std::optional<std::string> read_next_line() = 0;

  /// Indicates whether the reader encountered an unrecoverable error
  virtual bool has_error() const = 0;

  /// Returns error message if any
  virtual std::string get_error() const = 0;
};

}  // namespace ui
