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
        bool is_active;
        std::vector<std::unique_ptr<Filter>> sub_filters;
        FilterLogic logic;
        std::string highlight_color;
        size_t match_count;
        
        // Compiled regex for performance (when using TextRegex)
        mutable std::optional<std::regex> compiled_regex;
        mutable bool regex_compilation_attempted;
        
    public:
        // Default constructor
        Filter() : type(FilterType::TextContains), is_active(true), 
                  logic(FilterLogic::And), match_count(0), 
                  regex_compilation_attempted(false) {}
        
        // Main constructor
        Filter(const std::string& filter_name, FilterType filter_type, const std::string& filter_criteria)
            : name(filter_name), type(filter_type), criteria(filter_criteria), 
              is_active(true), logic(FilterLogic::And), highlight_color(""), 
              match_count(0), regex_compilation_attempted(false) {}
        
        // Properties using macros
        CK_PROPERTY(name);
        CK_PROPERTY(type);
        CK_PROPERTY(criteria);
        CK_PROPERTY(is_active);
        CK_PROPERTY(logic);
        CK_PROPERTY(highlight_color);
        CK_PROPERTY_GET(match_count);
        
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
    };
    
} // namespace ue_log