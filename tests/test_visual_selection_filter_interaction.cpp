#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

/**
 * Test visual selection mode interaction with active filters.
 * Verifies that visual selection works correctly when filters are applied.
 */
int main() {
    std::cout << "🧪 Testing visual selection mode with active filters...\n\n";
    
    // Create a test log file with different log levels
    std::ofstream file("test_filter_interaction.log");
    file << "[2024.01.15-10.30.45:123][456]LogTemp: Warning: First warning message\n";
    file << "[2024.01.15-10.30.46:124][457]LogCore: Error: First error message\n";
    file << "[2024.01.15-10.30.47:125][458]LogTemp: Info: First info message\n";
    file << "[2024.01.15-10.30.48:126][459]LogCore: Debug: First debug message\n";
    file << "[2024.01.15-10.30.49:127][460]LogTemp: Warning: Second warning message\n";
    file << "[2024.01.15-10.30.50:128][461]LogCore: Error: Second error message\n";
    file.close();
    
    // Create and initialize MainWindow
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_filter_interaction.log")) {
        std::cout << "❌ Failed to load test file\n";
        return 1;
    }
    
    auto component = window.CreateFTXUIComponent();
    
    // Navigate to ensure we have a valid selection
    ftxui::Event j_key = ftxui::Event::Character('j');
    component->OnEvent(j_key);
    
    std::cout << "📋 Test 1: Visual selection with no filters\n";
    std::cout << "Total entries: " << window.GetDisplayedEntries().size() << "\n";
    
    // Enter visual selection mode
    ftxui::Event v_key = ftxui::Event::Character('v');
    bool handled = component->OnEvent(v_key);
    
    if (!handled || !window.IsVisualSelectionMode()) {
        std::cout << "❌ Failed to enter visual selection mode\n";
        return 1;
    }
    std::cout << "✅ Visual selection mode activated with all entries visible\n";
    
    // Exit visual selection mode
    ftxui::Event esc_key = ftxui::Event::Escape;
    component->OnEvent(esc_key);
    
    std::cout << "\n📋 Test 2: Apply filter and test visual selection\n";
    
    // Create a search filter for "Error" messages
    ftxui::Event slash_key = ftxui::Event::Character('/');
    component->OnEvent(slash_key);
    
    // Type "Error"
    component->OnEvent(ftxui::Event::Character('E'));
    component->OnEvent(ftxui::Event::Character('r'));
    component->OnEvent(ftxui::Event::Character('r'));
    component->OnEvent(ftxui::Event::Character('o'));
    component->OnEvent(ftxui::Event::Character('r'));
    
    // Confirm search
    component->OnEvent(ftxui::Event::Return);
    
    std::cout << "Filtered entries: " << window.GetDisplayedEntries().size() << "\n";
    
    // Try to enter visual selection mode with filtered results
    handled = component->OnEvent(v_key);
    
    if (!handled || !window.IsVisualSelectionMode()) {
        std::cout << "❌ Failed to enter visual selection mode with filtered results\n";
        return 1;
    }
    std::cout << "✅ Visual selection mode works with filtered results\n";
    
    // Test navigation within filtered results
    ftxui::Event k_key = ftxui::Event::Character('k');
    handled = component->OnEvent(k_key);
    
    if (!handled) {
        std::cout << "❌ Navigation should work in visual selection mode with filters\n";
        return 1;
    }
    std::cout << "✅ Navigation works in visual selection mode with filters\n";
    
    // Test copy functionality with filtered results
    ftxui::Event y_key = ftxui::Event::Character('y');
    handled = component->OnEvent(y_key);
    
    if (!handled || window.IsVisualSelectionMode()) {
        std::cout << "❌ Copy and exit should work with filtered results\n";
        return 1;
    }
    std::cout << "✅ Copy and exit works with filtered results\n";
    
    std::cout << "\n📋 Test 3: Filter changes while in visual selection mode\n";
    
    // Clear search first
    component->OnEvent(esc_key);
    
    // Navigate and enter visual selection mode
    component->OnEvent(j_key);
    component->OnEvent(v_key);
    
    if (!window.IsVisualSelectionMode()) {
        std::cout << "❌ Failed to re-enter visual selection mode\n";
        return 1;
    }
    
    // Try to activate search while in visual selection mode (should be blocked)
    handled = component->OnEvent(slash_key);
    
    if (!handled) {
        std::cout << "❌ Search activation should be blocked in visual selection mode\n";
        return 1;
    }
    
    // Verify search is not active
    if (window.IsSearchActive()) {
        std::cout << "❌ Search should not be active (blocked in visual selection mode)\n";
        return 1;
    }
    std::cout << "✅ Filter changes blocked in visual selection mode\n";
    
    // Exit visual selection mode
    component->OnEvent(esc_key);
    
    // Clean up
    std::remove("test_filter_interaction.log");
    
    std::cout << "\n🎉 All visual selection filter interaction tests passed!\n";
    return 0;
}