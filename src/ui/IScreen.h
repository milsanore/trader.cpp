#ifndef UIISCREEN_H
#define UIISCREEN_H

#include <ftxui/component/screen_interactive.hpp>

namespace UI {

class IScreen {
public:
    virtual void Loop(ftxui::Component renderer) = 0;
    virtual void PostEvent(const ftxui::Event&) = 0;
    virtual ~IScreen() = default;
};

} // UI

#endif //UIISCREEN_H
