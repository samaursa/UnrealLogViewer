#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>

int main() {
    std::cout << "Testing updated keyboard shortcuts..." << std::endl;
    
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
    
    // Test include filter creation (number keys 1-5)
    std::cout << "Testing include filter creation..." << std::endl;
    main_window.CreateDirectColumnFilter(2); // Logger column
    std::cout << "Include filter test result: " << main_window.GetLastError() << std::endl;
    
    // Test exclude filter creation (SHIFT+number keys)
    std::cout << "Testing exclude filter creation..." << std::endl;
    main_window.CreateDirectColumnExcludeFilter(2); // Logger column exclude
    std::cout << "Exclude filter test result: " << main_window.GetLastError() << std::endl;
    
    // Test search promotion with new column mapping
    std::cout << "Testing search promotion with new column mapping..." << std::endl;
    main_window.ShowSearch();
    main_window.AppendToSearch("Error");
    main_window.ConfirmSearch(); // This should enable search promotion
    
    // Test promoting to different columns (this would normally be done via keyboard)
    main_window.PromoteSearchToColumnFilter(4); // Key 5 - Message contains
    std::cout << "Search promotion test result: " << main_window.GetLastError() << std::endl;
    
    std::cout << "All keyboard shortcut tests completed!" << std::endl;
    return 0;
}