#pragma once

class WindowSwitcher {
private:
    int selected_window_;

public:
    WindowSwitcher() : selected_window_(0) {}

    int GetSelectedWindow() const { return selected_window_; }
    void SetSelectedWindow(int window) { selected_window_ = window; }

    bool HandleWindowSwitch(char c, int max_windows);
};