#pragma once

#include <ftxui/component/screen_interactive.hpp>

#include "iscreen.h"

namespace ui {

class FtxuiScreen : public IScreen {
 public:
  FtxuiScreen() : screen_(ftxui::ScreenInteractive::Fullscreen()) {}

  void loop(ftxui::Component renderer) override { screen_.Loop(renderer); }

  void post_event(const ftxui::Event &event) override { screen_.PostEvent(event); }

 private:
  ftxui::ScreenInteractive screen_;
};

}  // namespace ui
