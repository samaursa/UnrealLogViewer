#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

int main() {
    // Create a test log file
    std::string test_file = "test_tailing_fix.log";
    std::ofstream file(test_file);
    file << "[2024-01-01 10:00:00] Info: Initial log entry 1\n";
    file << "[2024-01-01 10:00:01] Info: Initial log entry 2\n";
    file << "[2024-01-01 10:00:02] Info: Initial log entry 3\n";
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
    
    std::cout << "Tailing started. Is tailing: " << (window.IsTailing() ? "Yes" : "No") << std::endl;
    
    // Simulate adding new log entries
    std::thread writer_thread([&test_file]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        for (int i = 4; i <= 10; ++i) {
            std::ofstream file(test_file, std::ios::app);
            file << "[2024-01-01 10:00:0" << i << "] Info: New log entry " << i << "\n";
            file.close();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // Wait for new entries to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    std::cout << "Final entries count: " << window.GetDisplayedEntries().size() << std::endl;
    std::cout << "Selected entry index: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Test navigation cancels tailing
    window.ScrollUp();
    std::cout << "After scroll up - Is tailing: " << (window.IsTailing() ? "Yes" : "No") << std::endl;
    
    writer_thread.join();
    
    // Clean up
    std::filesystem::remove(test_file);
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}