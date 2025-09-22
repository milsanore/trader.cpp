#ifndef UIFTXUISCREEN_H
#define UIFTXUISCREEN_H

#include <ftxui/component/screen_interactive.hpp>
#include "IScreen.h"

namespace UI {

class FtxuiScreen : public IScreen {
public:
    FtxuiScreen() : screen_(ftxui::ScreenInteractive::TerminalOutput()) {}

    void Loop(ftxui::Component renderer) override {
        screen_.Loop(renderer);
    }

    void PostEvent(const ftxui::Event& event) override {
        screen_.PostEvent(event);
    }

private:
    ftxui::ScreenInteractive screen_;
};

}

#endif //UIFTXUISCREEN_H
