#pragma once

#include <ftxui/component/screen_interactive.hpp>

/*
step 1: fetch balances of each instrument
step 2: sortable/searchable collection
*/

namespace ui {

class TrafficBox {
 public:
  TrafficBox();

  // Return the FTXUI component to plug into layout
  ftxui::Component get_component();

 private:
  ftxui::Component component_;
};

}  // namespace ui
