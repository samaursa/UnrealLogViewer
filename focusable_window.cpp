#include "focusable_window.h"
#include "window_manager.h"
#include <algorithm>

using namespace ftxui;

FocusableWindow::FocusableWindow(int id, const std::string& title, WindowManager* wm)
    : id_(id), title_(title), window_manager_(wm) {

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

void FocusableWindow::AddComponent(Component component) {
    container_->Add(component);
}

bool FocusableWindow::IsFocused() const {
    return window_manager_ && window_manager_->GetFocusedWindowId() == id_;
}

void FocusableWindow::TakeFocus() {
    if (window_manager_) {
        window_manager_->SetFocusedWindow(id_);
    }
    container_->TakeFocus();
}

Element FocusableWindow::Render() const {
    std::string display_title = "[" + std::to_string(id_) + "] " + title_;

    if (IsFocused()) {
        std::transform(display_title.begin(), display_title.end(),
                      display_title.begin(), ::toupper);
    }

    Element content = content_renderer_ ? content_renderer_() : text("Empty window");
    return window(text(display_title), content);
}
