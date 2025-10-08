#pragma once

#include <ftxui/component/screen_interactive.hpp>

namespace ui {

class IScreen {
 public:
  virtual ~IScreen() = default;
  virtual void loop(ftxui::Component renderer) = 0;
  virtual void post_event(const ftxui::Event&) = 0;
};

}  // namespace ui
