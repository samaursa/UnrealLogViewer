#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file
    std::ofstream file("test_visual_keyboard.log");
    file << "[2024.01.15-10.30.45:123][456]LogTemp: Warning: Test message 1\n";
    file << "[2024.01.15-10.30.46:124][457]LogTemp: Error: Test message 2\n";
    file << "[2024.01.15-10.30.47:125][458]LogCore: Info: Test message 3\n";
    file << "[2024.01.15-10.30.48:126][459]LogCore: Debug: Test message 4\n";
    file << "[2024.01.15-10.30.49:127][460]LogCore: Info: Test message 5\n";
    file.close();
    
    // Create and initialize MainWindow
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_visual_keyboard.log")) {
        std::cout << "❌ Failed to load test file\n";
        return 1;
    }
    
    std::cout << "✅ Test file loaded successfully\n";
    
    auto component = window.CreateFTXUIComponent();
    
    // Test 1: Enter visual selection mode with 'v' key
    ftxui::Event v_key = ftxui::Event::Character('v');
    bool handled = component->OnEvent(v_key);
    
    if (!handled || !window.IsVisualSelectionMode()) {
        std::cout << "❌ Failed to enter visual selection mode with 'v' key\n";
        return 1;
    }
    std::cout << "✅ Entered visual selection mode with 'v' key\n";
    
    // Test 2: ESC key exits visual selection mode
    ftxui::Event esc_key = ftxui::Event::Escape;
    handled = component->OnEvent(esc_key);
    
    if (!handled || window.IsVisualSelectionMode()) {
        std::cout << "❌ ESC key should exit visual selection mode\n";
        return 1;
    }
    std::cout << "✅ ESC key exits visual selection mode\n";
    
    // Test 3: Re-enter visual selection mode and test navigation keys
    component->OnEvent(v_key); // Re-enter visual selection mode
    
    if (!window.IsVisualSelectionMode()) {
        std::cout << "❌ Failed to re-enter visual selection mode\n";
        return 1;
    }
    
    // Test j key (down navigation)
    ftxui::Event j_key = ftxui::Event::Character('j');
    handled = component->OnEvent(j_key);
    
    if (!handled) {
        std::cout << "❌ 'j' key should be handled in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ 'j' key handled in visual selection mode\n";
    
    // Test k key (up navigation)
    ftxui::Event k_key = ftxui::Event::Character('k');
    handled = component->OnEvent(k_key);
    
    if (!handled) {
        std::cout << "❌ 'k' key should be handled in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ 'k' key handled in visual selection mode\n";
    
    // Test arrow keys
    ftxui::Event arrow_down = ftxui::Event::ArrowDown;
    handled = component->OnEvent(arrow_down);
    
    if (!handled) {
        std::cout << "❌ Arrow down should be handled in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ Arrow down handled in visual selection mode\n";
    
    ftxui::Event arrow_up = ftxui::Event::ArrowUp;
    handled = component->OnEvent(arrow_up);
    
    if (!handled) {
        std::cout << "❌ Arrow up should be handled in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ Arrow up handled in visual selection mode\n";
    
    // Test Ctrl+d (half-page down)
    ftxui::Event ctrl_d = ftxui::Event::Character(static_cast<char>(4));
    handled = component->OnEvent(ctrl_d);
    
    if (!handled) {
        std::cout << "❌ Ctrl+d should be handled in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ Ctrl+d handled in visual selection mode\n";
    
    // Test Ctrl+u (half-page up)
    ftxui::Event ctrl_u = ftxui::Event::Character(static_cast<char>(21));
    handled = component->OnEvent(ctrl_u);
    
    if (!handled) {
        std::cout << "❌ Ctrl+u should be handled in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ Ctrl+u handled in visual selection mode\n";
    
    // Test y key (copy and exit)
    ftxui::Event y_key = ftxui::Event::Character('y');
    handled = component->OnEvent(y_key);
    
    if (!handled || window.IsVisualSelectionMode()) {
        std::cout << "❌ 'y' key should copy and exit visual selection mode\n";
        return 1;
    }
    std::cout << "✅ 'y' key handled and exited visual selection mode\n";
    
    // Test 4: Help keys should be allowed in visual selection mode
    component->OnEvent(v_key); // Re-enter visual selection mode
    
    ftxui::Event h_key = ftxui::Event::Character('h');
    handled = component->OnEvent(h_key);
    
    // Help should not be handled by visual selection (returns false to allow normal help handling)
    if (handled) {
        std::cout << "❌ 'h' key should be allowed to pass through for help in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ 'h' key passes through for help in visual selection mode\n";
    
    ftxui::Event question_key = ftxui::Event::Character('?');
    handled = component->OnEvent(question_key);
    
    if (handled) {
        std::cout << "❌ '?' key should be allowed to pass through for help in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ '?' key passes through for help in visual selection mode\n";
    
    ftxui::Event f1_key = ftxui::Event::F1;
    handled = component->OnEvent(f1_key);
    
    if (handled) {
        std::cout << "❌ F1 key should be allowed to pass through for help in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ F1 key passes through for help in visual selection mode\n";
    
    // Test 5: Other keys should be blocked in visual selection mode
    ftxui::Event f_key = ftxui::Event::Character('f');
    handled = component->OnEvent(f_key);
    
    if (!handled) {
        std::cout << "❌ 'f' key should be blocked in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ 'f' key blocked in visual selection mode\n";
    
    ftxui::Event slash_key = ftxui::Event::Character('/');
    handled = component->OnEvent(slash_key);
    
    if (!handled) {
        std::cout << "❌ '/' key should be blocked in visual selection mode\n";
        return 1;
    }
    std::cout << "✅ '/' key blocked in visual selection mode\n";
    
    // Test 6: Ensure we're still in visual selection mode after blocked keys
    if (!window.IsVisualSelectionMode()) {
        std::cout << "❌ Should still be in visual selection mode after blocked keys\n";
        return 1;
    }
    std::cout << "✅ Still in visual selection mode after blocked keys\n";
    
    // Clean up
    component->OnEvent(ftxui::Event::Escape); // Exit visual selection mode
    std::remove("test_visual_keyboard.log");
    
    std::cout << "\n🎉 All visual selection keyboard event handling tests passed!\n";
    return 0;
}