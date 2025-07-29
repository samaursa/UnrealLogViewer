#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Testing new UX features..." << std::endl;
    
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
    
    // Test search highlighting functionality
    std::cout << "Testing search highlighting..." << std::endl;
    main_window.ShowSearch();
    main_window.AppendToSearch("Error");
    
    // Test clipboard functionality (this will show an error message on non-Windows or if clipboard fails)
    std::cout << "Testing clipboard functionality..." << std::endl;
    main_window.CopyCurrentLineToClipboard();
    std::cout << "Clipboard test result: " << main_window.GetLastError() << std::endl;
    
    // Test exclude filter creation
    std::cout << "Testing exclude filter creation..." << std::endl;
    main_window.CreateDirectColumnExcludeFilter(3); // Exclude by log level
    std::cout << "Exclude filter test result: " << main_window.GetLastError() << std::endl;
    
    std::cout << "All tests completed!" << std::endl;
    return 0;
}