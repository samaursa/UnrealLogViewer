#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>

int main() {
    std::cout << "Testing filter highlighting functionality..." << std::endl;
    
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
    std::cout << "Filter panel visibility: " << (main_window.IsFilterPanelVisible() ? "visible" : "hidden") << std::endl;
    
    // Create a text-based include filter that should be highlighted
    main_window.CreateDirectColumnFilter(4); // Message column filter
    std::cout << "Created message filter: " << main_window.GetLastError() << std::endl;
    
    // Check if filter panel has filters and select one
    auto* filter_panel = main_window.GetFilterPanel();
    if (filter_panel) {
        filter_panel->RefreshFilters();
        filter_panel->SetSelectedFilterIndex(0); // Select the first filter
        
        const auto* selected_filter = filter_panel->GetSelectedFilter();
        if (selected_filter) {
            std::cout << "Selected filter type: " << static_cast<int>(selected_filter->Get_type()) << std::endl;
            std::cout << "Selected filter criteria: '" << selected_filter->Get_criteria() << "'" << std::endl;
            std::cout << "Selected filter state: " << static_cast<int>(selected_filter->GetFilterState()) << std::endl;
        } else {
            std::cout << "No filter selected" << std::endl;
        }
    }
    
    // Test getting the filter highlight term
    std::string highlight_term = main_window.GetFilterHighlightTerm();
    std::cout << "Filter highlight term: '" << highlight_term << "'" << std::endl;
    
    // Create an exclude filter that should NOT be highlighted
    main_window.CreateDirectColumnExcludeFilter(3); // Level column exclude filter
    std::cout << "Created exclude filter: " << main_window.GetLastError() << std::endl;
    
    // Test that exclude filters don't provide highlight terms
    std::string exclude_highlight = main_window.GetFilterHighlightTerm();
    std::cout << "Exclude filter highlight term: '" << exclude_highlight << "'" << std::endl;
    
    std::cout << "Filter highlighting tests completed!" << std::endl;
    return 0;
}