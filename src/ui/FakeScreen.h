#ifndef UIFAKESCREEN_H
#define UIFAKESCREEN_H

#include <vector>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include "IScreen.h"

namespace UI {

class FakeScreen : public IScreen {
public:
    void Loop(ftxui::Component renderer) override {
        loop_called = true;
        last_renderer = renderer;

        // Optionally simulate one render
        if (simulate_loop) {
            renderer->Render();
        }
    }

    void PostEvent(const ftxui::Event& event) override {
        posted_events.push_back(event);
    }

    // For test inspection
    bool loop_called = false;
    ftxui::Component last_renderer;
    std::vector<ftxui::Event> posted_events;

    bool simulate_loop = false; // Enable to auto-render once during test
};

}

#endif //UIFAKESCREEN_H