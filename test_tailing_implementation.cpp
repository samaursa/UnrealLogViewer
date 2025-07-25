#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

int main() {
    using namespace ue_log;
    
    // Create a test log file
    std::string test_file = "test_tailing.log";
    std::ofstream file(test_file);
    file << "[2024-01-01-12.00.00:000][0]LogTemp: Display: Initial log entry\n";
    file.close();
    
    // Create MainWindow and load the file
    MainWindow window;
    window.Initialize();
    
    if (!window.LoadLogFile(test_file)) {
        std::cout << "Failed to load test file: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "File loaded successfully. Initial entries: " << window.GetDisplayedEntries().size() << std::endl;
    
    // Test StartTailing
    if (window.StartTailing()) {
        std::cout << "Tailing started successfully. Status: " << window.GetLastError() << std::endl;
        std::cout << "IsTailing: " << (window.IsTailing() ? "true" : "false") << std::endl;
    } else {
        std::cout << "Failed to start tailing: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    // Test StopTailing
    window.StopTailing();
    std::cout << "Tailing stopped. Status: " << window.GetLastError() << std::endl;
    std::cout << "IsTailing: " << (window.IsTailing() ? "true" : "false") << std::endl;
    
    // Test starting tailing without a file
    MainWindow window2;
    window2.Initialize();
    if (!window2.StartTailing()) {
        std::cout << "Correctly failed to start tailing without file: " << window2.GetLastError() << std::endl;
    }
    
    std::cout << "Tailing implementation test completed successfully!" << std::endl;
    
    // Clean up
    std::remove(test_file.c_str());
    
    return 0;
}