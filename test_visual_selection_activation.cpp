#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file
    std::ofstream file("test_visual_activation.log");
    file << "[2024.01.15-10.30.45:123][456]LogTemp: Warning: Test message 1\n";
    file << "[2024.01.15-10.30.46:124][457]LogTemp: Error: Test message 2\n";
    file << "[2024.01.15-10.30.47:125][458]LogCore: Info: Test message 3\n";
    file.close();
    
    // Create and initialize MainWindow
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_visual_activation.log")) {
        std::cout << "❌ Failed to load test file\n";
        return 1;
    }
    
    std::cout << "✅ Test file loaded successfully\n";
    
    // Test initial state
    if (window.IsVisualSelectionMode()) {
        std::cout << "❌ Visual selection mode should not be active initially\n";
        return 1;
    }
    std::cout << "✅ Initial state: Visual selection mode is inactive\n";
    
    // Test EnterVisualSelectionMode
    window.EnterVisualSelectionMode();
    
    if (!window.IsVisualSelectionMode()) {
        std::cout << "❌ Visual selection mode should be active after EnterVisualSelectionMode\n";
        return 1;
    }
    std::cout << "✅ Visual selection mode activated successfully\n";
    
    // Test selection range
    auto range = window.GetVisualSelectionRange();
    if (range.first != 0 || range.second != 0) {
        std::cout << "❌ Visual selection range should be (0,0), got (" << range.first << "," << range.second << ")\n";
        return 1;
    }
    std::cout << "✅ Visual selection range is correct: (0,0)\n";
    
    // Test selection size
    if (window.GetVisualSelectionSize() != 1) {
        std::cout << "❌ Visual selection size should be 1, got " << window.GetVisualSelectionSize() << "\n";
        return 1;
    }
    std::cout << "✅ Visual selection size is correct: 1\n";
    
    // Test 'v' key activation through event system
    window.ExitVisualSelectionMode(); // Reset state
    
    auto component = window.CreateFTXUIComponent();
    ftxui::Event v_key = ftxui::Event::Character('v');
    bool handled = component->OnEvent(v_key);
    
    if (!handled) {
        std::cout << "❌ 'v' key event should be handled\n";
        return 1;
    }
    std::cout << "✅ 'v' key event handled successfully\n";
    
    if (!window.IsVisualSelectionMode()) {
        std::cout << "❌ Visual selection mode should be active after 'v' key press\n";
        return 1;
    }
    std::cout << "✅ Visual selection mode activated by 'v' key\n";
    
    // Test status bar rendering (basic check)
    auto rendered = window.Render();
    if (!rendered) {
        std::cout << "❌ Window should render successfully\n";
        return 1;
    }
    std::cout << "✅ Window renders successfully with visual selection mode\n";
    
    // Test ExitVisualSelectionMode
    window.ExitVisualSelectionMode();
    
    if (window.IsVisualSelectionMode()) {
        std::cout << "❌ Visual selection mode should be inactive after ExitVisualSelectionMode\n";
        return 1;
    }
    std::cout << "✅ Visual selection mode deactivated successfully\n";
    
    // Clean up
    std::remove("test_visual_activation.log");
    
    std::cout << "\n🎉 All visual selection mode activation tests passed!\n";
    return 0;
}