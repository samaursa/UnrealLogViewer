#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file with many entries
    std::ofstream test_file("test_g_visual.log");
    for (int i = 0; i < 1000; i++) {
        test_file << "[2024-01-01-12.00." << std::setfill('0') << std::setw(2) << (i % 60) << ":000][" 
                  << std::setw(4) << i << "]LogTemp: Test message " << i << "\n";
    }
    test_file.close();
    
    // Create MainWindow instance
    MainWindow window;
    window.Initialize();
    window.SetTerminalSize(80, 25); // Set a reasonable terminal size
    
    // Load the test file
    if (!window.LoadLogFile("test_g_visual.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    std::cout << "Total entries: " << window.GetDisplayedEntries().size() << std::endl;
    
    // Start at the beginning (simulate user starting at top)
    window.ScrollToTop();
    std::cout << "\n=== Initial state (at top) ===" << std::endl;
    std::cout << "Selected index: " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "Tailing state: " << (window.IsTailing() ? "LIVE" : "STATIC") << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Move to middle to make the effect more visible
    int middle_index = window.GetDisplayedEntries().size() / 2;
    window.GoToLine(middle_index);
    std::cout << "\n=== After moving to middle ===" << std::endl;
    std::cout << "Selected index: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Now simulate pressing 'G' key
    std::cout << "\n=== Simulating 'G' key press ===" << std::endl;
    
    // This is what the G key handler does:
    window.ScrollToBottom();
    if (!window.IsTailing()) {
        window.StartTailing();
    }
    
    std::cout << "After 'G' key:" << std::endl;
    std::cout << "Selected index: " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "Tailing state: " << (window.IsTailing() ? "LIVE" : "STATIC") << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Show the difference
    int total_entries = window.GetDisplayedEntries().size();
    std::cout << "\nVisual change:" << std::endl;
    std::cout << "- Moved from index " << middle_index << " to index " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "- That's a jump of " << (window.GetSelectedEntryIndex() - middle_index) << " entries" << std::endl;
    std::cout << "- Now at entry " << (window.GetSelectedEntryIndex() + 1) << " of " << total_entries << std::endl;
    
    // Test that pressing 'G' again has no effect (as per requirements)
    std::cout << "\n=== Testing 'G' key again (should have no effect) ===" << std::endl;
    int before_second_g = window.GetSelectedEntryIndex();
    bool before_tailing = window.IsTailing();
    
    // Simulate pressing 'G' again
    window.ScrollToBottom();
    if (!window.IsTailing()) {
        window.StartTailing();
    }
    
    std::cout << "After second 'G' key:" << std::endl;
    std::cout << "Selected index: " << window.GetSelectedEntryIndex() << " (was " << before_second_g << ")" << std::endl;
    std::cout << "Tailing state: " << (window.IsTailing() ? "LIVE" : "STATIC") << " (was " << (before_tailing ? "LIVE" : "STATIC") << ")" << std::endl;
    std::cout << "Change: " << (window.GetSelectedEntryIndex() == before_second_g && window.IsTailing() == before_tailing ? "NONE (correct)" : "UNEXPECTED") << std::endl;
    
    // Clean up
    std::remove("test_g_visual.log");
    
    return 0;
}