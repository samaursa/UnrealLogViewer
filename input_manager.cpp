#include "input_manager.h"

using namespace ftxui;

void InputManager::AddInputWindow(int id, const std::string& title, std::string* content, const std::string& placeholder) {
    windows_.push_back(std::make_unique<InputWindow>(id, title, content, placeholder));
}

Component InputManager::CreateComponent() {
    Components components;
    for (auto& window : windows_) {
        components.push_back(window->GetComponent());
    }

    auto container = Container::Vertical(components);

    container |= CatchEvent([this](Event event) {
        // Window switching
        if (event.is_character()) {
            char c = event.character()[0];
            if (switcher_.HandleWindowSwitch(c, windows_.size())) {
                return true;
            }
        }

        // Enter focuses correct input
        if (event == Event::Return) {
            escape_pressed_ = false;
            int selected = switcher_.GetSelectedWindow();
            if (selected < windows_.size()) {
                windows_[selected]->TakeFocus();
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

    for (int i = 0; i < windows_.size(); ++i) {
        elements.push_back(windows_[i]->Render(i == selected, escape_pressed_));
    }

    elements.push_back(text("Window: " + std::to_string(selected) +
                           " | Focus: " + std::string(escape_pressed_ ? "OFF" : "ON")));

    return vbox(elements) | border;
}