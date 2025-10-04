#pragma once

#include <ftxui/component/screen_interactive.hpp>

namespace ui {

class IScreen {
 public:
  virtual ~IScreen() = default;
  virtual void Loop(ftxui::Component renderer) = 0;
  virtual void PostEvent(const ftxui::Event &) = 0;
};

}  // namespace ui

#pragma once
