#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file
    std::ofstream file("test_reload.log");
    file << "[2024.01.15-10.30.45:123][456]LogTemp: Info: Test message 1\n";
    file << "[2024.01.15-10.30.46:124][457]LogTemp: Info: Test message 2\n";
    file.close();
    
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_reload.log")) {
        std::cout << "❌ Failed to load test file\n";
        return 1;
    }
    
    // Enter visual selection mode
    window.EnterVisualSelectionMode();
    std::cout << "Before reload - Visual selection mode: " << (window.IsVisualSelectionMode() ? "ON" : "OFF") << std::endl;
    
    // Reload file
    window.ReloadLogFile();
    std::cout << "After reload - Visual selection mode: " << (window.IsVisualSelectionMode() ? "ON" : "OFF") << std::endl;
    
    // Clean up
    std::remove("test_reload.log");
    
    return 0;
}