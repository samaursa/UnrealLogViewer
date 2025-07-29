#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>

int main() {
    std::cout << "Testing search promotion bug..." << std::endl;
    
    // Create a config manager and main window
    ue_log::ConfigManager config_manager;
    ue_log::MainWindow main_window(&config_manager);
    
    // Initialize the main window
    main_window.Initialize();
    
    // Load a test log file
    bool loaded = main_window.LoadLogFile("tests/sample_logs/unreal_sample.log");
    if (!loaded) {
        std::cout << "Failed to load test log file: " << main_window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Log file loaded successfully!" << std::endl;
    
    // Show the filter panel
    main_window.ToggleFilterPanel();
    
    // Start a search for "Ability"
    main_window.ShowSearch();
    main_window.AppendToSearch("Ability");
    main_window.ConfirmSearch();
    
    std::cout << "Search confirmed. Status: " << main_window.GetLastError() << std::endl;
    
    // Promote the search to a message filter (column 4 = key 5)
    main_window.PromoteSearchToColumnFilter(4);
    
    std::cout << "Search promoted. Status: " << main_window.GetLastError() << std::endl;
    
    // Check the created filter
    auto* filter_panel = main_window.GetFilterPanel();
    if (filter_panel) {
        filter_panel->RefreshFilters();
        filter_panel->SetSelectedFilterIndex(0);
        
        const auto* selected_filter = filter_panel->GetSelectedFilter();
        if (selected_filter) {
            std::cout << "Filter name: '" << selected_filter->Get_name() << "'" << std::endl;
            std::cout << "Filter criteria: '" << selected_filter->Get_criteria() << "'" << std::endl;
            std::cout << "Filter type: " << static_cast<int>(selected_filter->Get_type()) << std::endl;
        } else {
            std::cout << "No filter found after promotion" << std::endl;
        }
    }
    
    // Test the highlight term extraction
    std::string highlight_term = main_window.GetFilterHighlightTerm();
    std::cout << "Extracted highlight term: '" << highlight_term << "'" << std::endl;
    
    return 0;
}