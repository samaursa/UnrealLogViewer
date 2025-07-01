#include "log_viewer.h"
#include "window_manager.h"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/screen/terminal.hpp"

using namespace ftxui;

Component LogViewer::CreateUI() {
    auto wm = std::make_shared<WindowManager>();

    // Create windows
    auto file_window = wm->AddWindow(0, "File");
    auto search_window = wm->AddWindow(1, "Search");
    auto log_window = wm->AddWindow(2, "Log");
    auto categories_window = wm->AddWindow(3, "Categories");
    auto expanded_window = wm->AddWindow(4, "Expanded");

    // Set content renderers
    file_window->SetContentRenderer([this] {
        return vbox({
            text("Enter file path here"),
            text("Status: " + (selected_window_ == 0 ? std::string("FOCUSED") : std::string("unfocused")))
        });
    });

    search_window->SetContentRenderer([this] {
        return vbox({
            text("Enter search terms here"),
            text("Status: " + (selected_window_ == 1 ? std::string("FOCUSED") : std::string("unfocused")))
        });
    });

    log_window->SetContentRenderer([this] {
        return vbox({
            text("Time | Category | Level | Message"),
            text("Log entries will appear here"),
            text("Line: " + std::to_string(selected_line_)),
            text("Status: " + (selected_window_ == 2 ? std::string("FOCUSED") : std::string("unfocused")))
        });
    });

    categories_window->SetContentRenderer([this] {
        return vbox({
            text("Category filters:"),
            text("[ ] LogTemp"),
            text("[x] LogCore"),
            text("[ ] LogWarning"),
            text("Status: " + (selected_window_ == 3 ? std::string("FOCUSED") : std::string("unfocused")))
        });
    });

    expanded_window->SetContentRenderer([this] {
        return vbox({
            text("Selected log line details:"),
            text("Raw: Full log line content here"),
            text("Category: LogTemp"),
            text("Time: 12:34:56"),
            text("Status: " + (selected_window_ == 4 ? std::string("FOCUSED") : std::string("unfocused")))
        });
    });

    // Set event handlers
    log_window->SetEventHandler([this](Event event) {
        if (selected_window_ == 2) {
            if (event == Event::ArrowUp) {
                selected_line_ = std::max(0, selected_line_ - 1);
                return true;
            }
            if (event == Event::ArrowDown) {
                selected_line_++;
                return true;
            }
        }
        return false;
    });

    // Update selected_window_ when focus changes
    auto main_container = wm->GetMainContainer();
    main_container |= CatchEvent([this, wm](Event event) {
        selected_window_ = wm->GetFocusedWindowId();
        return false;
    });

    return Renderer(main_container, [wm, file_window, search_window, log_window,
                                   categories_window, expanded_window, this] {
        auto screen_size = Terminal::Size();
        int height = screen_size.dimy;

        // Render all windows with layout
        auto file_content = file_window->Render() | size(HEIGHT, EQUAL, height / 12);
        auto search_content = search_window->Render() | size(HEIGHT, EQUAL, height / 12);
        auto log_content = log_window->Render() | flex;
        auto categories_content = categories_window->Render() | size(WIDTH, EQUAL, screen_size.dimx * 3 / 10);
        auto expanded_content = expanded_window->Render() | size(HEIGHT, EQUAL, height / 3);

        auto status_bar = hbox({
            text("Focus: "),
            text(selected_window_ == -1 ? "NONE" : std::to_string(selected_window_)),
            text(" | ESC: Clear | 0:File 1:Search 2:Log 3:Categories 4:Expanded")
        }) | border;

        return vbox({
            hbox({
                vbox({
                    file_content,
                    search_content
                }) | flex,
                categories_content
            }) | size(HEIGHT, EQUAL, height / 6),
            hbox({
                vbox({
                    log_content,
                    expanded_content
                }) | flex,
                text("") | size(WIDTH, EQUAL, screen_size.dimx * 3 / 10)
            }) | flex,
            status_bar
        });
    });
}

void LogViewer::Run() {
    std::cout << "\033]0;Unreal Log Viewer\007" << std::flush;
    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(CreateUI());
}

int main() {
    LogViewer viewer;
    viewer.Run();
    return 0;
}
