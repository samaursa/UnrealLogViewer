#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>

int main() {
    // Create a larger test log file with multiple entries containing "test"
    std::ofstream test_file("test_comprehensive.log");
    for (int i = 0; i < 50; ++i) {
        if (i % 5 == 2) {
            test_file << "[2024-01-01 10:00:" << std::setfill('0') << std::setw(2) << i 
                     << "] INFO: This is a test message " << i << "\n";
        } else {
            test_file << "[2024-01-01 10:00:" << std::setfill('0') << std::setw(2) << i 
                     << "] DEBUG: Regular log entry " << i << "\n";
        }
    }
    test_file.close();
    
    // Create config manager and main window
    ue_log::ConfigManager config_manager;
    ue_log::MainWindow main_window(&config_manager);
    
    // Initialize and load the test file
    main_window.Initialize();
    main_window.SetTerminalSize(120, 30);
    
    if (!main_window.LoadLogFile("test_comprehensive.log")) {
        std::cerr << "Failed to load test file: " << main_window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Test file loaded successfully!" << std::endl;
    std::cout << "Total entries: " << main_window.GetDisplayedEntries().size() << std::endl;
    
    // Test search functionality
    main_window.PerformSearch("test");
    std::cout << "Search performed for 'test'" << std::endl;
    
    // Expected matches should be at positions: 2, 7, 12, 17, 22, 27, 32, 37, 42, 47
    std::cout << "Initial position after search: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    // Test forward navigation
    std::cout << "\n=== Testing Forward Navigation (n) ===" << std::endl;
    for (int i = 0; i < 5; ++i) {
        main_window.FindNext();
        std::cout << "After FindNext " << (i+1) << ": " << main_window.GetSelectedEntryIndex() << std::endl;
    }
    
    // Test reverse navigation
    std::cout << "\n=== Testing Reverse Navigation (N) ===" << std::endl;
    for (int i = 0; i < 3; ++i) {
        main_window.FindPrevious();
        std::cout << "After FindPrevious " << (i+1) << ": " << main_window.GetSelectedEntryIndex() << std::endl;
    }
    
    // Test navigation from middle position
    std::cout << "\n=== Testing Navigation from Middle Position ===" << std::endl;
    main_window.JumpToLine(21); // Move to middle of log (line 21 = index 20)
    std::cout << "Moved to position: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    main_window.FindNext(); // Should find next match after position 20
    std::cout << "FindNext from position 20: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    main_window.FindPrevious(); // Should find previous match before current position
    std::cout << "FindPrevious from current: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    // Test wrap-around behavior
    std::cout << "\n=== Testing Wrap-around Behavior ===" << std::endl;
    main_window.JumpToLine(49); // Move near end (line 49 = index 48)
    std::cout << "Moved to position: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    main_window.FindNext(); // Should find last match
    std::cout << "FindNext from position 48: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    main_window.FindNext(); // Should wrap to first match
    std::cout << "FindNext (should wrap to first): " << main_window.GetSelectedEntryIndex() << std::endl;
    
    main_window.FindPrevious(); // Should wrap to last match
    std::cout << "FindPrevious (should wrap to last): " << main_window.GetSelectedEntryIndex() << std::endl;
    
    std::cout << "\nComprehensive search and scrollbar test completed!" << std::endl;
    
    // Clean up
    std::remove("test_comprehensive.log");
    
    return 0;
}