#pragma once

#include <ftxui/component/screen_interactive.hpp>

/*
step 1: fetch balances of each instrument
step 2: sortable/searchable collection
*/

namespace UI {

class WalletBox {
 public:
  WalletBox();

  // Return the FTXUI component to plug into layout
  ftxui::Component GetComponent();

 private:
  ftxui::Component component_;
};

}  // namespace UI
