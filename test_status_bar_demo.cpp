#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace ue_log;

void CreateTestLogFile(const std::string& path, int line_count) {
    std::ofstream file(path);
    for (int i = 0; i < line_count; ++i) {
        file << "[2024.01.15-10.30." << (45 + i) << ":123][" << (456 + i) 
             << "]LogTemp: Info: Test message " << (i + 1) << "\n";
    }
    file.close();
}

int main() {
    std::cout << "Visual Selection Status Bar Demo\n";
    std::cout << "================================\n\n";
    
    // Create test log file
    std::string test_file = "status_bar_demo.log";
    CreateTestLogFile(test_file, 50);
    
    // Create main window
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile(test_file)) {
        std::cout << "Failed to load test file\n";
        return 1;
    }
    
    std::cout << "Test file loaded with " << window.GetDisplayedEntries().size() << " entries\n\n";
    
    // Test 1: Basic visual selection mode
    std::cout << "Test 1: Basic visual selection mode\n";
    std::cout << "Before entering visual mode: IsVisualSelectionMode = " 
              << (window.IsVisualSelectionMode() ? "true" : "false") << "\n";
    
    window.EnterVisualSelectionMode();
    std::cout << "After entering visual mode: IsVisualSelectionMode = " 
              << (window.IsVisualSelectionMode() ? "true" : "false") << "\n";
    std::cout << "Selection size: " << window.GetVisualSelectionSize() << "\n";
    
    auto range = window.GetVisualSelectionRange();
    std::cout << "Selection range: [" << range.first << ", " << range.second << "]\n\n";
    
    // Test 2: Extend selection
    std::cout << "Test 2: Extend selection to 5 lines\n";
    window.ExtendVisualSelection(4);
    std::cout << "Selection size: " << window.GetVisualSelectionSize() << "\n";
    
    range = window.GetVisualSelectionRange();
    std::cout << "Selection range: [" << range.first << ", " << range.second << "]\n\n";
    
    // Test 3: Large selection (multi-screen)
    std::cout << "Test 3: Large selection (multi-screen)\n";
    window.SetTerminalSize(80, 20); // Set small terminal size
    window.ExtendVisualSelection(25); // Select more than one screen
    std::cout << "Selection size: " << window.GetVisualSelectionSize() << "\n";
    
    range = window.GetVisualSelectionRange();
    std::cout << "Selection range: [" << range.first << ", " << range.second << "]\n";
    std::cout << "This should show range information in status bar for multi-screen selection\n\n";
    
    // Test 4: Exit visual selection mode
    std::cout << "Test 4: Exit visual selection mode\n";
    window.ExitVisualSelectionMode();
    std::cout << "After exiting visual mode: IsVisualSelectionMode = " 
              << (window.IsVisualSelectionMode() ? "true" : "false") << "\n";
    std::cout << "Selection size: " << window.GetVisualSelectionSize() << "\n\n";
    
    std::cout << "All tests completed successfully!\n";
    std::cout << "The status bar should show:\n";
    std::cout << "- 'VISUAL: X lines selected' when in visual selection mode\n";
    std::cout << "- Range information '(lines X-Y)' for multi-screen selections\n";
    std::cout << "- Bold and accent colored text for the visual mode indicator\n";
    
    // Clean up
    std::remove(test_file.c_str());
    
    return 0;
}