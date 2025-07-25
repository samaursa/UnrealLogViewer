#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file with multiple entries
    std::ofstream test_file("test_g_scroll.log");
    test_file << "[2024-01-01-12.00.00:000][  0]LogTemp: Test message 1\n";
    test_file << "[2024-01-01-12.00.01:000][  1]LogTemp: Test message 2\n";
    test_file << "[2024-01-01-12.00.02:000][  2]LogTemp: Test message 3\n";
    test_file << "[2024-01-01-12.00.03:000][  3]LogTemp: Test message 4\n";
    test_file << "[2024-01-01-12.00.04:000][  4]LogTemp: Test message 5\n";
    test_file.close();
    
    // Create MainWindow instance
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_g_scroll.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    std::cout << "Entries loaded: " << window.GetDisplayedEntries().size() << std::endl;
    
    // Initially should be at the first entry (index 0)
    std::cout << "Initial selected entry index: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Simulate pressing 'G' key by calling the methods directly
    window.ScrollToBottom();
    
    // Check if we're now at the last entry
    int expected_last_index = static_cast<int>(window.GetDisplayedEntries().size()) - 1;
    int actual_index = window.GetSelectedEntryIndex();
    
    std::cout << "After ScrollToBottom:" << std::endl;
    std::cout << "  Expected last index: " << expected_last_index << std::endl;
    std::cout << "  Actual selected index: " << actual_index << std::endl;
    
    if (actual_index == expected_last_index) {
        std::cout << "✓ PASS: ScrollToBottom correctly moved to last entry" << std::endl;
    } else {
        std::cout << "✗ FAIL: ScrollToBottom did not move to last entry" << std::endl;
        return 1;
    }
    
    // Now test StartTailing
    if (window.StartTailing()) {
        std::cout << "✓ PASS: StartTailing succeeded" << std::endl;
        std::cout << "  Status: " << window.GetLastError() << std::endl;
        std::cout << "  IsTailing: " << (window.IsTailing() ? "true" : "false") << std::endl;
    } else {
        std::cout << "✗ FAIL: StartTailing failed" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    // Clean up
    std::remove("test_g_scroll.log");
    
    std::cout << "\n✓ All G key scroll-to-end tests passed!" << std::endl;
    return 0;
}