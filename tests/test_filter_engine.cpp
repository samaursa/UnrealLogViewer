#include <catch2/catch_test_macros.hpp>
#include "../lib/filter_engine/filter.h"
#include "../lib/filter_engine/filter_engine.h"
#include "../lib/log_parser/log_entry.h"

using namespace ue_log;

TEST_CASE("Filter default constructor initializes correctly", "[filter]") {
    Filter filter;
    
    REQUIRE(filter.Get_name().empty());
    REQUIRE(filter.Get_type() == FilterType::TextContains);
    REQUIRE(filter.Get_criteria().empty());
    REQUIRE(filter.Get_is_active() == true);
    REQUIRE(filter.Get_logic() == FilterLogic::And);
    REQUIRE(filter.Get_highlight_color().empty());
    REQUIRE(filter.Get_match_count() == 0);
    REQUIRE(filter.GetSubFilterCount() == 0);
    REQUIRE_FALSE(filter.IsValid()); // Invalid because name and criteria are empty
}

TEST_CASE("Filter main constructor works correctly", "[filter]") {
    Filter filter("TestFilter", FilterType::LoggerName, "LogTemp");
    
    REQUIRE(filter.Get_name() == "TestFilter");
    REQUIRE(filter.Get_type() == FilterType::LoggerName);
    REQUIRE(filter.Get_criteria() == "LogTemp");
    REQUIRE(filter.Get_is_active() == true);
    REQUIRE(filter.Get_logic() == FilterLogic::And);
    REQUIRE(filter.Get_match_count() == 0);
    REQUIRE(filter.IsValid());
}

TEST_CASE("Filter property setters work correctly", "[filter]") {
    Filter filter;
    
    filter.Request_name("MyFilter");
    filter.Request_type(FilterType::TextRegex);
    filter.Request_criteria("test.*pattern");
    filter.Request_is_active(false);
    filter.Request_logic(FilterLogic::Or);
    filter.Request_highlight_color("#FF0000");
    
    REQUIRE(filter.Get_name() == "MyFilter");
    REQUIRE(filter.Get_type() == FilterType::TextRegex);
    REQUIRE(filter.Get_criteria() == "test.*pattern");
    REQUIRE(filter.Get_is_active() == false);
    REQUIRE(filter.Get_logic() == FilterLogic::Or);
    REQUIRE(filter.Get_highlight_color() == "#FF0000");
}

TEST_CASE("Filter validation works correctly", "[filter]") {
    SECTION("Valid filter") {
        Filter filter("ValidFilter", FilterType::TextContains, "test");
        REQUIRE(filter.IsValid());
        REQUIRE(filter.GetValidationError().empty());
    }
    
    SECTION("Invalid filter - empty name") {
        Filter filter("", FilterType::TextContains, "test");
        REQUIRE_FALSE(filter.IsValid());
        REQUIRE(filter.GetValidationError() == "Filter name cannot be empty");
    }
    
    SECTION("Invalid filter - empty criteria") {
        Filter filter("TestFilter", FilterType::TextContains, "");
        REQUIRE_FALSE(filter.IsValid());
        REQUIRE(filter.GetValidationError() == "Filter criteria cannot be empty");
    }
    
    SECTION("Invalid regex filter") {
        Filter filter("RegexFilter", FilterType::TextRegex, "[invalid regex");
        REQUIRE_FALSE(filter.IsValid());
        REQUIRE(filter.GetValidationError().find("Invalid regex pattern") != std::string::npos);
    }
    
    SECTION("Valid regex filter") {
        Filter filter("RegexFilter", FilterType::TextRegex, "test.*pattern");
        REQUIRE(filter.IsValid());
        REQUIRE(filter.GetValidationError().empty());
    }
}

TEST_CASE("Filter sub-filter management works correctly", "[filter]") {
    Filter parent_filter("ParentFilter", FilterType::LoggerName, "LogTemp");
    
    SECTION("Add sub-filters") {
        auto sub_filter1 = std::make_unique<Filter>("SubFilter1", FilterType::TextContains, "error");
        auto sub_filter2 = std::make_unique<Filter>("SubFilter2", FilterType::LogLevel, "Warning");
        
        parent_filter.AddSubFilter(std::move(sub_filter1));
        parent_filter.AddSubFilter(std::move(sub_filter2));
        
        REQUIRE(parent_filter.GetSubFilterCount() == 2);
        REQUIRE(parent_filter.GetSubFilters().size() == 2);
    }
    
    SECTION("Find sub-filters") {
        auto sub_filter = std::make_unique<Filter>("FindMe", FilterType::TextContains, "test");
        parent_filter.AddSubFilter(std::move(sub_filter));
        
        Filter* found = parent_filter.FindSubFilter("FindMe");
        REQUIRE(found != nullptr);
        REQUIRE(found->Get_name() == "FindMe");
        
        Filter* not_found = parent_filter.FindSubFilter("NotThere");
        REQUIRE(not_found == nullptr);
    }
    
    SECTION("Remove sub-filters") {
        auto sub_filter = std::make_unique<Filter>("RemoveMe", FilterType::TextContains, "test");
        parent_filter.AddSubFilter(std::move(sub_filter));
        
        REQUIRE(parent_filter.GetSubFilterCount() == 1);
        
        parent_filter.RemoveSubFilter("RemoveMe");
        REQUIRE(parent_filter.GetSubFilterCount() == 0);
    }
    
    SECTION("Nested sub-filter search") {
        auto sub_filter = std::make_unique<Filter>("Level1", FilterType::TextContains, "test");
        auto nested_filter = std::make_unique<Filter>("Level2", FilterType::LogLevel, "Error");
        
        sub_filter->AddSubFilter(std::move(nested_filter));
        parent_filter.AddSubFilter(std::move(sub_filter));
        
        Filter* found = parent_filter.FindSubFilter("Level2");
        REQUIRE(found != nullptr);
        REQUIRE(found->Get_name() == "Level2");
    }
}

TEST_CASE("Filter TextContains matching works correctly", "[filter]") {
    Filter filter("ContainsFilter", FilterType::TextContains, "error");
    
    LogEntry matching_entry("LogTemp", "This is an error message", "raw line");
    LogEntry non_matching_entry("LogTemp", "This is a warning message", "raw line");
    
    REQUIRE(filter.Matches(matching_entry));
    REQUIRE_FALSE(filter.Matches(non_matching_entry));
}

TEST_CASE("Filter TextExact matching works correctly", "[filter]") {
    Filter filter("ExactFilter", FilterType::TextExact, "exact message");
    
    LogEntry matching_entry("LogTemp", "exact message", "raw line");
    LogEntry non_matching_entry("LogTemp", "exact message with extra", "raw line");
    
    REQUIRE(filter.Matches(matching_entry));
    REQUIRE_FALSE(filter.Matches(non_matching_entry));
}

TEST_CASE("Filter TextRegex matching works correctly", "[filter]") {
    Filter filter("RegexFilter", FilterType::TextRegex, "error.*code.*\\d+");
    
    LogEntry matching_entry("LogTemp", "error with code 123", "raw line");
    LogEntry non_matching_entry("LogTemp", "error without code", "raw line");
    
    REQUIRE(filter.Matches(matching_entry));
    REQUIRE_FALSE(filter.Matches(non_matching_entry));
}

TEST_CASE("Filter LoggerName matching works correctly", "[filter]") {
    Filter filter("LoggerFilter", FilterType::LoggerName, "LogTemp");
    
    LogEntry matching_entry("LogTemp", "test message", "raw line");
    LogEntry non_matching_entry("LogEngine", "test message", "raw line");
    
    REQUIRE(filter.Matches(matching_entry));
    REQUIRE_FALSE(filter.Matches(non_matching_entry));
}

TEST_CASE("Filter LogLevel matching works correctly", "[filter]") {
    Filter filter("LevelFilter", FilterType::LogLevel, "Error");
    
    LogEntry matching_entry(
        LogEntryType::Structured,
        std::string("2025.07.16-10.01.25:951"),
        425,
        "LogTemp",
        std::string("Error"),
        "test message",
        "raw line",
        1
    );
    
    LogEntry non_matching_entry(
        LogEntryType::Structured,
        std::string("2025.07.16-10.01.25:951"),
        425,
        "LogTemp",
        std::string("Warning"),
        "test message",
        "raw line",
        1
    );
    
    LogEntry no_level_entry("LogTemp", "test message", "raw line");
    
    REQUIRE(filter.Matches(matching_entry));
    REQUIRE_FALSE(filter.Matches(non_matching_entry));
    REQUIRE_FALSE(filter.Matches(no_level_entry));
}

TEST_CASE("Filter FrameRange matching works correctly", "[filter]") {
    SECTION("Single frame number") {
        Filter filter("FrameFilter", FilterType::FrameRange, "425");
        
        LogEntry matching_entry(
            LogEntryType::Structured,
            std::string("2025.07.16-10.01.25:951"),
            425,
            "LogTemp",
            std::string("Info"),
            "test message",
            "raw line",
            1
        );
        
        LogEntry non_matching_entry(
            LogEntryType::Structured,
            std::string("2025.07.16-10.01.25:951"),
            500,
            "LogTemp",
            std::string("Info"),
            "test message",
            "raw line",
            1
        );
        
        REQUIRE(filter.Matches(matching_entry));
        REQUIRE_FALSE(filter.Matches(non_matching_entry));
    }
    
    SECTION("Frame range") {
        Filter filter("RangeFilter", FilterType::FrameRange, "400-450");
        
        LogEntry in_range_entry(
            LogEntryType::Structured,
            std::string("2025.07.16-10.01.25:951"),
            425,
            "LogTemp",
            std::string("Info"),
            "test message",
            "raw line",
            1
        );
        
        LogEntry out_of_range_entry(
            LogEntryType::Structured,
            std::string("2025.07.16-10.01.25:951"),
            500,
            "LogTemp",
            std::string("Info"),
            "test message",
            "raw line",
            1
        );
        
        REQUIRE(filter.Matches(in_range_entry));
        REQUIRE_FALSE(filter.Matches(out_of_range_entry));
    }
}

TEST_CASE("Filter inactive state works correctly", "[filter]") {
    Filter filter("InactiveFilter", FilterType::TextContains, "test");
    filter.Request_is_active(false);
    
    LogEntry matching_entry("LogTemp", "test message", "raw line");
    
    // Should not match when inactive
    REQUIRE_FALSE(filter.Matches(matching_entry));
    
    // Should match when activated
    filter.Request_is_active(true);
    REQUIRE(filter.Matches(matching_entry));
}

TEST_CASE("Filter match counting works correctly", "[filter]") {
    Filter filter("CountFilter", FilterType::TextContains, "test");
    
    REQUIRE(filter.Get_match_count() == 0);
    
    filter.IncrementMatchCount();
    filter.IncrementMatchCount();
    filter.IncrementMatchCount();
    
    REQUIRE(filter.Get_match_count() == 3);
    
    filter.ResetMatchCount();
    REQUIRE(filter.Get_match_count() == 0);
}

TEST_CASE("Filter sub-filter logic works correctly", "[filter]") {
    LogEntry test_entry(
        LogEntryType::Structured,
        std::string("2025.07.16-10.01.25:951"),
        425,
        "LogTemp",
        std::string("Error"),
        "test error message",
        "raw line",
        1
    );
    
    SECTION("AND logic - all sub-filters must match") {
        Filter parent_filter("ParentAND", FilterType::LoggerName, "LogTemp");
        parent_filter.Request_logic(FilterLogic::And);
        
        auto sub_filter1 = std::make_unique<Filter>("Sub1", FilterType::TextContains, "test");
        auto sub_filter2 = std::make_unique<Filter>("Sub2", FilterType::LogLevel, "Error");
        auto sub_filter3 = std::make_unique<Filter>("Sub3", FilterType::TextContains, "nonexistent");
        
        parent_filter.AddSubFilter(std::move(sub_filter1));
        parent_filter.AddSubFilter(std::move(sub_filter2));
        
        // Should match when all conditions are met
        REQUIRE(parent_filter.Matches(test_entry));
        
        // Should not match when one condition fails
        parent_filter.AddSubFilter(std::move(sub_filter3));
        REQUIRE_FALSE(parent_filter.Matches(test_entry));
    }
    
    SECTION("OR logic - any sub-filter can match") {
        Filter parent_filter("ParentOR", FilterType::LoggerName, "WrongLogger");
        parent_filter.Request_logic(FilterLogic::Or);
        
        auto sub_filter1 = std::make_unique<Filter>("Sub1", FilterType::TextContains, "nonexistent");
        auto sub_filter2 = std::make_unique<Filter>("Sub2", FilterType::LogLevel, "Error");
        
        parent_filter.AddSubFilter(std::move(sub_filter1));
        parent_filter.AddSubFilter(std::move(sub_filter2));
        
        // Should match because one sub-filter matches (even though parent doesn't)
        REQUIRE(parent_filter.Matches(test_entry));
    }
}

TEST_CASE("Filter ToString method works correctly", "[filter]") {
    Filter filter("TestFilter", FilterType::TextContains, "test message");
    filter.IncrementMatchCount();
    filter.IncrementMatchCount();
    
    std::string str = filter.ToString();
    
    REQUIRE(str.find("TestFilter") != std::string::npos);
    REQUIRE(str.find("TextContains") != std::string::npos);
    REQUIRE(str.find("test message") != std::string::npos);
    REQUIRE(str.find("Active: Yes") != std::string::npos);
    REQUIRE(str.find("Matches: 2") != std::string::npos);
}

// FilterEngine tests

TEST_CASE("FilterEngine default constructor initializes correctly", "[filter_engine]") {
    FilterEngine engine;
    
    REQUIRE(engine.GetFilterCount() == 0);
    REQUIRE(engine.Get_total_entries_processed() == 0);
    REQUIRE(engine.Get_total_matches_found() == 0);
    REQUIRE(engine.GetTotalActiveFilters() == 0);
    REQUIRE(engine.GetRegexCacheSize() == 0);
    REQUIRE(engine.GetActiveFilters().empty());
}

TEST_CASE("FilterEngine filter management works correctly", "[filter_engine]") {
    FilterEngine engine;
    
    SECTION("Add valid filters") {
        auto filter1 = std::make_unique<Filter>("Filter1", FilterType::TextContains, "error");
        auto filter2 = std::make_unique<Filter>("Filter2", FilterType::LoggerName, "LogTemp");
        
        Result result1 = engine.AddFilter(std::move(filter1));
        Result result2 = engine.AddFilter(std::move(filter2));
        
        REQUIRE(result1.IsSuccess());
        REQUIRE(result2.IsSuccess());
        REQUIRE(engine.GetFilterCount() == 2);
        REQUIRE(engine.GetTotalActiveFilters() == 2);
    }
    
    SECTION("Reject null filter") {
        Result result = engine.AddFilter(nullptr);
        REQUIRE(result.IsError());
        REQUIRE(result.Get_error_message().find("null filter") != std::string::npos);
    }
    
    SECTION("Reject invalid filter") {
        auto invalid_filter = std::make_unique<Filter>("", FilterType::TextContains, ""); // Invalid
        Result result = engine.AddFilter(std::move(invalid_filter));
        REQUIRE(result.IsError());
        REQUIRE(result.Get_error_message().find("invalid filter") != std::string::npos);
    }
    
    SECTION("Reject duplicate filter names") {
        auto filter1 = std::make_unique<Filter>("DuplicateName", FilterType::TextContains, "test");
        auto filter2 = std::make_unique<Filter>("DuplicateName", FilterType::LoggerName, "LogTemp");
        
        Result result1 = engine.AddFilter(std::move(filter1));
        Result result2 = engine.AddFilter(std::move(filter2));
        
        REQUIRE(result1.IsSuccess());
        REQUIRE(result2.IsError());
        REQUIRE(result2.Get_error_message().find("already exists") != std::string::npos);
        REQUIRE(engine.GetFilterCount() == 1);
    }
}

TEST_CASE("FilterEngine filter finding works correctly", "[filter_engine]") {
    FilterEngine engine;
    
    auto filter1 = std::make_unique<Filter>("FindMe", FilterType::TextContains, "test");
    auto filter2 = std::make_unique<Filter>("Parent", FilterType::LoggerName, "LogTemp");
    auto sub_filter = std::make_unique<Filter>("SubFilter", FilterType::LogLevel, "Error");
    
    filter2->AddSubFilter(std::move(sub_filter));
    
    engine.AddFilter(std::move(filter1));
    engine.AddFilter(std::move(filter2));
    
    SECTION("Find primary filter") {
        Filter* found = engine.FindFilter("FindMe");
        REQUIRE(found != nullptr);
        REQUIRE(found->Get_name() == "FindMe");
    }
    
    SECTION("Find sub-filter") {
        Filter* found = engine.FindFilter("SubFilter");
        REQUIRE(found != nullptr);
        REQUIRE(found->Get_name() == "SubFilter");
    }
    
    SECTION("Filter not found") {
        Filter* not_found = engine.FindFilter("NotThere");
        REQUIRE(not_found == nullptr);
    }
}

TEST_CASE("FilterEngine filter removal works correctly", "[filter_engine]") {
    FilterEngine engine;
    
    auto filter = std::make_unique<Filter>("RemoveMe", FilterType::TextContains, "test");
    engine.AddFilter(std::move(filter));
    
    REQUIRE(engine.GetFilterCount() == 1);
    
    SECTION("Remove existing filter") {
        Result result = engine.RemoveFilter("RemoveMe");
        REQUIRE(result.IsSuccess());
        REQUIRE(engine.GetFilterCount() == 0);
    }
    
    SECTION("Remove non-existent filter") {
        Result result = engine.RemoveFilter("NotThere");
        REQUIRE(result.IsError());
        REQUIRE(result.Get_error_message().find("not found") != std::string::npos);
    }
}

TEST_CASE("FilterEngine filtering operations work correctly", "[filter_engine]") {
    FilterEngine engine;
    
    // Create test entries
    LogEntry error_entry("LogTemp", "This is an error message", "raw line 1");
    LogEntry warning_entry("LogEngine", "This is a warning message", "raw line 2");
    LogEntry info_entry("LogTemp", "This is an info message", "raw line 3");
    
    std::vector<LogEntry> test_entries = {error_entry, warning_entry, info_entry};
    
    SECTION("No filters - all entries pass") {
        std::vector<LogEntry> filtered = engine.ApplyFilters(test_entries);
        REQUIRE(filtered.size() == 3);
        
        REQUIRE(engine.PassesFilters(error_entry));
        REQUIRE(engine.PassesFilters(warning_entry));
        REQUIRE(engine.PassesFilters(info_entry));
    }
    
    SECTION("Single filter - text contains") {
        auto filter = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "error");
        engine.AddFilter(std::move(filter));
        
        std::vector<LogEntry> filtered = engine.ApplyFilters(test_entries);
        REQUIRE(filtered.size() == 1);
        REQUIRE(filtered[0].Get_message().find("error") != std::string::npos);
        
        REQUIRE(engine.PassesFilters(error_entry));
        REQUIRE_FALSE(engine.PassesFilters(warning_entry));
        REQUIRE_FALSE(engine.PassesFilters(info_entry));
    }
    
    SECTION("Multiple filters - OR logic between primary filters") {
        auto error_filter = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "error");
        auto logger_filter = std::make_unique<Filter>("LoggerFilter", FilterType::LoggerName, "LogEngine");
        
        engine.AddFilter(std::move(error_filter));
        engine.AddFilter(std::move(logger_filter));
        
        std::vector<LogEntry> filtered = engine.ApplyFilters(test_entries);
        REQUIRE(filtered.size() == 2); // error_entry and warning_entry should match
        
        REQUIRE(engine.PassesFilters(error_entry));   // Matches ErrorFilter
        REQUIRE(engine.PassesFilters(warning_entry)); // Matches LoggerFilter
        REQUIRE_FALSE(engine.PassesFilters(info_entry)); // Matches neither
    }
    
    SECTION("Inactive filters don't match") {
        auto filter = std::make_unique<Filter>("InactiveFilter", FilterType::TextContains, "error");
        filter->Request_is_active(false);
        engine.AddFilter(std::move(filter));
        
        std::vector<LogEntry> filtered = engine.ApplyFilters(test_entries);
        REQUIRE(filtered.size() == 3); // All entries pass because no active filters
        
        // When no active filters exist, all entries pass
        REQUIRE(engine.PassesFilters(error_entry)); // No active filters, so passes
    }
}

TEST_CASE("FilterEngine statistics work correctly", "[filter_engine]") {
    FilterEngine engine;
    
    auto error_filter = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "error");
    auto warning_filter = std::make_unique<Filter>("WarningFilter", FilterType::TextContains, "warning");
    
    engine.AddFilter(std::move(error_filter));
    engine.AddFilter(std::move(warning_filter));
    
    LogEntry error_entry("LogTemp", "This is an error message", "raw line 1");
    LogEntry warning_entry("LogEngine", "This is a warning message", "raw line 2");
    LogEntry info_entry("LogTemp", "This is an info message", "raw line 3");
    
    std::vector<LogEntry> test_entries = {error_entry, warning_entry, info_entry};
    
    SECTION("Statistics updated by ApplyFilters") {
        std::vector<LogEntry> filtered = engine.ApplyFilters(test_entries);
        
        REQUIRE(engine.Get_total_entries_processed() == 3);
        REQUIRE(engine.Get_total_matches_found() == 2); // error and warning entries
        
        // Check individual filter match counts
        Filter* error_filter_ptr = engine.FindFilter("ErrorFilter");
        Filter* warning_filter_ptr = engine.FindFilter("WarningFilter");
        
        REQUIRE(error_filter_ptr->Get_match_count() == 1);
        REQUIRE(warning_filter_ptr->Get_match_count() == 1);
    }
    
    SECTION("Statistics updated by UpdateFilterStatistics") {
        engine.UpdateFilterStatistics(test_entries);
        
        REQUIRE(engine.Get_total_entries_processed() == 3);
        REQUIRE(engine.Get_total_matches_found() == 2);
    }
    
    SECTION("Reset statistics") {
        engine.ApplyFilters(test_entries);
        
        REQUIRE(engine.Get_total_entries_processed() > 0);
        REQUIRE(engine.Get_total_matches_found() > 0);
        
        engine.ResetAllStatistics();
        
        REQUIRE(engine.Get_total_entries_processed() == 0);
        REQUIRE(engine.Get_total_matches_found() == 0);
        
        Filter* error_filter_ptr = engine.FindFilter("ErrorFilter");
        REQUIRE(error_filter_ptr->Get_match_count() == 0);
    }
}

TEST_CASE("FilterEngine regex caching works correctly", "[filter_engine]") {
    FilterEngine engine;
    
    SECTION("Cache regex patterns") {
        REQUIRE(engine.GetRegexCacheSize() == 0);
        
        const std::regex& regex1 = engine.GetCachedRegex("test.*pattern");
        REQUIRE(engine.GetRegexCacheSize() == 1);
        
        const std::regex& regex2 = engine.GetCachedRegex("another.*pattern");
        REQUIRE(engine.GetRegexCacheSize() == 2);
        
        // Same pattern should return cached version
        const std::regex& regex3 = engine.GetCachedRegex("test.*pattern");
        REQUIRE(engine.GetRegexCacheSize() == 2); // No new entry
        REQUIRE(&regex1 == &regex3); // Same object reference
    }
    
    SECTION("Handle invalid regex patterns") {
        const std::regex& invalid_regex = engine.GetCachedRegex("[invalid regex");
        // Should not crash and should return a valid regex object
        REQUIRE(engine.GetRegexCacheSize() == 0); // Invalid patterns not cached
    }
    
    SECTION("Clear regex cache") {
        engine.GetCachedRegex("pattern1");
        engine.GetCachedRegex("pattern2");
        REQUIRE(engine.GetRegexCacheSize() == 2);
        
        engine.ClearRegexCache();
        REQUIRE(engine.GetRegexCacheSize() == 0);
    }
}

TEST_CASE("FilterEngine validation works correctly", "[filter_engine]") {
    FilterEngine engine;
    
    SECTION("All valid filters") {
        auto filter1 = std::make_unique<Filter>("Valid1", FilterType::TextContains, "test");
        auto filter2 = std::make_unique<Filter>("Valid2", FilterType::LoggerName, "LogTemp");
        
        engine.AddFilter(std::move(filter1));
        engine.AddFilter(std::move(filter2));
        
        Result result = engine.ValidateAllFilters();
        REQUIRE(result.IsSuccess());
        REQUIRE(engine.GetInvalidFilterNames().empty());
    }
    
    SECTION("Invalid filter rejected by AddFilter") {
        auto valid_filter = std::make_unique<Filter>("Valid", FilterType::TextContains, "test");
        auto invalid_filter = std::make_unique<Filter>("Invalid", FilterType::TextRegex, "[invalid regex");
        
        Result valid_result = engine.AddFilter(std::move(valid_filter));
        Result invalid_result = engine.AddFilter(std::move(invalid_filter));
        
        REQUIRE(valid_result.IsSuccess());
        REQUIRE(invalid_result.IsError());
        REQUIRE(invalid_result.Get_error_message().find("invalid filter") != std::string::npos);
        
        // Only the valid filter should be added
        REQUIRE(engine.GetFilterCount() == 1);
        
        // ValidateAllFilters should succeed since only valid filters are in the engine
        Result validation_result = engine.ValidateAllFilters();
        REQUIRE(validation_result.IsSuccess());
        REQUIRE(engine.GetInvalidFilterNames().empty());
    }
}

TEST_CASE("FilterEngine utility methods work correctly", "[filter_engine]") {
    FilterEngine engine;
    
    auto filter1 = std::make_unique<Filter>("Filter1", FilterType::TextContains, "error");
    auto filter2 = std::make_unique<Filter>("Filter2", FilterType::LoggerName, "LogTemp");
    filter2->Request_is_active(false); // Make this one inactive
    
    engine.AddFilter(std::move(filter1));
    engine.AddFilter(std::move(filter2));
    
    SECTION("Get active filters") {
        std::vector<Filter*> active_filters = engine.GetActiveFilters();
        REQUIRE(active_filters.size() == 1);
        REQUIRE(active_filters[0]->Get_name() == "Filter1");
        
        REQUIRE(engine.GetTotalActiveFilters() == 1);
    }
    
    SECTION("Set all filters active/inactive") {
        engine.SetAllFiltersActive(false);
        REQUIRE(engine.GetTotalActiveFilters() == 0);
        
        engine.SetAllFiltersActive(true);
        REQUIRE(engine.GetTotalActiveFilters() == 2);
    }
    
    SECTION("Statistics summary") {
        LogEntry test_entry("LogTemp", "This is an error message", "raw line");
        std::vector<LogEntry> entries = {test_entry};
        
        engine.ApplyFilters(entries);
        
        std::string summary = engine.GetStatisticsSummary();
        REQUIRE(summary.find("Total Filters: 2") != std::string::npos);
        REQUIRE(summary.find("Active Filters:") != std::string::npos);
        REQUIRE(summary.find("Entries Processed:") != std::string::npos);
        REQUIRE(summary.find("Filter Details:") != std::string::npos);
    }
    
    SECTION("Clear all filters") {
        REQUIRE(engine.GetFilterCount() == 2);
        
        engine.ClearAllFilters();
        
        REQUIRE(engine.GetFilterCount() == 0);
        REQUIRE(engine.GetTotalActiveFilters() == 0);
        REQUIRE(engine.Get_total_entries_processed() == 0);
        REQUIRE(engine.Get_total_matches_found() == 0);
        REQUIRE(engine.GetRegexCacheSize() == 0);
    }
}

TEST_CASE("FilterEngine three-state filtering works correctly", "[filter_engine]") {
    FilterEngine engine;
    
    // Create test entries
    LogEntry error_entry("LogTemp", "This is an error message", "raw line");
    LogEntry warning_entry("LogTemp", "This is a warning message", "raw line");
    LogEntry info_entry("LogTemp", "This is an info message", "raw line");
    
    // Create filters
    auto error_filter = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "error");
    auto warning_filter = std::make_unique<Filter>("WarningFilter", FilterType::TextContains, "warning");
    
    Filter* error_filter_ptr = error_filter.get();
    Filter* warning_filter_ptr = warning_filter.get();
    
    engine.AddFilter(std::move(error_filter));
    engine.AddFilter(std::move(warning_filter));
    
    SECTION("Include filters work correctly") {
        // Both filters in INCLUDE state (default)
        REQUIRE(error_filter_ptr->GetFilterState() == FilterState::INCLUDE);
        REQUIRE(warning_filter_ptr->GetFilterState() == FilterState::INCLUDE);
        
        // Should pass entries that match either filter (OR logic)
        REQUIRE(engine.PassesFilters(error_entry));
        REQUIRE(engine.PassesFilters(warning_entry));
        REQUIRE_FALSE(engine.PassesFilters(info_entry));
    }
    
    SECTION("Exclude filters work correctly") {
        // Set both filters to EXCLUDE state
        error_filter_ptr->SetFilterState(FilterState::EXCLUDE);
        warning_filter_ptr->SetFilterState(FilterState::EXCLUDE);
        
        // Should pass entries that don't match any exclude filter
        REQUIRE_FALSE(engine.PassesFilters(error_entry));
        REQUIRE_FALSE(engine.PassesFilters(warning_entry));
        REQUIRE(engine.PassesFilters(info_entry));
    }
    
    SECTION("Mixed include and exclude filters work correctly") {
        // Error filter: INCLUDE, Warning filter: EXCLUDE
        error_filter_ptr->SetFilterState(FilterState::INCLUDE);
        warning_filter_ptr->SetFilterState(FilterState::EXCLUDE);
        
        // Should pass entries that match include filters AND don't match exclude filters
        REQUIRE(engine.PassesFilters(error_entry));    // Matches include, doesn't match exclude
        REQUIRE_FALSE(engine.PassesFilters(warning_entry)); // Doesn't match include, matches exclude
        REQUIRE_FALSE(engine.PassesFilters(info_entry));    // Doesn't match include
    }
    
    SECTION("Disabled filters are ignored") {
        // Set one filter to disabled
        error_filter_ptr->SetFilterState(FilterState::DISABLED);
        warning_filter_ptr->SetFilterState(FilterState::INCLUDE);
        
        // Only warning filter should be active
        REQUIRE(engine.GetTotalActiveFilters() == 1);
        REQUIRE_FALSE(engine.PassesFilters(error_entry));
        REQUIRE(engine.PassesFilters(warning_entry));
        REQUIRE_FALSE(engine.PassesFilters(info_entry));
    }
    
    SECTION("All filters disabled means all entries pass") {
        error_filter_ptr->SetFilterState(FilterState::DISABLED);
        warning_filter_ptr->SetFilterState(FilterState::DISABLED);
        
        REQUIRE(engine.GetTotalActiveFilters() == 0);
        REQUIRE(engine.PassesFilters(error_entry));
        REQUIRE(engine.PassesFilters(warning_entry));
        REQUIRE(engine.PassesFilters(info_entry));
    }
}

TEST_CASE("FilterEngine GetMatchingFilters works correctly", "[filter_engine]") {
    FilterEngine engine;
    
    auto error_filter = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "error");
    auto temp_filter = std::make_unique<Filter>("TempFilter", FilterType::LoggerName, "LogTemp");
    auto warning_filter = std::make_unique<Filter>("WarningFilter", FilterType::TextContains, "warning");
    
    engine.AddFilter(std::move(error_filter));
    engine.AddFilter(std::move(temp_filter));
    engine.AddFilter(std::move(warning_filter));
    
    SECTION("Entry matches multiple filters") {
        LogEntry entry("LogTemp", "This is an error message", "raw line");
        
        std::vector<const Filter*> matching = engine.GetMatchingFilters(entry);
        REQUIRE(matching.size() == 2); // ErrorFilter and TempFilter should match
        
        std::vector<std::string> names;
        for (const Filter* filter : matching) {
            names.push_back(filter->Get_name());
        }
        
        REQUIRE(std::find(names.begin(), names.end(), "ErrorFilter") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "TempFilter") != names.end());
    }
    
    SECTION("Entry matches no filters") {
        LogEntry entry("LogEngine", "This is an info message", "raw line");
        
        std::vector<const Filter*> matching = engine.GetMatchingFilters(entry);
        REQUIRE(matching.empty());
    }
}

TEST_CASE("Filter three-state functionality works correctly", "[filter]") {
    Filter filter("ThreeStateFilter", FilterType::TextContains, "error");
    
    LogEntry matching_entry("LogTemp", "This is an error message", "raw line");
    LogEntry non_matching_entry("LogTemp", "This is an info message", "raw line");
    
    SECTION("Default INCLUDE state") {
        REQUIRE(filter.GetFilterState() == FilterState::INCLUDE);
        REQUIRE(filter.ShouldInclude(matching_entry));
        REQUIRE_FALSE(filter.ShouldInclude(non_matching_entry));
        REQUIRE_FALSE(filter.ShouldExclude(matching_entry));
        REQUIRE(filter.Matches(matching_entry));
        REQUIRE(filter.IsActive());
    }
    
    SECTION("EXCLUDE state after cycling") {
        filter.CycleFilterState();
        REQUIRE(filter.GetFilterState() == FilterState::EXCLUDE);
        REQUIRE_FALSE(filter.ShouldInclude(matching_entry));
        REQUIRE(filter.ShouldExclude(matching_entry));
        REQUIRE_FALSE(filter.ShouldExclude(non_matching_entry));
        REQUIRE(filter.Matches(matching_entry)); // Still matches for exclusion logic
        REQUIRE(filter.IsActive());
    }
    
    SECTION("DISABLED state after cycling twice") {
        filter.CycleFilterState();
        filter.CycleFilterState();
        REQUIRE(filter.GetFilterState() == FilterState::DISABLED);
        REQUIRE_FALSE(filter.ShouldInclude(matching_entry));
        REQUIRE_FALSE(filter.ShouldExclude(matching_entry));
        REQUIRE_FALSE(filter.Matches(matching_entry));
        REQUIRE_FALSE(filter.IsActive());
    }
    
    SECTION("Cycle back to INCLUDE") {
        filter.CycleFilterState();
        filter.CycleFilterState();
        filter.CycleFilterState();
        REQUIRE(filter.GetFilterState() == FilterState::INCLUDE);
        REQUIRE(filter.ShouldInclude(matching_entry));
        REQUIRE(filter.Matches(matching_entry));
        REQUIRE(filter.IsActive());
    }
    
    SECTION("Backward compatibility with is_active") {
        filter.Request_is_active(false);
        REQUIRE(filter.GetFilterState() == FilterState::DISABLED);
        REQUIRE_FALSE(filter.IsActive());
        REQUIRE_FALSE(filter.Matches(matching_entry));
        
        filter.Request_is_active(true);
        REQUIRE(filter.GetFilterState() == FilterState::INCLUDE);
        REQUIRE(filter.IsActive());
        REQUIRE(filter.Matches(matching_entry));
    }
    
    SECTION("Filter validation works with all states") {
        // Valid filter should be valid in all states
        REQUIRE(filter.IsValid());
        
        filter.SetFilterState(FilterState::EXCLUDE);
        REQUIRE(filter.IsValid());
        
        filter.SetFilterState(FilterState::DISABLED);
        REQUIRE(filter.IsValid());
        
        // Invalid filter should be invalid in all states
        Filter invalid_filter("", FilterType::TextContains, "");
        REQUIRE_FALSE(invalid_filter.IsValid());
        
        invalid_filter.SetFilterState(FilterState::EXCLUDE);
        REQUIRE_FALSE(invalid_filter.IsValid());
        
        invalid_filter.SetFilterState(FilterState::DISABLED);
        REQUIRE_FALSE(invalid_filter.IsValid());
    }
}

TEST_CASE("Filter JSON serialization works correctly", "[filter]") {
    SECTION("Basic filter serialization") {
        Filter filter("TestFilter", FilterType::TextContains, "test message");
        filter.Request_highlight_color("#FF0000");
        filter.IncrementMatchCount();
        filter.IncrementMatchCount();
        
        std::string json = filter.ToJson();
        
        REQUIRE(json.find("\"name\": \"TestFilter\"") != std::string::npos);
        REQUIRE(json.find("\"type\": 0") != std::string::npos); // TextContains = 0
        REQUIRE(json.find("\"criteria\": \"test message\"") != std::string::npos);
        REQUIRE(json.find("\"is_active\": true") != std::string::npos);
        REQUIRE(json.find("\"highlight_color\": \"#FF0000\"") != std::string::npos);
        REQUIRE(json.find("\"match_count\": 2") != std::string::npos);
    }
    
    SECTION("Filter deserialization") {
        std::string json = R"({
  "name": "DeserializedFilter",
  "type": 2,
  "criteria": "test.*pattern",
  "is_active": false,
  "logic": 1,
  "highlight_color": "#00FF00",
  "match_count": 5,
  "sub_filters": []
})";
        
        auto filter = Filter::FromJson(json);
        REQUIRE(filter != nullptr);
        REQUIRE(filter->Get_name() == "DeserializedFilter");
        REQUIRE(filter->Get_type() == FilterType::TextRegex);
        REQUIRE(filter->Get_criteria() == "test.*pattern");
        REQUIRE(filter->Get_is_active() == false);
        REQUIRE(filter->Get_logic() == FilterLogic::Or);
        REQUIRE(filter->Get_highlight_color() == "#00FF00");
        REQUIRE(filter->Get_match_count() == 5);
    }
    
    SECTION("JSON string escaping") {
        Filter filter("Test\"Filter", FilterType::TextContains, "message with \"quotes\" and \n newlines");
        
        std::string json = filter.ToJson();
        
        REQUIRE(json.find("Test\\\"Filter") != std::string::npos);
        REQUIRE(json.find("\\\"quotes\\\"") != std::string::npos);
        REQUIRE(json.find("\\n") != std::string::npos);
    }
}

TEST_CASE("FilterEngine persistence works correctly", "[filter_engine]") {
    FilterEngine engine;
    
    // Create test filters
    auto filter1 = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "error");
    auto filter2 = std::make_unique<Filter>("RegexFilter", FilterType::TextRegex, "test.*pattern");
    filter2->Request_highlight_color("#FF0000");
    filter2->Request_is_active(false);
    
    engine.AddFilter(std::move(filter1));
    engine.AddFilter(std::move(filter2));
    
    // Add some statistics
    LogEntry test_entry("LogTemp", "This is an error message", "raw line");
    std::vector<LogEntry> entries = {test_entry};
    engine.ApplyFilters(entries);
    
    SECTION("JSON serialization") {
        std::string json = engine.SerializeFiltersToJson();
        
        REQUIRE(json.find("\"version\": \"1.0\"") != std::string::npos);
        REQUIRE(json.find("\"total_entries_processed\":") != std::string::npos);
        REQUIRE(json.find("\"total_matches_found\":") != std::string::npos);
        REQUIRE(json.find("\"filters\": [") != std::string::npos);
        REQUIRE(json.find("ErrorFilter") != std::string::npos);
        REQUIRE(json.find("RegexFilter") != std::string::npos);
    }
    
    SECTION("Save and load filters to/from file") {
        std::string filename = "test_filters.json";
        
        // Save filters
        Result save_result = engine.SaveFiltersToFile(filename);
        REQUIRE(save_result.IsSuccess());
        
        // Create a new engine and load filters
        FilterEngine new_engine;
        Result load_result = new_engine.LoadFiltersFromFile(filename);
        REQUIRE(load_result.IsSuccess());
        
        // Verify filters were loaded correctly
        REQUIRE(new_engine.GetFilterCount() == 2);
        
        Filter* loaded_error_filter = new_engine.FindFilter("ErrorFilter");
        Filter* loaded_regex_filter = new_engine.FindFilter("RegexFilter");
        
        REQUIRE(loaded_error_filter != nullptr);
        REQUIRE(loaded_regex_filter != nullptr);
        
        REQUIRE(loaded_error_filter->Get_type() == FilterType::TextContains);
        REQUIRE(loaded_error_filter->Get_criteria() == "error");
        REQUIRE(loaded_error_filter->Get_is_active() == true);
        
        REQUIRE(loaded_regex_filter->Get_type() == FilterType::TextRegex);
        REQUIRE(loaded_regex_filter->Get_criteria() == "test.*pattern");
        REQUIRE(loaded_regex_filter->Get_highlight_color() == "#FF0000");
        REQUIRE(loaded_regex_filter->Get_is_active() == false);
        
        // Clean up test file
        std::remove(filename.c_str());
    }
    
    SECTION("JSON deserialization") {
        std::string json = engine.SerializeFiltersToJson();
        
        FilterEngine new_engine;
        Result result = new_engine.DeserializeFiltersFromJson(json);
        REQUIRE(result.IsSuccess());
        
        REQUIRE(new_engine.GetFilterCount() == 2);
        REQUIRE(new_engine.FindFilter("ErrorFilter") != nullptr);
        REQUIRE(new_engine.FindFilter("RegexFilter") != nullptr);
    }
    
    SECTION("Handle file errors") {
        // Try to save to invalid path
        Result save_result = engine.SaveFiltersToFile("/invalid/path/test.json");
        REQUIRE(save_result.IsError());
        REQUIRE(save_result.Get_error_message().find("Cannot open file") != std::string::npos);
        
        // Try to load non-existent file
        Result load_result = engine.LoadFiltersFromFile("non_existent_file.json");
        REQUIRE(load_result.IsError());
        REQUIRE(load_result.Get_error_message().find("Cannot open file") != std::string::npos);
    }
    
    SECTION("Handle invalid JSON") {
        std::string invalid_json = "{ invalid json }";
        
        FilterEngine new_engine;
        Result result = new_engine.DeserializeFiltersFromJson(invalid_json);
        REQUIRE(result.IsError());
    }
    
    SECTION("Debug JSON parsing") {
        // Create a simple test with just one filter first
        FilterEngine simple_engine;
        auto simple_filter = std::make_unique<Filter>("SimpleFilter", FilterType::TextContains, "test");
        simple_engine.AddFilter(std::move(simple_filter));
        
        std::string simple_json = simple_engine.SerializeFiltersToJson();
        
        FilterEngine load_engine;
        Result result = load_engine.DeserializeFiltersFromJson(simple_json);
        REQUIRE(result.IsSuccess());
        REQUIRE(load_engine.GetFilterCount() == 1);
        
        Filter* loaded_filter = load_engine.FindFilter("SimpleFilter");
        REQUIRE(loaded_filter != nullptr);
        REQUIRE(loaded_filter->Get_name() == "SimpleFilter");
        REQUIRE(loaded_filter->Get_criteria() == "test");
    }
}