#pragma once

#include <ftxui/component/screen_interactive.hpp>

/*
step 1: vector of trades
step 2: sortable/searchable collection
step 3: fetch historical trades
*/

namespace ui {

class TradeBox {
 public:
  // Constructor: takes a label string
  TradeBox();

  // Return the FTXUI component to plug into layout
  ftxui::Component get_component();

 private:
  ftxui::Component component_;
};

}  // namespace ui
