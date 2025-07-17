#pragma once

#include "../../macros.h"
#include "../common/result.h"
#include "filter.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <regex>

namespace ue_log {
    
    class FilterEngine {
        CK_GENERATED_BODY(FilterEngine);
        
    private:
        std::vector<std::unique_ptr<Filter>> primary_filters;
        std::unordered_map<std::string, std::regex> compiled_regex_cache;
        size_t total_entries_processed;
        size_t total_matches_found;
        
    public:
        // Default constructor
        FilterEngine();
        
        // Destructor
        ~FilterEngine() = default;
        
        // Properties using macros
        CK_PROPERTY_GET(primary_filters);
        CK_PROPERTY_GET(total_entries_processed);
        CK_PROPERTY_GET(total_matches_found);
        
        // Filter management
        Result AddFilter(std::unique_ptr<Filter> filter);
        Result RemoveFilter(const std::string& filter_name);
        Filter* FindFilter(const std::string& filter_name);
        void ClearAllFilters();
        size_t GetFilterCount() const;
        
        // Filtering operations
        std::vector<LogEntry> ApplyFilters(const std::vector<LogEntry>& entries);
        bool PassesFilters(const LogEntry& entry);
        std::vector<const Filter*> GetMatchingFilters(const LogEntry& entry);
        
        // Statistics and management
        void UpdateFilterStatistics(const std::vector<LogEntry>& entries);
        void ResetAllStatistics();
        size_t GetTotalActiveFilters() const;
        std::vector<Filter*> GetActiveFilters();
        
        // Regex caching (shared across all filters)
        const std::regex& GetCachedRegex(const std::string& pattern);
        void ClearRegexCache();
        size_t GetRegexCacheSize() const;
        
        // Validation
        Result ValidateAllFilters();
        std::vector<std::string> GetInvalidFilterNames();
        
        // Utility methods
        std::string GetStatisticsSummary() const;
        void SetAllFiltersActive(bool active);
        
        // Filter persistence
        Result SaveFiltersToFile(const std::string& filename) const;
        Result LoadFiltersFromFile(const std::string& filename);
        std::string SerializeFiltersToJson() const;
        Result DeserializeFiltersFromJson(const std::string& json_data);
        
    private:
        // Internal helper methods
        bool CompileAndCacheRegex(const std::string& pattern);
        void UpdateMatchCounts(const LogEntry& entry, const std::vector<const Filter*>& matching_filters);
        size_t FindMatchingBrace(const std::string& json_data, size_t start_pos) const;
    };
    
} // namespace ue_log