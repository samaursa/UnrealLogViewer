#include "log_viewer.h"
#include "input_manager.h"
#include "ftxui/component/screen_interactive.hpp"

using namespace ftxui;

Component LogViewer::CreateUI() {
    auto manager = std::make_shared<InputManager>();
    manager->AddInputWindow(0, "FILE", &file_path_, "Enter file path...");
    manager->AddInputWindow(1, "SEARCH", &search_term_, "Search logs...");
    manager->AddLogWindow(2, "LOG");

    auto component = manager->CreateComponent();

    return Renderer(component, [manager] {
        return manager->Render();
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
