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
    }
    
    // Timestamp column
    row_elements.push_back(RenderTimestamp(entry));
    
    // Frame number column
    row_elements.push_back(RenderFrameNumber(entry));
    
    // Logger badge column
    row_elements.push_back(RenderLoggerBadge(entry));
    
    // Log level column
    row_elements.push_back(RenderLogLevel(entry));
    
    // Message column (flexible width)
    row_elements.push_back(RenderMessage(entry, is_selected) | flex);
    
    // Create the row with proper spacing
    Element row = hbox(row_elements);
    
    // Apply selection highlighting
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
    }
    
    // Column headers
    header_elements.push_back(
        text(PadText("Timestamp", spacing.timestamp_width)) | bold
    );
    header_elements.push_back(
        text(PadText("Frame", spacing.frame_width)) | bold
    );
    header_elements.push_back(
        text(PadText("Logger", spacing.logger_badge_width)) | bold
    );
    header_elements.push_back(
        text(PadText("Level", spacing.level_width)) | bold
    );
    header_elements.push_back(
        text("Message") | bold | flex
    );
    
    return hbox(header_elements) | color(theme_manager_->GetHighlightColor());
}

Element LogEntryRenderer::RenderLineNumber(int relative_number, bool is_current) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    std::string line_text;
    if (relative_number == 0 && is_current) {
        line_text = "  0  ";  // Current line indicator
    } else if (relative_number > 0) {
        line_text = " " + std::to_string(relative_number) + " ";  // No + sign needed for vim users
    } else if (relative_number < 0) {
        line_text = " " + std::to_string(std::abs(relative_number)) + " ";  // Show absolute value, no - sign needed for vim users
    } else {
        line_text = "     ";  // Empty for non-relative mode
    }
    
    Element element = text(PadText(line_text, spacing.line_number_width));
    
    if (is_current) {
        element = element | bold | color(theme_manager_->GetHighlightColor());
    }
    
    return element;
}

Element LogEntryRenderer::RenderTimestamp(const LogEntry& entry) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    std::string timestamp_str = entry.Get_timestamp().has_value() ? 
                               entry.Get_timestamp().value() : "N/A";
    
    return text(PadText(timestamp_str, spacing.timestamp_width));
}

Element LogEntryRenderer::RenderFrameNumber(const LogEntry& entry) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    std::string frame_str = entry.Get_frame_number().has_value() ? 
                           std::to_string(entry.Get_frame_number().value()) : "N/A";
    
    return text(PadText(frame_str, spacing.frame_width));
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
    
    // Apply color and make errors/warnings more prominent
    if (level == "Error") {
        return element | color(level_color) | bold;
    } else if (level == "Warning") {
        return element | color(level_color);
    } else {
        return element | color(level_color);
    }
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

} // namespace ue_log