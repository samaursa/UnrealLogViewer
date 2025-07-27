#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace ue_log;

int main() {
    std::cout << "Testing incremental context line handling during tailing..." << std::endl;
    
    // Create a test log file with initial content
    std::ofstream test_file("test_context_tailing.log");
    test_file << "[2024-01-01-12.00.00:000][  0]LogTemp: Initial message 1\n";
    test_file << "[2024-01-01-12.00.01:000][  1]LogTemp: Initial message 2\n";
    test_file << "[2024-01-01-12.00.02:000][  2]LogTemp: Initial message 3\n";
    test_file.close();
    
    // Create MainWindow and load the file
    MainWindow window;
    window.Initialize();
    
    if (!window.LoadLogFile("test_context_tailing.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    
    // Set context lines to test the fix
    window.SetContextLines(2);
    std::cout << "✓ Set context lines to 2" << std::endl;
    
    // Get initial entry count using GetDisplayedEntries()
    size_t initial_count = window.GetDisplayedEntries().size();
    std::cout << "Initial filtered entry count: " << initial_count << std::endl;
    
    // Start tailing
    if (!window.StartTailing()) {
        std::cout << "✗ FAIL: Could not start tailing" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "✓ Started tailing" << std::endl;
    
    // Wait a moment for tailing to be fully set up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Simulate new log lines being added to the file
    std::cout << "Simulating new log lines by appending to file..." << std::endl;
    
    // Append new lines to the file (simulating real-time log updates)
    std::ofstream append_file("test_context_tailing.log", std::ios::app);
    append_file << "[2024-01-01-12.00.03:000][  3]LogTemp: New message 4\n";
    append_file << "[2024-01-01-12.00.04:000][  4]LogTemp: New message 5\n";
    append_file.flush();
    append_file.close();
    
    // Wait for FileMonitor to detect the changes and process them
    std::cout << "Waiting for FileMonitor to detect changes..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Check the entry count after adding new lines
    size_t final_count = window.GetDisplayedEntries().size();
    std::cout << "Final filtered entry count: " << final_count << std::endl;
    
    // The key test: with context lines enabled, we should see an incremental increase
    // rather than a doubling of entries (which would happen with the old bug)
    if (final_count > initial_count && final_count < initial_count * 2) {
        std::cout << "✓ PASS: Entry count increased incrementally (" << initial_count 
                  << " -> " << final_count << ")" << std::endl;
        std::cout << "✓ Context lines are being handled incrementally, not causing full rebuild" << std::endl;
    } else if (final_count >= initial_count * 2) {
        std::cout << "✗ FAIL: Entry count doubled or more (" << initial_count 
                  << " -> " << final_count << ")" << std::endl;
        std::cout << "✗ This suggests the old bug is still present (full rebuild happening)" << std::endl;
        return 1;
    } else if (final_count == initial_count) {
        std::cout << "⚠ WARNING: Entry count didn't increase (" << initial_count 
                  << " -> " << final_count << ")" << std::endl;
        std::cout << "⚠ This might be due to FileMonitor timing - let's check if tailing is working" << std::endl;
        
        // Try one more time with a longer wait
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        final_count = window.GetDisplayedEntries().size();
        std::cout << "Entry count after longer wait: " << final_count << std::endl;
        
        if (final_count == initial_count) {
            std::cout << "✗ FAIL: New entries were not processed even after waiting" << std::endl;
            return 1;
        }
    }
    
    // Test that tailing is still active
    if (window.IsTailing()) {
        std::cout << "✓ PASS: Tailing is still active after processing new entries" << std::endl;
    } else {
        std::cout << "✗ FAIL: Tailing should still be active" << std::endl;
        return 1;
    }
    
    // Additional test: verify that the fix prevents the exponential growth issue
    std::cout << "\nTesting multiple incremental updates..." << std::endl;
    size_t count_before_second_update = final_count;
    
    // Add more lines
    std::ofstream append_file2("test_context_tailing.log", std::ios::app);
    append_file2 << "[2024-01-01-12.00.05:000][  5]LogTemp: New message 6\n";
    append_file2.flush();
    append_file2.close();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    size_t count_after_second_update = window.GetDisplayedEntries().size();
    
    std::cout << "Entry count after second update: " << count_after_second_update << std::endl;
    
    if (count_after_second_update > count_before_second_update && 
        count_after_second_update < count_before_second_update * 2) {
        std::cout << "✓ PASS: Second update also handled incrementally" << std::endl;
    } else {
        std::cout << "✗ FAIL: Second update caused unexpected growth pattern" << std::endl;
        return 1;
    }
    
    // Clean up
    window.StopTailing();
    std::remove("test_context_tailing.log");
    
    std::cout << "\n✓ All tests passed! Context line incremental handling is working correctly." << std::endl;
    std::cout << "✓ The fix prevents the 'whole log re-added' issue during live tailing." << std::endl;
    return 0;
}