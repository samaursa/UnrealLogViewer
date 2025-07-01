#include "focusable_window.h"
#include <algorithm>

using namespace ftxui;

FocusableWindow::FocusableWindow(int id, const std::string& title, std::function<int()> get_focused_id, std::function<void(int)> set_focused_id)
    : id_(id), title_(title), get_focused_id_(get_focused_id), set_focused_id_(set_focused_id) {
    container_ = Container::Vertical({});

    container_ |= CatchEvent([this](Event event) {
        if (event_handler_) {
            return event_handler_(event);
        }
        return false;
    });
}

void FocusableWindow::SetContentRenderer(std::function<Element()> renderer) {
    content_renderer_ = renderer;
}

void FocusableWindow::SetEventHandler(std::function<bool(Event)> handler) {
    event_handler_ = handler;
}

bool FocusableWindow::IsFocused() const {
    return get_focused_id_() == id_;
}

void FocusableWindow::TakeFocus() {
    set_focused_id_(id_);
    container_->TakeFocus();
}

Element FocusableWindow::Render() const {
    std::string display_title = "[" + std::to_string(id_) + "] " +
        (IsFocused() ? title_ : title_);

    // Convert to uppercase when focused
    if (IsFocused()) {
        std::transform(display_title.begin(), display_title.end(),
                      display_title.begin(), ::toupper);
    }

    Element content = content_renderer_ ? content_renderer_() : text("Empty window");

    return window(text(display_title), content);
}
