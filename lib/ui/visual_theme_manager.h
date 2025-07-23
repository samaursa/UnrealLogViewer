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
        int timestamp_width = 15;       // "12:34:56.789000" - increased for better readability
        int frame_width = 8;           // " [12345]"
        int logger_badge_width = 18;   // "[LoggerName   ]" - slightly reduced for better balance
        int level_width = 8;           // " ERROR  "
        int column_padding = 2;        // Spaces between columns (deprecated - now using visual separators)
        bool use_visual_separators = true;  // Use │ characters between columns
        bool align_numbers_right = true;    // Right-align numeric columns (line, frame)
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
     * Get background color for a log level (for prominent styling).
     * @param log_level The log level (Error, Warning, Info, Debug, etc.)
     * @return FTXUI Color for the log level background, or default if no special background needed
     */
    ftxui::Color GetLogLevelBackgroundColor(const std::string& log_level) const;
    
    /**
     * Check if a log level should be displayed with prominent styling.
     * @param log_level The log level to check
     * @return True if the log level should be prominently displayed
     */
    bool IsLogLevelProminent(const std::string& log_level) const;
    
    /**
     * Check if a log level should be displayed with bold text.
     * @param log_level The log level to check
     * @return True if the log level should use bold text
     */
    bool ShouldLogLevelUseBold(const std::string& log_level) const;
    
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
    
    /**
     * Get the visual separator string for columns.
     * @return String to use as column separator
     */
    std::string GetColumnSeparator() const;
    
    /**
     * Enable or disable visual column separators.
     * @param enabled True to enable visual separators, false to use spacing only
     */
    void SetVisualSeparatorsEnabled(bool enabled);

    // Visual polish and consistency methods
    
    /**
     * Get the focus indicator color for interactive elements.
     * @return FTXUI Color for focus indicators
     */
    ftxui::Color GetFocusColor() const;
    
    /**
     * Get the hover color for interactive elements.
     * @return FTXUI Color for hover states
     */
    ftxui::Color GetHoverColor() const;
    
    /**
     * Get the border color for UI elements.
     * @return FTXUI Color for borders and separators
     */
    ftxui::Color GetBorderColor() const;
    
    /**
     * Get the muted text color for less important information.
     * @return FTXUI Color for muted text
     */
    ftxui::Color GetMutedTextColor() const;
    
    /**
     * Get the accent color for important UI elements.
     * @return FTXUI Color for accents
     */
    ftxui::Color GetAccentColor() const;
    
    /**
     * Check if the current theme is optimized for reduced eye strain.
     * @return True if using eye strain reduction colors
     */
    bool IsEyeStrainReductionEnabled() const;
    
    /**
     * Enable or disable eye strain reduction color scheme.
     * @param enabled True to enable eye strain reduction
     */
    void SetEyeStrainReductionEnabled(bool enabled);
    
    /**
     * Get the appropriate font weight for a UI element type.
     * @param element_type Type of UI element (header, body, emphasis, etc.)
     * @return True if element should use bold font weight
     */
    bool GetFontWeight(const std::string& element_type) const;
    
    /**
     * Get the appropriate font size indicator for a UI element type.
     * @param element_type Type of UI element
     * @return Font size indicator (0=normal, 1=large, -1=small)
     */
    int GetFontSize(const std::string& element_type) const;

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
    
    // Visual polish configuration
    bool eye_strain_reduction_enabled_ = true;
    
    // Font configuration
    std::unordered_map<std::string, bool> font_weights_;
    std::unordered_map<std::string, int> font_sizes_;
    
    // Helper methods
    
    /**
     * Initialize the color palette with default colors.
     */
    void InitializeColorPalette();
    
    /**
     * Initialize font configuration with default settings.
     */
    void InitializeFontConfiguration();
    
    /**
     * Assign a new color to a logger from the cycling palette.
     * @param logger_name The logger name to assign a color to
     * @return The assigned color
     */
    ftxui::Color AssignLoggerColor(const std::string& logger_name);
};

} // namespace ue_log