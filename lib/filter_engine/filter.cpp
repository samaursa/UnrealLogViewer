#include "filter.h"
#include <algorithm>
#include <sstream>

namespace ue_log {
    
    void Filter::AddSubFilter(std::unique_ptr<Filter> sub_filter) {
        if (sub_filter) {
            sub_filters.push_back(std::move(sub_filter));
        }
    }
    
    void Filter::RemoveSubFilter(const std::string& sub_filter_name) {
        sub_filters.erase(
            std::remove_if(sub_filters.begin(), sub_filters.end(),
                [&sub_filter_name](const std::unique_ptr<Filter>& filter) {
                    return filter && filter->Get_name() == sub_filter_name;
                }),
            sub_filters.end()
        );
    }
    
    Filter* Filter::FindSubFilter(const std::string& sub_filter_name) {
        auto it = std::find_if(sub_filters.begin(), sub_filters.end(),
            [&sub_filter_name](const std::unique_ptr<Filter>& filter) {
                return filter && filter->Get_name() == sub_filter_name;
            });
        
        if (it != sub_filters.end()) {
            return it->get();
        }
        
        // Search recursively in sub-filters
        for (const auto& sub_filter : sub_filters) {
            if (sub_filter) {
                Filter* found = sub_filter->FindSubFilter(sub_filter_name);
                if (found) {
                    return found;
                }
            }
        }
        
        return nullptr;
    }
    
    const std::vector<std::unique_ptr<Filter>>& Filter::GetSubFilters() const {
        return sub_filters;
    }
    
    size_t Filter::GetSubFilterCount() const {
        return sub_filters.size();
    }
    
    bool Filter::Matches(const LogEntry& entry) const {
        if (!is_active) {
            return false;
        }
        
        // First check if this filter matches
        bool this_matches = false;
        
        switch (type) {
            case FilterType::TextContains:
                this_matches = MatchesTextContains(entry);
                break;
            case FilterType::TextExact:
                this_matches = MatchesTextExact(entry);
                break;
            case FilterType::TextRegex:
                this_matches = MatchesTextRegex(entry);
                break;
            case FilterType::LoggerName:
                this_matches = MatchesLoggerName(entry);
                break;
            case FilterType::LogLevel:
                this_matches = MatchesLogLevel(entry);
                break;
            case FilterType::TimeRange:
                this_matches = MatchesTimeRange(entry);
                break;
            case FilterType::FrameRange:
                this_matches = MatchesFrameRange(entry);
                break;
        }
        
        // If no sub-filters, return this filter's result
        if (sub_filters.empty()) {
            return this_matches;
        }
        
        // Apply sub-filter logic
        bool sub_filters_match = false;
        
        if (logic == FilterLogic::And) {
            // All sub-filters must match
            sub_filters_match = true;
            for (const auto& sub_filter : sub_filters) {
                if (sub_filter && !sub_filter->Matches(entry)) {
                    sub_filters_match = false;
                    break;
                }
            }
        } else { // FilterLogic::Or
            // Any sub-filter must match
            sub_filters_match = false;
            for (const auto& sub_filter : sub_filters) {
                if (sub_filter && sub_filter->Matches(entry)) {
                    sub_filters_match = true;
                    break;
                }
            }
        }
        
        // Combine this filter's result with sub-filters based on logic
        if (logic == FilterLogic::And) {
            return this_matches && sub_filters_match;
        } else {
            return this_matches || sub_filters_match;
        }
    }
    
    void Filter::IncrementMatchCount() {
        match_count++;
    }
    
    void Filter::ResetMatchCount() {
        match_count = 0;
        // Reset sub-filter counts too
        for (auto& sub_filter : sub_filters) {
            if (sub_filter) {
                sub_filter->ResetMatchCount();
            }
        }
    }
    
    bool Filter::IsValid() const {
        if (name.empty() || criteria.empty()) {
            return false;
        }
        
        // Validate regex if it's a regex filter
        if (type == FilterType::TextRegex) {
            return CompileRegex();
        }
        
        return true;
    }
    
    std::string Filter::GetValidationError() const {
        if (name.empty()) {
            return "Filter name cannot be empty";
        }
        
        if (criteria.empty()) {
            return "Filter criteria cannot be empty";
        }
        
        if (type == FilterType::TextRegex && !CompileRegex()) {
            return "Invalid regex pattern: " + criteria;
        }
        
        return "";
    }
    
    std::string Filter::ToString() const {
        std::ostringstream oss;
        oss << "Filter[" << name << "] ";
        
        switch (type) {
            case FilterType::TextContains: oss << "TextContains"; break;
            case FilterType::TextExact: oss << "TextExact"; break;
            case FilterType::TextRegex: oss << "TextRegex"; break;
            case FilterType::LoggerName: oss << "LoggerName"; break;
            case FilterType::LogLevel: oss << "LogLevel"; break;
            case FilterType::TimeRange: oss << "TimeRange"; break;
            case FilterType::FrameRange: oss << "FrameRange"; break;
        }
        
        oss << ": '" << criteria << "'";
        oss << " (Active: " << (is_active ? "Yes" : "No") << ")";
        oss << " (Matches: " << match_count << ")";
        
        if (!sub_filters.empty()) {
            oss << " [" << sub_filters.size() << " sub-filters]";
        }
        
        return oss.str();
    }
    
    // Private matching methods
    
    bool Filter::MatchesTextContains(const LogEntry& entry) const {
        const std::string& message = entry.Get_message();
        return message.find(criteria) != std::string::npos;
    }
    
    bool Filter::MatchesTextExact(const LogEntry& entry) const {
        return entry.Get_message() == criteria;
    }
    
    bool Filter::MatchesTextRegex(const LogEntry& entry) const {
        try {
            const std::regex& regex = GetCompiledRegex();
            return std::regex_search(entry.Get_message(), regex);
        } catch (const std::exception&) {
            return false;
        }
    }
    
    bool Filter::MatchesLoggerName(const LogEntry& entry) const {
        return entry.Get_logger_name() == criteria;
    }
    
    bool Filter::MatchesLogLevel(const LogEntry& entry) const {
        if (!entry.HasLogLevel()) {
            return false;
        }
        return entry.Get_log_level().value() == criteria;
    }
    
    bool Filter::MatchesTimeRange(const LogEntry& entry) const {
        if (!entry.HasTimestamp()) {
            return false;
        }
        
        // For now, simple string comparison
        // TODO: Implement proper timestamp range parsing
        const std::string& timestamp = entry.Get_timestamp().value();
        return timestamp.find(criteria) != std::string::npos;
    }
    
    bool Filter::MatchesFrameRange(const LogEntry& entry) const {
        if (!entry.HasFrameNumber()) {
            return false;
        }
        
        // Parse criteria as "min-max" or single number
        size_t dash_pos = criteria.find('-');
        if (dash_pos != std::string::npos) {
            // Range format: "100-200"
            try {
                int min_frame = std::stoi(criteria.substr(0, dash_pos));
                int max_frame = std::stoi(criteria.substr(dash_pos + 1));
                int entry_frame = entry.Get_frame_number().value();
                return entry_frame >= min_frame && entry_frame <= max_frame;
            } catch (const std::exception&) {
                return false;
            }
        } else {
            // Single frame number
            try {
                int target_frame = std::stoi(criteria);
                return entry.Get_frame_number().value() == target_frame;
            } catch (const std::exception&) {
                return false;
            }
        }
    }
    
    const std::regex& Filter::GetCompiledRegex() const {
        if (!compiled_regex.has_value() && !regex_compilation_attempted) {
            CompileRegex();
        }
        
        if (compiled_regex.has_value()) {
            return compiled_regex.value();
        }
        
        // Return a default regex that matches nothing if compilation failed
        static const std::regex empty_regex("(?!)"); // Negative lookahead that never matches
        return empty_regex;
    }
    
    bool Filter::CompileRegex() const {
        if (regex_compilation_attempted) {
            return compiled_regex.has_value();
        }
        
        regex_compilation_attempted = true;
        
        try {
            compiled_regex = std::regex(criteria, std::regex_constants::icase);
            return true;
        } catch (const std::regex_error&) {
            compiled_regex.reset();
            return false;
        }
    }
    
} // namespace ue_log