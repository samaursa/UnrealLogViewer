#include "lib/file_monitor/file_monitor.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

int main() {
    // Create a test file
    std::string test_file = "debug_monitor.log";
    std::ofstream file(test_file);
    file << "Line 1\n";
    file << "Line 2\n";
    file << "Line 3\n";
    file.close();
    
    // Create FileMonitor
    ue_log::FileMonitor monitor;
    
    int callback_count = 0;
    int total_lines_received = 0;
    
    // Set callback to track what we receive
    monitor.SetCallback([&](const std::string& file_path, const std::vector<std::string>& new_lines) {
        callback_count++;
        total_lines_received += new_lines.size();
        std::cout << "Callback " << callback_count << ": Received " << new_lines.size() << " lines" << std::endl;
        for (const auto& line : new_lines) {
            std::cout << "  Line: " << line << std::endl;
        }
    });
    
    // Start monitoring
    auto result = monitor.StartMonitoring(test_file);
    if (result.IsError()) {
        std::cerr << "Failed to start monitoring: " << result.Get_error_message() << std::endl;
        return 1;
    }
    
    std::cout << "Monitoring started. Adding new lines..." << std::endl;
    
    // Wait a bit, then add a line
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "Adding Line 4..." << std::endl;
    std::ofstream append1(test_file, std::ios::app);
    append1 << "Line 4\n";
    append1.close();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "Adding Line 5..." << std::endl;
    std::ofstream append2(test_file, std::ios::app);
    append2 << "Line 5\n";
    append2.close();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Total callbacks: " << callback_count << std::endl;
    std::cout << "Total lines received: " << total_lines_received << std::endl;
    std::cout << "Expected: 2 lines (Line 4 and Line 5)" << std::endl;
    
    // Stop monitoring
    monitor.StopMonitoring();
    
    // Clean up
    std::filesystem::remove(test_file);
    
    return 0;
}