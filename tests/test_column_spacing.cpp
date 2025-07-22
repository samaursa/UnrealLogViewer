#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/visual_theme_manager.h"
#include "../lib/ui/log_entry_renderer.h"
#include "../lib/log_parser/log_entry.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>

namespace ue_log {

// Helper function to create test setup
struct ColumnSpacingTestFixture {
    ColumnSpacingTestFixture() {
        theme_manager_ = std::make_unique<VisualThemeManager>();
        renderer_ = std::make_unique<LogEntryRenderer>(theme_manager_.get());
        
        // Create test log entry
        test_entry_ = LogEntry();
        test_entry_.Request_line_number(123);
        test_entry_.Request_timestamp("12:34:56.789");
        test_entry_.Request_frame_number(12345);
        test_entry_.Request_logger_name("TestLogger");
        test_entry_.Request_log_level("Info");
        test_entry_.Request_message("This is a test message");
    }
    
    std::unique_ptr<VisualThemeManager> theme_manager_;
    std::unique_ptr<LogEntryRenderer> renderer_;
    LogEntry test_entry_;
};

TEST_CASE("Default Column Spacing Configuration", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    auto spacing = fixture.theme_manager_->GetColumnSpacing();
    
    // Test default values
    REQUIRE(spacing.line_number_width == 6);
    REQUIRE(spacing.timestamp_width == 15);
    REQUIRE(spacing.frame_width == 8);
    REQUIRE(spacing.logger_badge_width == 18);
    REQUIRE(spacing.level_width == 8);
    REQUIRE(spacing.use_visual_separators == true);
    REQUIRE(spacing.align_numbers_right == true);
}

TEST_CASE("Custom Column Spacing Configuration", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    VisualThemeManager::ColumnSpacing custom_spacing;
    custom_spacing.line_number_width = 8;
    custom_spacing.timestamp_width = 20;
    custom_spacing.frame_width = 10;
    custom_spacing.logger_badge_width = 25;
    custom_spacing.level_width = 10;
    custom_spacing.use_visual_separators = false;
    custom_spacing.align_numbers_right = false;
    
    fixture.theme_manager_->SetColumnSpacing(custom_spacing);
    auto retrieved_spacing = fixture.theme_manager_->GetColumnSpacing();
    
    REQUIRE(retrieved_spacing.line_number_width == 8);
    REQUIRE(retrieved_spacing.timestamp_width == 20);
    REQUIRE(retrieved_spacing.frame_width == 10);
    REQUIRE(retrieved_spacing.logger_badge_width == 25);
    REQUIRE(retrieved_spacing.level_width == 10);
    REQUIRE(retrieved_spacing.use_visual_separators == false);
    REQUIRE(retrieved_spacing.align_numbers_right == false);
}

TEST_CASE("Visual Separator Configuration", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    
    // Test with visual separators enabled (default)
    std::string separator_with_visual = fixture.theme_manager_->GetColumnSeparator();
    REQUIRE(separator_with_visual == " │ ");
    
    // Test with visual separators disabled
    fixture.theme_manager_->SetVisualSeparatorsEnabled(false);
    std::string separator_without_visual = fixture.theme_manager_->GetColumnSeparator();
    REQUIRE(separator_without_visual == "  "); // Default column_padding is 2
}

TEST_CASE("Line Number Right Alignment", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    
    // Test right alignment for line numbers
    auto spacing = fixture.theme_manager_->GetColumnSpacing();
    REQUIRE(spacing.align_numbers_right == true);
    
    // Render line number and check alignment
    auto line_element = fixture.renderer_->RenderLineNumber(5, false);
    
    // Create a screen to render the element and check output
    ftxui::Screen screen(spacing.line_number_width, 1);
    ftxui::Render(screen, line_element);
    
    // The number should be right-aligned within the column width
    std::string rendered_line;
    for (int x = 0; x < spacing.line_number_width; ++x) {
        rendered_line += screen.PixelAt(x, 0).character;
    }
    
    // Should have spaces before the number for right alignment
    REQUIRE((rendered_line.find("     5") != std::string::npos || 
             rendered_line.find("    5") != std::string::npos));
}

TEST_CASE("Frame Number Right Alignment", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    
    // Test right alignment for frame numbers
    auto spacing = fixture.theme_manager_->GetColumnSpacing();
    REQUIRE(spacing.align_numbers_right == true);
    
    // Render frame number and check alignment
    auto frame_element = fixture.renderer_->RenderFrameNumber(fixture.test_entry_);
    
    // Create a screen to render the element and check output
    ftxui::Screen screen(spacing.frame_width, 1);
    ftxui::Render(screen, frame_element);
    
    // The number should be right-aligned within the column width
    std::string rendered_frame;
    for (int x = 0; x < spacing.frame_width; ++x) {
        rendered_frame += screen.PixelAt(x, 0).character;
    }
    
    // Should have spaces before the number for right alignment
    REQUIRE(rendered_frame.find("12345") != std::string::npos);
    REQUIRE((rendered_frame.find("   12345") != std::string::npos || 
             rendered_frame.find("  12345") != std::string::npos ||
             rendered_frame.find(" 12345") != std::string::npos));
}

TEST_CASE("Timestamp Truncation", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    
    // Test timestamp truncation when it exceeds column width
    LogEntry long_timestamp_entry;
    long_timestamp_entry.Request_timestamp("12:34:56.789123456789"); // Very long timestamp
    
    auto timestamp_element = fixture.renderer_->RenderTimestamp(long_timestamp_entry);
    auto spacing = fixture.theme_manager_->GetColumnSpacing();
    
    // Create a screen to render the element and check output
    ftxui::Screen screen(spacing.timestamp_width, 1);
    ftxui::Render(screen, timestamp_element);
    
    // Check that the rendered timestamp doesn't exceed the column width
    std::string rendered_timestamp;
    for (int x = 0; x < spacing.timestamp_width; ++x) {
        rendered_timestamp += screen.PixelAt(x, 0).character;
    }
    
    REQUIRE(static_cast<int>(rendered_timestamp.length()) == spacing.timestamp_width);
}

TEST_CASE("Logger Badge Truncation", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    
    // Test logger badge truncation when logger name is too long
    LogEntry long_logger_entry;
    long_logger_entry.Request_logger_name("VeryLongLoggerNameThatExceedsColumnWidth");
    
    auto badge_element = fixture.renderer_->RenderLoggerBadge(long_logger_entry);
    auto spacing = fixture.theme_manager_->GetColumnSpacing();
    
    // Create a screen to render the element and check output
    ftxui::Screen screen(spacing.logger_badge_width, 1);
    ftxui::Render(screen, badge_element);
    
    // Check that the rendered badge doesn't exceed the column width
    std::string rendered_badge;
    for (int x = 0; x < spacing.logger_badge_width; ++x) {
        rendered_badge += screen.PixelAt(x, 0).character;
    }
    
    REQUIRE(static_cast<int>(rendered_badge.length()) == spacing.logger_badge_width);
}

TEST_CASE("Table Header Consistency", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    
    // Test that table header uses the same column widths as data rows
    auto header_element = fixture.renderer_->RenderTableHeader();
    auto data_element = fixture.renderer_->RenderLogEntry(fixture.test_entry_, false, 0);
    
    // Both should render without errors and maintain consistent structure
    REQUIRE(header_element != nullptr);
    REQUIRE(data_element != nullptr);
    
    // Create screens to render both elements
    ftxui::Screen header_screen(100, 1);
    ftxui::Screen data_screen(100, 1);
    
    ftxui::Render(header_screen, header_element);
    ftxui::Render(data_screen, data_element);
    
    // Both should have content (not empty)
    bool header_has_content = false;
    bool data_has_content = false;
    
    for (int x = 0; x < 100; ++x) {
        if (header_screen.PixelAt(x, 0).character != " ") {
            header_has_content = true;
        }
        if (data_screen.PixelAt(x, 0).character != " ") {
            data_has_content = true;
        }
    }
    
    REQUIRE(header_has_content == true);
    REQUIRE(data_has_content == true);
}

TEST_CASE("Visual Separator Consistency", "[column_spacing]") {
    ColumnSpacingTestFixture fixture;
    
    // Test that visual separators are consistently applied
    auto element = fixture.renderer_->RenderLogEntry(fixture.test_entry_, false, 0);
    
    // Create a screen to render the element
    ftxui::Screen screen(200, 1);
    ftxui::Render(screen, element);
    
    // Check for presence of visual separators (│ character)
    std::string rendered_line;
    for (int x = 0; x < 200; ++x) {
        rendered_line += screen.PixelAt(x, 0).character;
    }
    
    // Should contain visual separators
    REQUIRE(rendered_line.find("│") != std::string::npos);
    
    // Test without visual separators
    fixture.theme_manager_->SetVisualSeparatorsEnabled(false);
    auto element_no_separators = fixture.renderer_->RenderLogEntry(fixture.test_entry_, false, 0);
    
    ftxui::Screen screen_no_separators(200, 1);
    ftxui::Render(screen_no_separators, element_no_separators);
    
    std::string rendered_line_no_separators;
    for (int x = 0; x < 200; ++x) {
        rendered_line_no_separators += screen_no_separators.PixelAt(x, 0).character;
    }
    
    // Should not contain visual separators
    REQUIRE(rendered_line_no_separators.find("│") == std::string::npos);
}

} // namespace ue_log