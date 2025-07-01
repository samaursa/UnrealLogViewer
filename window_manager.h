#pragma once
#include "focusable_window.h"
#include <vector>
#include <memory>

class WindowManager {
private:
    std::vector<std::unique_ptr<FocusableWindow>> windows_;
    int focused_window_id_;
    ftxui::Component main_container_;

public:
    WindowManager();

    FocusableWindow* AddWindow(int id, const std::string& title);
    void SetLayout(std::function<ftxui::Element()> layout_renderer);

    ftxui::Component GetMainContainer() const { return main_container_; }

    void FocusWindow(int id);
    void ClearFocus();

    FocusableWindow* GetWindow(int id);
    int GetFocusedWindowId() const { return focused_window_id_; }
};
