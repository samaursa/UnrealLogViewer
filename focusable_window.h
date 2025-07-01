#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <functional>

class WindowManager;

class FocusableWindow {
private:
    int id_;
    std::string title_;
    ftxui::Component container_;
    std::function<ftxui::Element()> content_renderer_;
    std::function<bool(ftxui::Event)> event_handler_;
    WindowManager* window_manager_;

public:
    FocusableWindow(int id, const std::string& title, WindowManager* wm);

    void SetContentRenderer(std::function<ftxui::Element()> renderer);
    void SetEventHandler(std::function<bool(ftxui::Event)> handler);
    void AddComponent(ftxui::Component component);

    ftxui::Component GetContainer() const { return container_; }
    ftxui::Element Render() const;

    bool IsFocused() const;
    void TakeFocus();
    int GetId() const { return id_; }
};
