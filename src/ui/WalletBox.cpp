#include "WalletBox.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

namespace UI {

WalletBox::WalletBox(const std::string& initial_label) : label_(initial_label) {
  // component_ =
  //     Renderer([&] { return window(text("Wallet Box") | bold, text(label_) | center);
  //     });

  component_ = Renderer([this](bool focused) {
    auto body = text(focused ? "Focused" : "Not Focused");
    return vbox(body) | border | (focused ? bold : dim);
  });
}

void WalletBox::SetLabel(const std::string& new_label) {
  if (label_ != new_label) {
    label_ = new_label;
    // You must call screen->PostEvent(...) from outside
  }
}

Component WalletBox::GetComponent() { return component_; }

}  // namespace UI
