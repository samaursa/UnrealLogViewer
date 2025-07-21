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