#include "visual_theme_manager.h"

namespace ue_log {

VisualThemeManager::VisualThemeManager() 
    : next_color_index_(0) {
    InitializeColorPalette();
    InitializeFontConfiguration();
}

void VisualThemeManager::InitializeColorPalette() {
    // Initialize logger badge colors with a diverse, accessible palette
    // Optimized for eye strain reduction and extended use
    if (eye_strain_reduction_enabled_) {
        logger_color_palette_ = {
            ftxui::Color::RGB(100, 149, 237),  // Cornflower blue - easier on eyes
            ftxui::Color::RGB(60, 179, 113),   // Medium sea green - softer green
            ftxui::Color::RGB(72, 209, 204),   // Medium turquoise - gentle cyan
            ftxui::Color::RGB(186, 85, 211),   // Medium orchid - softer magenta
            ftxui::Color::RGB(255, 215, 0),    // Gold - warmer yellow
            ftxui::Color::RGB(135, 206, 250),  // Light sky blue - very gentle
            ftxui::Color::RGB(144, 238, 144),  // Light green - easy on eyes
            ftxui::Color::RGB(175, 238, 238),  // Pale turquoise - very soft
            ftxui::Color::RGB(221, 160, 221)   // Plum - gentle purple
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
    // Handle Unreal Engine specific log levels with eye strain optimized colors
    if (log_level == "Error") {
        return eye_strain_reduction_enabled_ ? 
               ftxui::Color::RGB(255, 99, 99) :   // Softer red for eye strain reduction
               ftxui::Color::Red;                  // Bright red for maximum visibility
    } else if (log_level == "Warning") {
        return eye_strain_reduction_enabled_ ? 
               ftxui::Color::RGB(255, 215, 0) :   // Gold instead of bright yellow
               ftxui::Color::Yellow;               // Bright yellow for clear distinction
    } else if (log_level == "Display") {
        return eye_strain_reduction_enabled_ ? 
               ftxui::Color::RGB(240, 240, 240) : // Slightly off-white for comfort
               ftxui::Color::White;
    } else if (log_level == "Verbose") {
        return eye_strain_reduction_enabled_ ? 
               ftxui::Color::RGB(169, 169, 169) : // Darker gray for better contrast
               ftxui::Color::GrayLight;
    } else if (log_level == "VeryVerbose") {
        return eye_strain_reduction_enabled_ ? 
               ftxui::Color::RGB(128, 128, 128) : // Medium gray
               ftxui::Color::GrayDark;
    } else if (log_level == "Trace") {
        return eye_strain_reduction_enabled_ ? 
               ftxui::Color::RGB(72, 209, 204) :  // Medium turquoise
               ftxui::Color::CyanLight;
    }
    
    // Fallback for generic log levels (for backward compatibility)
    if (log_level == "Info") {
        return eye_strain_reduction_enabled_ ? 
               ftxui::Color::RGB(240, 240, 240) : 
               ftxui::Color::White;
    } else if (log_level == "Debug") {
        return eye_strain_reduction_enabled_ ? 
               ftxui::Color::RGB(169, 169, 169) : 
               ftxui::Color::GrayLight;
    }
    
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(240, 240, 240) : 
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
           ftxui::Color::RGB(24, 24, 24) :  // Very dark gray instead of pure black
           ftxui::Color::Black;
}

ftxui::Color VisualThemeManager::GetTextColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(240, 240, 240) : // Slightly off-white for comfort
           ftxui::Color::White;
}

ftxui::Color VisualThemeManager::GetHighlightColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(100, 149, 237) : // Cornflower blue - easier on eyes
           ftxui::Color::Blue;
}

ftxui::Color VisualThemeManager::GetFocusColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(135, 206, 250) : // Light sky blue for focus
           ftxui::Color::BlueLight;
}

ftxui::Color VisualThemeManager::GetHoverColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(176, 196, 222) : // Light steel blue for hover
           ftxui::Color::GrayLight;
}

ftxui::Color VisualThemeManager::GetBorderColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(105, 105, 105) : // Dim gray for borders
           ftxui::Color::GrayDark;
}

ftxui::Color VisualThemeManager::GetMutedTextColor() const {
    return eye_strain_reduction_enabled_ ? 
           ftxui::Color::RGB(169, 169, 169) : // Dark gray for muted text
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