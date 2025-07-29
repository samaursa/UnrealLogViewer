#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <fstream>

int main() {
    // Create a large test log file to test scrollbar
    std::ofstream test_file("test_scrollbar_debug.log");
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
    
    if (!main_window.LoadLogFile("test_scrollbar_debug.log")) {
        std::cerr << "Failed to load test file: " << main_window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Test file loaded successfully!" << std::endl;
    std::cout << "Total entries: " << main_window.GetDisplayedEntries().size() << std::endl;
    
    // Test scrollbar at different positions with debug info
    std::vector<int> test_positions = {0, 25, 50, 75, 99};
    
    for (int pos : test_positions) {
        main_window.JumpToLine(pos + 1); // JumpToLine uses 1-based indexing
        
        std::cout << "\n=== Position " << pos << " ===" << std::endl;
        std::cout << "Selected entry index: " << main_window.GetSelectedEntryIndex() << std::endl;
        
        // Calculate expected scrollbar values
        int visible_height = 25 - 4; // Approximate visible height (window - borders/status)
        int total_entries = 100;
        double position_ratio = static_cast<double>(pos) / std::max(1, total_entries - 1);
        double visible_ratio = static_cast<double>(visible_height) / static_cast<double>(total_entries);
        int scrollbar_height = std::max(10, visible_height - 2);
        int thumb_size = std::max(1, static_cast<int>(scrollbar_height * visible_ratio));
        int thumb_position = static_cast<int>((scrollbar_height - thumb_size) * position_ratio);
        
        std::cout << "Debug info:" << std::endl;
        std::cout << "  Position ratio: " << position_ratio << std::endl;
        std::cout << "  Visible ratio: " << visible_ratio << std::endl;
        std::cout << "  Scrollbar height: " << scrollbar_height << std::endl;
        std::cout << "  Thumb size: " << thumb_size << std::endl;
        std::cout << "  Thumb position: " << thumb_position << std::endl;
        
        // Render the main window to see the scrollbar
        auto element = main_window.Render();
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(80), ftxui::Dimension::Fixed(25));
        ftxui::Render(screen, element);
        
        // Print the entire rightmost few columns to see the scrollbar
        std::cout << "Right side of screen (columns 75-79):" << std::endl;
        for (int row = 1; row < 24; ++row) { // Skip top and bottom borders
            for (int col = 75; col < 80; ++col) {
                auto& pixel = screen.PixelAt(col, row);
                std::cout << (pixel.character.empty() ? " " : pixel.character);
            }
            std::cout << " |" << std::endl;
        }
    }
    
    std::cout << "\nScrollbar debug test completed!" << std::endl;
    
    // Clean up
    std::remove("test_scrollbar_debug.log");
    
    return 0;
}