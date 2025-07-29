#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <fstream>

int main() {
    // Create a large test log file to test scrollbar
    std::ofstream test_file("test_scrollbar.log");
    for (int i = 0; i < 100; ++i) {
        test_file << "[2024-01-01 10:00:" << std::setfill('0') << std::setw(2) << (i % 60) 
                 << "] INFO: Log entry number " << i << " with some content\n";
    }
    test_file.close();
    
    // Create config manager and main window
    ue_log::ConfigManager config_manager;
    ue_log::MainWindow main_window(&config_manager);
    
    // Initialize and load the test file
    main_window.Initialize();
    main_window.SetTerminalSize(80, 25); // Smaller window to force scrolling
    
    if (!main_window.LoadLogFile("test_scrollbar.log")) {
        std::cerr << "Failed to load test file: " << main_window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Test file loaded successfully!" << std::endl;
    std::cout << "Total entries: " << main_window.GetDisplayedEntries().size() << std::endl;
    
    // Test scrollbar at different positions
    std::vector<int> test_positions = {0, 25, 50, 75, 99};
    
    for (int pos : test_positions) {
        main_window.JumpToLine(pos + 1); // JumpToLine uses 1-based indexing
        
        std::cout << "\n=== Position " << pos << " ===" << std::endl;
        std::cout << "Selected entry index: " << main_window.GetSelectedEntryIndex() << std::endl;
        
        // Render the main window to see the scrollbar
        auto element = main_window.Render();
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(80), ftxui::Dimension::Fixed(25));
        ftxui::Render(screen, element);
        
        // Print a portion of the screen to show the scrollbar
        std::cout << "Scrollbar visualization (rightmost column):" << std::endl;
        for (int row = 2; row < 20; ++row) { // Skip header and footer
            auto& pixel = screen.PixelAt(79, row); // Rightmost column
            std::cout << (pixel.character.empty() ? " " : pixel.character) << std::endl;
        }
    }
    
    std::cout << "\nScrollbar visual test completed!" << std::endl;
    
    // Clean up
    std::remove("test_scrollbar.log");
    
    return 0;
}