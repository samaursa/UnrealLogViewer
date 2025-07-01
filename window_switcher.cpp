#include "window_switcher.h"

bool WindowSwitcher::HandleWindowSwitch(char c, int max_windows) {
    if (c >= '0' && c < ('0' + max_windows)) {
        selected_window_ = c - '0';
        return true;
    }
    return false;
}