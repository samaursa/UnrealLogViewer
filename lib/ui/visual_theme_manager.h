#pragma once

#include <ftxui/screen/color.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace ue_log {

/**
 * Manages visual themes, color palettes, and layout configuration for the UI.
 * Provides consistent color assignment for logger badges and visual styling.
 */
class VisualThemeManager {
public:
    /**
     * Configuration structure for column spacing and layout.
     */
    struct ColumnSpacing {
        int line_number_width = 6;      // "  123 " or " +12 " for relative
        int timestamp_width = 12;       // "12:34:56.789"
        int frame_width = 8;           // " [12345]"
        int logger_badge_width = 20;   // "[LoggerName    ]"
        int level_width = 8;           // " ERROR  "
        int column_padding = 2;        // Spaces between columns
    };

    /**
     * Default constructor.
     */
    VisualThemeManager();

    /**
     * Destructor.
     */
    ~VisualThemeManager() = default;

    // Color management methods
    
    /**
     * Get a consistent color for a logger name.
     * Colors are assigned cyclically and cached for consistency.
     * @param logger_name The name of the logger
     * @return FTXUI Color for the logger
     */
    ftxui::Color GetLoggerColor(const std::string& logger_name);
    
    /**
     * Get color for a log level.
     * @param log_level The log level (Error, Warning, Info, Debug, etc.)
     * @return FTXUI Color for the log level
     */
    ftxui::Color GetLogLevelColor(const std::string& log_level) const;
    
    /**
     * Get the default background color.
     * @return FTXUI Color for background
     */
    ftxui::Color GetBackgroundColor() const;
    
    /**
     * Get the default text color.
     * @return FTXUI Color for text
     */
    ftxui::Color GetTextColor() const;
    
    /**
     * Get the highlight color for selections.
     * @return FTXUI Color for highlights
     */
    ftxui::Color GetHighlightColor() const;

    // Layout configuration methods
    
    /**
     * Get the current column spacing configuration.
     * @return ColumnSpacing structure with current settings
     */
    ColumnSpacing GetColumnSpacing() const;
    
    /**
     * Set the column spacing configuration.
     * @param spacing New column spacing configuration
     */
    void SetColumnSpacing(const ColumnSpacing& spacing);

    // Utility methods
    
    /**
     * Reset logger color assignments.
     * Clears the cache and starts color cycling from the beginning.
     */
    void ResetLoggerColors();
    
    /**
     * Get the number of available logger colors.
     * @return Number of colors in the cycling palette
     */
    size_t GetAvailableLoggerColorCount() const;

private:
    // Color palette for logger badges (cycling through these)
    std::vector<ftxui::Color> logger_color_palette_;
    
    // Cache for consistent logger color assignment
    std::unordered_map<std::string, ftxui::Color> logger_color_cache_;
    
    // Index for cycling through logger colors
    size_t next_color_index_;
    
    // Column spacing configuration
    ColumnSpacing column_spacing_;
    
    // Helper methods
    
    /**
     * Initialize the color palette with default colors.
     */
    void InitializeColorPalette();
    
    /**
     * Assign a new color to a logger from the cycling palette.
     * @param logger_name The logger name to assign a color to
     * @return The assigned color
     */
    ftxui::Color AssignLoggerColor(const std::string& logger_name);
};

} // namespace ue_log