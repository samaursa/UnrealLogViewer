#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/visual_theme_manager.h"
#include <ftxui/screen/color.hpp>
#include <set>

using namespace ue_log;

TEST_CASE("VisualThemeManager Basic Functionality", "[ui][visual_theme_manager]") {
    
    SECTION("Default constructor initializes correctly") {
        VisualThemeManager theme_manager;
        
        // Check that default colors are available (may be different in eye strain mode)
        auto bg_color = theme_manager.GetBackgroundColor();
        auto text_color = theme_manager.GetTextColor();
        auto highlight_color = theme_manager.GetHighlightColor();
        
        // Colors should be valid (not null/default)
        REQUIRE(bg_color != ftxui::Color::Default);
        REQUIRE(text_color != ftxui::Color::Default);
        REQUIRE(highlight_color != ftxui::Color::Default);
        
        // Check that color palette is initialized
        REQUIRE(theme_manager.GetAvailableLoggerColorCount() > 0);
        
        // Check default column spacing
        auto spacing = theme_manager.GetColumnSpacing();
        REQUIRE(spacing.line_number_width == 6);
        REQUIRE(spacing.timestamp_width == 15);  // Updated to match current implementation
        REQUIRE(spacing.frame_width == 8);
        REQUIRE(spacing.logger_badge_width == 18);  // Updated to match current implementation
        REQUIRE(spacing.level_width == 8);
        REQUIRE(spacing.column_padding == 2);
    }
}

TEST_CASE("VisualThemeManager Logger Color Assignment", "[ui][visual_theme_manager]") {
    VisualThemeManager theme_manager;
    
    SECTION("Logger colors are assigned consistently") {
        std::string logger1 = "LogTemp";
        std::string logger2 = "LogCore";
        
        // Get colors for the same logger multiple times
        ftxui::Color color1_first = theme_manager.GetLoggerColor(logger1);
        ftxui::Color color1_second = theme_manager.GetLoggerColor(logger1);
        ftxui::Color color2_first = theme_manager.GetLoggerColor(logger2);
        
        // Same logger should always get the same color
        REQUIRE(color1_first == color1_second);
        
        // Different loggers should get different colors (at least initially)
        REQUIRE(color1_first != color2_first);
    }
    
    SECTION("Logger color cycling works correctly") {
        VisualThemeManager theme_manager;
        size_t available_colors = theme_manager.GetAvailableLoggerColorCount();
        
        std::vector<ftxui::Color> assigned_colors;
        
        // Assign colors to multiple loggers
        for (size_t i = 0; i < available_colors; ++i) {
            std::string logger_name = "Logger" + std::to_string(i);
            ftxui::Color color = theme_manager.GetLoggerColor(logger_name);
            assigned_colors.push_back(color);
        }
        
        // Test cycling - the next logger should get the first color again
        std::string next_logger = "Logger" + std::to_string(available_colors);
        ftxui::Color cycled_color = theme_manager.GetLoggerColor(next_logger);
        REQUIRE(cycled_color == assigned_colors[0]);
        
        // Test that we get different colors for different loggers (at least the first few)
        if (available_colors > 1) {
            REQUIRE(assigned_colors[0] != assigned_colors[1]);
        }
        if (available_colors > 2) {
            REQUIRE(assigned_colors[1] != assigned_colors[2]);
        }
    }
    
    SECTION("Logger color reset works correctly") {
        VisualThemeManager theme_manager;
        
        // Assign colors to some loggers
        ftxui::Color color1 = theme_manager.GetLoggerColor("Logger1");
        ftxui::Color color2 = theme_manager.GetLoggerColor("Logger2");
        
        // Reset colors
        theme_manager.ResetLoggerColors();
        
        // After reset, the same logger should get the first color again
        ftxui::Color color1_after_reset = theme_manager.GetLoggerColor("Logger1");
        ftxui::Color color3_after_reset = theme_manager.GetLoggerColor("Logger3");
        
        // The first logger after reset should get the first color from the palette
        REQUIRE(color1_after_reset == color1);
        
        // The second logger after reset should get the second color from the palette
        REQUIRE(color3_after_reset == color2);
    }
    
    SECTION("Empty logger name handling") {
        VisualThemeManager theme_manager;
        
        // Empty logger name should still get a color
        ftxui::Color empty_color = theme_manager.GetLoggerColor("");
        
        // Should be consistent
        ftxui::Color empty_color2 = theme_manager.GetLoggerColor("");
        REQUIRE(empty_color == empty_color2);
        
        // Should be different from a named logger
        ftxui::Color named_color = theme_manager.GetLoggerColor("LogTemp");
        REQUIRE(empty_color != named_color);
    }
}

TEST_CASE("VisualThemeManager Log Level Colors", "[ui][visual_theme_manager]") {
    VisualThemeManager theme_manager;
    
    SECTION("Standard log levels have appropriate colors") {
        // Colors may be different in eye strain reduction mode, but should be distinct
        auto error_color = theme_manager.GetLogLevelColor("Error");
        auto warning_color = theme_manager.GetLogLevelColor("Warning");
        auto info_color = theme_manager.GetLogLevelColor("Info");
        auto debug_color = theme_manager.GetLogLevelColor("Debug");
        
        // All colors should be valid
        REQUIRE(error_color != ftxui::Color::Default);
        REQUIRE(warning_color != ftxui::Color::Default);
        REQUIRE(info_color != ftxui::Color::Default);
        REQUIRE(debug_color != ftxui::Color::Default);
        
        // Error and warning should be distinct from normal colors
        REQUIRE(error_color != info_color);
        REQUIRE(warning_color != info_color);
    }
    
    SECTION("Unreal Engine specific log levels have appropriate colors") {
        // Colors may be different in eye strain reduction mode, but should be distinct
        auto display_color = theme_manager.GetLogLevelColor("Display");
        auto verbose_color = theme_manager.GetLogLevelColor("Verbose");
        auto very_verbose_color = theme_manager.GetLogLevelColor("VeryVerbose");
        auto trace_color = theme_manager.GetLogLevelColor("Trace");
        
        // All colors should be valid
        REQUIRE(display_color != ftxui::Color::Default);
        REQUIRE(verbose_color != ftxui::Color::Default);
        REQUIRE(very_verbose_color != ftxui::Color::Default);
        REQUIRE(trace_color != ftxui::Color::Default);
        
        // Colors should be distinct where appropriate
        REQUIRE(verbose_color != very_verbose_color);
    }
    
    SECTION("Unknown log levels get default color") {
        // Unknown levels should get the default text color (may be off-white in eye strain mode)
        auto unknown_color = theme_manager.GetLogLevelColor("Unknown");
        auto empty_color = theme_manager.GetLogLevelColor("");
        auto custom_color = theme_manager.GetLogLevelColor("CustomLevel");
        
        // All should get the same default color
        REQUIRE(unknown_color == empty_color);
        REQUIRE(empty_color == custom_color);
        
        // Should be valid colors
        REQUIRE(unknown_color != ftxui::Color::Default);
    }
    
    SECTION("Case sensitivity in log levels") {
        // Test case variations - only exact "Error" should get error color
        auto error_color = theme_manager.GetLogLevelColor("Error");
        auto lowercase_error_color = theme_manager.GetLogLevelColor("error");
        auto default_color = theme_manager.GetLogLevelColor("Unknown");
        
        // Only exact "Error" should get special treatment
        REQUIRE(error_color != lowercase_error_color);
        REQUIRE(lowercase_error_color == default_color); // Should get default color
        
        auto uppercase_error_color = theme_manager.GetLogLevelColor("ERROR");
        REQUIRE(uppercase_error_color == default_color); // Should be default, not Error
        
        // Exact "Error" should be different from default
        REQUIRE(error_color != default_color);
    }
}

TEST_CASE("VisualThemeManager Enhanced Visual Hierarchy", "[ui][visual_theme_manager]") {
    VisualThemeManager theme_manager;
    
    SECTION("Log level background colors") {
        // Errors should have special background color
        REQUIRE(theme_manager.GetLogLevelBackgroundColor("Error") == ftxui::Color::RedLight);
        
        // Other levels should use default background
        REQUIRE(theme_manager.GetLogLevelBackgroundColor("Warning") == theme_manager.GetBackgroundColor());
        REQUIRE(theme_manager.GetLogLevelBackgroundColor("Info") == theme_manager.GetBackgroundColor());
        REQUIRE(theme_manager.GetLogLevelBackgroundColor("Debug") == theme_manager.GetBackgroundColor());
        REQUIRE(theme_manager.GetLogLevelBackgroundColor("Display") == theme_manager.GetBackgroundColor());
    }
    
    SECTION("Prominent log level identification") {
        // Error and Warning should be prominent
        REQUIRE(theme_manager.IsLogLevelProminent("Error"));
        REQUIRE(theme_manager.IsLogLevelProminent("Warning"));
        
        // Other levels should not be prominent
        REQUIRE_FALSE(theme_manager.IsLogLevelProminent("Info"));
        REQUIRE_FALSE(theme_manager.IsLogLevelProminent("Debug"));
        REQUIRE_FALSE(theme_manager.IsLogLevelProminent("Display"));
        REQUIRE_FALSE(theme_manager.IsLogLevelProminent("Verbose"));
        REQUIRE_FALSE(theme_manager.IsLogLevelProminent("Unknown"));
    }
    
    SECTION("Bold text usage for log levels") {
        // Error and Warning should use bold text
        REQUIRE(theme_manager.ShouldLogLevelUseBold("Error"));
        REQUIRE(theme_manager.ShouldLogLevelUseBold("Warning"));
        
        // Other levels should not use bold text
        REQUIRE_FALSE(theme_manager.ShouldLogLevelUseBold("Info"));
        REQUIRE_FALSE(theme_manager.ShouldLogLevelUseBold("Debug"));
        REQUIRE_FALSE(theme_manager.ShouldLogLevelUseBold("Display"));
        REQUIRE_FALSE(theme_manager.ShouldLogLevelUseBold("Verbose"));
        REQUIRE_FALSE(theme_manager.ShouldLogLevelUseBold("Unknown"));
    }
    
    SECTION("Visual hierarchy consistency") {
        // All prominent levels should also use bold
        std::vector<std::string> all_levels = {"Error", "Warning", "Info", "Debug", "Display", "Verbose", "VeryVerbose", "Trace"};
        
        for (const auto& level : all_levels) {
            if (theme_manager.IsLogLevelProminent(level)) {
                REQUIRE(theme_manager.ShouldLogLevelUseBold(level));
            }
        }
    }
}

TEST_CASE("VisualThemeManager Column Spacing Configuration", "[ui][visual_theme_manager]") {
    VisualThemeManager theme_manager;
    
    SECTION("Column spacing can be modified") {
        VisualThemeManager::ColumnSpacing new_spacing;
        new_spacing.line_number_width = 10;
        new_spacing.timestamp_width = 15;
        new_spacing.frame_width = 12;
        new_spacing.logger_badge_width = 25;
        new_spacing.level_width = 10;
        new_spacing.column_padding = 3;
        
        theme_manager.SetColumnSpacing(new_spacing);
        
        auto retrieved_spacing = theme_manager.GetColumnSpacing();
        REQUIRE(retrieved_spacing.line_number_width == 10);
        REQUIRE(retrieved_spacing.timestamp_width == 15);
        REQUIRE(retrieved_spacing.frame_width == 12);
        REQUIRE(retrieved_spacing.logger_badge_width == 25);
        REQUIRE(retrieved_spacing.level_width == 10);
        REQUIRE(retrieved_spacing.column_padding == 3);
    }
    
    SECTION("Column spacing is independent per instance") {
        VisualThemeManager theme_manager1;
        VisualThemeManager theme_manager2;
        
        VisualThemeManager::ColumnSpacing spacing1;
        spacing1.line_number_width = 8;
        
        VisualThemeManager::ColumnSpacing spacing2;
        spacing2.line_number_width = 12;
        
        theme_manager1.SetColumnSpacing(spacing1);
        theme_manager2.SetColumnSpacing(spacing2);
        
        REQUIRE(theme_manager1.GetColumnSpacing().line_number_width == 8);
        REQUIRE(theme_manager2.GetColumnSpacing().line_number_width == 12);
    }
}

TEST_CASE("VisualThemeManager Color Consistency Across Sessions", "[ui][visual_theme_manager]") {
    
    SECTION("Logger color assignment is deterministic within session") {
        VisualThemeManager theme_manager;
        
        std::vector<std::string> loggers = {"LogTemp", "LogCore", "LogEngine", "LogAI", "LogNetwork"};
        std::vector<ftxui::Color> first_assignment;
        std::vector<ftxui::Color> second_assignment;
        
        // First assignment pass
        for (const auto& logger : loggers) {
            first_assignment.push_back(theme_manager.GetLoggerColor(logger));
        }
        
        // Second assignment pass (should be identical due to caching)
        for (const auto& logger : loggers) {
            second_assignment.push_back(theme_manager.GetLoggerColor(logger));
        }
        
        // All assignments should be identical
        REQUIRE(first_assignment.size() == second_assignment.size());
        for (size_t i = 0; i < first_assignment.size(); ++i) {
            REQUIRE(first_assignment[i] == second_assignment[i]);
        }
    }
    
    SECTION("New theme manager instances start with same color sequence") {
        VisualThemeManager theme_manager1;
        VisualThemeManager theme_manager2;
        
        // Both should assign the same first color to the same logger
        ftxui::Color color1 = theme_manager1.GetLoggerColor("TestLogger");
        ftxui::Color color2 = theme_manager2.GetLoggerColor("TestLogger");
        
        REQUIRE(color1 == color2);
    }
}

TEST_CASE("VisualThemeManager Edge Cases", "[ui][visual_theme_manager]") {
    
    SECTION("Many loggers beyond palette size") {
        VisualThemeManager theme_manager;
        size_t palette_size = theme_manager.GetAvailableLoggerColorCount();
        
        std::vector<ftxui::Color> colors;
        
        // Assign colors to twice the palette size
        for (size_t i = 0; i < palette_size * 2; ++i) {
            std::string logger_name = "Logger" + std::to_string(i);
            colors.push_back(theme_manager.GetLoggerColor(logger_name));
        }
        
        // Colors should cycle - first and (palette_size + 1)th should be the same
        REQUIRE(colors[0] == colors[palette_size]);
        REQUIRE(colors[1] == colors[palette_size + 1]);
    }
    
    SECTION("Very long logger names") {
        VisualThemeManager theme_manager;
        
        std::string long_logger(1000, 'A'); // 1000 character logger name
        ftxui::Color color1 = theme_manager.GetLoggerColor(long_logger);
        ftxui::Color color2 = theme_manager.GetLoggerColor(long_logger);
        
        // Should still work consistently
        REQUIRE(color1 == color2);
    }
    
    SECTION("Special characters in logger names") {
        VisualThemeManager theme_manager;
        
        std::vector<std::string> special_loggers = {
            "Log::Temp",
            "Log-Core",
            "Log_Engine",
            "Log@Network",
            "Log#AI",
            "Log$System"
        };
        
        for (const auto& logger : special_loggers) {
            ftxui::Color color = theme_manager.GetLoggerColor(logger);
            
            // Should be consistent
            ftxui::Color color2 = theme_manager.GetLoggerColor(logger);
            REQUIRE(color == color2);
        }
        
        // Test that at least the first two loggers get different colors
        if (special_loggers.size() >= 2) {
            ftxui::Color color1 = theme_manager.GetLoggerColor(special_loggers[0]);
            ftxui::Color color2 = theme_manager.GetLoggerColor(special_loggers[1]);
            REQUIRE(color1 != color2);
        }
    }
}