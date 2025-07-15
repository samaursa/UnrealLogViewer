#include "log_viewer.h"
#include "input_manager.h"
#include "ftxui/component/screen_interactive.hpp"

using namespace ftxui;

Component LogViewer::CreateUI() {
    if (file_path_.empty()) {
        file_path_ = "test.log";
    }

    auto manager = std::make_shared<InputManager>();
    manager->AddInputWindow(0, "FILE", &file_path_, "Enter file path...");
    manager->AddLogWindow(2, "LOG");
    manager->AddExpandedWindow(3, "EXPANDED");
    manager->SetupHierarchicalSearch();

    manager->SetFilterManager(&filter_manager_);

    manager->SetFileLoadCallback([this, manager]() {
        manager->SetDebugMessage("Loading file: " + file_path_);
        LoadFile();
        manager->SetDebugMessage("Loaded " + std::to_string(log_entries_.size()) + " entries");
        manager->SetLogEntries(&log_entries_);

        // Initialize first search level with all entries
        if (auto* search_mgr = manager->GetSearchManager()) {
            std::vector<size_t> all_indices;
            for (size_t i = 0; i < log_entries_.size(); ++i) {
                all_indices.push_back(i);
            }
            search_mgr->UpdateFilteredIndices(0, all_indices);
        }

        UpdateFilteredEntries();
        if (auto* log_window = manager->GetLogWindow()) {
            log_window->SetFilteredEntries(&filtered_indices_);
        }
    });

    manager->SetSearchUpdateCallback([this, manager]() {
        if (auto* search_mgr = manager->GetSearchManager()) {
            const auto& levels = search_mgr->GetSearchLevels();

            // Start with all entries for level 0
            std::vector<size_t> current_indices;
            for (size_t i = 0; i < log_entries_.size(); ++i) {
                current_indices.push_back(i);
            }

            // Apply each search level sequentially
            for (size_t level = 0; level < levels.size(); ++level) {
                const std::string& term = levels[level].term;
                if (!term.empty()) {
                    std::vector<size_t> filtered;
                    for (size_t idx : current_indices) {
                        if (filter_manager_.MatchesFilters(log_entries_[idx], term)) {
                            filtered.push_back(idx);
                        }
                    }
                    current_indices = filtered;
                }

                // Update this level's filtered indices
                search_mgr->UpdateFilteredIndices(level, current_indices);
            }

            // Update the main filtered indices with the final result
            filtered_indices_ = current_indices;
            if (auto* log_window = manager->GetLogWindow()) {
                log_window->SetFilteredEntries(&filtered_indices_);
            }

            manager->SetDebugMessage("Hierarchical search: " + std::to_string(filtered_indices_.size()) + " matches");
        }
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
