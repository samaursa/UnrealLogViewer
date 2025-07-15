#pragma once
#include "input_window.h"
#include "log_window.h"
#include "expanded_window.h"
#include "window_switcher.h"
#include "filter_manager.h"
#include "hierarchical_search_manager.h"
#include "ftxui/component/component.hpp"
#include <vector>
#include <memory>

class InputManager {
private:
    std::vector<std::unique_ptr<InputWindow>> input_windows_;
    std::unique_ptr<LogWindow> log_window_;
    std::unique_ptr<ExpandedWindow> expanded_window_;
    std::unique_ptr<HierarchicalSearchManager> search_manager_;
    std::vector<ftxui::Component> search_inputs_;
    WindowSwitcher switcher_;
    bool escape_pressed_;
    std::string debug_message_;
    std::string* search_term_; // Legacy
    std::function<void()> file_load_callback_;
    std::function<void(const std::string&)> search_callback_;
    std::function<void()> search_update_callback_;
    FilterManager* filter_manager_;

    void UpdateSearchInputs();

public:
    InputManager() : escape_pressed_(false), search_term_(nullptr), filter_manager_(nullptr) {}

    void AddInputWindow(int id, const std::string& title, std::string* content, const std::string& placeholder);
    void AddLogWindow(int id, const std::string& title);
    void AddExpandedWindow(int id, const std::string& title);
    void SetupHierarchicalSearch();
    void SetFileLoadCallback(std::function<void()> callback);
    void SetLogEntries(const std::vector<LogEntry>* entries);
    void SetSearchCallback(std::function<void(const std::string&)> callback);
    void SetSearchUpdateCallback(std::function<void()> callback);
    void SetSearchTerm(std::string* search_term); // Legacy
    void SetFilterManager(FilterManager* manager);
    void SetDebugMessage(const std::string& message);

    LogWindow* GetLogWindow() const { return log_window_.get(); }
    HierarchicalSearchManager* GetSearchManager() const { return search_manager_.get(); }

    ftxui::Component CreateComponent();
    ftxui::Element Render() const;
};
