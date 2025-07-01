#include "input_manager.h"

using namespace ftxui;

void InputManager::AddInputWindow(int id, const std::string& title, std::string* content, const std::string& placeholder) {
    input_windows_.push_back(std::make_unique<InputWindow>(id, title, content, placeholder));
}

void InputManager::AddLogWindow(int id, const std::string& title) {
    log_window_ = std::make_unique<LogWindow>(id, title);
}

void InputManager::AddExpandedWindow(int id, const std::string& title) {
    expanded_window_ = std::make_unique<ExpandedWindow>(id, title);
}

Component InputManager::CreateComponent() {
    Components components;
    for (auto& window : input_windows_) {
        components.push_back(window->GetComponent());
    }

    auto container = Container::Vertical(components);

    container |= CatchEvent([this](Event event) {
        // Log window always handles arrow keys and page up/down
        if (log_window_ && log_window_->HandleEvent(event)) {
            return true;
        }

        // Window switching
        if (event.is_character()) {
            char c = event.character()[0];
            int total_windows = input_windows_.size() + (log_window_ ? 1 : 0) + (expanded_window_ ? 1 : 0);
            if (switcher_.HandleWindowSwitch(c, total_windows)) {
                return true;
            }
        }

        // Enter focuses correct input
        if (event == Event::Return) {
            escape_pressed_ = false;
            int selected = switcher_.GetSelectedWindow();
            if (selected < input_windows_.size()) {
                input_windows_[selected]->TakeFocus();
            }
            return true;
        }

        if (escape_pressed_) {
            return true;
        }

        if (event == Event::Escape) {
            escape_pressed_ = true;
            return true;
        }

        return false;
    });

    return container;
}

Element InputManager::Render() const {
    Elements elements;
    int selected = switcher_.GetSelectedWindow();

    for (int i = 0; i < input_windows_.size(); ++i) {
        elements.push_back(input_windows_[i]->Render(i == selected, escape_pressed_));
    }

    if (log_window_) {
        int log_id = input_windows_.size();
        elements.push_back(log_window_->Render(selected == log_id));
    }

    if (expanded_window_) {
        int expanded_id = input_windows_.size() + (log_window_ ? 1 : 0);
        int selected_log_line = log_window_ ? log_window_->GetSelectedLine() : 0;
        elements.push_back(expanded_window_->Render(selected == expanded_id, selected_log_line));
    }

    elements.push_back(text("Window: " + std::to_string(selected) +
                           " | Focus: " + std::string(escape_pressed_ ? "OFF" : "ON")));

    return vbox(elements) | border;
}
