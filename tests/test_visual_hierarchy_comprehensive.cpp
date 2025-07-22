#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/visual_theme_manager.h"
#include "../lib/ui/log_entry_renderer.h"
#include "../lib/log_parser/log_entry.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>

using namespace ue_log;
using namespace ftxui;

TEST_CASE("Visual Hierarchy Comprehensive Requirements Validation", "[ui][visual_hierarchy][comprehensive]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    SECTION("Requirement 3.1: Error log entries have prominent visual styling") {
        LogEntry error_entry(LogEntryType::Structured, 
                           std::string("12:34:56.789"), 
                           123, 
                           "TestLogger", 
                           std::string("Error"), 
                           "Critical error occurred", 
                           "[12:34:56.789][123]TestLogger: Error: Critical error occurred", 
                           1);
        
        // Error should be prominent
        REQUIRE(theme_manager->IsLogLevelProminent("Error"));
        
        // Error should use bold text
        REQUIRE(theme_manager->ShouldLogLevelUseBold("Error"));
        
        // Error should have special background color
        REQUIRE(theme_manager->GetLogLevelBackgroundColor("Error") != theme_manager->GetBackgroundColor());
        
        // Error should have distinct color
        Color error_color = theme_manager->GetLogLevelColor("Error");
        Color normal_color = theme_manager->GetLogLevelColor("Display");
        REQUIRE(error_color != normal_color);
        
        // Rendering should not crash
        Element error_rendered = renderer->RenderLogEntry(error_entry, false, 0);
        Element error_level = renderer->RenderLogLevel(error_entry);
        Element error_message = renderer->RenderMessage(error_entry, false);
        
        // Test that error entries render with visual indicators
        Screen screen(120, 3);
        Render(screen, error_rendered);
        std::string output = screen.ToString();
        
        // Should contain visual indicator for prominent entries
        REQUIRE(output.find("▌") != std::string::npos);
    }
    
    SECTION("Requirement 3.2: Warning log entries have distinct visual styling") {
        LogEntry warning_entry(LogEntryType::Structured, 
                             std::string("12:34:56.789"), 
                             123, 
                             "TestLogger", 
                             std::string("Warning"), 
                             "Warning: potential issue detected", 
                             "[12:34:56.789][123]TestLogger: Warning: Warning: potential issue detected", 
                             1);
        
        // Warning should be prominent
        REQUIRE(theme_manager->IsLogLevelProminent("Warning"));
        
        // Warning should use bold text
        REQUIRE(theme_manager->ShouldLogLevelUseBold("Warning"));
        
        // Warning should have distinct color from error and normal
        Color warning_color = theme_manager->GetLogLevelColor("Warning");
        Color error_color = theme_manager->GetLogLevelColor("Error");
        Color normal_color = theme_manager->GetLogLevelColor("Display");
        
        REQUIRE(warning_color != error_color);
        REQUIRE(warning_color != normal_color);
        
        // Rendering should not crash
        Element warning_rendered = renderer->RenderLogEntry(warning_entry, false, 0);
        Element warning_level = renderer->RenderLogLevel(warning_entry);
        Element warning_message = renderer->RenderMessage(warning_entry, false);
        
        // Test that warning entries render with visual indicators
        Screen screen(120, 3);
        Render(screen, warning_rendered);
        std::string output = screen.ToString();
        
        // Should contain visual indicator for prominent entries
        REQUIRE(output.find("▌") != std::string::npos);
    }
    
    SECTION("Requirement 3.3: Normal log entries have subtle but readable styling") {
        std::vector<std::string> normal_levels = {"Display", "Info", "Verbose", "VeryVerbose", "Trace", "Debug"};
        
        for (const auto& level : normal_levels) {
            LogEntry normal_entry(LogEntryType::Structured, 
                                std::string("12:34:56.789"), 
                                123, 
                                "TestLogger", 
                                std::string(level), 
                                "Normal message for " + level, 
                                "[12:34:56.789][123]TestLogger: " + level + ": Normal message for " + level, 
                                1);
            
            // Normal levels should not be prominent
            REQUIRE_FALSE(theme_manager->IsLogLevelProminent(level));
            
            // Normal levels should not use bold text
            REQUIRE_FALSE(theme_manager->ShouldLogLevelUseBold(level));
            
            // Normal levels should use default background
            REQUIRE(theme_manager->GetLogLevelBackgroundColor(level) == theme_manager->GetBackgroundColor());
            
            // Should still have assigned colors (not default white for all)
            Color level_color = theme_manager->GetLogLevelColor(level);
            // Each level should have a color assigned (basic validation)
            
            // Rendering should not crash
            Element normal_rendered = renderer->RenderLogEntry(normal_entry, false, 0);
            Element normal_level = renderer->RenderLogLevel(normal_entry);
            Element normal_message = renderer->RenderMessage(normal_entry, false);
            
            // Test that normal entries have subtle visual indicators for column alignment
            Screen screen(120, 3);
            Render(screen, normal_rendered);
            std::string output = screen.ToString();
            
            // All entries should have visual indicators for consistent column alignment
            REQUIRE(output.find("▌") != std::string::npos);
        }
    }
    
    SECTION("Requirement 3.4: Different log levels have appropriate color coding") {
        std::vector<std::string> all_levels = {"Error", "Warning", "Display", "Info", "Verbose", "VeryVerbose", "Trace", "Debug"};
        std::vector<Color> assigned_colors;
        
        // Collect all assigned colors
        for (const auto& level : all_levels) {
            assigned_colors.push_back(theme_manager->GetLogLevelColor(level));
        }
        
        // Critical levels should have distinct, prominent colors
        Color error_color = theme_manager->GetLogLevelColor("Error");
        Color warning_color = theme_manager->GetLogLevelColor("Warning");
        
        // Error and Warning should have different colors
        REQUIRE(error_color != warning_color);
        
        // Test specific color expectations for Unreal Engine levels
        REQUIRE(theme_manager->GetLogLevelColor("Error") == Color::Red);
        REQUIRE(theme_manager->GetLogLevelColor("Warning") == Color::Yellow);
        REQUIRE(theme_manager->GetLogLevelColor("Display") == Color::White);
        REQUIRE(theme_manager->GetLogLevelColor("Verbose") == Color::GrayLight);
        REQUIRE(theme_manager->GetLogLevelColor("VeryVerbose") == Color::GrayDark);
        REQUIRE(theme_manager->GetLogLevelColor("Trace") == Color::CyanLight);
        
        // Generic levels should also have appropriate colors
        REQUIRE(theme_manager->GetLogLevelColor("Info") == Color::White);
        REQUIRE(theme_manager->GetLogLevelColor("Debug") == Color::GrayLight);
    }
    
    SECTION("Requirement 3.6: Color coding maintains good contrast and accessibility") {
        std::vector<std::string> test_levels = {"Error", "Warning", "Display", "Info", "Verbose", "Debug"};
        
        for (const auto& level : test_levels) {
            Color text_color = theme_manager->GetLogLevelColor(level);
            Color bg_color = theme_manager->GetLogLevelBackgroundColor(level);
            
            // Basic validation - colors should be valid FTXUI colors
            // More detailed accessibility testing would require color contrast calculations
            // For now, we ensure that critical levels have distinct, high-visibility colors
            
            if (level == "Error") {
                // Error should use high-contrast red
                REQUIRE(text_color == Color::Red);
                REQUIRE(bg_color == Color::RedLight);
            } else if (level == "Warning") {
                // Warning should use high-contrast yellow
                REQUIRE(text_color == Color::Yellow);
                REQUIRE(bg_color == theme_manager->GetBackgroundColor()); // No special background for warnings
            }
        }
    }
    
    SECTION("Requirement 3.7: Syntax highlighting enhances readability without being overwhelming") {
        // Test that visual hierarchy works well with different message content
        std::vector<std::pair<std::string, std::string>> test_cases = {
            {"Error", "Critical system failure: Unable to allocate memory"},
            {"Warning", "Performance warning: Frame rate dropped below 30 FPS"},
            {"Display", "Player connected: PlayerName123"},
            {"Verbose", "Debug info: Function XYZ executed in 1.2ms"},
        };
        
        for (const auto& test_case : test_cases) {
            const std::string& level = test_case.first;
            const std::string& message = test_case.second;
            
            LogEntry entry(LogEntryType::Structured, 
                         std::string("12:34:56.789"), 
                         123, 
                         "TestLogger", 
                         std::string(level), 
                         message, 
                         "[12:34:56.789][123]TestLogger: " + level + ": " + message, 
                         1);
            
            // Test that message rendering applies appropriate styling
            Element message_element = renderer->RenderMessage(entry, false);
            Element full_entry = renderer->RenderLogEntry(entry, false, 0);
            
            // Render to verify no crashes and basic functionality
            Screen screen(120, 3);
            Render(screen, full_entry);
            std::string output = screen.ToString();
            
            // Should contain the message content
            REQUIRE(output.find(message.substr(0, 20)) != std::string::npos);
            
            // Prominent levels should have visual indicators
            if (theme_manager->IsLogLevelProminent(level)) {
                REQUIRE(output.find("▌") != std::string::npos);
            }
        }
    }
    
    SECTION("Visual hierarchy interaction with selection state") {
        LogEntry error_entry(LogEntryType::Structured, 
                           std::string("12:34:56.789"), 
                           123, 
                           "TestLogger", 
                           std::string("Error"), 
                           "Error message", 
                           "[12:34:56.789][123]TestLogger: Error: Error message", 
                           1);
        
        LogEntry normal_entry(LogEntryType::Structured, 
                            std::string("12:34:56.789"), 
                            123, 
                            "TestLogger", 
                            std::string("Display"), 
                            "Normal message", 
                            "[12:34:56.789][123]TestLogger: Display: Normal message", 
                            1);
        
        // Test that visual hierarchy works with both selected and unselected states
        Element error_selected = renderer->RenderLogEntry(error_entry, true, 0);
        Element error_unselected = renderer->RenderLogEntry(error_entry, false, 0);
        Element normal_selected = renderer->RenderLogEntry(normal_entry, true, 0);
        Element normal_unselected = renderer->RenderLogEntry(normal_entry, false, 0);
        
        // All should render without crashing
        Screen screen(120, 3);
        
        Render(screen, error_selected);
        std::string error_selected_output = screen.ToString();
        
        Render(screen, error_unselected);
        std::string error_unselected_output = screen.ToString();
        
        Render(screen, normal_selected);
        std::string normal_selected_output = screen.ToString();
        
        Render(screen, normal_unselected);
        std::string normal_unselected_output = screen.ToString();
        
        // All entries should have visual indicators for consistent column alignment
        REQUIRE(error_selected_output.find("▌") != std::string::npos);
        REQUIRE(error_unselected_output.find("▌") != std::string::npos);
        REQUIRE(normal_selected_output.find("▌") != std::string::npos);
        REQUIRE(normal_unselected_output.find("▌") != std::string::npos);
    }
}

TEST_CASE("Visual Hierarchy Edge Cases and Robustness", "[ui][visual_hierarchy][edge_cases]") {
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    SECTION("Unknown log levels are handled gracefully") {
        LogEntry unknown_entry(LogEntryType::Structured, 
                             std::string("12:34:56.789"), 
                             123, 
                             "TestLogger", 
                             std::string("UnknownLevel"), 
                             "Message with unknown level", 
                             "[12:34:56.789][123]TestLogger: UnknownLevel: Message with unknown level", 
                             1);
        
        // Unknown levels should not be prominent
        REQUIRE_FALSE(theme_manager->IsLogLevelProminent("UnknownLevel"));
        REQUIRE_FALSE(theme_manager->ShouldLogLevelUseBold("UnknownLevel"));
        
        // Should get default color
        REQUIRE(theme_manager->GetLogLevelColor("UnknownLevel") == Color::White);
        REQUIRE(theme_manager->GetLogLevelBackgroundColor("UnknownLevel") == theme_manager->GetBackgroundColor());
        
        // Should render without crashing
        Element rendered = renderer->RenderLogEntry(unknown_entry, false, 0);
        Screen screen(120, 3);
        Render(screen, rendered);
    }
    
    SECTION("Empty and null log levels are handled") {
        LogEntry empty_level_entry(LogEntryType::Structured, 
                                  std::string("12:34:56.789"), 
                                  123, 
                                  "TestLogger", 
                                  std::string(""), 
                                  "Message with empty level", 
                                  "[12:34:56.789][123]TestLogger: : Message with empty level", 
                                  1);
        
        LogEntry no_level_entry(LogEntryType::SemiStructured, 
                               std::string("12:34:56.789"), 
                               123, 
                               "TestLogger", 
                               std::nullopt, 
                               "Message with no level", 
                               "[12:34:56.789][123]TestLogger: Message with no level", 
                               1);
        
        // Should handle gracefully
        Element empty_rendered = renderer->RenderLogEntry(empty_level_entry, false, 0);
        Element no_level_rendered = renderer->RenderLogEntry(no_level_entry, false, 0);
        
        Screen screen(120, 3);
        Render(screen, empty_rendered);
        Render(screen, no_level_rendered);
    }
    
    SECTION("Case sensitivity in log level handling") {
        std::vector<std::string> case_variants = {"error", "ERROR", "Error", "eRrOr"};
        
        for (const auto& variant : case_variants) {
            LogEntry entry(LogEntryType::Structured, 
                         std::string("12:34:56.789"), 
                         123, 
                         "TestLogger", 
                         std::string(variant), 
                         "Test message", 
                         "[12:34:56.789][123]TestLogger: " + variant + ": Test message", 
                         1);
            
            // Only exact "Error" should be treated as error level
            if (variant == "Error") {
                REQUIRE(theme_manager->IsLogLevelProminent(variant));
                REQUIRE(theme_manager->GetLogLevelColor(variant) == Color::Red);
            } else {
                REQUIRE_FALSE(theme_manager->IsLogLevelProminent(variant));
                REQUIRE(theme_manager->GetLogLevelColor(variant) == Color::White);
            }
            
            // Should render without crashing
            Element rendered = renderer->RenderLogEntry(entry, false, 0);
            Screen screen(120, 3);
            Render(screen, rendered);
        }
    }
}