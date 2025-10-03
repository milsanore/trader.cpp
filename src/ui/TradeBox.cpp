#include "TradeBox.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

namespace UI {

TradeBox::TradeBox(const std::string& initial_label) : label_(initial_label) {
  component_ = Renderer([this](bool focused) {
    auto body = text(focused ? "Focused" : "Not Focused");
    return vbox(body) | border | (focused ? bold : dim);
  });
}

void TradeBox::SetLabel(const std::string& new_label) {
  if (label_ != new_label) {
    label_ = new_label;
    // You must call screen->PostEvent(...) from outside
  }
}

Component TradeBox::GetComponent() { return component_; }

}  // namespace UI
