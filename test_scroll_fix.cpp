#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

int main() {
    // Create a test log file with many entries
    std::string test_file = "test_scroll_fix.log";
    std::ofstream file(test_file);
    
    // Create 50 initial entries to fill more than a typical screen
    for (int i = 1; i <= 50; ++i) {
        file << "[2024-01-01 10:00:" << std::setfill('0') << std::setw(2) << i 
             << "] Info: Initial log entry " << i << "\n";
    }
    file.close();
    
    // Create MainWindow instance
    ue_log::ConfigManager config;
    ue_log::MainWindow window(&config);
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile(test_file)) {
        std::cerr << "Failed to load test file" << std::endl;
        return 1;
    }
    
    std::cout << "Initial entries loaded: " << window.GetDisplayedEntries().size() << std::endl;
    
    // Start tailing
    if (!window.StartTailing()) {
        std::cerr << "Failed to start tailing" << std::endl;
        return 1;
    }
    
    std::cout << "Tailing started. Selected entry index: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Add new entries to test auto-scroll
    std::ofstream append_file(test_file, std::ios::app);
    for (int i = 51; i <= 60; ++i) {
        append_file << "[2024-01-01 10:01:" << std::setfill('0') << std::setw(2) << (i-50) 
                   << "] Info: New log entry " << i << "\n";
        append_file.flush();
        
        // Give time for file monitor to detect changes
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    append_file.close();
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Final entries count: " << window.GetDisplayedEntries().size() << std::endl;
    std::cout << "Final selected entry index: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Verify that the selected entry is the last one (auto-scroll worked)
    int expected_last_index = static_cast<int>(window.GetDisplayedEntries().size()) - 1;
    if (window.GetSelectedEntryIndex() == expected_last_index) {
        std::cout << "✓ PASS: Auto-scroll correctly positioned at last entry" << std::endl;
    } else {
        std::cout << "✗ FAIL: Auto-scroll did not position at last entry" << std::endl;
        std::cout << "  Expected: " << expected_last_index << ", Got: " << window.GetSelectedEntryIndex() << std::endl;
    }
    
    // Test that navigation cancels tailing
    window.ScrollUp();
    if (!window.IsTailing()) {
        std::cout << "✓ PASS: Navigation correctly cancelled tailing" << std::endl;
    } else {
        std::cout << "✗ FAIL: Navigation did not cancel tailing" << std::endl;
    }
    
    // Clean up
    std::filesystem::remove(test_file);
    
    std::cout << "Scroll fix test completed!" << std::endl;
    return 0;
}