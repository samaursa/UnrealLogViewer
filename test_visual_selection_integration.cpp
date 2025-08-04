#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file
    std::ofstream file("test_visual_integration.log");
    file << "[2024.01.15-10.30.45:123][456]LogTemp: Warning: Test message 1\n";
    file << "[2024.01.15-10.30.46:124][457]LogTemp: Error: Test message 2\n";
    file << "[2024.01.15-10.30.47:125][458]LogCore: Info: Test message 3\n";
    file.close();
    
    // Create and initialize MainWindow
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_visual_integration.log")) {
        std::cout << "❌ Failed to load test file\n";
        return 1;
    }
    
    std::cout << "✅ Test file loaded successfully\n";
    
    auto component = window.CreateFTXUIComponent();
    
    // Test integration: Normal navigation should work when NOT in visual selection mode
    ftxui::Event j_key = ftxui::Event::Character('j');
    bool handled = component->OnEvent(j_key);
    
    if (!handled) {
        std::cout << "❌ 'j' key should be handled for normal navigation\n";
        return 1;
    }
    std::cout << "✅ Normal navigation works when not in visual selection mode\n";
    
    // Test integration: Enter visual selection mode
    ftxui::Event v_key = ftxui::Event::Character('v');
    handled = component->OnEvent(v_key);
    
    if (!handled || !window.IsVisualSelectionMode()) {
        std::cout << "❌ Failed to enter visual selection mode\n";
        return 1;
    }
    std::cout << "✅ Entered visual selection mode successfully\n";
    
    // Test integration: Normal shortcuts should be blocked in visual selection mode
    ftxui::Event f_key = ftxui::Event::Character('f'); // Filter panel toggle
    handled = component->OnEvent(f_key);
    
    if (!handled) {
        std::cout << "❌ Filter panel toggle should be blocked in visual selection mode\n";
        return 1;
    }
    
    // Verify filter panel is not visible (blocked)
    if (window.IsFilterPanelVisible()) {
        std::cout << "❌ Filter panel should not be visible (blocked in visual selection mode)\n";
        return 1;
    }
    std::cout << "✅ Filter panel toggle blocked in visual selection mode\n";
    
    // Test integration: Search should be blocked in visual selection mode
    ftxui::Event slash_key = ftxui::Event::Character('/');
    handled = component->OnEvent(slash_key);
    
    if (!handled) {
        std::cout << "❌ Search should be blocked in visual selection mode\n";
        return 1;
    }
    
    // Verify search is not active (blocked)
    if (window.IsSearchActive()) {
        std::cout << "❌ Search should not be active (blocked in visual selection mode)\n";
        return 1;
    }
    std::cout << "✅ Search blocked in visual selection mode\n";
    
    // Test integration: Help should still work in visual selection mode
    ftxui::Event h_key = ftxui::Event::Character('h');
    handled = component->OnEvent(h_key);
    
    if (handled) {
        std::cout << "❌ Help should pass through in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ Help passes through in visual selection mode\n";
    
    // Test integration: ESC should exit visual selection mode and restore normal functionality
    ftxui::Event esc_key = ftxui::Event::Escape;
    handled = component->OnEvent(esc_key);
    
    if (!handled || window.IsVisualSelectionMode()) {
        std::cout << "❌ ESC should exit visual selection mode\n";
        return 1;
    }
    std::cout << "✅ ESC exits visual selection mode\n";
    
    // Test integration: Normal shortcuts should work again after exiting visual selection mode
    handled = component->OnEvent(f_key); // Filter panel toggle
    
    if (!handled) {
        std::cout << "❌ Filter panel toggle should work after exiting visual selection mode\n";
        return 1;
    }
    std::cout << "✅ Normal shortcuts work after exiting visual selection mode\n";
    
    // Clean up
    std::remove("test_visual_integration.log");
    
    std::cout << "\n🎉 All visual selection integration tests passed!\n";
    return 0;
}