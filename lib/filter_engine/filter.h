#pragma once

#include "../../macros.h"
#include "../log_parser/log_entry.h"
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <optional>

namespace ue_log {
    
    enum class FilterType {
        TextContains,     // Message contains text
        TextExact,        // Message exact match
        TextRegex,        // Message regex match
        LoggerName,       // Logger name match
        LogLevel,         // Log level match
        TimeRange,        // Timestamp range
        FrameRange        // Frame number range
    };
    
    enum class FilterState {
        INCLUDE = 0,      // Show only entries that match (green checkmark)
        EXCLUDE = 1,      // Hide entries that match (red negative sign)
        DISABLED = 2      // Ignore filter completely (greyed out)
    };
    
    enum class FilterLogic {
        And,              // All sub-filters must match
        Or                // Any sub-filter must match
    };
    
    class Filter {
        CK_GENERATED_BODY(Filter);
        
    private:
        std::string name;
        FilterType type;
        std::string criteria;
        FilterState filter_state;
        bool is_active; // Kept for backward compatibility
        std::vector<std::unique_ptr<Filter>> sub_filters;
        FilterLogic logic;
        std::string highlight_color;
        size_t match_count;
        
        // Compiled regex for performance (when using TextRegex)
        mutable std::optional<std::regex> compiled_regex;
        mutable bool regex_compilation_attempted;
        
    public:
        // Default constructor
        Filter() : type(FilterType::TextContains), filter_state(FilterState::INCLUDE), 
                  is_active(true), logic(FilterLogic::And), match_count(0), 
                  regex_compilation_attempted(false) {}
        
        // Main constructor
        Filter(const std::string& filter_name, FilterType filter_type, const std::string& filter_criteria)
            : name(filter_name), type(filter_type), criteria(filter_criteria), 
              filter_state(FilterState::INCLUDE), is_active(true), logic(FilterLogic::And), highlight_color(""), 
              match_count(0), regex_compilation_attempted(false) {}
        
        // Properties using macros
        CK_PROPERTY(name);
        CK_PROPERTY(type);
        CK_PROPERTY(criteria);
        CK_PROPERTY(filter_state);
        CK_PROPERTY(logic);
        CK_PROPERTY(highlight_color);
        CK_PROPERTY_GET(match_count);
        
        // Manual is_active property for backward compatibility
        const bool& Get_is_active() const { return is_active; }
        bool& Get_is_active() { return is_active; }
        auto Request_is_active(const bool& value) -> Filter& { 
            SetActive(value); 
            return *this; 
        }
        auto Updateis_active(std::function<void(bool&)> func) -> Filter& {
            func(is_active);
            SetFilterState(is_active ? FilterState::INCLUDE : FilterState::DISABLED);
            return *this;
        }
        
        // Filter state management
        FilterState GetFilterState() const { return filter_state; }
        void SetFilterState(FilterState state) { 
            filter_state = state; 
            is_active = (state != FilterState::DISABLED); // Keep in sync
        }
        void CycleFilterState();
        
        // Backward compatibility methods for existing is_active API
        bool IsActive() const { return filter_state != FilterState::DISABLED; }
        void SetActive(bool active) { 
            filter_state = active ? FilterState::INCLUDE : FilterState::DISABLED;
            is_active = active; // Keep in sync
        }
        
        // New matching helper methods
        bool ShouldInclude(const LogEntry& entry) const;
        bool ShouldExclude(const LogEntry& entry) const;
        
        // Sub-filter management
        void AddSubFilter(std::unique_ptr<Filter> sub_filter);
        void RemoveSubFilter(const std::string& sub_filter_name);
        Filter* FindSubFilter(const std::string& sub_filter_name);
        const std::vector<std::unique_ptr<Filter>>& GetSubFilters() const;
        size_t GetSubFilterCount() const;
        
        // Filtering functionality
        bool Matches(const LogEntry& entry) const;
        void IncrementMatchCount();
        void ResetMatchCount();
        
        // Validation
        bool IsValid() const;
        std::string GetValidationError() const;
        
        // Utility methods
        std::string ToString() const;
        
        // Serialization
        std::string ToJson() const;
        static std::unique_ptr<Filter> FromJson(const std::string& json_data);
        
    private:
        // JSON helper methods
        std::string EscapeJsonString(const std::string& str) const;
        static std::string UnescapeJsonString(const std::string& str);
        
    private:
        // Internal matching methods for different filter types
        bool MatchesTextContains(const LogEntry& entry) const;
        bool MatchesTextExact(const LogEntry& entry) const;
        bool MatchesTextRegex(const LogEntry& entry) const;
        bool MatchesLoggerName(const LogEntry& entry) const;
        bool MatchesLogLevel(const LogEntry& entry) const;
        bool MatchesTimeRange(const LogEntry& entry) const;
        bool MatchesFrameRange(const LogEntry& entry) const;
        
        // Helper methods
        const std::regex& GetCompiledRegex() const;
        bool CompileRegex() const;
        bool MatchesInternal(const LogEntry& entry) const;
    };
    
} // namespace ue_log