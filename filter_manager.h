#pragma once
#include "log_entry.h"
#include "ftxui/dom/elements.hpp"
#include <unordered_map>
#include <string>

struct CategoryFilter {
    std::string name;
    bool enabled = true;
    ftxui::Color color = ftxui::Color::Default;
};

class FilterManager {
public:
    void AddCategory(const std::string& category);
    bool MatchesFilters(const LogEntry& entry, const std::string& search_term);
    ftxui::Color GetEntryColor(const LogEntry& entry);
    void ToggleCategory(const std::string& category);
    bool IsCategoryEnabled(const std::string& category);

    const std::unordered_map<std::string, CategoryFilter>& GetCategories() const { return category_filters_; }
    std::unordered_map<std::string, CategoryFilter>& GetCategories() { return category_filters_; }

private:
    std::unordered_map<std::string, CategoryFilter> category_filters_;
    bool ContainsIgnoreCase(const std::string& haystack, const std::string& needle);
    ftxui::Color AssignCategoryColor(const std::string& category, size_t index);
};