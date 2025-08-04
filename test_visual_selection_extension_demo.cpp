#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace ue_log;

int main() {
    // Create a test log file
    std::string test_file = "demo_visual_extension.log";
    std::ofstream file(test_file);
    
    for (int i = 0; i < 15; ++i) {
        file << "[2024-01-01 10:00:" << std::setfill('0') << std::setw(2) << i 
             << "][Info][TestLogger] Test message " << (i + 1) << std::endl;
    }
    file.close();
    
    // Create main window and load file
    ConfigManager config_manager;
    MainWindow main_window(&config_manager);
    main_window.Initialize();
    
    bool loaded = main_window.LoadLogFile(test_file);
    if (!loaded) {
        std::cerr << "Failed to load test file" << std::endl;
        return 1;
    }
    
    // Wait for file to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "=== Visual Selection Extension Demo ===" << std::endl;
    std::cout << "Total entries: " << main_window.GetDisplayedEntries().size() << std::endl;
    
    // Test 1: Basic extension
    std::cout << "\n--- Test 1: Basic Extension ---" << std::endl;
    main_window.GoToLine(5);  // Go to line 5 (index 4)
    std::cout << "Initial position: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    main_window.EnterVisualSelectionMode();
    std::cout << "Visual selection mode: " << (main_window.IsVisualSelectionMode() ? "ON" : "OFF") << std::endl;
    
    auto initial_range = main_window.GetVisualSelectionRange();
    std::cout << "Initial range: [" << initial_range.first << ", " << initial_range.second << "]" << std::endl;
    std::cout << "Initial size: " << main_window.GetVisualSelectionSize() << std::endl;
    
    // Extend selection down by 3 lines
    main_window.ExtendVisualSelection(7);  // Extend to index 7
    std::cout << "After extending to index 7:" << std::endl;
    std::cout << "Current position: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    auto extended_range = main_window.GetVisualSelectionRange();
    std::cout << "Extended range: [" << extended_range.first << ", " << extended_range.second << "]" << std::endl;
    std::cout << "Extended size: " << main_window.GetVisualSelectionSize() << std::endl;
    
    // Test 2: Bounds checking
    std::cout << "\n--- Test 2: Bounds Checking ---" << std::endl;
    main_window.ExtendVisualSelection(20);  // Try to extend beyond bounds
    std::cout << "After extending to index 20 (beyond bounds):" << std::endl;
    std::cout << "Current position: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    auto bounds_range = main_window.GetVisualSelectionRange();
    std::cout << "Bounds-checked range: [" << bounds_range.first << ", " << bounds_range.second << "]" << std::endl;
    std::cout << "Bounds-checked size: " << main_window.GetVisualSelectionSize() << std::endl;
    
    // Test 3: Upward extension
    std::cout << "\n--- Test 3: Upward Extension ---" << std::endl;
    main_window.ExtendVisualSelection(2);  // Extend upward to index 2
    std::cout << "After extending upward to index 2:" << std::endl;
    std::cout << "Current position: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    auto upward_range = main_window.GetVisualSelectionRange();
    std::cout << "Upward range: [" << upward_range.first << ", " << upward_range.second << "]" << std::endl;
    std::cout << "Upward size: " << main_window.GetVisualSelectionSize() << std::endl;
    
    // Test 4: Lower bounds checking
    std::cout << "\n--- Test 4: Lower Bounds Checking ---" << std::endl;
    main_window.ExtendVisualSelection(-5);  // Try to extend below 0
    std::cout << "After extending to index -5 (below bounds):" << std::endl;
    std::cout << "Current position: " << main_window.GetSelectedEntryIndex() << std::endl;
    
    auto lower_bounds_range = main_window.GetVisualSelectionRange();
    std::cout << "Lower bounds range: [" << lower_bounds_range.first << ", " << lower_bounds_range.second << "]" << std::endl;
    std::cout << "Lower bounds size: " << main_window.GetVisualSelectionSize() << std::endl;
    
    // Exit visual selection mode
    main_window.ExitVisualSelectionMode();
    std::cout << "\nVisual selection mode after exit: " << (main_window.IsVisualSelectionMode() ? "ON" : "OFF") << std::endl;
    
    // Clean up
    std::remove(test_file.c_str());
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    return 0;
}