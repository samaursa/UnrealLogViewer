#pragma once
#include "input_window.h"
#include "log_window.h"
#include "expanded_window.h"
#include "window_switcher.h"
#include "ftxui/component/component.hpp"
#include <vector>
#include <memory>

class InputManager {
private:
    std::vector<std::unique_ptr<InputWindow>> input_windows_;
    std::unique_ptr<LogWindow> log_window_;
    std::unique_ptr<ExpandedWindow> expanded_window_;
    WindowSwitcher switcher_;
    bool escape_pressed_;

public:
    InputManager() : escape_pressed_(false) {}

    void AddInputWindow(int id, const std::string& title, std::string* content, const std::string& placeholder);
    void AddLogWindow(int id, const std::string& title);
    void AddExpandedWindow(int id, const std::string& title);
    ftxui::Component CreateComponent();
    ftxui::Element Render() const;
};
