#ifndef UI_FTX_UI_SCREEN_H
#define UI_FTX_UI_SCREEN_H

#include <ftxui/component/screen_interactive.hpp>

#include "IScreen.h"

namespace UI {

class FtxuiScreen : public IScreen {
 public:
  FtxuiScreen() : screen_(ftxui::ScreenInteractive::Fullscreen()) {}

  void Loop(ftxui::Component renderer) override { screen_.Loop(renderer); }

  void PostEvent(const ftxui::Event &event) override { screen_.PostEvent(event); }

 private:
  ftxui::ScreenInteractive screen_;
};

}  // namespace UI

#endif  // UI_FTX_UI_SCREEN_H
