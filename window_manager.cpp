#include "window_manager.h"

using namespace ftxui;

WindowManager::WindowManager() : focused_window_id_(-1) {
    main_container_ = Container::Vertical({});

    // Global focus switching
    main_container_ |= CatchEvent([this](Event event) {
        if (event == Event::Escape) {
            ClearFocus();
            return true;
        }

        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '0' && c <= '9') {
                int id = c - '0';
                FocusWindow(id);
                return true;
            }
        }

        return false;
    });
}

FocusableWindow* WindowManager::AddWindow(int id, const std::string& title) {
    auto window = std::make_unique<FocusableWindow>(id, title, this);
    FocusableWindow* ptr = window.get();
    windows_.push_back(std::move(window));
    main_container_->Add(ptr->GetContainer());
    return ptr;
}

void WindowManager::FocusWindow(int id) {
    auto* window = GetWindow(id);
    if (window) {
        window->TakeFocus();
    }
}

void WindowManager::ClearFocus() {
    focused_window_id_ = -1;
}

FocusableWindow* WindowManager::GetWindow(int id) {
    auto it = std::find_if(windows_.begin(), windows_.end(),
                          [id](const auto& w) {
                              return w->GetId() == id;
                          });
    return it != windows_.end() ? it->get() : nullptr;
}
