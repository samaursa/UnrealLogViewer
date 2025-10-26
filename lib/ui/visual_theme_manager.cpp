#include "visual_theme_manager.h"

namespace ue_log {

VisualThemeManager::VisualThemeManager() 
    : next_color_index_(0) {
    InitializeColorPalette();
    InitializeFontConfiguration();
}

void VisualThemeManager::InitializeColorPalette() {
    // Initialize logger badge colors with a diverse, accessible palette
    // Optimized for light themes with soft, readable colors
    if (eye_strain_reduction_enabled_) {
        logger_color_palette_ = {
            ftxui::Color::RGB(80, 120, 180),   // Muted blue - readable on light backgrounds
            ftxui::Color::RGB(100, 140, 100),  // Soft forest green - gentle and readable
            ftxui::Color::RGB(90, 150, 140),   // Muted teal - easy on eyes
            ftxui::Color::RGB(140, 90, 140),   // Soft purple - not too bright
            ftxui::Color::RGB(180, 140, 60),   // Warm brown-gold - much softer than yellow
            ftxui::Color::RGB(70, 110, 160),   // Darker sky blue - better contrast
            ftxui::Color::RGB(120, 160, 120),  // Medium green - good readability
            ftxui::Color::RGB(100, 130, 150),  // Blue-gray - professional look
            ftxui::Color::RGB(150, 100, 130)   // Muted rose - gentle on eyes
        };
    } else {
        // Standard high-contrast palette for maximum visibility
        logger_color_palette_ = {
            ftxui::Color::Blue,
            ftxui::Color::Green,
            ftxui::Color::Cyan,
            ftxui::Color::Magenta,
            ftxui::Color::Yellow,
            ftxui::Color::BlueLight,
            ftxui::Color::GreenLight,
            ftxui::Color::CyanLight,
            ftxui::Color::MagentaLight
        };
    }
}

void VisualThemeManager::InitializeFontConfiguration() {
    // Configure font weights for different UI element types
    font_weights_["header"] = true;        // Headers should be bold
    font_weights_["body"] = false;         // Body text should be normal
    font_weights_["emphasis"] = true;      // Emphasized text should be bold
    font_weights_["error"] = true;         // Errors should be bold
    font_weights_["warning"] = true;       // Warnings should be bold
    font_weights_["status"] = false;       // Status text should be normal
    font_weights_["muted"] = false;        // Muted text should be normal
    font_weights_["button"] = false;       // Buttons should be normal (unless focused)
    font_weights_["label"] = false;        // Labels should be normal
    
    // Configure font sizes (0=normal, 1=large, -1=small)
    font_sizes_["header"] = 0;             // Headers use normal size (bold provides emphasis)
    font_sizes_["body"] = 0;               // Body text uses normal size
    font_sizes_["emphasis"] = 0;           // Emphasized text uses normal size
    font_sizes_["error"] = 0;              // Errors use normal size (color/bold provides emphasis)
    font_sizes_["warning"] = 0;            // Warnings use normal size
    font_sizes_["status"] = -1;            // Status text is slightly smaller
    font_sizes_["muted"] = -1;             // Muted text is slightly smaller
    font_sizes_["button"] = 0;             // Buttons use normal size
    font_sizes_["label"] = 0;              // Labels use normal size
}

ftxui::Color VisualThemeManager::GetLoggerColor(const std::string& logger_name) {
    // Check if we already have a color assigned for this logger
    auto it = logger_color_cache_.find(logger_name);
    if (it != logger_color_cache_.end()) {
        return it->second;
    }
    
    // Assign a new color from the cycling palette
    return AssignLoggerColor(logger_name);
}

ftxui::Color VisualThemeManager::AssignLoggerColor(const std::string& logger_name) {
    // Get the next color from the palette
    ftxui::Color assigned_color = logger_color_palette_[next_color_index_];
    
    // Cache the assignment for consistency
    logger_color_cache_[logger_name] = assigned_color;
    
    // Advance to the next color, cycling back to the beginning if needed
    next_color_index_ = (next_color_index_ + 1) % logger_color_palette_.size();
    
    return assigned_color;
}

ftxui::Color VisualThemeManager::GetLogLevelColor(const std::string& log_level) const {
    // Handle Unreal Engine specific log levels with light theme friendly colors
    if (log_level == "Error") {
        return eye_strain_reduction_enabled_ ?
               ftxui::Color::RGB(140, 60, 60) :   // Darker red - better contrast against light red backgrounds
               ftxui::Color::Red;                  // Bright red for maximum visibility
    } else if (log_level == "Warning") {
        return eye_strain_reduction_enabled_ ?
               ftxui::Color::RGB(180, 120, 40) :  // Darker orange-brown - better contrast
               ftxui::Color::Yellow;               // Bright yellow for clear distinction
    } else if (log_level == "Display") {
        return eye_strain_reduction_enabled_ ?
               ftxui::Color::RGB(50, 50, 50) :    // Darker gray for better readability
               ftxui::Color::White;
    } else if (log_level == "Verbose") {
        return eye_strain_reduction_enabled_ ?
               ftxui::Color::RGB(120, 120, 120) : // Medium gray - gentle contrast
               ftxui::Color::GrayLight;
    } else if (log_level == "VeryVerbose") {
        return eye_strain_reduction_enabled_ ?
               ftxui::Color::RGB(150, 150, 150) : // Light gray - subtle but readable
               ftxui::Color::GrayDark;
    } else if (log_level == "Trace") {
        return eye_strain_reduction_enabled_ ?
               ftxui::Color::RGB(80, 140, 160) :  // Soft teal-blue - gentle on eyes
               ftxui::Color::CyanLight;
    }
    
    // Fallback for generic log levels (for backward compatibility)
    if (log_level == "Info") {
        return eye_strain_reduction_enabled_ ?
               ftxui::Color::RGB(50, 50, 50) :    // Darker gray for better readability
               ftxui::Color::White;
    } else if (log_level == "Debug") {
        return eye_strain_reduction_enabled_ ?
               ftxui::Color::RGB(120, 120, 120) : // Medium gray
               ftxui::Color::GrayLight;
    }

    return eye_strain_reduction_enabled_ ?
           ftxui::Color::RGB(50, 50, 50) :       // Default darker gray for readability
           ftxui::Color::White; // Default color for unknown levels
}

ftxui::Color VisualThemeManager::GetLogLevelBackgroundColor(const std::string& log_level) const {
    // Only provide background colors for the most critical levels
    if (log_level == "Error") {
        return ftxui::Color::RedLight;  // Light red background for errors
    }
    
    // No special background for other levels - return default background
    return GetBackgroundColor();
}

bool VisualThemeManager::IsLogLevelProminent(const std::string& log_level) const {
    // Error and Warning levels should be prominently displayed
    return (log_level == "Error" || log_level == "Warning");
}

bool VisualThemeManager::ShouldLogLevelUseBold(const std::string& log_level) const {
    // Errors should always be bold, warnings should be bold for prominence
    return (log_level == "Error" || log_level == "Warning");
}

ftxui::Color VisualThemeManager::GetBackgroundColor() const {
    return eye_strain_reduction_enabled_ ?
           ftxui::Color::RGB(250, 250, 250) : // Light background for light theme
           ftxui::Color::Black;
}

ftxui::Color VisualThemeManager::GetTextColor() const {
    return eye_strain_reduction_enabled_ ?
           ftxui::Color::RGB(40, 40, 40) :    // Darker gray for better readability
           ftxui::Color::White;
}

ftxui::Color VisualThemeManager::GetHighlightColor() const {
    return eye_strain_reduction_enabled_ ?
           ftxui::Color::RGB(140, 170, 200) : // Soft blue-gray - gentle on eyes
           ftxui::Color::Blue;
}

ftxui::Color VisualThemeManager::GetVisualSelectionColor() const {
    // Visual selection should use a distinct color from normal selection
    // Using darker text for good contrast on light selection background
    return eye_strain_reduction_enabled_ ?
           ftxui::Color::RGB(40, 40, 40) :    // Dark text for light background
           ftxui::Color::White;
}

ftxui::Color VisualThemeManager::GetVisualSelectionBackgroundColor() const {
    // Visual selection background should be distinct from normal highlight
    // Using a soft lavender tone for light themes
    return eye_strain_reduction_enabled_ ?
           ftxui::Color::RGB(200, 180, 220) : // Soft lavender - gentle but distinct
           ftxui::Color::Magenta;             // Bright magenta for high contrast
}

ftxui::Color VisualThemeManager::GetFocusColor() const {
    return eye_strain_reduction_enabled_ ?
           ftxui::Color::RGB(100, 140, 180) : // Muted blue for focus - readable on light theme
           ftxui::Color::BlueLight;
}

ftxui::Color VisualThemeManager::GetHoverColor() const {
    return eye_strain_reduction_enabled_ ?
           ftxui::Color::RGB(160, 180, 200) : // Soft blue-gray for hover
           ftxui::Color::GrayLight;
}

ftxui::Color VisualThemeManager::GetBorderColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(60, 60, 60) :     // Darker gray for borders on light theme
           ftxui::Color::GrayDark;
}

ftxui::Color VisualThemeManager::GetMutedTextColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(180, 180, 180) : // Lighter gray for muted text on light theme
           ftxui::Color::GrayLight;
}

ftxui::Color VisualThemeManager::GetAccentColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(255, 215, 0) :   // Gold for accents
           ftxui::Color::Yellow;
}

bool VisualThemeManager::IsEyeStrainReductionEnabled() const {
    return eye_strain_reduction_enabled_;
}

void VisualThemeManager::SetEyeStrainReductionEnabled(bool enabled) {
    if (eye_strain_reduction_enabled_ != enabled) {
        eye_strain_reduction_enabled_ = enabled;
        // Reinitialize color palette with new settings
        InitializeColorPalette();
        // Clear logger color cache to apply new colors
        ResetLoggerColors();
    }
}

bool VisualThemeManager::GetFontWeight(const std::string& element_type) const {
    auto it = font_weights_.find(element_type);
    return it != font_weights_.end() ? it->second : false;
}

int VisualThemeManager::GetFontSize(const std::string& element_type) const {
    auto it = font_sizes_.find(element_type);
    return it != font_sizes_.end() ? it->second : 0;
}

VisualThemeManager::ColumnSpacing VisualThemeManager::GetColumnSpacing() const {
    return column_spacing_;
}

void VisualThemeManager::SetColumnSpacing(const ColumnSpacing& spacing) {
    column_spacing_ = spacing;
}

void VisualThemeManager::ResetLoggerColors() {
    logger_color_cache_.clear();
    next_color_index_ = 0;
}

size_t VisualThemeManager::GetAvailableLoggerColorCount() const {
    return logger_color_palette_.size();
}

std::string VisualThemeManager::GetColumnSeparator() const {
    if (column_spacing_.use_visual_separators) {
        return " │ ";
    } else {
        return std::string(column_spacing_.column_padding, ' ');
    }
}

void VisualThemeManager::SetVisualSeparatorsEnabled(bool enabled) {
    column_spacing_.use_visual_separators = enabled;
}

} // namespace ue_log