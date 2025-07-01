#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

class LogViewer;

class UIRenderer {
private:
    LogViewer* viewer_;

public:
    explicit UIRenderer(LogViewer* viewer) : viewer_(viewer) {}

    ftxui::Component CreateEventHandler(ftxui::Component main_container);
    ftxui::Element RenderHeader(ftxui::Component file_input, ftxui::Component load_button,
                               ftxui::Component tail_checkbox, ftxui::Component filter_checkbox,
                               ftxui::Component search_input) const;
    ftxui::Element RenderLogTable(int table_height) const;
    ftxui::Element RenderExpandedView() const;
    ftxui::Element RenderFilterPanel(int filter_width, int log_height) const;
    ftxui::Element RenderStatusBar() const;
};
