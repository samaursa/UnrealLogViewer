#pragma once

#include "visual_theme_manager.h"
#include "../log_parser/log_entry.h"
#include <ftxui/dom/elements.hpp>
#include <memory>

namespace ue_log {

/**
 * Handles rendering of individual log entries with enhanced visual formatting.
 * Provides logger badge system, proper column spacing, and visual hierarchy.
 */
class LogEntryRenderer {
public:
    /**
     * Constructor.
     * @param theme_manager Pointer to the visual theme manager for color and layout configuration
     */
    explicit LogEntryRenderer(VisualThemeManager* theme_manager);
    
    /**
     * Destructor.
     */
    ~LogEntryRenderer() = default;
    
    // Main rendering methods
    
    /**
     * Render a complete log entry with all columns and styling.
     * @param entry The log entry to render
     * @param is_selected Whether this entry is currently selected
     * @param relative_line_number Optional relative line number for vim-style display
     * @return FTXUI Element representing the rendered log entry
     */
    ftxui::Element RenderLogEntry(const LogEntry& entry, bool is_selected, 
                                 int relative_line_number = 0) const;
    
    /**
     * Render the table header with column labels.
     * @return FTXUI Element representing the table header
     */
    ftxui::Element RenderTableHeader() const;
    
    // Component rendering methods
    
    /**
     * Render line number column (relative or absolute).
     * @param relative_number The relative line number (0 for current line)
     * @param is_current Whether this is the current/selected line
     * @return FTXUI Element for the line number column
     */
    ftxui::Element RenderLineNumber(int relative_number, bool is_current) const;
    
    /**
     * Render timestamp column.
     * @param entry The log entry containing timestamp data
     * @return FTXUI Element for the timestamp column
     */
    ftxui::Element RenderTimestamp(const LogEntry& entry) const;
    
    /**
     * Render frame number column.
     * @param entry The log entry containing frame data
     * @return FTXUI Element for the frame number column
     */
    ftxui::Element RenderFrameNumber(const LogEntry& entry) const;
    
    /**
     * Render logger badge with consistent color assignment.
     * @param entry The log entry containing logger name
     * @return FTXUI Element for the logger badge
     */
    ftxui::Element RenderLoggerBadge(const LogEntry& entry) const;
    
    /**
     * Render log level column with appropriate styling.
     * @param entry The log entry containing log level data
     * @return FTXUI Element for the log level column
     */
    ftxui::Element RenderLogLevel(const LogEntry& entry) const;
    
    /**
     * Render message column with word wrapping and highlighting support.
     * @param entry The log entry containing the message
     * @param is_selected Whether this entry is selected (for highlighting)
     * @return FTXUI Element for the message column
     */
    ftxui::Element RenderMessage(const LogEntry& entry, bool is_selected) const;
    
    // Configuration methods
    
    /**
     * Enable or disable word wrapping for message content.
     * @param enabled True to enable word wrapping, false to disable
     */
    void SetWordWrapEnabled(bool enabled) { word_wrap_enabled_ = enabled; }
    
    /**
     * Enable or disable line number display.
     * @param show True to show line numbers, false to hide
     */
    void SetShowLineNumbers(bool show) { show_line_numbers_ = show; }
    
    /**
     * Check if word wrapping is enabled.
     * @return True if word wrapping is enabled
     */
    bool IsWordWrapEnabled() const { return word_wrap_enabled_; }
    
    /**
     * Check if line numbers are shown.
     * @return True if line numbers are shown
     */
    bool IsShowLineNumbers() const { return show_line_numbers_; }

private:
    // Dependencies
    VisualThemeManager* theme_manager_;
    
    // Configuration
    bool word_wrap_enabled_ = false;
    bool show_line_numbers_ = true;
    
    // Helper methods
    
    /**
     * Create a badge element with text and background color.
     * @param text The text to display in the badge
     * @param color The background color for the badge
     * @return FTXUI Element representing the badge
     */
    ftxui::Element CreateBadgeElement(const std::string& text, ftxui::Color color) const;
    
    /**
     * Apply log level specific styling to an element.
     * @param element The element to style
     * @param level The log level string
     * @return Styled FTXUI Element
     */
    ftxui::Element ApplyLogLevelStyling(ftxui::Element element, const std::string& level) const;
    
    /**
     * Apply row-level visual hierarchy styling based on log level.
     * @param element The row element to style
     * @param level The log level string
     * @param is_selected Whether the row is currently selected
     * @return Styled FTXUI Element with appropriate visual hierarchy
     */
    ftxui::Element ApplyRowLevelHierarchy(ftxui::Element element, const std::string& level, bool is_selected) const;
    
    /**
     * Truncate text to fit within a maximum width.
     * @param text The text to truncate
     * @param max_width Maximum width in characters
     * @return Truncated text string
     */
    std::string TruncateText(const std::string& text, int max_width) const;
    
    /**
     * Pad text to a specific width with spaces.
     * @param text The text to pad
     * @param width The target width
     * @return Padded text string
     */
    std::string PadText(const std::string& text, int width) const;
};

} // namespace ue_log