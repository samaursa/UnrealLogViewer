#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace ue_log;

int main() {
    std::cout << "Testing different poll intervals for file monitoring...\n" << std::endl;
    
    // Create a test log file
    std::ofstream test_file("test_poll.log");
    test_file << "[2024-01-01-12.00.00:000][  0]LogTemp: Initial message\n";
    test_file.close();
    
    // Create MainWindow instance
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_poll.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    std::cout << "Initial entries: " << window.GetDisplayedEntries().size() << std::endl;
    
    // Test different poll intervals
    std::vector<int> intervals = {25, 50, 100, 250}; // milliseconds
    
    for (int interval : intervals) {
        std::cout << "\n=== Testing " << interval << "ms poll interval ===" << std::endl;
        
        // Set the poll interval
        window.SetTailingPollInterval(interval);
        
        // Start tailing
        if (!window.StartTailing()) {
            std::cout << "✗ FAIL: Could not start tailing" << std::endl;
            continue;
        }
        
        std::cout << "✓ Tailing started with " << interval << "ms interval" << std::endl;
        std::cout << "Status: " << window.GetLastError() << std::endl;
        
        // Add new content to the file
        std::ofstream append_file("test_poll.log", std::ios::app);
        auto start_time = std::chrono::steady_clock::now();
        
        append_file << "[2024-01-01-12.00.01:000][  1]LogTemp: Test message for " << interval << "ms interval\n";
        append_file.flush();
        append_file.close();
        
        // Wait and check how quickly the change is detected
        int initial_count = window.GetDisplayedEntries().size();
        bool detected = false;
        auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
        
        while (std::chrono::steady_clock::now() < timeout && !detected) {
            if (window.GetDisplayedEntries().size() > initial_count) {
                auto end_time = std::chrono::steady_clock::now();
                auto detection_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                std::cout << "✓ Change detected in " << detection_time.count() << "ms" << std::endl;
                detected = true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        if (!detected) {
            std::cout << "✗ Change not detected within 1000ms" << std::endl;
        }
        
        // Stop tailing for next test
        window.StopTailing();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Clean up
    std::remove("test_poll.log");
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Lower poll intervals (25ms, 50ms) = faster detection, higher CPU usage" << std::endl;
    std::cout << "Higher poll intervals (100ms, 250ms) = slower detection, lower CPU usage" << std::endl;
    std::cout << "Default is 100ms, modified to 50ms in StartTailing()" << std::endl;
    
    return 0;
}