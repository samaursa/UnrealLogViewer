#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>

int main() {
    std::cout << "Debugging filter description duplication..." << std::endl;
    
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
    
    std::cout << "=== Testing Search Promotion ===" << std::endl;
    
    // Test 1: Simple search promotion
    std::cout << "\n1. Testing simple search promotion:" << std::endl;
    main_window.ShowSearch();
    main_window.AppendToSearch("Ability");
    std::cout << "Search query after append: '" << main_window.GetLastError() << "'" << std::endl;
    
    main_window.ConfirmSearch();
    std::cout << "Search confirmed: '" << main_window.GetLastError() << "'" << std::endl;
    
    // Show filter panel and promote
    main_window.ToggleFilterPanel();
    main_window.PromoteSearchToColumnFilter(4); // Message contains
    std::cout << "Search promoted: '" << main_window.GetLastError() << "'" << std::endl;
    
    // Check the created filter
    auto* filter_panel = main_window.GetFilterPanel();
    if (filter_panel) {
        filter_panel->RefreshFilters();
        const auto* selected_filter = filter_panel->GetSelectedFilter();
        if (selected_filter) {
            std::cout << "Filter name: '" << selected_filter->Get_name() << "'" << std::endl;
            std::cout << "Filter criteria: '" << selected_filter->Get_criteria() << "'" << std::endl;
        }
    }
    
    std::cout << "\n=== Testing Direct Filter Creation ===" << std::endl;
    
    // Test 2: Direct filter creation
    std::cout << "\n2. Testing direct message filter creation:" << std::endl;
    main_window.CreateDirectColumnFilter(4); // Message column
    std::cout << "Direct filter created: '" << main_window.GetLastError() << "'" << std::endl;
    
    // Check the new filter
    if (filter_panel) {
        filter_panel->RefreshFilters();
        filter_panel->SetSelectedFilterIndex(1); // Second filter
        const auto* selected_filter = filter_panel->GetSelectedFilter();
        if (selected_filter) {
            std::cout << "Second filter name: '" << selected_filter->Get_name() << "'" << std::endl;
            std::cout << "Second filter criteria: '" << selected_filter->Get_criteria() << "'" << std::endl;
        }
    }
    
    return 0;
}