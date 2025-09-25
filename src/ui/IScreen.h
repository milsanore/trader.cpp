#ifndef UI_ISCREEN_H
#define UI_ISCREEN_H

#include <ftxui/component/screen_interactive.hpp>

namespace UI {

class IScreen {
 public:
  virtual void Loop(ftxui::Component renderer) = 0;
  virtual void PostEvent(const ftxui::Event &) = 0;
  virtual ~IScreen() = default;
};

}  // namespace UI

#endif  // UI_ISCREEN_H
