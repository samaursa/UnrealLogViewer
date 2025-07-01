#include "filter_manager.h"
#include <algorithm>
#include <cctype>
#include <vector>

using namespace ftxui;

void FilterManager::AddCategory(const std::string& category) {
    if (category_filters_.find(category) == category_filters_.end()) {
        CategoryFilter filter;
        filter.name = category;
        filter.enabled = true;
        filter.color = AssignCategoryColor(category, category_filters_.size());
        category_filters_[category] = std::move(filter);
    }
}

bool FilterManager::ContainsIgnoreCase(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    auto it = std::search(haystack.begin(), haystack.end(),
                         needle.begin(), needle.end(),
                         [](char ch1, char ch2) {
                             return std::tolower(ch1) == std::tolower(ch2);
                         });
    return it != haystack.end();
}

bool FilterManager::MatchesFilters(const LogEntry& entry, const std::string& search_term) {
    // Global search
    if (!search_term.empty()) {
        if (!ContainsIgnoreCase(entry.message, search_term) &&
            !ContainsIgnoreCase(entry.category, search_term) &&
            !ContainsIgnoreCase(entry.level, search_term)) {
            return false;
        }
    }

    // Category filter
    if (!entry.category.empty()) {
        auto it = category_filters_.find(entry.category);
        if (it != category_filters_.end() && !it->second.enabled) {
            return false;
        }
    }

    return true;
}

Color FilterManager::GetEntryColor(const LogEntry& entry) {
    // Category color first
    auto it = category_filters_.find(entry.category);
    if (it != category_filters_.end() && it->second.color != Color::Default) {
        return it->second.color;
    }

    // Default colors based on log level
    std::string level_lower = entry.level;
    std::transform(level_lower.begin(), level_lower.end(), level_lower.begin(), ::tolower);

    if (level_lower.find("error") != std::string::npos) return Color::Red;
    if (level_lower.find("warning") != std::string::npos) return Color::Yellow;
    if (level_lower.find("verbose") != std::string::npos) return Color::GrayDark;
    if (level_lower.find("display") != std::string::npos) return Color::White;

    return Color::Default;
}

void FilterManager::ToggleCategory(const std::string& category) {
    auto it = category_filters_.find(category);
    if (it != category_filters_.end()) {
        it->second.enabled = !it->second.enabled;
    }
}

bool FilterManager::IsCategoryEnabled(const std::string& category) {
    auto it = category_filters_.find(category);
    return it != category_filters_.end() ? it->second.enabled : true;
}

Color FilterManager::AssignCategoryColor(const std::string& category, size_t index) {
    // Assign colors based on category type
    if (category.find("Error") != std::string::npos) {
        return Color::Red;
    } else if (category.find("Warning") != std::string::npos) {
        return Color::Yellow;
    } else if (category.find("LogTemp") != std::string::npos) {
        return Color::Cyan;
    } else if (category.find("LogCore") != std::string::npos) {
        return Color::Green;
    } else if (category.find("LogInit") != std::string::npos) {
        return Color::Blue;
    } else if (category.find("LogWindows") != std::string::npos) {
        return Color::Magenta;
    } else if (category.find("Display") != std::string::npos) {
        return Color::White;
    } else {
        // Cycle through colors for other categories
        static std::vector<Color> colors = {
            Color::CyanLight, Color::GreenLight, Color::MagentaLight,
            Color::BlueLight, Color::YellowLight, Color::RedLight,
            Color::GrayLight
        };
        return colors[index % colors.size()];
    }
}