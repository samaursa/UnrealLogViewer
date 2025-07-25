#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace ue_log;

int main() {
    // Create a test log file with many entries to test scrolling
    std::ofstream test_file("test_g_interactive.log");
    for (int i = 0; i < 100; i++) {
        test_file << "[2024-01-01-12.00." << std::setfill('0') << std::setw(2) << i << ":000][" 
                  << std::setw(3) << i << "]LogTemp: Test message " << i << "\n";
    }
    test_file.close();
    
    // Create MainWindow instance
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_g_interactive.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    std::cout << "Total entries: " << window.GetDisplayedEntries().size() << std::endl;
    
    // Start at the beginning (simulate user starting at top)
    window.ScrollToTop();
    std::cout << "After ScrollToTop - selected index: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Now simulate pressing 'G' key
    std::cout << "\nSimulating 'G' key press..." << std::endl;
    
    // First go to the end of the log (this is what the G key handler does)
    window.ScrollToBottom();
    std::cout << "After ScrollToBottom - selected index: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Then start tailing if not already active
    if (!window.IsTailing()) {
        bool result = window.StartTailing();
        std::cout << "StartTailing result: " << (result ? "success" : "failed") << std::endl;
        std::cout << "Status: " << window.GetLastError() << std::endl;
    }
    
    std::cout << "Tailing state: " << (window.IsTailing() ? "true" : "false") << std::endl;
    
    // Simulate new log entries being written
    std::cout << "\nSimulating new log entries..." << std::endl;
    std::ofstream append_file("test_g_interactive.log", std::ios::app);
    for (int i = 100; i < 105; i++) {
        append_file << "[2024-01-01-12.00." << std::setfill('0') << std::setw(2) << i << ":000][" 
                    << std::setw(3) << i << "]LogTemp: NEW message " << i << "\n";
        append_file.flush();
        
        // Wait a bit for file monitoring to detect changes
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "Added entry " << i << " - Total entries: " << window.GetDisplayedEntries().size() 
                  << ", Selected: " << window.GetSelectedEntryIndex() << std::endl;
    }
    append_file.close();
    
    // Wait a bit more to ensure all changes are processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "\nFinal state:" << std::endl;
    std::cout << "Total entries: " << window.GetDisplayedEntries().size() << std::endl;
    std::cout << "Selected index: " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "Tailing state: " << (window.IsTailing() ? "true" : "false") << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Test navigation cancellation
    std::cout << "\nTesting navigation cancellation with 'j' key..." << std::endl;
    window.SelectNextEntry();
    std::cout << "After j key - Tailing state: " << (window.IsTailing() ? "true" : "false") << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Clean up
    std::remove("test_g_interactive.log");
    
    return 0;
}