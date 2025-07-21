#include "visual_theme_manager.h"

namespace ue_log {

VisualThemeManager::VisualThemeManager() 
    : next_color_index_(0) {
    InitializeColorPalette();
}

void VisualThemeManager::InitializeColorPalette() {
    // Initialize logger badge colors with a diverse, accessible palette
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
    if (log_level == "Error") {
        return ftxui::Color::RedLight;
    } else if (log_level == "Warning") {
        return ftxui::Color::YellowLight;
    } else if (log_level == "Info") {
        return ftxui::Color::White;
    } else if (log_level == "Debug") {
        return ftxui::Color::GrayLight;
    }
    return ftxui::Color::White; // Default color
}

ftxui::Color VisualThemeManager::GetBackgroundColor() const {
    return ftxui::Color::Black;
}

ftxui::Color VisualThemeManager::GetTextColor() const {
    return ftxui::Color::White;
}

ftxui::Color VisualThemeManager::GetHighlightColor() const {
    return ftxui::Color::Blue;
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

} // namespace ue_log