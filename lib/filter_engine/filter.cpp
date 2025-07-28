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
    
    void Filter::CycleFilterState() {
        switch (filter_state) {
            case FilterState::INCLUDE:
                filter_state = FilterState::EXCLUDE;
                break;
            case FilterState::EXCLUDE:
                filter_state = FilterState::DISABLED;
                break;
            case FilterState::DISABLED:
                filter_state = FilterState::INCLUDE;
                break;
        }
        is_active = (filter_state != FilterState::DISABLED); // Keep in sync
    }
    
    bool Filter::ShouldInclude(const LogEntry& entry) const {
        if (filter_state != FilterState::INCLUDE) {
            return false;
        }
        return MatchesInternal(entry);
    }
    
    bool Filter::ShouldExclude(const LogEntry& entry) const {
        if (filter_state != FilterState::EXCLUDE) {
            return false;
        }
        return MatchesInternal(entry);
    }
    
    bool Filter::Matches(const LogEntry& entry) const {
        if (filter_state == FilterState::DISABLED) {
            return false;
        }

        auto result = MatchesInternal(entry);

        return filter_state == FilterState::EXCLUDE ? !result : result;
    }
    
    bool Filter::MatchesInternal(const LogEntry& entry) const {
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
        oss << " (Active: " << (IsActive() ? "Yes" : "No") << ")";
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
    
    // Serialization methods
    
    std::string Filter::ToJson() const {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"name\": \"" << EscapeJsonString(name) << "\",\n";
        oss << "  \"type\": " << static_cast<int>(type) << ",\n";
        oss << "  \"criteria\": \"" << EscapeJsonString(criteria) << "\",\n";
        oss << "  \"filter_state\": " << static_cast<int>(filter_state) << ",\n";
        oss << "  \"is_active\": " << (IsActive() ? "true" : "false") << ",\n";
        oss << "  \"logic\": " << static_cast<int>(logic) << ",\n";
        oss << "  \"highlight_color\": \"" << EscapeJsonString(highlight_color) << "\",\n";
        oss << "  \"match_count\": " << match_count << ",\n";
        oss << "  \"sub_filters\": [\n";
        
        for (size_t i = 0; i < sub_filters.size(); ++i) {
            if (sub_filters[i]) {
                oss << "    " << sub_filters[i]->ToJson();
                if (i < sub_filters.size() - 1) {
                    oss << ",";
                }
                oss << "\n";
            }
        }
        
        oss << "  ]\n";
        oss << "}";
        
        return oss.str();
    }
    
    std::unique_ptr<Filter> Filter::FromJson(const std::string& json_data) {
        // Simple JSON parsing - in a real implementation, you'd use a proper JSON library
        // For now, this is a basic implementation for testing purposes
        
        auto filter = std::make_unique<Filter>();
        
        // Extract name
        size_t name_start = json_data.find("\"name\": \"") + 9;
        size_t name_end = json_data.find("\"", name_start);
        if (name_start != std::string::npos && name_end != std::string::npos) {
            filter->name = UnescapeJsonString(json_data.substr(name_start, name_end - name_start));
        }
        
        // Extract type
        size_t type_start = json_data.find("\"type\": ") + 8;
        size_t type_end = json_data.find(",", type_start);
        if (type_start != std::string::npos && type_end != std::string::npos) {
            int type_value = std::stoi(json_data.substr(type_start, type_end - type_start));
            filter->type = static_cast<FilterType>(type_value);
        }
        
        // Extract criteria
        size_t criteria_start = json_data.find("\"criteria\": \"") + 13;
        size_t criteria_end = json_data.find("\"", criteria_start);
        if (criteria_start != std::string::npos && criteria_end != std::string::npos) {
            filter->criteria = UnescapeJsonString(json_data.substr(criteria_start, criteria_end - criteria_start));
        }
        
        // Extract filter_state (new format) or is_active (backward compatibility)
        size_t state_start = json_data.find("\"filter_state\": ");
        if (state_start != std::string::npos) {
            // New format with filter_state
            state_start += 16;
            size_t state_end = json_data.find(",", state_start);
            if (state_end != std::string::npos) {
                int state_value = std::stoi(json_data.substr(state_start, state_end - state_start));
                filter->filter_state = static_cast<FilterState>(state_value);
                filter->is_active = (filter->filter_state != FilterState::DISABLED); // Keep in sync
            }
        } else {
            // Backward compatibility with is_active
            size_t active_start = json_data.find("\"is_active\": ");
            if (active_start != std::string::npos) {
                active_start += 13;
                bool is_active_value = json_data.substr(active_start, 4) == "true";
                filter->filter_state = is_active_value ? FilterState::INCLUDE : FilterState::DISABLED;
                filter->is_active = is_active_value; // Keep in sync
            }
        }
        
        // Extract logic
        size_t logic_start = json_data.find("\"logic\": ") + 9;
        size_t logic_end = json_data.find(",", logic_start);
        if (logic_start != std::string::npos && logic_end != std::string::npos) {
            int logic_value = std::stoi(json_data.substr(logic_start, logic_end - logic_start));
            filter->logic = static_cast<FilterLogic>(logic_value);
        }
        
        // Extract highlight_color
        size_t color_start = json_data.find("\"highlight_color\": \"") + 20;
        size_t color_end = json_data.find("\"", color_start);
        if (color_start != std::string::npos && color_end != std::string::npos) {
            filter->highlight_color = UnescapeJsonString(json_data.substr(color_start, color_end - color_start));
        }
        
        // Extract match_count
        size_t count_start = json_data.find("\"match_count\": ") + 15;
        size_t count_end = json_data.find(",", count_start);
        if (count_start != std::string::npos && count_end != std::string::npos) {
            filter->match_count = std::stoull(json_data.substr(count_start, count_end - count_start));
        }
        
        // TODO: Parse sub_filters (would require more complex JSON parsing)
        // For now, we'll skip sub-filter parsing in this basic implementation
        
        return filter;
    }
    
    std::string Filter::EscapeJsonString(const std::string& str) const {
        std::string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }
    
    std::string Filter::UnescapeJsonString(const std::string& str) {
        std::string unescaped;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case '"': unescaped += '"'; i++; break;
                    case '\\': unescaped += '\\'; i++; break;
                    case 'n': unescaped += '\n'; i++; break;
                    case 'r': unescaped += '\r'; i++; break;
                    case 't': unescaped += '\t'; i++; break;
                    default: unescaped += str[i]; break;
                }
            } else {
                unescaped += str[i];
            }
        }
        return unescaped;
    }
    
} // namespace ue_log