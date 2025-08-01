#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

int main() {
    try {
        // Create a test log file with initial content
        std::string test_file = "test_poll_interval.log";
        std::ofstream file(test_file);
        file << "[2024-01-01 10:00:01] Info: Initial entry 1\n";
        file << "[2024-01-01 10:00:02] Info: Initial entry 2\n";
        file << "[2024-01-01 10:00:03] Info: Initial entry 3\n";
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
        
        std::cout << "Tailing started. Entries: " << window.GetDisplayedEntries().size() << std::endl;
        
        // Add one new line and see what happens
        std::ofstream append_file(test_file, std::ios::app);
        append_file << "[2024-01-01 10:00:04] Info: New entry 4\n";
        append_file.close();
        
        // Wait for file monitor to detect the change
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        std::cout << "After adding 1 line - Entries: " << window.GetDisplayedEntries().size() << std::endl;
        
        // Add another line
        std::ofstream append_file2(test_file, std::ios::app);
        append_file2 << "[2024-01-01 10:00:05] Info: New entry 5\n";
        append_file2.close();
        
        // Wait for file monitor to detect the change
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        std::cout << "After adding 2nd line - Entries: " << window.GetDisplayedEntries().size() << std::endl;
        
        // Expected: 3 initial + 1 + 1 = 5 entries
        // If bug exists: we'll see much more than 5
        
        if (window.GetDisplayedEntries().size() == 5) {
            std::cout << "✓ PASS: Correct number of entries (5)" << std::endl;
        } else {
            std::cout << "✗ FAIL: Expected 5 entries, got " << window.GetDisplayedEntries().size() << std::endl;
        }
        
        // Clean up
        std::filesystem::remove(test_file);
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}