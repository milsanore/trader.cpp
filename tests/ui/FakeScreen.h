#ifndef TEST_FAKE_SCREEN_H
#define TEST_FAKE_SCREEN_H

#include <vector>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include "ui/IScreen.h"

class FakeScreen : public UI::IScreen {
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

#endif
