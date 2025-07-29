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
        row_elements.push_back(CreateSeparator());
    }
    
    // Timestamp column
    row_elements.push_back(RenderTimestamp(entry));
    row_elements.push_back(CreateSeparator());
    
    // Frame number column
    row_elements.push_back(RenderFrameNumber(entry));
    row_elements.push_back(CreateSeparator());
    
    // Logger badge column
    row_elements.push_back(RenderLoggerBadge(entry));
    row_elements.push_back(CreateSeparator());
    
    // Log level column
    row_elements.push_back(RenderLogLevel(entry));
    row_elements.push_back(CreateSeparator());
    
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
    
    // Add visual hierarchy indicator space to match log entries
    header_elements.push_back(text(" "));
    
    // Line number header (if enabled)
    if (show_line_numbers_) {
        Element line_header = text(PadText("Line", spacing.line_number_width));
        line_header = ApplyVisualPolish(line_header, "header");
        header_elements.push_back(line_header);
        header_elements.push_back(CreateSeparator());
    }
    
    // Column headers with quick filter numbers and consistent visual polish
    Element timestamp_header = text(PadText("1:Timestamp", spacing.timestamp_width));
    timestamp_header = ApplyVisualPolish(timestamp_header, "header");
    header_elements.push_back(timestamp_header);
    header_elements.push_back(CreateSeparator());
    
    Element frame_header = text(PadText("2:Frame", spacing.frame_width));
    frame_header = ApplyVisualPolish(frame_header, "header");
    header_elements.push_back(frame_header);
    header_elements.push_back(CreateSeparator());
    
    Element logger_header = text(PadText("3:Logger", spacing.logger_badge_width));
    logger_header = ApplyVisualPolish(logger_header, "header");
    header_elements.push_back(logger_header);
    header_elements.push_back(CreateSeparator());
    
    Element level_header = text(PadText("4:Level", spacing.level_width));
    level_header = ApplyVisualPolish(level_header, "header");
    header_elements.push_back(level_header);
    header_elements.push_back(CreateSeparator());
    
    Element message_header = text("5:Message") | flex;
    message_header = ApplyVisualPolish(message_header, "header");
    header_elements.push_back(message_header);
    
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
    
    // Apply visual polish
    std::string element_type = is_current ? "emphasis" : "muted";
    element = ApplyVisualPolish(element, element_type);
    
    if (is_current) {
        element = element | color(theme_manager_->GetHighlightColor());
    } else {
        element = element | color(theme_manager_->GetMutedTextColor());
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
    
    Element element = text(PadText(timestamp_str, spacing.timestamp_width));
    element = ApplyVisualPolish(element, "body");
    
    // Use muted color for timestamps to reduce visual noise
    return element | color(theme_manager_->GetMutedTextColor());
}

Element LogEntryRenderer::RenderFrameNumber(const LogEntry& entry) const {
    auto spacing = theme_manager_->GetColumnSpacing();
    
    std::string frame_str = entry.Get_frame_number().has_value() ? 
                           std::to_string(entry.Get_frame_number().value()) : "N/A";
    
    Element element;
    // Apply right alignment for numbers if configured
    if (spacing.align_numbers_right && entry.Get_frame_number().has_value()) {
        // Right-align the number within the column width
        int padding_needed = spacing.frame_width - static_cast<int>(frame_str.length());
        if (padding_needed > 0) {
            frame_str = std::string(padding_needed, ' ') + frame_str;
        }
        element = text(frame_str.substr(0, spacing.frame_width));
    } else {
        element = text(PadText(frame_str, spacing.frame_width));
    }
    
    element = ApplyVisualPolish(element, "body");
    
    // Use muted color for frame numbers to reduce visual noise
    return element | color(theme_manager_->GetMutedTextColor());
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
    
    // Errors already have color and bold styling - no need for additional emphasis
    
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
        indicator_color = theme_manager_->GetLogLevelColor("Error");
        // Errors get a subtle red background tint for the entire row (unless selected)
        if (!is_selected && theme_manager_->IsEyeStrainReductionEnabled()) {
            element = element | bgcolor(Color::RGB(30, 15, 15)); // Very subtle dark red tint
        } else if (!is_selected) {
            element = element | bgcolor(Color::RGB(40, 20, 20)); // Dark red tint
        }
    } else if (level == "Warning") {
        indicator_color = theme_manager_->GetLogLevelColor("Warning");
        // Warnings get a subtle yellow background tint for the entire row (unless selected)
        if (!is_selected && theme_manager_->IsEyeStrainReductionEnabled()) {
            element = element | bgcolor(Color::RGB(30, 30, 15)); // Very subtle dark yellow tint
        } else if (!is_selected) {
            element = element | bgcolor(Color::RGB(40, 40, 20)); // Dark yellow tint
        }
    } else if (level.empty()) {
        // For entries without log levels, use a very subtle border indicator
        indicator_color = theme_manager_->GetBorderColor();
    } else {
        // For normal entries, use a subtle indicator that matches the log level color
        indicator_color = theme_manager_->GetLogLevelColor(level);
        // Make the indicator more subtle for normal entries by using border color
        if (indicator_color == Color::White || indicator_color == theme_manager_->GetTextColor()) {
            indicator_color = theme_manager_->GetBorderColor();
        }
    }
    
    // Add the left border indicator for all entries to maintain column alignment
    // Use a more subtle indicator character for better visual balance
    element = hbox({
        text("▌") | color(indicator_color),
        element
    });
    
    return element;
}

Element LogEntryRenderer::ApplyRowLevelHierarchyWithSearch(Element element, const std::string& level, 
                                                          bool is_selected, bool is_search_match) const {
    // Get the appropriate indicator color for this log level
    Color indicator_color;
    
    if (is_search_match) {
        // Use a distinct color for search matches - bright cyan to stand out
        indicator_color = Color::Cyan;
    } else if (level == "Error") {
        indicator_color = theme_manager_->GetLogLevelColor("Error");
        // Errors get a subtle red background tint for the entire row (unless selected)
        if (!is_selected && theme_manager_->IsEyeStrainReductionEnabled()) {
            element = element | bgcolor(Color::RGB(30, 15, 15)); // Very subtle dark red tint
        } else if (!is_selected) {
            element = element | bgcolor(Color::RGB(40, 20, 20)); // Dark red tint
        }
    } else if (level == "Warning") {
        indicator_color = theme_manager_->GetLogLevelColor("Warning");
        // Warnings get a subtle yellow background tint for the entire row (unless selected)
        if (!is_selected && theme_manager_->IsEyeStrainReductionEnabled()) {
            element = element | bgcolor(Color::RGB(30, 30, 15)); // Very subtle dark yellow tint
        } else if (!is_selected) {
            element = element | bgcolor(Color::RGB(40, 40, 20)); // Dark yellow tint
        }
    } else if (level.empty()) {
        // For entries without log levels, use a very subtle border indicator
        indicator_color = theme_manager_->GetBorderColor();
    } else {
        // For normal entries, use a subtle indicator that matches the log level color
        indicator_color = theme_manager_->GetLogLevelColor(level);
        // Make the indicator more subtle for normal entries by using border color
        if (indicator_color == Color::White || indicator_color == theme_manager_->GetTextColor()) {
            indicator_color = theme_manager_->GetBorderColor();
        }
    }
    
    // Add the left border indicator for all entries to maintain column alignment
    // Use a more subtle indicator character for better visual balance
    element = hbox({
        text("▌") | color(indicator_color),
        element
    });
    
    return element;
}

Element LogEntryRenderer::ApplyVisualPolish(Element element, const std::string& element_type, 
                                           bool is_interactive, bool is_focused, 
                                           bool is_hovered) const {
    // Apply font weight based on element type
    if (theme_manager_->GetFontWeight(element_type)) {
        element = element | bold;
    }
    
    // Apply interactive states
    if (is_interactive) {
        if (is_focused) {
            element = element | color(theme_manager_->GetFocusColor());
        } else if (is_hovered) {
            element = element | color(theme_manager_->GetHoverColor());
        }
    }
    
    // Apply element-specific styling
    if (element_type == "header") {
        element = element | color(theme_manager_->GetHighlightColor());
    } else if (element_type == "muted") {
        element = element | color(theme_manager_->GetMutedTextColor());
    } else if (element_type == "emphasis") {
        element = element | color(theme_manager_->GetAccentColor());
    } else if (element_type == "error") {
        element = element | color(theme_manager_->GetLogLevelColor("Error"));
        if (theme_manager_->GetFontWeight("error")) {
            element = element | bold;
        }
    } else if (element_type == "warning") {
        element = element | color(theme_manager_->GetLogLevelColor("Warning"));
        if (theme_manager_->GetFontWeight("warning")) {
            element = element | bold;
        }
    }
    
    return element;
}

Element LogEntryRenderer::CreateSeparator() const {
    std::string separator = theme_manager_->GetColumnSeparator();
    return text(separator) | color(theme_manager_->GetBorderColor());
}

Element LogEntryRenderer::RenderLogEntryWithSearchHighlight(const LogEntry& entry, bool is_selected, 
                                                           int relative_line_number,
                                                           const std::string& search_query, 
                                                           bool case_sensitive) const {
    std::vector<Element> row_elements;
    
    // Get column spacing configuration
    auto spacing = theme_manager_->GetColumnSpacing();
    
    // Line number column (if enabled)
    if (show_line_numbers_) {
        row_elements.push_back(RenderLineNumber(relative_line_number, is_selected));
        row_elements.push_back(CreateSeparator());
    }
    
    // Timestamp column
    row_elements.push_back(RenderTimestamp(entry));
    row_elements.push_back(CreateSeparator());
    
    // Frame number column
    row_elements.push_back(RenderFrameNumber(entry));
    row_elements.push_back(CreateSeparator());
    
    // Logger badge column
    row_elements.push_back(RenderLoggerBadge(entry));
    row_elements.push_back(CreateSeparator());
    
    // Log level column
    row_elements.push_back(RenderLogLevel(entry));
    row_elements.push_back(CreateSeparator());
    
    // Message column with search highlighting (flexible width)
    row_elements.push_back(RenderMessageWithSearchHighlight(entry, is_selected, search_query, case_sensitive) | flex);
    
    // Create the row with proper spacing
    Element row = hbox(row_elements);
    
    // Check if this entry contains a search match
    bool is_search_match = false;
    if (!search_query.empty()) {
        std::string search_text = entry.Get_message() + " " + entry.Get_logger_name();
        if (entry.Get_log_level().has_value()) {
            search_text += " " + entry.Get_log_level().value();
        }
        
        if (case_sensitive) {
            is_search_match = search_text.find(search_query) != std::string::npos;
        } else {
            std::string lower_query = search_query;
            std::string lower_text = search_text;
            std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);
            std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
            is_search_match = lower_text.find(lower_query) != std::string::npos;
        }
    }
    
    // Apply row-level visual hierarchy with search indication
    std::string level = entry.Get_log_level().has_value() ? entry.Get_log_level().value() : "";
    row = ApplyRowLevelHierarchyWithSearch(row, level, is_selected, is_search_match);
    
    // Apply selection highlighting (after hierarchy styling to ensure it takes precedence)
    if (is_selected) {
        row = row | inverted;
    }
    
    return row;
}

Element LogEntryRenderer::RenderMessageWithSearchHighlight(const LogEntry& entry, bool is_selected, 
                                                          const std::string& search_query, 
                                                          bool case_sensitive) const {
    using namespace ftxui;
    
    if (search_query.empty()) {
        return RenderMessage(entry, is_selected);
    }
    
    std::string message = entry.Get_message();
    std::vector<Element> elements;
    
    // Find all matches in the message
    std::vector<size_t> match_positions;
    std::string search_text = message;
    std::string query = search_query;
    
    if (!case_sensitive) {
        std::transform(search_text.begin(), search_text.end(), search_text.begin(), ::tolower);
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);
    }
    
    size_t pos = 0;
    while ((pos = search_text.find(query, pos)) != std::string::npos) {
        match_positions.push_back(pos);
        pos += query.length();
    }
    
    if (match_positions.empty()) {
        return RenderMessage(entry, is_selected);
    }
    
    // Build highlighted message
    size_t last_pos = 0;
    
    for (size_t match_pos : match_positions) {
        // Add text before the match
        if (match_pos > last_pos) {
            std::string before_match = message.substr(last_pos, match_pos - last_pos);
            if (word_wrap_enabled_) {
                elements.push_back(paragraph(before_match));
            } else {
                elements.push_back(text(before_match));
            }
        }
        
        // Add highlighted match
        std::string match_text = message.substr(match_pos, search_query.length());
        Element highlighted_match = text(match_text) | bgcolor(Color::Yellow) | color(Color::Black) | bold;
        elements.push_back(highlighted_match);
        
        last_pos = match_pos + search_query.length();
    }
    
    // Add remaining text after the last match
    if (last_pos < message.length()) {
        std::string after_matches = message.substr(last_pos);
        if (word_wrap_enabled_) {
            elements.push_back(paragraph(after_matches));
        } else {
            elements.push_back(text(after_matches));
        }
    }
    
    Element result = hbox(elements);
    
    // Apply word wrapping if enabled
    if (word_wrap_enabled_) {
        result = result | flex;
    }
    
    return result;
}

} // namespace ue_log