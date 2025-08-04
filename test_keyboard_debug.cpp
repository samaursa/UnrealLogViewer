#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file
    std::ofstream file("test_keyboard_debug.log");
    for (int i = 0; i < 10; ++i) {
        file << "[2024.01.15-10.30." << (45 + i) << ":123][" << (456 + i) << "]LogTemp: Info: Test message " << (i + 1) << "\n";
    }
    file.close();
    
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_keyboard_debug.log")) {
        std::cout << "❌ Failed to load test file\n";
        return 1;
    }
    
    std::cout << "Loaded " << window.GetDisplayedEntries().size() << " entries\n";
    std::cout << "Initial selected index: " << window.GetSelectedEntryIndex() << "\n";
    
    auto component = window.CreateFTXUIComponent();
    
    // Enter visual selection mode
    ftxui::Event v_key = ftxui::Event::Character('v');
    bool handled = component->OnEvent(v_key);
    std::cout << "'v' key handled: " << (handled ? "YES" : "NO") << "\n";
    std::cout << "Visual selection mode: " << (window.IsVisualSelectionMode() ? "ON" : "OFF") << "\n";
    std::cout << "Selection size: " << window.GetVisualSelectionSize() << "\n";
    
    // Try j key
    ftxui::Event j_key = ftxui::Event::Character('j');
    handled = component->OnEvent(j_key);
    std::cout << "'j' key handled: " << (handled ? "YES" : "NO") << "\n";
    std::cout << "Selection size after j: " << window.GetVisualSelectionSize() << "\n";
    std::cout << "Selected index after j: " << window.GetSelectedEntryIndex() << "\n";
    
    auto range = window.GetVisualSelectionRange();
    std::cout << "Selection range: (" << range.first << ", " << range.second << ")\n";
    
    // Try another j key
    handled = component->OnEvent(j_key);
    std::cout << "Second 'j' key handled: " << (handled ? "YES" : "NO") << "\n";
    std::cout << "Selection size after second j: " << window.GetVisualSelectionSize() << "\n";
    std::cout << "Selected index after second j: " << window.GetSelectedEntryIndex() << "\n";
    
    range = window.GetVisualSelectionRange();
    std::cout << "Selection range after second j: (" << range.first << ", " << range.second << ")\n";
    
    // Clean up
    std::remove("test_keyboard_debug.log");
    
    return 0;
}