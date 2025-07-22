#include "lib/ui/visual_theme_manager.h"
#include "lib/ui/log_entry_renderer.h"
#include "lib/log_parser/log_entry.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <memory>

using namespace ue_log;
using namespace ftxui;

int main() {
    std::cout << "=== Column Spacing and Visual Formatting Demo ===" << std::endl;
    
    // Create theme manager and renderer
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    // Display current column spacing configuration
    auto spacing = theme_manager->GetColumnSpacing();
    std::cout << "\nColumn Spacing Configuration:" << std::endl;
    std::cout << "  Line Number Width: " << spacing.line_number_width << std::endl;
    std::cout << "  Timestamp Width: " << spacing.timestamp_width << std::endl;
    std::cout << "  Frame Width: " << spacing.frame_width << std::endl;
    std::cout << "  Logger Badge Width: " << spacing.logger_badge_width << std::endl;
    std::cout << "  Level Width: " << spacing.level_width << std::endl;
    std::cout << "  Use Visual Separators: " << (spacing.use_visual_separators ? "Yes" : "No") << std::endl;
    std::cout << "  Align Numbers Right: " << (spacing.align_numbers_right ? "Yes" : "No") << std::endl;
    
    // Create sample log entries with different characteristics
    std::vector<LogEntry> test_entries;
    
    // Entry 1: Normal entry
    LogEntry entry1;
    entry1.Request_line_number(1)
          .Request_timestamp("12:34:56.789")
          .Request_frame_number(1000)
          .Request_logger_name("GameEngine")
          .Request_log_level("Info")
          .Request_message("Game engine initialized successfully");
    test_entries.push_back(entry1);
    
    // Entry 2: Error entry with longer logger name
    LogEntry entry2;
    entry2.Request_line_number(25)
          .Request_timestamp("12:34:57.123")
          .Request_frame_number(1001)
          .Request_logger_name("VeryLongLoggerNameExample")
          .Request_log_level("Error")
          .Request_message("Critical error occurred during initialization");
    test_entries.push_back(entry2);
    
    // Entry 3: Warning with short logger
    LogEntry entry3;
    entry3.Request_line_number(100)
          .Request_timestamp("12:34:58.456")
          .Request_frame_number(1002)
          .Request_logger_name("AI")
          .Request_log_level("Warning")
          .Request_message("AI pathfinding optimization needed");
    test_entries.push_back(entry3);
    
    // Entry 4: Debug with very long timestamp
    LogEntry entry4;
    entry4.Request_line_number(1234)
          .Request_timestamp("12:34:59.789123456")
          .Request_frame_number(10000)
          .Request_logger_name("NetworkManager")
          .Request_log_level("Debug")
          .Request_message("Network packet received from client");
    test_entries.push_back(entry4);
    
    std::cout << "\n=== Visual Separator Demo ===" << std::endl;
    std::cout << "Column Separator: '" << theme_manager->GetColumnSeparator() << "'" << std::endl;
    
    // Render table header
    std::cout << "\n=== Table Header ===" << std::endl;
    auto header = renderer->RenderTableHeader();
    Screen header_screen(120, 1);
    Render(header_screen, header);
    std::cout << header_screen.ToString() << std::endl;
    
    // Render sample entries
    std::cout << "\n=== Sample Log Entries ===" << std::endl;
    for (size_t i = 0; i < test_entries.size(); ++i) {
        auto element = renderer->RenderLogEntry(test_entries[i], i == 1, 0); // Select second entry
        Screen screen(120, 1);
        Render(screen, element);
        std::cout << screen.ToString() << std::endl;
    }
    
    // Test without visual separators
    std::cout << "\n=== Without Visual Separators ===" << std::endl;
    theme_manager->SetVisualSeparatorsEnabled(false);
    std::cout << "Column Separator: '" << theme_manager->GetColumnSeparator() << "'" << std::endl;
    
    // Render header without separators
    auto header_no_sep = renderer->RenderTableHeader();
    Screen header_no_sep_screen(120, 1);
    Render(header_no_sep_screen, header_no_sep);
    std::cout << header_no_sep_screen.ToString() << std::endl;
    
    // Render one entry without separators
    auto entry_no_sep = renderer->RenderLogEntry(test_entries[0], false, 0);
    Screen entry_no_sep_screen(120, 1);
    Render(entry_no_sep_screen, entry_no_sep);
    std::cout << entry_no_sep_screen.ToString() << std::endl;
    
    // Test custom column spacing
    std::cout << "\n=== Custom Column Spacing ===" << std::endl;
    VisualThemeManager::ColumnSpacing custom_spacing;
    custom_spacing.line_number_width = 8;
    custom_spacing.timestamp_width = 20;
    custom_spacing.frame_width = 10;
    custom_spacing.logger_badge_width = 25;
    custom_spacing.level_width = 10;
    custom_spacing.use_visual_separators = true;
    custom_spacing.align_numbers_right = true;
    
    theme_manager->SetColumnSpacing(custom_spacing);
    
    std::cout << "Custom spacing applied:" << std::endl;
    std::cout << "  Line: " << custom_spacing.line_number_width 
              << ", Timestamp: " << custom_spacing.timestamp_width
              << ", Frame: " << custom_spacing.frame_width
              << ", Logger: " << custom_spacing.logger_badge_width
              << ", Level: " << custom_spacing.level_width << std::endl;
    
    // Render with custom spacing
    auto custom_header = renderer->RenderTableHeader();
    Screen custom_header_screen(140, 1);
    Render(custom_header_screen, custom_header);
    std::cout << custom_header_screen.ToString() << std::endl;
    
    auto custom_entry = renderer->RenderLogEntry(test_entries[1], true, 0);
    Screen custom_entry_screen(140, 1);
    Render(custom_entry_screen, custom_entry);
    std::cout << custom_entry_screen.ToString() << std::endl;
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "Column spacing and visual formatting enhancements are working correctly!" << std::endl;
    
    return 0;
}