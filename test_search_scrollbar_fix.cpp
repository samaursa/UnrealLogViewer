#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>

int main() {
    // Create a test log file with multiple entries containing "test"
    std::ofstream test_file("test_search_scrollbar.log");
    test_file << "[2024-01-01 10:00:00] INFO: Starting application\n";
    test_file << "[2024-01-01 10:00:01] DEBUG: Loading configuration\n";
    test_file << "[2024-01-01 10:00:02] INFO: This is a test message\n";
    test_file << "[2024-01-01 10:00:03] WARNING: Memory usage high\n";
    test_file << "[2024-01-01 10:00:04] ERROR: Failed to connect\n";
    test_file << "[2024-01-01 10:00:05] INFO: Another test entry here\n";
    test_file << "[2024-01-01 10:00:06] DEBUG: Processing data\n";
    test_file << "[2024-01-01 10:00:07] INFO: Test completed successfully\n";
    test_file << "[2024-01-01 10:00:08] INFO: Shutting down\n";
    test_file << "[2024-01-01 10:00:09] INFO: Final test message\n";
    test_file.close();
    
    // Create config manager and main window
    ue_log::ConfigManager config_manager;
    ue_log::MainWindow main_window(&config_manager);
    
    // Initialize and load the test file
    main_window.Initialize();
    main_window.SetTerminalSize(120, 30);
    
    if (!main_window.LoadLogFile("test_search_scrollbar.log")) {
        std::cerr << "Failed to load test file: " << main_window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Test file loaded successfully!" << std::endl;
    std::cout << "Total entries: " << main_window.GetDisplayedEntries().size() << std::endl;
    
    // Test search functionality
    main_window.PerformSearch("test");
    std::cout << "Search performed for 'test'" << std::endl;
    
    // Test navigation - should find entries at positions 2, 5, 7, 9 (0-indexed)
    std::cout << "Current selected entry: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    // Test FindNext from current position
    main_window.FindNext();
    std::cout << "After FindNext: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    main_window.FindNext();
    std::cout << "After second FindNext: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    // Test FindPrevious
    main_window.FindPrevious();
    std::cout << "After FindPrevious: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    std::cout << "Search and scrollbar fix test completed!" << std::endl;
    
    // Clean up
    std::remove("test_search_scrollbar.log");
    
    return 0;
}