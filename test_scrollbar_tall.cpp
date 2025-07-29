#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <fstream>

int main() {
    // Create a large test log file to test scrollbar
    std::ofstream test_file("test_scrollbar_tall.log");
    for (int i = 0; i < 100; ++i) {
        test_file << "[2024-01-01 10:00:" << std::setfill('0') << std::setw(2) << (i % 60) 
                 << "] INFO: Log entry number " << i << " with some content\n";
    }
    test_file.close();
    
    // Create config manager and main window
    ue_log::ConfigManager config_manager;
    ue_log::MainWindow main_window(&config_manager);
    
    // Initialize and load the test file with a taller window
    main_window.Initialize();
    main_window.SetTerminalSize(80, 40); // Taller window
    
    if (!main_window.LoadLogFile("test_scrollbar_tall.log")) {
        std::cerr << "Failed to load test file: " << main_window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Test file loaded successfully!" << std::endl;
    std::cout << "Total entries: " << main_window.GetDisplayedEntries().size() << std::endl;
    
    // Test scrollbar at different positions with taller window
    std::vector<int> test_positions = {0, 50, 99};
    
    for (int pos : test_positions) {
        main_window.JumpToLine(pos + 1);
        
        std::cout << "\n=== Position " << pos << " (Tall Window) ===" << std::endl;
        std::cout << "Selected entry index: " << main_window.GetSelectedEntryIndex() << std::endl;
        
        // Render the main window to see the scrollbar
        auto element = main_window.Render();
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(80), ftxui::Dimension::Fixed(40));
        ftxui::Render(screen, element);
        
        // Print the scrollbar column to see if it spans more height
        std::cout << "Scrollbar column (column 79):" << std::endl;
        for (int row = 1; row < 35; ++row) { // Check more rows
            auto& pixel = screen.PixelAt(79, row);
            char ch = pixel.character.empty() ? ' ' : pixel.character[0];
            std::cout << ch << " (row " << row << ")" << std::endl;
        }
    }
    
    std::cout << "\nTall window scrollbar test completed!" << std::endl;
    
    // Clean up
    std::remove("test_scrollbar_tall.log");
    
    return 0;
}