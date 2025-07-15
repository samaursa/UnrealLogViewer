#include "input_manager.h"
#include "hierarchical_search_manager.h"
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

void InputManager::SetupHierarchicalSearch() {
    search_manager_ = std::make_unique<HierarchicalSearchManager>();
    search_manager_->SetUpdateCallback([this]() {
        if (search_update_callback_) {
            search_update_callback_();
        }
    });

    // Create input components for each search level
    UpdateSearchInputs();
}

void InputManager::UpdateSearchInputs() {
    search_inputs_.clear();

    for (size_t i = 0; i < search_manager_->GetLevelCount(); ++i) {
        auto& level = search_manager_->GetSearchLevels()[i];
        auto input = Input(&const_cast<std::string&>(level.term), "Search level " + std::to_string(i + 1));
        search_inputs_.push_back(input);
    }

    // Focus the current level
    if (search_manager_->GetFocusedLevel() < search_inputs_.size()) {
        search_inputs_[search_manager_->GetFocusedLevel()]->TakeFocus();
    }
}

void InputManager::SetFilterManager(FilterManager* manager) {
    filter_manager_ = manager;
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

void InputManager::SetSearchUpdateCallback(std::function<void()> callback) {
    search_update_callback_ = callback;
}

void InputManager::SetSearchTerm(std::string* search_term) {
    // Legacy support - not used with hierarchical search
    search_term_ = search_term;
}

void InputManager::SetDebugMessage(const std::string& message) {
    debug_message_ = message;
}

Component InputManager::CreateComponent() {
    Components components;

    // File input
    for (auto& window : input_windows_) {
        components.push_back(window->GetComponent());
    }

    // Search inputs container
    auto search_container = Container::Vertical({});
    for (auto& input : search_inputs_) {
        search_container->Add(input);
    }
    components.push_back(search_container);

    auto container = Container::Vertical(components);

    // Make container focusable and handle focus manually
    container = container | CatchEvent([this](Event event) {
        // Handle hierarchical search controls first
        if (event == Event::Character('\t')) { // Tab key
            if (search_manager_) {
                search_manager_->FocusNext();
                UpdateSearchInputs();
                return true;
            }
        }

        // F1 to add search level
        if (event == Event::F1) {
            if (search_manager_) {
                search_manager_->AddSearchLevel();
                UpdateSearchInputs();
                return true;
            }
        }

        // F2 to remove search level
        if (event == Event::F2) {
            if (search_manager_) {
                search_manager_->RemoveSearchLevel();
                UpdateSearchInputs();
                return true;
            }
        }

        // Window switching
        if (event.is_character()) {
            char c = event.character()[0];
            int total_windows = input_windows_.size() + 1 + (log_window_ ? 1 : 0) + (expanded_window_ ? 1 : 0);
            if (switcher_.HandleWindowSwitch(c, total_windows)) {
                // Don't take focus automatically - let Enter do it
                return true;
            }
        }

        // Enter focuses the selected window
        if (event == Event::Return) {
            escape_pressed_ = false;
            int selected = switcher_.GetSelectedWindow();
            if (selected == 0) {
                // Focus file input and trigger callback
                if (!input_windows_.empty()) {
                    input_windows_[0]->TakeFocus();
                }
                if (file_load_callback_) {
                    file_load_callback_();
                }
                return true;
            } else if (selected == 1) {
                // Focus current search level and trigger search
                if (search_manager_ && search_manager_->GetFocusedLevel() < search_inputs_.size()) {
                    search_inputs_[search_manager_->GetFocusedLevel()]->TakeFocus();
                }
                if (search_update_callback_) {
                    search_update_callback_();
                }
                return true;
            }
        }

        // Log window handles navigation events
        if (log_window_ && log_window_->HandleEvent(event)) {
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

    // File input at top
    if (!input_windows_.empty()) {
        elements.push_back(input_windows_[0]->Render(selected == 0, escape_pressed_));
    }

    // Hierarchical search inputs
    if (search_manager_) {
        Elements search_elements;
        search_elements.push_back(text("Hierarchical Search (Tab: switch, F1: add, F2: remove):") | bold | color(Color::Yellow));

        for (size_t i = 0; i < search_manager_->GetLevelCount(); ++i) {
            const auto& level = search_manager_->GetSearchLevels()[i];
            bool is_focused = (selected == 1 && i == search_manager_->GetFocusedLevel());

            std::string level_title = "Level " + std::to_string(i + 1);
            if (is_focused) {
                level_title = ">>> " + level_title + " <<<";
            }

            auto level_element = vbox({
                text(level_title) | (is_focused ? color(Color::Green) : color(Color::GrayLight)),
                hbox({
                    text("Search: "),
                    (i < search_inputs_.size() ? search_inputs_[i]->Render() : text(level.term)) | flex
                })
            });

            if (is_focused) {
                level_element = level_element | border;
            }

            search_elements.push_back(level_element);
        }

        elements.push_back(window(text("[1] HIERARCHICAL SEARCH"), vbox(search_elements)));
    }

    // Status bar
    auto status_text = "Window: " + std::to_string(selected) +
                      " | Focus: " + std::string(escape_pressed_ ? "OFF" : "ON");

    if (log_window_) {
        int total = log_window_->GetLogEntries() ? log_window_->GetLogEntries()->size() : 0;
        status_text += " | Line: " + std::to_string(log_window_->GetSelectedLine()) + "/" + std::to_string(total);
    }

    if (search_manager_) {
        status_text += " | Search Levels: " + std::to_string(search_manager_->GetLevelCount());
    }

    if (!debug_message_.empty()) {
        status_text += " | " + debug_message_;
    }
    auto status = text(status_text);

    // Main layout
    if (log_window_ && expanded_window_) {
        int log_id = 2; // After file(0) and search(1)
        int expanded_id = 3;

        auto screen_size = ftxui::Terminal::Size();
        int search_height = 3 + (search_manager_ ? search_manager_->GetLevelCount() * 3 : 0);
        int available_height = screen_size.dimy - 4 - search_height - 8 - 3; // file(4) + search + expanded(8) + status(3)

        Elements main_content;
        main_content.insert(main_content.end(), elements.begin(), elements.end());
        main_content.push_back(log_window_->Render(selected == log_id, available_height) | flex);
        main_content.push_back(expanded_window_->Render(selected == expanded_id, log_window_->GetSelectedEntry()) | size(HEIGHT, EQUAL, 8));

        return vbox({
            vbox(main_content) | flex,
            status
        }) | border;
    }

    // Fallback
    if (log_window_) {
        int log_id = 2;
        elements.push_back(log_window_->Render(selected == log_id));
    }
    if (expanded_window_) {
        int expanded_id = 2 + (log_window_ ? 1 : 0);
        const LogEntry* selected_entry = log_window_ ? log_window_->GetSelectedEntry() : nullptr;
        elements.push_back(expanded_window_->Render(selected == expanded_id, selected_entry));
    }
    elements.push_back(status);

    return vbox(elements) | border;
}
