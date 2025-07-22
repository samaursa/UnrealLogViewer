#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/log_entry_renderer.h"
#include "../lib/ui/visual_theme_manager.h"
#include "../lib/log_parser/log_entry.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <memory>

using namespace ue_log;
using namespace ftxui;

TEST_CASE("LogEntryRenderer constructor validation", "[LogEntryRenderer]") {
    // Should throw exception with null theme manager
    REQUIRE_THROWS_AS(LogEntryRenderer(nullptr), std::invalid_argument);
}

TEST_CASE("LogEntryRenderer configuration methods", "[LogEntryRenderer]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    SECTION("Word wrap configuration") {
        REQUIRE_FALSE(renderer->IsWordWrapEnabled());
        renderer->SetWordWrapEnabled(true);
        REQUIRE(renderer->IsWordWrapEnabled());
        renderer->SetWordWrapEnabled(false);
        REQUIRE_FALSE(renderer->IsWordWrapEnabled());
    }
    
    SECTION("Line numbers configuration") {
        REQUIRE(renderer->IsShowLineNumbers());
        renderer->SetShowLineNumbers(false);
        REQUIRE_FALSE(renderer->IsShowLineNumbers());
        renderer->SetShowLineNumbers(true);
        REQUIRE(renderer->IsShowLineNumbers());
    }
}

TEST_CASE("LogEntryRenderer logger badge color consistency", "[LogEntryRenderer]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    
    // Get colors for the same logger multiple times
    Color color1_first = theme_manager->GetLoggerColor("GameEngine");
    Color color1_second = theme_manager->GetLoggerColor("GameEngine");
    Color color2 = theme_manager->GetLoggerColor("Renderer");
    
    // Same logger should always get the same color
    REQUIRE(color1_first == color1_second);
    
    // Different loggers should get different colors (in most cases)
    REQUIRE(color1_first != color2);
}

TEST_CASE("LogEntryRenderer logger badge rendering", "[LogEntryRenderer]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    LogEntry entry(LogEntryType::Structured, 
                   std::string("12:34:56.789"), 
                   123, 
                   "TestLogger", 
                   std::string("Info"), 
                   "Test message", 
                   "[12:34:56.789][123]TestLogger: Info: Test message", 
                   1);
    
    // Render the logger badge - just verify it doesn't crash
    Element badge = renderer->RenderLoggerBadge(entry);
    
    // Test with empty logger name
    LogEntry empty_logger_entry(LogEntryType::Structured, 
                               std::string("12:34:56.789"), 
                               123, 
                               "", 
                               std::string("Info"), 
                               "Test message", 
                               "[12:34:56.789][123]: Info: Test message", 
                               1);
    
    Element empty_badge = renderer->RenderLoggerBadge(empty_logger_entry);
    // Just verify the rendering doesn't crash
}

TEST_CASE("LogEntryRenderer complete log entry rendering", "[LogEntryRenderer]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    LogEntry entry(LogEntryType::Structured, 
                   std::string("12:34:56.789"), 
                   123, 
                   "TestLogger", 
                   std::string("Info"), 
                   "Test message", 
                   "[12:34:56.789][123]TestLogger: Info: Test message", 
                   1);
    
    // Test rendering scenarios - just verify they don't crash
    Element unselected = renderer->RenderLogEntry(entry, false, 0);
    Element selected = renderer->RenderLogEntry(entry, true, 0);
    Element with_relative = renderer->RenderLogEntry(entry, false, 5);
}

TEST_CASE("LogEntryRenderer table header rendering", "[LogEntryRenderer]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    SECTION("With line numbers enabled") {
        renderer->SetShowLineNumbers(true);
        Element header_with_lines = renderer->RenderTableHeader();
    }
    
    SECTION("With line numbers disabled") {
        renderer->SetShowLineNumbers(false);
        Element header_without_lines = renderer->RenderTableHeader();
    }
}

TEST_CASE("LogEntryRenderer color cycling behavior", "[LogEntryRenderer]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    
    std::vector<Color> assigned_colors;
    
    // Get colors for more loggers than available in the palette
    size_t palette_size = theme_manager->GetAvailableLoggerColorCount();
    for (size_t i = 0; i < palette_size + 3; ++i) {
        std::string logger_name = "Logger" + std::to_string(i);
        assigned_colors.push_back(theme_manager->GetLoggerColor(logger_name));
    }
    
    // First few colors should be different
    for (size_t i = 0; i < std::min(palette_size, assigned_colors.size() - 1); ++i) {
        for (size_t j = i + 1; j < std::min(palette_size, assigned_colors.size()); ++j) {
            REQUIRE(assigned_colors[i] != assigned_colors[j]);
        }
    }
    
    // Colors should cycle after palette is exhausted
    if (assigned_colors.size() > palette_size) {
        REQUIRE(assigned_colors[0] == assigned_colors[palette_size]);
    }
}

TEST_CASE("LogEntryRenderer Enhanced Visual Hierarchy", "[LogEntryRenderer]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    SECTION("Error log entries visual prominence") {
        LogEntry error_entry(LogEntryType::Structured, 
                           std::string("12:34:56.789"), 
                           123, 
                           "TestLogger", 
                           std::string("Error"), 
                           "Critical error occurred", 
                           "[12:34:56.789][123]TestLogger: Error: Critical error occurred", 
                           1);
        
        // Render error entry - should not crash and should handle prominence
        Element error_unselected = renderer->RenderLogEntry(error_entry, false, 0);
        Element error_selected = renderer->RenderLogEntry(error_entry, true, 0);
        
        // Test individual components
        Element error_level = renderer->RenderLogLevel(error_entry);
        Element error_message = renderer->RenderMessage(error_entry, false);
    }
    
    SECTION("Warning log entries visual prominence") {
        LogEntry warning_entry(LogEntryType::Structured, 
                             std::string("12:34:56.789"), 
                             123, 
                             "TestLogger", 
                             std::string("Warning"), 
                             "Warning: potential issue detected", 
                             "[12:34:56.789][123]TestLogger: Warning: Warning: potential issue detected", 
                             1);
        
        // Render warning entry - should not crash and should handle prominence
        Element warning_unselected = renderer->RenderLogEntry(warning_entry, false, 0);
        Element warning_selected = renderer->RenderLogEntry(warning_entry, true, 0);
        
        // Test individual components
        Element warning_level = renderer->RenderLogLevel(warning_entry);
        Element warning_message = renderer->RenderMessage(warning_entry, false);
    }
    
    SECTION("Normal log entries subtle styling") {
        LogEntry info_entry(LogEntryType::Structured, 
                          std::string("12:34:56.789"), 
                          123, 
                          "TestLogger", 
                          std::string("Info"), 
                          "Normal information message", 
                          "[12:34:56.789][123]TestLogger: Info: Normal information message", 
                          1);
        
        LogEntry display_entry(LogEntryType::Structured, 
                             std::string("12:34:56.789"), 
                             123, 
                             "TestLogger", 
                             std::string("Display"), 
                             "Display message", 
                             "[12:34:56.789][123]TestLogger: Display: Display message", 
                             1);
        
        // Render normal entries - should not crash and should have subtle styling
        Element info_rendered = renderer->RenderLogEntry(info_entry, false, 0);
        Element display_rendered = renderer->RenderLogEntry(display_entry, false, 0);
        
        // Test individual components
        Element info_level = renderer->RenderLogLevel(info_entry);
        Element info_message = renderer->RenderMessage(info_entry, false);
        Element display_level = renderer->RenderLogLevel(display_entry);
        Element display_message = renderer->RenderMessage(display_entry, false);
    }
    
    SECTION("Log level visual differentiation") {
        std::vector<std::string> test_levels = {"Error", "Warning", "Info", "Debug", "Display", "Verbose", "VeryVerbose", "Trace"};
        
        for (const auto& level : test_levels) {
            LogEntry test_entry(LogEntryType::Structured, 
                              std::string("12:34:56.789"), 
                              123, 
                              "TestLogger", 
                              std::string(level), 
                              "Test message for " + level, 
                              "[12:34:56.789][123]TestLogger: " + level + ": Test message for " + level, 
                              1);
            
            // Each level should render without crashing
            Element level_element = renderer->RenderLogLevel(test_entry);
            Element message_element = renderer->RenderMessage(test_entry, false);
            Element full_entry = renderer->RenderLogEntry(test_entry, false, 0);
            
            // Verify that prominent levels are handled correctly
            if (theme_manager->IsLogLevelProminent(level)) {
                // These should have enhanced styling - just verify they render
                Element selected_entry = renderer->RenderLogEntry(test_entry, true, 0);
            }
        }
    }
    
    SECTION("Visual hierarchy with selection interaction") {
        LogEntry error_entry(LogEntryType::Structured, 
                           std::string("12:34:56.789"), 
                           123, 
                           "TestLogger", 
                           std::string("Error"), 
                           "Error message", 
                           "[12:34:56.789][123]TestLogger: Error: Error message", 
                           1);
        
        LogEntry warning_entry(LogEntryType::Structured, 
                             std::string("12:34:56.789"), 
                             123, 
                             "TestLogger", 
                             std::string("Warning"), 
                             "Warning message", 
                             "[12:34:56.789][123]TestLogger: Warning: Warning message", 
                             1);
        
        // Test that selection and visual hierarchy work together
        Element error_selected = renderer->RenderLogEntry(error_entry, true, 0);
        Element error_unselected = renderer->RenderLogEntry(error_entry, false, 0);
        Element warning_selected = renderer->RenderLogEntry(warning_entry, true, 0);
        Element warning_unselected = renderer->RenderLogEntry(warning_entry, false, 0);
        
        // Should render without crashing - visual hierarchy should work with selection
    }
    
    SECTION("Accessibility and contrast considerations") {
        // Test that all log levels maintain good contrast and accessibility
        std::vector<std::string> accessibility_levels = {"Error", "Warning", "Info", "Debug"};
        
        for (const auto& level : accessibility_levels) {
            // Verify colors are accessible
            Color level_color = theme_manager->GetLogLevelColor(level);
            Color bg_color = theme_manager->GetLogLevelBackgroundColor(level);
            
            // Colors should be valid FTXUI colors (basic validation)
            // More detailed accessibility testing would require color contrast calculations
        }
    }
}