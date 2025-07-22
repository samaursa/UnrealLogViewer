#include "log_entry_renderer.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <algorithm>
#include <stdexcept>

namespace ue_log {

using namespace ftxui;

LogEntryRenderer::LogEntryRenderer(VisualThemeManager* theme_manager)
    : theme_manager_(theme_manager) {
    if (!theme_manager_) {
        throw std::invalid_argument("VisualThemeManager cannot be null");
    }
}

Element LogEntryRenderer::RenderLogEntry(const LogEntry& entry, bool is_selected, 
                                        int relative_line_number) const {
    std::vector<Element> row_elements;
    
    // Get column spacing configuration
    auto spacing = theme_manager_->GetColumnSpacing();
    
    // Line number column (if enabled)
    if (show_line_numbers_) {
        row_elements.push_back(RenderLineNumber(relative_line_number, is_selected));
        // Add visual separator after line number
        std::string separator = theme_manager_->GetColumnSeparator();
        row_elements.push_back(text(separator) | color(Color::GrayDark));
    }
    
    // Timestamp column
    row_elements.push_back(RenderTimestamp(entry));
    // Add visual separator after timestamp
    std::string separator = theme_manager_->GetColumnSeparator();
    row_elements.push_back(text(separator) | color(Color::GrayDark));
    
    // Frame number column
    row_elements.push_back(RenderFrameNumber(entry));
    // Add visual separator after frame
    row_elements.push_back(text(separator) | color(Color::GrayDark));
    
    // Logger badge column
    row_elements.push_back(RenderLoggerBadge(entry));
    // Add visual separator after logger
    row_elements.push_back(text(separator) | color(Color::GrayDark));
    
    // Log level column
    row_elements.push_back(RenderLogLevel(entry));
    // Add visual separator after log level
    row_elements.push_back(text(separator) | color(Color::GrayDark));
    
    // Message column (flexible width)
    row_elements.push_back(RenderMessage(entry, is_selected) | flex);
    
    // Create the row with proper spacing
    Element row = hbox(row_elements);
    
    // Apply row-level visual hierarchy - always apply to maintain column alignment
    std::string level = entry.Get_log_level().has_value() ? entry.Get_log_level().value() : "";
    row = ApplyRowLevelHierarchy(row, level, is_selected);
    
    // Apply selection highlighting (after hierarchy styling to ensure it takes precedence)
    if (is_selected) {
        row = row | inverted;
    }
    
    return row;
}

Element LogEntryRenderer::RenderTableHeader() const {
    std::vector<Element> header_elements;
    auto spacing = theme_manager_->GetColumnSpacing();
    
    // Line number header (if enabled)
    if (show_line_numbers_) {
        header_elements.push_back(
            text(PadText("Line", spacing.line_number_width)) | bold
        );
        // Add visual separator after line number header
        std::string separator = theme_manager_->GetColumnSeparator();
        header_elements.push_back(text(separator) | color(Color::GrayDark));
    }
    
    // Column headers with separators
    std::string separator = theme_manager_->GetColumnSeparator();
    
    header_elements.push_back(
        text(PadText("Timestamp", spacing.timestamp_width)) | bold
    );
    header_elements.push_back(text(separator) | color(Color::GrayDark));
    
    header_elements.push_back(
        text(PadText("Frame", spacing.frame_width)) | bold
    );
    header_elements.push_back(text(separator) | color(Color::GrayDark));
    
    header_elements.push_back(
        text(PadText("Logger", spacing.logger_badge_width)) | bold
    );
    header_elements.push_back(text(separator) | color(Color::GrayDark));
    
    header_elements.push_back(
        text(PadText("Level", spacing.level_width)) | bold
    );
    header_elements.push_back(text(separator) | color(Color::GrayDark));
    
    header_elements.push_back(
        text("Message") | bold | flex
    );
    
    return hbox(header_elements) | color(theme_manager_->GetHighlightColor());
}

Element LogEntryRenderer::RenderLineNumber(int relative_number, bool is_current) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    std::string line_text;
    if (relative_number == 0 && is_current) {
        line_text = "0";  // Current line indicator
    } else if (relative_number > 0) {
        line_text = std::to_string(relative_number);  // No + sign needed for vim users
    } else if (relative_number < 0) {
        line_text = std::to_string(std::abs(relative_number));  // Show absolute value, no - sign needed for vim users
    } else {
        line_text = "";  // Empty for non-relative mode
    }
    
    // Apply right alignment for numbers if configured
    Element element;
    if (spacing.align_numbers_right && !line_text.empty()) {
        // Right-align the number within the column width
        int padding_needed = spacing.line_number_width - static_cast<int>(line_text.length());
        if (padding_needed > 0) {
            line_text = std::string(padding_needed, ' ') + line_text;
        }
        element = text(line_text.substr(0, spacing.line_number_width));
    } else {
        element = text(PadText(line_text, spacing.line_number_width));
    }
    
    if (is_current) {
        element = element | bold | color(theme_manager_->GetHighlightColor());
    }
    
    return element;
}

Element LogEntryRenderer::RenderTimestamp(const LogEntry& entry) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    std::string timestamp_str = entry.Get_timestamp().has_value() ? 
                               entry.Get_timestamp().value() : "N/A";
    
    // Ensure timestamp fits within the allocated width
    if (static_cast<int>(timestamp_str.length()) > spacing.timestamp_width) {
        timestamp_str = TruncateText(timestamp_str, spacing.timestamp_width);
    }
    
    return text(PadText(timestamp_str, spacing.timestamp_width));
}

Element LogEntryRenderer::RenderFrameNumber(const LogEntry& entry) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    std::string frame_str = entry.Get_frame_number().has_value() ? 
                           std::to_string(entry.Get_frame_number().value()) : "N/A";
    
    // Apply right alignment for numbers if configured
    if (spacing.align_numbers_right && entry.Get_frame_number().has_value()) {
        // Right-align the number within the column width
        int padding_needed = spacing.frame_width - static_cast<int>(frame_str.length());
        if (padding_needed > 0) {
            frame_str = std::string(padding_needed, ' ') + frame_str;
        }
        return text(frame_str.substr(0, spacing.frame_width));
    } else {
        return text(PadText(frame_str, spacing.frame_width));
    }
}

Element LogEntryRenderer::RenderLoggerBadge(const LogEntry& entry) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    std::string logger_name = entry.Get_logger_name();
    
    // Get consistent color for this logger
    Color logger_color = theme_manager_->GetLoggerColor(logger_name);
    
    // Create badge with truncated logger name if necessary
    std::string badge_text = TruncateText(logger_name, spacing.logger_badge_width - 2);
    
    return CreateBadgeElement(badge_text, logger_color);
}

Element LogEntryRenderer::RenderLogLevel(const LogEntry& entry) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    std::string level_str = entry.Get_log_level().has_value() ? 
                           entry.Get_log_level().value() : "N/A";
    
    Element level_element = text(PadText(level_str, spacing.level_width));
    
    // Apply log level styling
    if (entry.Get_log_level().has_value()) {
        level_element = ApplyLogLevelStyling(level_element, entry.Get_log_level().value());
    }
    
    return level_element;
}

Element LogEntryRenderer::RenderMessage(const LogEntry& entry, bool is_selected) const {
    Element message_element;
    
    if (word_wrap_enabled_) {
        message_element = paragraph(entry.Get_message());
    } else {
        message_element = text(entry.Get_message());
    }
    
    // Apply visual hierarchy to message content for prominent log levels
    if (entry.Get_log_level().has_value()) {
        const std::string& level = entry.Get_log_level().value();
        if (theme_manager_->IsLogLevelProminent(level)) {
            Color level_color = theme_manager_->GetLogLevelColor(level);
            message_element = message_element | color(level_color);
            
            // Make error messages bold for better readability
            if (level == "Error") {
                message_element = message_element | bold;
            }
        }
    }
    
    return message_element;
}

Element LogEntryRenderer::CreateBadgeElement(const std::string& text, Color color) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    // Create padded text that fits within the badge width
    std::string padded_text = PadText(text, spacing.logger_badge_width);
    
    // Create badge with background color and contrasting text
    return ftxui::text(padded_text) | bgcolor(color) | ftxui::color(Color::Black);
}

Element LogEntryRenderer::ApplyLogLevelStyling(Element element, const std::string& level) const {
    Color level_color = theme_manager_->GetLogLevelColor(level);
    Color bg_color = theme_manager_->GetLogLevelBackgroundColor(level);
    
    // Start with the base element and color
    Element styled_element = element | color(level_color);
    
    // Apply bold styling for prominent levels
    if (theme_manager_->ShouldLogLevelUseBold(level)) {
        styled_element = styled_element | bold;
    }
    
    // Apply background color for critical levels (like errors)
    if (bg_color != theme_manager_->GetBackgroundColor()) {
        styled_element = styled_element | bgcolor(bg_color);
    }
    
    // Add additional visual emphasis for errors
    if (level == "Error") {
        // Make errors even more prominent with inverted colors for maximum visibility
        styled_element = styled_element | inverted;
    }
    
    return styled_element;
}

std::string LogEntryRenderer::TruncateText(const std::string& text, int max_width) const {
    if (max_width <= 0) {
        return "";
    }
    
    if (static_cast<int>(text.length()) <= max_width) {
        return text;
    }
    
    // Truncate and add ellipsis if there's room
    if (max_width >= 3) {
        return text.substr(0, max_width - 3) + "...";
    } else {
        return text.substr(0, max_width);
    }
}

std::string LogEntryRenderer::PadText(const std::string& text, int width) const {
    if (width <= 0) {
        return text;
    }
    
    if (static_cast<int>(text.length()) >= width) {
        return text.substr(0, width);
    }
    
    return text + std::string(width - text.length(), ' ');
}

Element LogEntryRenderer::ApplyRowLevelHierarchy(Element element, const std::string& level, bool is_selected) const {
    // Get the appropriate indicator color for this log level
    Color indicator_color;
    
    if (level == "Error") {
        indicator_color = Color::RedLight;
        // Errors get a subtle red background tint for the entire row (unless selected)
        if (!is_selected) {
            element = element | bgcolor(Color::RGB(40, 20, 20)); // Dark red tint
        }
    } else if (level == "Warning") {
        indicator_color = Color::YellowLight;
        // Warnings get a subtle yellow background tint for the entire row (unless selected)
        if (!is_selected) {
            element = element | bgcolor(Color::RGB(40, 40, 20)); // Dark yellow tint
        }
    } else if (level.empty()) {
        // For entries without log levels, use a very subtle gray indicator
        indicator_color = Color::GrayDark;
    } else {
        // For normal entries, use a subtle indicator that matches the log level color
        indicator_color = theme_manager_->GetLogLevelColor(level);
        // Make the indicator more subtle for normal entries by using a darker version
        if (indicator_color == Color::White) {
            indicator_color = Color::GrayDark;
        } else if (indicator_color == Color::GrayLight) {
            indicator_color = Color::GrayDark;
        }
        // For other colors, use them as-is but they'll be more subtle than Error/Warning
    }
    
    // Add the left border indicator for all entries to maintain column alignment
    element = hbox({
        text("▌") | color(indicator_color),
        element
    });
    
    return element;
}

} // namespace ue_log