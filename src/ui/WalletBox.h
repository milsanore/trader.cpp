#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

/*
step 1: fetch balances of each instrument
step 2: sortable/searchable collection
*/

namespace UI {

class WalletBox {
 public:
  // Constructor: takes a label string
  WalletBox(const std::string& initial_label);

  // Update the label
  void SetLabel(const std::string& new_label);

  // Return the FTXUI component to plug into layout
  ftxui::Component GetComponent();

 private:
  std::string label_;
  ftxui::Component component_;
};

}  // namespace UI
