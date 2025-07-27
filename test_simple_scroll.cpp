#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>

int main() {
    try {
        // Create a simple test log file
        std::string test_file = "test_simple_scroll.log";
        std::ofstream file(test_file);
        
        // Create just a few entries to avoid complexity
        for (int i = 1; i <= 10; ++i) {
            file << "[2024-01-01 10:00:0" << i << "] Info: Log entry " << i << "\n";
        }
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
        
        std::cout << "Entries loaded: " << window.GetDisplayedEntries().size() << std::endl;
        std::cout << "Initial selected index: " << window.GetSelectedEntryIndex() << std::endl;
        
        // Test basic tailing functionality
        if (!window.StartTailing()) {
            std::cerr << "Failed to start tailing" << std::endl;
            return 1;
        }
        
        std::cout << "Tailing started: " << (window.IsTailing() ? "Yes" : "No") << std::endl;
        std::cout << "Selected index after tailing: " << window.GetSelectedEntryIndex() << std::endl;
        
        // Test navigation cancels tailing
        window.ScrollUp();
        std::cout << "After ScrollUp - Tailing: " << (window.IsTailing() ? "Yes" : "No") << std::endl;
        
        // Clean up
        std::filesystem::remove(test_file);
        
        std::cout << "Simple scroll test completed successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
}