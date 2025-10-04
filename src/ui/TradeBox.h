#pragma once

#include <ftxui/component/screen_interactive.hpp>

/*
step 1: vector of trades
step 2: sortable/searchable collection
step 3: fetch historical trades
*/

namespace UI {

class TradeBox {
 public:
  // Constructor: takes a label string
  TradeBox();

  // Return the FTXUI component to plug into layout
  ftxui::Component GetComponent();

 private:
  ftxui::Component component_;
};

}  // namespace UI
