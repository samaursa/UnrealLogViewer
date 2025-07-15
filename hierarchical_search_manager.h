#pragma once
#include <vector>
#include <string>
#include <functional>

struct SearchLevel {
    std::string term;
    std::vector<size_t> filtered_indices;
    bool is_active = true;
};

class HierarchicalSearchManager {
private:
    std::vector<SearchLevel> search_levels_;
    int focused_level_ = 0;
    std::function<void()> update_callback_;

public:
    HierarchicalSearchManager() {
        // Always have at least one search level
        search_levels_.push_back(SearchLevel{});
    }

    void SetUpdateCallback(std::function<void()> callback) {
        update_callback_ = callback;
    }

    void AddSearchLevel() {
        SearchLevel new_level;
        new_level.filtered_indices = GetCurrentFilteredIndices();
        search_levels_.push_back(new_level);
        focused_level_ = search_levels_.size() - 1;
        if (update_callback_) update_callback_();
    }

    void RemoveSearchLevel() {
        if (search_levels_.size() > 1) {
            search_levels_.pop_back();
            focused_level_ = std::min(focused_level_, (int)search_levels_.size() - 1);
            if (update_callback_) update_callback_();
        }
    }

    void FocusNext() {
        focused_level_ = (focused_level_ + 1) % search_levels_.size();
    }

    void FocusPrevious() {
        focused_level_ = (focused_level_ - 1 + search_levels_.size()) % search_levels_.size();
    }

    std::string& GetFocusedSearchTerm() {
        return search_levels_[focused_level_].term;
    }

    void SetSearchTerm(int level, const std::string& term) {
        if (level >= 0 && level < search_levels_.size()) {
            search_levels_[level].term = term;
        }
    }

    const std::vector<size_t>& GetCurrentFilteredIndices() const {
        for (int i = search_levels_.size() - 1; i >= 0; i--) {
            if (!search_levels_[i].filtered_indices.empty()) {
                return search_levels_[i].filtered_indices;
            }
        }
        // Return empty if no filtered indices found
        static std::vector<size_t> empty;
        return empty;
    }

    void UpdateFilteredIndices(int level, const std::vector<size_t>& indices) {
        if (level >= 0 && level < search_levels_.size()) {
            search_levels_[level].filtered_indices = indices;
        }
    }

    const std::vector<SearchLevel>& GetSearchLevels() const { return search_levels_; }
    int GetFocusedLevel() const { return focused_level_; }
    size_t GetLevelCount() const { return search_levels_.size(); }
};
