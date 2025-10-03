#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

/*
step 1: vector of trades
step 2: sortable/searchable collection
step 3: fetch historical trades
*/

namespace UI {

class TradeBox {
 public:
  // Constructor: takes a label string
  TradeBox(const std::string& initial_label);

  // Update the label
  void SetLabel(const std::string& new_label);

  // Return the FTXUI component to plug into layout
  ftxui::Component GetComponent();

 private:
  std::string label_;
  ftxui::Component component_;
};

}  // namespace UI
