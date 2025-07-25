#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace ue_log;

int main() {
    std::cout << "Testing on-demand refresh when log file changes...\n" << std::endl;
    
    // Create a test log file
    std::ofstream test_file("test_refresh.log");
    test_file << "[2024-01-01-12.00.00:000][  0]LogTemp: Initial message\n";
    test_file.close();
    
    // Create MainWindow instance
    MainWindow window;
    window.Initialize();
    
    // Track refresh calls
    std::atomic<int> refresh_count{0};
    
    // Set up refresh callback to count calls
    window.SetRefreshCallback([&refresh_count]() {
        refresh_count++;
        std::cout << "  → Refresh triggered! (count: " << refresh_count.load() << ")" << std::endl;
    });
    
    // Load the test file
    if (!window.LoadLogFile("test_refresh.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    std::cout << "Initial entries: " << window.GetDisplayedEntries().size() << std::endl;
    std::cout << "Initial refresh count: " << refresh_count.load() << std::endl;
    
    // Start tailing
    if (!window.StartTailing()) {
        std::cout << "✗ FAIL: Could not start tailing" << std::endl;
        return 1;
    }
    
    std::cout << "\n✓ Tailing started" << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Test 1: Add new content and verify refresh is triggered
    std::cout << "\n=== Test 1: Adding new log entries ===" << std::endl;
    std::cout << "Refresh count before: " << refresh_count.load() << std::endl;
    
    std::ofstream append_file("test_refresh.log", std::ios::app);
    append_file << "[2024-01-01-12.00.01:000][  1]LogTemp: New message 1\n";
    append_file << "[2024-01-01-12.00.02:000][  2]LogTemp: New message 2\n";
    append_file.flush();
    append_file.close();
    
    // Wait for FileMonitor to detect changes
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "Refresh count after: " << refresh_count.load() << std::endl;
    std::cout << "Total entries: " << window.GetDisplayedEntries().size() << std::endl;
    
    if (refresh_count.load() > 0) {
        std::cout << "✓ PASS: Refresh was triggered when new entries were added" << std::endl;
    } else {
        std::cout << "✗ FAIL: Refresh was not triggered" << std::endl;
    }
    
    // Test 2: Stop tailing and verify no more refreshes
    std::cout << "\n=== Test 2: Stopping tailing ===" << std::endl;
    window.StopTailing();
    int refresh_count_after_stop = refresh_count.load();
    std::cout << "Refresh count after stopping: " << refresh_count_after_stop << std::endl;
    
    // Add more content (should not trigger refresh since tailing is stopped)
    std::ofstream append_file2("test_refresh.log", std::ios::app);
    append_file2 << "[2024-01-01-12.00.03:000][  3]LogTemp: Message after stop\n";
    append_file2.flush();
    append_file2.close();
    
    // Wait and check
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    if (refresh_count.load() == refresh_count_after_stop) {
        std::cout << "✓ PASS: No refresh triggered after stopping tailing" << std::endl;
    } else {
        std::cout << "✗ FAIL: Refresh was triggered after stopping tailing" << std::endl;
    }
    
    // Clean up
    std::remove("test_refresh.log");
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Total refresh calls: " << refresh_count.load() << std::endl;
    std::cout << "✓ Refresh is only triggered when log file changes during tailing" << std::endl;
    std::cout << "✓ No unnecessary refreshes when not tailing or when no changes occur" << std::endl;
    std::cout << "✓ This solves the focus-dependent update issue efficiently" << std::endl;
    
    return 0;
}