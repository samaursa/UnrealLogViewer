#include "input_manager.h"
#include "ftxui/screen/terminal.hpp"

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

void InputManager::SetFileLoadCallback(std::function<void()> callback) {
    file_load_callback_ = callback;
}

void InputManager::SetLogEntries(const std::vector<LogEntry>* entries) {
    if (log_window_) {
        log_window_->SetLogEntries(entries);
    }
}

void InputManager::SetSearchCallback(std::function<void(const std::string&)> callback) {
    search_callback_ = callback;
}

void InputManager::SetSearchTerm(std::string* search_term) {
    search_term_ = search_term;
}

void InputManager::SetDebugMessage(const std::string& message) {
    debug_message_ = message;
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

        // Enter focuses correct input or triggers actions
        if (event == Event::Return) {
            escape_pressed_ = false;
            int selected = switcher_.GetSelectedWindow();
            if (selected < input_windows_.size()) {
                input_windows_[selected]->TakeFocus();
                // Trigger callbacks based on window
                if (selected == 0 && file_load_callback_) {
                    file_load_callback_();
                } else if (selected == 1 && search_callback_ && search_term_) {
                    search_callback_(*search_term_);
                }
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

    // Input windows at top
    Elements top_elements;
    for (int i = 0; i < input_windows_.size(); ++i) {
        top_elements.push_back(input_windows_[i]->Render(i == selected, escape_pressed_));
    }

    // Status bar at bottom
    auto status_text = "Window: " + std::to_string(selected) +
                      " | Focus: " + std::string(escape_pressed_ ? "OFF" : "ON");

    // Add log navigation info
    if (log_window_) {
        int total = log_window_->GetLogEntries() ? log_window_->GetLogEntries()->size() : 0;
        status_text += " | Line: " + std::to_string(log_window_->GetSelectedLine()) + "/" + std::to_string(total);
    }

    if (!debug_message_.empty()) {
        status_text += " | " + debug_message_;
    }
    auto status = text(status_text);

    // Log window fills middle, expanded at bottom
    if (log_window_ && expanded_window_) {
        int log_id = input_windows_.size();
        int expanded_id = log_id + 1;
        int selected_log_line = log_window_->GetSelectedLine();

        // Calculate available height for log window
        auto screen_size = ftxui::Terminal::Size();
        int available_height = screen_size.dimy - 6 - 8 - 3; // inputs(6) + expanded(8) + borders/status(3)

        return vbox({
            vbox(top_elements) | size(HEIGHT, EQUAL, 6),
            log_window_->Render(selected == log_id, available_height) | flex,
            expanded_window_->Render(selected == expanded_id, log_window_->GetSelectedEntry()) | size(HEIGHT, EQUAL, 8),
            status
        }) | border;
    }

    // Fallback for missing windows
    elements.insert(elements.end(), top_elements.begin(), top_elements.end());
    if (log_window_) {
        int log_id = input_windows_.size();
        elements.push_back(log_window_->Render(selected == log_id));
    }
    if (expanded_window_) {
        int expanded_id = input_windows_.size() + (log_window_ ? 1 : 0);
        const LogEntry* selected_entry = log_window_ ? log_window_->GetSelectedEntry() : nullptr;
        elements.push_back(expanded_window_->Render(selected == expanded_id, selected_entry));
    }
    elements.push_back(status);

    return vbox(elements) | border;
}
