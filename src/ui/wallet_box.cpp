#include "wallet_box.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using ftxui::bold;
using ftxui::border;
using ftxui::Component;
using ftxui::dim;
using ftxui::Renderer;
using ftxui::text;
using ftxui::vbox;

namespace ui {

WalletBox::WalletBox() {
  component_ = Renderer([this](bool focused) {
    auto body = text(focused ? "Focused" : "Not Focused");
    return vbox(body) | border | (focused ? bold : dim);
  });
}

Component WalletBox::get_component() {
  return component_;
}

}  // namespace ui
