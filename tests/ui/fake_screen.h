#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <vector>

#include "ui/app/iscreen.h"

class FakeScreen : public ui::IScreen {
 public:
  void loop(ftxui::Component renderer) override {
    loop_called = true;
    last_renderer = renderer;

    // Optionally simulate one render
    if (simulate_loop) {
      renderer->Render();
    }
  }

  void post_event(const ftxui::Event& event) override { posted_events.push_back(event); }

  // For test inspection
  bool loop_called = false;
  ftxui::Component last_renderer;
  std::vector<ftxui::Event> posted_events;

  bool simulate_loop = false;  // Enable to auto-render once during test
};
