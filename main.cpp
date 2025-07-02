#include "log_viewer.h"
#include "input_manager.h"
#include "ftxui/component/screen_interactive.hpp"

using namespace ftxui;

Component LogViewer::CreateUI() {
    // Set default file path for debugging
    if (file_path_.empty()) {
        file_path_ = "test.log";
    }

    auto manager = std::make_shared<InputManager>();
    manager->AddInputWindow(0, "FILE", &file_path_, "Enter file path...");
    manager->AddInputWindow(1, "SEARCH", &search_term_, "Search logs...");
    manager->AddLogWindow(2, "LOG");
    manager->AddExpandedWindow(3, "EXPANDED");

    // Set file load callback
    manager->SetFileLoadCallback([this, manager]() {
        manager->SetDebugMessage("Loading file: " + file_path_);
        LoadFile();
        manager->SetDebugMessage("Loaded " + std::to_string(log_entries_.size()) + " entries");
        manager->SetLogEntries(&log_entries_);
    });

    auto component = manager->CreateComponent();

    return Renderer(component, [manager] {
        return manager->Render();
    });
}

void LogViewer::Run() {
    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(CreateUI());
}

int main() {
    LogViewer viewer;
    viewer.Run();
    return 0;
}
