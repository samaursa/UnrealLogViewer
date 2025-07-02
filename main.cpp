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

    // Set search term reference
    manager->SetSearchTerm(&search_term_);

    // Set file load callback
    manager->SetFileLoadCallback([this, manager]() {
        manager->SetDebugMessage("Loading file: " + file_path_);
        LoadFile();
        manager->SetDebugMessage("Loaded " + std::to_string(log_entries_.size()) + " entries");
        manager->SetLogEntries(&log_entries_);
        UpdateFilteredEntries();
        if (auto* log_window = manager->GetLogWindow()) {
            log_window->SetFilteredEntries(&filtered_indices_);
        }
    });

    // Set search callback
    manager->SetSearchCallback([this, manager](const std::string& term) {
        search_term_ = term;
        UpdateFilteredEntries();
        if (auto* log_window = manager->GetLogWindow()) {
            log_window->SetFilteredEntries(&filtered_indices_);
        }
        manager->SetDebugMessage("Search: " + std::to_string(filtered_indices_.size()) + " matches");
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
