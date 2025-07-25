#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace ue_log;

int main() {
    // Create a test log file with some initial content
    std::ofstream test_file("test_g_key.log");
    test_file << "[2024-01-01-12.00.00:000][  0]LogTemp: Initial message 1\n";
    test_file << "[2024-01-01-12.00.01:000][  1]LogTemp: Initial message 2\n";
    test_file << "[2024-01-01-12.00.02:000][  2]LogTemp: Initial message 3\n";
    test_file.close();
    
    // Create MainWindow instance
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_g_key.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    std::cout << "Initial entries count: " << window.GetDisplayedEntries().size() << std::endl;
    std::cout << "Initial selected index: " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "Initial tailing state: " << (window.IsTailing() ? "true" : "false") << std::endl;
    
    // Test ScrollToBottom directly
    std::cout << "\nTesting ScrollToBottom()..." << std::endl;
    window.ScrollToBottom();
    std::cout << "After ScrollToBottom - selected index: " << window.GetSelectedEntryIndex() << std::endl;
    
    // Test StartTailing
    std::cout << "\nTesting StartTailing()..." << std::endl;
    bool tailing_result = window.StartTailing();
    std::cout << "StartTailing result: " << (tailing_result ? "success" : "failed") << std::endl;
    std::cout << "Tailing state: " << (window.IsTailing() ? "true" : "false") << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Add new content to the file to test monitoring
    std::cout << "\nAdding new content to file..." << std::endl;
    std::ofstream append_file("test_g_key.log", std::ios::app);
    append_file << "[2024-01-01-12.00.03:000][  3]LogTemp: New message 4\n";
    append_file << "[2024-01-01-12.00.04:000][  4]LogTemp: New message 5\n";
    append_file.close();
    
    // Wait a bit for file monitoring to detect changes
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "After adding content:" << std::endl;
    std::cout << "Entries count: " << window.GetDisplayedEntries().size() << std::endl;
    std::cout << "Selected index: " << window.GetSelectedEntryIndex() << std::endl;
    std::cout << "Tailing state: " << (window.IsTailing() ? "true" : "false") << std::endl;
    std::cout << "Status: " << window.GetLastError() << std::endl;
    
    // Clean up
    std::remove("test_g_key.log");
    
    return 0;
}