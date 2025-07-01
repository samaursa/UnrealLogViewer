#include "log_viewer.h"
#include "ftxui/component/screen_interactive.hpp"

using namespace ftxui;

Component LogViewer::CreateUI() {
    auto file_input = Input(&file_path_, "Enter file path...");
    auto search_input = Input(&search_term_, "Search logs...");
    auto container = Container::Vertical({file_input, search_input});

    container |= CatchEvent([this, file_input, search_input](Event event) {
        // Window switching
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '0' && c <= '1') {
                selected_window_ = c - '0';
                return true;
            }
        }

        // Enter focuses the correct input based on selected window
        if (event == Event::Return) {
            escape_pressed_ = false;
            if (selected_window_ == 0) {
                file_input->TakeFocus();
            } else if (selected_window_ == 1) {
                search_input->TakeFocus();
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

    return Renderer(container, [this, file_input, search_input] {
        auto file_element = escape_pressed_ ?
            text(file_path_.empty() ? "Enter file path..." : file_path_) | color(Color::GrayDark) :
            file_input->Render();

        auto search_element = escape_pressed_ ?
            text(search_term_.empty() ? "Search logs..." : search_term_) | color(Color::GrayDark) :
            search_input->Render();

        auto file_title = selected_window_ == 0 ? "[0] FILE" : "[0] file";
        auto search_title = selected_window_ == 1 ? "[1] SEARCH" : "[1] search";

        return vbox({
            window(text(file_title), vbox({
                hbox({
                    text("File: "),
                    file_element | flex
                }),
                text("Path: " + file_path_)
            })),
            window(text(search_title), vbox({
                hbox({
                    text("Search: "),
                    search_element | flex
                }),
                text("Term: " + search_term_)
            })),
            text("Window: " + std::to_string(selected_window_) + " | Focus: " + std::string(escape_pressed_ ? "OFF" : "ON"))
        }) | border;
    });
}

void LogViewer::Run() {
    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(CreateUI());
}

int main() {
    LogViewer viewer;
    viewer.Run();
    return 0;
}
