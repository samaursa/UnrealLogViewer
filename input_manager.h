#pragma once
#include "input_window.h"
#include "log_window.h"
#include "expanded_window.h"
#include "categories_window.h"
#include "window_switcher.h"
#include "filter_manager.h"
#include "ftxui/component/component.hpp"
#include <vector>
#include <memory>

class InputManager {
private:
    std::vector<std::unique_ptr<InputWindow>> input_windows_;
    std::unique_ptr<LogWindow> log_window_;
    std::unique_ptr<ExpandedWindow> expanded_window_;
    std::unique_ptr<CategoriesWindow> categories_window_;
    WindowSwitcher switcher_;
    bool escape_pressed_;
    std::string debug_message_;
    std::string* search_term_;
    std::function<void()> file_load_callback_;
    std::function<void(const std::string&)> search_callback_;

public:
    InputManager() : escape_pressed_(false), search_term_(nullptr) {}

    void AddInputWindow(int id, const std::string& title, std::string* content, const std::string& placeholder);
    void AddLogWindow(int id, const std::string& title);
    void AddExpandedWindow(int id, const std::string& title);
    void AddCategoriesWindow(int id, const std::string& title);
    void SetFileLoadCallback(std::function<void()> callback);
    void SetLogEntries(const std::vector<LogEntry>* entries);
    void SetSearchCallback(std::function<void(const std::string&)> callback);
    void SetSearchTerm(std::string* search_term);
    void SetFilterManager(FilterManager* manager);
    void SetDebugMessage(const std::string& message);
    LogWindow* GetLogWindow() const { return log_window_.get(); }
    ftxui::Component CreateComponent();
    ftxui::Element Render() const;
};
