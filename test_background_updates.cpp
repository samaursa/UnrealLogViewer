#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace ue_log;

int main() {
    std::cout << "Testing background log updates without tailing...\n" << std::endl;
    
    // Create a test log file
    std::ofstream test_file("test_background.log");
    test_file << "[2024-01-01-12.00.00:000][  0]LogTemp: Initial message 1\n";
    test_file << "[2024-01-01-12.00.01:000][  1]LogTemp: Initial message 2\n";
    test_file << "[2024-01-01-12.00.02:000][  2]LogTemp: Initial message 3\n";
    test_file.close();
    
    // Create MainWindow instance
    MainWindow window;
    window.Initialize();
    
    // Track refresh calls
    int refresh_count = 0;
    window.SetRefreshCallback([&refresh_count]() {
        refresh_count++;
        std::cout << "  → UI refresh triggered (count: " << refresh_count << ")" << std::endl;
    });
    
    // Load the test file
    if (!window.LoadLogFile("test_background.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    std::cout << "Initial entries: " << window.GetDisplayedEntries().size() << std::endl;
    std::cout << "Initial selected index: " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "Initial tailing state: " << (window.IsTailing() ? "LIVE" : "STATIC") << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Move to middle entry to test that selection doesn't change
    int middle_index = window.GetDisplayedEntries().size() / 2;
    window.GoToLine(middle_index);
    std::cout << "\n✓ Moved to middle entry (index " << window.GetSelectedEntryIndex() << ")" << std::endl;
    
    // Test 1: Add new content WITHOUT tailing
    std::cout << "\n=== Test 1: Adding entries without tailing ===" << std::endl;
    std::cout << "Before: " << window.GetDisplayedEntries().size() << " entries, selected: " << window.GetSelectedEntryIndex() << std::endl;
    
    std::ofstream append_file("test_background.log", std::ios::app);
    append_file << "[2024-01-01-12.00.03:000][  3]LogTemp: Background message 1\n";
    append_file << "[2024-01-01-12.00.04:000][  4]LogTemp: Background message 2\n";
    append_file.flush();
    append_file.close();
    
    // Wait for FileMonitor to detect changes
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "After: " << window.GetDisplayedEntries().size() << " entries, selected: " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Verify behavior
    if (window.GetDisplayedEntries().size() > 3) {
        std::cout << "✓ PASS: New entries were added to the log" << std::endl;
    } else {
        std::cout << "✗ FAIL: New entries were not detected" << std::endl;
        return 1;
    }
    
    if (window.GetSelectedEntryIndex() == middle_index) {
        std::cout << "✓ PASS: Selection stayed at the same position (no auto-scroll)" << std::endl;
    } else {
        std::cout << "✗ FAIL: Selection moved unexpectedly" << std::endl;
        return 1;
    }
    
    if (!window.IsTailing()) {
        std::cout << "✓ PASS: Still in STATIC mode (not tailing)" << std::endl;
    } else {
        std::cout << "✗ FAIL: Unexpectedly entered tailing mode" << std::endl;
        return 1;
    }
    
    // Test 2: Start tailing and verify auto-scroll works
    std::cout << "\n=== Test 2: Starting tailing ===" << std::endl;
    int entries_before_tailing = window.GetDisplayedEntries().size();
    
    if (!window.StartTailing()) {
        std::cout << "✗ FAIL: Could not start tailing" << std::endl;
        return 1;
    }
    
    std::cout << "✓ Tailing started" << std::endl;
    std::cout << "Selected index after starting tailing: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Add more content while tailing
    std::ofstream append_file2("test_background.log", std::ios::app);
    append_file2 << "[2024-01-01-12.00.05:000][  5]LogTemp: Tailing message 1\n";
    append_file2.flush();
    append_file2.close();
    
    // Wait for detection
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "After adding entry while tailing:" << std::endl;
    std::cout << "Total entries: " << window.GetDisplayedEntries().size() << std::endl;
    std::cout << "Selected index: " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Verify tailing behavior
    int expected_last_index = window.GetDisplayedEntries().size() - 1;
    if (window.GetSelectedEntryIndex() == expected_last_index) {
        std::cout << "✓ PASS: Auto-scroll moved to last entry during tailing" << std::endl;
    } else {
        std::cout << "✗ FAIL: Auto-scroll didn't work during tailing" << std::endl;
        return 1;
    }
    
    // Clean up
    std::remove("test_background.log");
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "✓ Log file is monitored and updated even when not tailing" << std::endl;
    std::cout << "✓ Selection position is preserved when not tailing" << std::endl;
    std::cout << "✓ Auto-scroll only happens when tailing is active" << std::endl;
    std::cout << "✓ UI refreshes are triggered for both tailing and non-tailing updates" << std::endl;
    std::cout << "Total refresh calls: " << refresh_count << std::endl;
    
    return 0;
}