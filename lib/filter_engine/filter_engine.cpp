#include "filter_engine.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>

namespace ue_log {
    
    FilterEngine::FilterEngine() 
        : total_entries_processed(0), total_matches_found(0) {
    }
    
    Result FilterEngine::AddFilter(std::unique_ptr<Filter> filter) {
        if (!filter) {
            return Result::Error(ErrorCode::InvalidLogFormat, "Cannot add null filter");
        }
        
        if (!filter->IsValid()) {
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Cannot add invalid filter: " + filter->GetValidationError());
        }
        
        // Check for duplicate names
        const std::string& name = filter->Get_name();
        if (FindFilter(name) != nullptr) {
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Filter with name '" + name + "' already exists");
        }
        
        primary_filters.push_back(std::move(filter));
        return Result::Success();
    }
    
    Result FilterEngine::RemoveFilter(const std::string& filter_name) {
        auto it = std::find_if(primary_filters.begin(), primary_filters.end(),
            [&filter_name](const std::unique_ptr<Filter>& filter) {
                return filter && filter->Get_name() == filter_name;
            });
        
        if (it == primary_filters.end()) {
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Filter '" + filter_name + "' not found");
        }
        
        primary_filters.erase(it);
        return Result::Success();
    }
    
    Filter* FilterEngine::FindFilter(const std::string& filter_name) {
        auto it = std::find_if(primary_filters.begin(), primary_filters.end(),
            [&filter_name](const std::unique_ptr<Filter>& filter) {
                return filter && filter->Get_name() == filter_name;
            });
        
        if (it != primary_filters.end()) {
            return it->get();
        }
        
        // Search in sub-filters recursively
        for (const auto& filter : primary_filters) {
            if (filter) {
                Filter* found = filter->FindSubFilter(filter_name);
                if (found) {
                    return found;
                }
            }
        }
        
        return nullptr;
    }
    
    void FilterEngine::ClearAllFilters() {
        primary_filters.clear();
        ClearRegexCache();
        ResetAllStatistics();
    }
    
    size_t FilterEngine::GetFilterCount() const {
        return primary_filters.size();
    }
    
    std::vector<LogEntry> FilterEngine::ApplyFilters(const std::vector<LogEntry>& entries) {
        std::vector<LogEntry> filtered_entries;
        
        // If no active filters, return all entries
        if (GetTotalActiveFilters() == 0) {
            return entries;
        }
        
        for (const LogEntry& entry : entries) {
            total_entries_processed++;
            
            if (PassesFilters(entry)) {
                filtered_entries.push_back(entry);
                total_matches_found++;
                
                // Update match counts for matching filters
                std::vector<const Filter*> matching_filters = GetMatchingFilters(entry);
                UpdateMatchCounts(entry, matching_filters);
            }
        }
        
        return filtered_entries;
    }
    
    bool FilterEngine::PassesFilters(const LogEntry& entry) {
        // If no active filters, entry passes
        if (GetTotalActiveFilters() == 0) {
            return true;
        }
        
        // Entry passes if ANY primary filter matches (OR logic between primary filters)
        for (const auto& filter : primary_filters) {
            if (filter && filter->Get_is_active() && filter->Matches(entry)) {
                return true;
            }
        }
        
        return false;
    }
    
    std::vector<const Filter*> FilterEngine::GetMatchingFilters(const LogEntry& entry) {
        std::vector<const Filter*> matching_filters;
        
        for (const auto& filter : primary_filters) {
            if (filter && filter->Get_is_active() && filter->Matches(entry)) {
                matching_filters.push_back(filter.get());
            }
        }
        
        return matching_filters;
    }
    
    void FilterEngine::UpdateFilterStatistics(const std::vector<LogEntry>& entries) {
        for (const LogEntry& entry : entries) {
            total_entries_processed++;
            
            std::vector<const Filter*> matching_filters = GetMatchingFilters(entry);
            if (!matching_filters.empty()) {
                total_matches_found++;
                UpdateMatchCounts(entry, matching_filters);
            }
        }
    }
    
    void FilterEngine::ResetAllStatistics() {
        total_entries_processed = 0;
        total_matches_found = 0;
        
        for (auto& filter : primary_filters) {
            if (filter) {
                filter->ResetMatchCount();
            }
        }
    }
    
    size_t FilterEngine::GetTotalActiveFilters() const {
        size_t active_count = 0;
        
        for (const auto& filter : primary_filters) {
            if (filter && filter->Get_is_active()) {
                active_count++;
            }
        }
        
        return active_count;
    }
    
    std::vector<Filter*> FilterEngine::GetActiveFilters() {
        std::vector<Filter*> active_filters;
        
        for (const auto& filter : primary_filters) {
            if (filter && filter->Get_is_active()) {
                active_filters.push_back(filter.get());
            }
        }
        
        return active_filters;
    }
    
    const std::regex& FilterEngine::GetCachedRegex(const std::string& pattern) {
        auto it = compiled_regex_cache.find(pattern);
        if (it != compiled_regex_cache.end()) {
            return it->second;
        }
        
        // Compile and cache the regex
        if (CompileAndCacheRegex(pattern)) {
            return compiled_regex_cache[pattern];
        }
        
        // Return a default regex that matches nothing if compilation failed
        static const std::regex empty_regex("(?!)"); // Negative lookahead that never matches
        return empty_regex;
    }
    
    void FilterEngine::ClearRegexCache() {
        compiled_regex_cache.clear();
    }
    
    size_t FilterEngine::GetRegexCacheSize() const {
        return compiled_regex_cache.size();
    }
    
    Result FilterEngine::ValidateAllFilters() {
        std::vector<std::string> invalid_filters;
        
        for (const auto& filter : primary_filters) {
            if (filter && !filter->IsValid()) {
                invalid_filters.push_back(filter->Get_name() + ": " + filter->GetValidationError());
            }
        }
        
        if (!invalid_filters.empty()) {
            std::ostringstream oss;
            oss << "Invalid filters found: ";
            for (size_t i = 0; i < invalid_filters.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << invalid_filters[i];
            }
            return Result::Error(ErrorCode::InvalidLogFormat, oss.str());
        }
        
        return Result::Success();
    }
    
    std::vector<std::string> FilterEngine::GetInvalidFilterNames() {
        std::vector<std::string> invalid_names;
        
        for (const auto& filter : primary_filters) {
            if (filter && !filter->IsValid()) {
                invalid_names.push_back(filter->Get_name());
            }
        }
        
        return invalid_names;
    }
    
    std::string FilterEngine::GetStatisticsSummary() const {
        std::ostringstream oss;
        oss << "FilterEngine Statistics:\n";
        oss << "  Total Filters: " << primary_filters.size() << "\n";
        oss << "  Active Filters: " << GetTotalActiveFilters() << "\n";
        oss << "  Entries Processed: " << total_entries_processed << "\n";
        oss << "  Total Matches: " << total_matches_found << "\n";
        oss << "  Regex Cache Size: " << compiled_regex_cache.size() << "\n";
        
        if (total_entries_processed > 0) {
            double match_rate = (static_cast<double>(total_matches_found) / total_entries_processed) * 100.0;
            oss << "  Match Rate: " << std::fixed << std::setprecision(2) << match_rate << "%\n";
        }
        
        // Individual filter statistics
        if (!primary_filters.empty()) {
            oss << "\nFilter Details:\n";
            for (const auto& filter : primary_filters) {
                if (filter) {
                    oss << "  - " << filter->Get_name() 
                        << " (" << (filter->Get_is_active() ? "Active" : "Inactive") << "): "
                        << filter->Get_match_count() << " matches\n";
                }
            }
        }
        
        return oss.str();
    }
    
    void FilterEngine::SetAllFiltersActive(bool active) {
        for (auto& filter : primary_filters) {
            if (filter) {
                filter->Request_is_active(active);
            }
        }
    }
    
    // Filter persistence methods
    
    Result FilterEngine::SaveFiltersToFile(const std::string& filename) const {
        try {
            std::string json_data = SerializeFiltersToJson();
            
            std::ofstream file(filename);
            if (!file.is_open()) {
                return Result::Error(ErrorCode::FileNotFound, 
                    "Cannot open file for writing: " + filename);
            }
            
            file << json_data;
            file.close();
            
            if (file.fail()) {
                return Result::Error(ErrorCode::FileNotFound, 
                    "Failed to write to file: " + filename);
            }
            
            return Result::Success();
        } catch (const std::exception& e) {
            return Result::Error(ErrorCode::FileNotFound, 
                "Exception while saving filters: " + std::string(e.what()));
        }
    }
    
    Result FilterEngine::LoadFiltersFromFile(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                return Result::Error(ErrorCode::FileNotFound, 
                    "Cannot open file for reading: " + filename);
            }
            
            std::string json_data((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
            file.close();
            
            return DeserializeFiltersFromJson(json_data);
        } catch (const std::exception& e) {
            return Result::Error(ErrorCode::FileNotFound, 
                "Exception while loading filters: " + std::string(e.what()));
        }
    }
    
    std::string FilterEngine::SerializeFiltersToJson() const {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"version\": \"1.0\",\n";
        oss << "  \"total_entries_processed\": " << total_entries_processed << ",\n";
        oss << "  \"total_matches_found\": " << total_matches_found << ",\n";
        oss << "  \"filters\": [\n";
        
        for (size_t i = 0; i < primary_filters.size(); ++i) {
            if (primary_filters[i]) {
                oss << "    " << primary_filters[i]->ToJson();
                if (i < primary_filters.size() - 1) {
                    oss << ",";
                }
                oss << "\n";
            }
        }
        
        oss << "  ]\n";
        oss << "}";
        
        return oss.str();
    }
    
    Result FilterEngine::DeserializeFiltersFromJson(const std::string& json_data) {
        try {
            // Clear existing filters
            ClearAllFilters();
            
            // Extract version (for future compatibility)
            size_t version_start = json_data.find("\"version\": \"") + 12;
            size_t version_end = json_data.find("\"", version_start);
            std::string version;
            if (version_start != std::string::npos && version_end != std::string::npos) {
                version = json_data.substr(version_start, version_end - version_start);
            }
            
            // Extract statistics
            size_t processed_start = json_data.find("\"total_entries_processed\": ") + 27;
            size_t processed_end = json_data.find(",", processed_start);
            if (processed_start != std::string::npos && processed_end != std::string::npos) {
                total_entries_processed = std::stoull(json_data.substr(processed_start, processed_end - processed_start));
            }
            
            size_t matches_start = json_data.find("\"total_matches_found\": ") + 23;
            size_t matches_end = json_data.find(",", matches_start);
            if (matches_start != std::string::npos && matches_end != std::string::npos) {
                total_matches_found = std::stoull(json_data.substr(matches_start, matches_end - matches_start));
            }
            
            // Extract filters array
            size_t filters_start = json_data.find("\"filters\": [");
            if (filters_start == std::string::npos) {
                return Result::Error(ErrorCode::InvalidLogFormat, "No filters array found in JSON");
            }
            
            // Find and parse each filter object in the array
            size_t search_pos = filters_start;
            
            while (search_pos < json_data.length()) {
                // Find the next opening brace for a filter object
                size_t filter_start = json_data.find('{', search_pos);
                if (filter_start == std::string::npos) {
                    break; // No more filter objects
                }
                
                // Find the matching closing brace
                size_t filter_end = FindMatchingBrace(json_data, filter_start);
                if (filter_end == std::string::npos) {
                    break; // Malformed JSON
                }
                
                // Extract the filter JSON and parse it
                std::string filter_json = json_data.substr(filter_start, filter_end - filter_start + 1);
                auto filter = Filter::FromJson(filter_json);
                if (filter && filter->IsValid()) {
                    Result add_result = AddFilter(std::move(filter));
                    if (add_result.IsError()) {
                        return add_result;
                    }
                }
                
                // Move search position past this filter
                search_pos = filter_end + 1;
                
                // Check if we've reached the end of the filters array
                size_t next_brace = json_data.find(']', search_pos);
                if (next_brace != std::string::npos && next_brace < json_data.find('{', search_pos)) {
                    break; // End of filters array
                }
            }
            
            return Result::Success();
        } catch (const std::exception& e) {
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Exception while deserializing filters: " + std::string(e.what()));
        }
    }
    
    // Private helper methods
    
    bool FilterEngine::CompileAndCacheRegex(const std::string& pattern) {
        try {
            std::regex compiled_regex(pattern, std::regex_constants::icase);
            compiled_regex_cache[pattern] = std::move(compiled_regex);
            return true;
        } catch (const std::regex_error&) {
            return false;
        }
    }
    
    void FilterEngine::UpdateMatchCounts(const LogEntry& entry, const std::vector<const Filter*>& matching_filters) {
        // Cast away const to update match counts (this is safe as we're the owner)
        for (const Filter* filter : matching_filters) {
            const_cast<Filter*>(filter)->IncrementMatchCount();
        }
    }
    
    size_t FilterEngine::FindMatchingBrace(const std::string& json_data, size_t start_pos) const {
        if (start_pos >= json_data.length() || json_data[start_pos] != '{') {
            return std::string::npos;
        }
        
        int brace_count = 0;
        bool in_string = false;
        bool escape_next = false;
        
        for (size_t i = start_pos; i < json_data.length(); ++i) {
            char c = json_data[i];
            
            if (escape_next) {
                escape_next = false;
            } else if (c == '\\') {
                escape_next = true;
            } else if (c == '"' && !escape_next) {
                in_string = !in_string;
            } else if (!in_string) {
                if (c == '{') {
                    brace_count++;
                } else if (c == '}') {
                    brace_count--;
                    if (brace_count == 0) {
                        return i;
                    }
                }
            }
        }
        
        return std::string::npos;
    }
    
} // namespace ue_log