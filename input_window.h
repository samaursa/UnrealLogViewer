#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <string>

class InputWindow {
private:
    int id_;
    std::string title_;
    std::string* content_;
    std::string placeholder_;
    ftxui::Component input_;

public:
    InputWindow(int id, const std::string& title, std::string* content, const std::string& placeholder);

    ftxui::Component GetComponent() const { return input_; }
    void TakeFocus() { input_->TakeFocus(); }

    ftxui::Element Render(bool is_selected, bool is_unfocused) const;
};