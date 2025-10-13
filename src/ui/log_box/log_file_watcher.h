#pragma once

#include <efsw/efsw.hpp>
#include <fstream>
#include <string>
#include <vector>

#include "../../utils/threading.h"
#include "ilog_watcher.h"
#include "spdlog/spdlog.h"

namespace ui {

class LogFileWatcher : public ILogWatcher, public efsw::FileWatchListener {
  using Callback = std::function<void(std::vector<std::string>)>;

 public:
  static constexpr std::string THREAD_NAME_ = "tradercppuiLOG";

  explicit LogFileWatcher(std::string directory, std::string filename)
      : directory_(directory), filename_(filename) {
    // OPEN FILE FOR READING LATER
    // TODO: trailing slashes on directory
    file_ = std::ifstream(directory + filename, std::ios::in);
    if (!file_) {
      throw std::runtime_error("could not open log file");
    }
    // CONFIGURE WATCHER FOR LATER
    efsw::WatchID watchID = watcher_.addWatch(directory, this, false);
    if (watchID == efsw::Errors::FileNotFound) {
      // TODO: thread_exception
      throw std::runtime_error("Watch file not found");
    }
  }

  void set_callback(Callback cb) override { cb_ = std::move(cb); }

  void start() override {
    worker_ = std::jthread{[this](const std::stop_token& stoken) {
      utils::Threading::set_thread_name(THREAD_NAME_);
      spdlog::info("starting watching log file on background thread, name [{}], id [{}]",
                   THREAD_NAME_, utils::Threading::get_os_thread_id());
      watcher_.watch();
    }};
  }

 private:
  // efsw override
  void handleFileAction(efsw::WatchID watchid,
                        const std::string& dir,
                        const std::string& filename,
                        efsw::Action action,
                        std::string oldFilename = "") override {
    if (filename != filename_) {
      return;
    }
    switch (action) {
      case efsw::Actions::Modified: {
        // TODO: paginate
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file_, line)) {
          lines.emplace_back(std::move(line));
        }
        if (file_.eof()) {
          file_.clear();
        }
        cb_(std::move(lines));
      } break;
      default:
        break;
    }
  }
  /// thread
  std::jthread worker_;
  std::function<void(std::stop_token)> worker_task_;
  // work
  std::string directory_;
  std::string filename_;
  std::ifstream file_;
  efsw::FileWatcher watcher_;
  // file-change callback
  Callback cb_;
};

}  // namespace ui
