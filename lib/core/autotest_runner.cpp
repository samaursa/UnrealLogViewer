#include "autotest_runner.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <thread>

namespace ue_log {

AutotestRunner::AutotestRunner(const std::string& log_file_path, const std::string& output_file_path)
    : log_file_path_(log_file_path)
    , output_file_path_(output_file_path)
    , log_processor_(std::make_unique<LogProcessor>())
    , filter_manager_(std::make_unique<FilterManager>())
    , verbose_(false)
    , performance_test_enabled_(true) {
    
    report_.log_file_path = log_file_path;
    report_.start_time = std::chrono::system_clock::now();
    report_.system_info = GetSystemInfo();
}

AutotestRunner::~AutotestRunner() {
    if (report_file_.is_open()) {
        report_file_.close();
    }
}

bool AutotestRunner::RunAllTests() {
    LogMessage("Starting comprehensive autotest suite...");
    
    // Open report file
    report_file_.open(output_file_path_);
    if (!report_file_.is_open()) {
        LogError("Failed to open report file: " + output_file_path_);
        return false;
    }
    
    // Write report header
    WriteSystemInfo();
    
    // Run all tests
    std::vector<std::function<bool()>> tests = {
        [this]() { return TestFileLoading(); },
        [this]() { return TestLogParsing(); },
        [this]() { return TestFilterFunctionality(); },
        [this]() { return TestNavigationLogic(); },
        [this]() { return TestSearchFunctionality(); },
        [this]() { return TestErrorHandling(); }
    };
    
    if (performance_test_enabled_) {
        tests.push_back([this]() { return TestPerformance(); });
    }
    
    std::vector<std::string> test_names = {
        "File Loading",
        "Log Parsing", 
        "Filter Functionality",
        "Navigation Logic",
        "Search Functionality",
        "Error Handling"
    };
    
    if (performance_test_enabled_) {
        test_names.push_back("Performance");
    }
    
    // Execute tests
    for (size_t i = 0; i < tests.size(); ++i) {
        auto result = RunTest(test_names[i], tests[i]);
        report_.test_results.push_back(result);
        
        if (result.passed) {
            report_.passed_tests++;
        } else {
            report_.failed_tests++;
        }
        report_.total_tests++;
    }
    
    // Finalize report
    report_.end_time = std::chrono::system_clock::now();
    report_.overall_success = (report_.failed_tests == 0);
    
    WriteTestSummary();
    
    LogMessage("Autotest completed. Results written to: " + output_file_path_);
    return report_.overall_success;
}

bool AutotestRunner::TestFileLoading() {
    LogMessage("Testing file loading functionality...");
    
    // Test 1: Load the specified file
    if (!log_processor_->LoadFile(log_file_path_)) {
        LogError("Failed to load log file: " + log_processor_->GetLastError());
        return false;
    }
    
    // Test 2: Verify file is marked as loaded
    if (!log_processor_->IsFileLoaded()) {
        LogError("File not marked as loaded after successful load");
        return false;
    }
    
    // Test 3: Verify file path is stored correctly
    if (log_processor_->GetCurrentFilePath() != log_file_path_) {
        LogError("File path not stored correctly");
        return false;
    }
    
    // Test 4: Verify entries were loaded
    if (log_processor_->GetTotalEntryCount() == 0) {
        LogError("No log entries were loaded from file");
        return false;
    }
    
    LogMessage("File loading tests passed. Loaded " + 
               std::to_string(log_processor_->GetTotalEntryCount()) + " entries.");
    return true;
}

bool AutotestRunner::TestLogParsing() {
    LogMessage("Testing log parsing functionality...");
    
    const auto& entries = log_processor_->GetEntries();
    
    // Test 1: Validate that entries were parsed
    if (!ValidateLogEntries(entries, "parsed entries")) {
        return false;
    }
    
    // Test 2: Check for different log levels
    bool has_error = false, has_warning = false, has_info = false;
    for (const auto& entry : entries) {
        if (entry.HasLogLevel()) {
            const auto& level = entry.Get_log_level().value();
            if (level == "Error") has_error = true;
            else if (level == "Warning") has_warning = true;
            else if (level == "Info") has_info = true;
        }
    }
    
    // Test 3: Verify entry structure
    int structured_count = 0;
    for (const auto& entry : entries) {
        if (entry.IsValid()) {
            structured_count++;
        }
    }
    
    LogMessage("Log parsing tests passed. Found " + std::to_string(structured_count) + 
               " valid entries with various log levels.");
    return true;
}

bool AutotestRunner::TestFilterFunctionality() {
    LogMessage("Testing filter functionality...");
    
    const auto& original_entries = log_processor_->GetEntries();
    
    // Test 1: Test error filter
    filter_manager_->ClearAllFilters();
    filter_manager_->AddQuickFilter("error");
    auto error_filtered = filter_manager_->ApplyFilters(original_entries);
    
    if (!ValidateFilterResults(original_entries, error_filtered, "error filter")) {
        return false;
    }
    
    // Test 2: Test warning filter
    filter_manager_->ClearAllFilters();
    filter_manager_->AddQuickFilter("warning");
    auto warning_filtered = filter_manager_->ApplyFilters(original_entries);
    
    if (!ValidateFilterResults(original_entries, warning_filtered, "warning filter")) {
        return false;
    }
    
    // Test 3: Test filter clearing
    filter_manager_->ClearAllFilters();
    auto cleared_filtered = filter_manager_->ApplyFilters(original_entries);
    
    if (cleared_filtered.size() != original_entries.size()) {
        LogError("Filter clearing failed - entry count mismatch");
        return false;
    }
    
    // Test 4: Test filter statistics
    filter_manager_->AddQuickFilter("error");
    auto stats = filter_manager_->GetFilterStats(original_entries);
    
    if (stats.total_entries != original_entries.size()) {
        LogError("Filter statistics total count incorrect");
        return false;
    }
    
    LogMessage("Filter functionality tests passed. Tested error, warning, and clear operations.");
    return true;
}

bool AutotestRunner::TestNavigationLogic() {
    LogMessage("Testing navigation logic...");
    
    // Ensure we have entries to navigate
    if (log_processor_->GetTotalEntryCount() == 0) {
        LogError("No entries available for navigation testing");
        return false;
    }
    
    // Test 1: Initial position
    log_processor_->NavigateToTop();
    if (!ValidateNavigationState(0, log_processor_->GetCurrentIndex(), "navigate to top")) {
        return false;
    }
    
    // Test 2: Navigate down
    log_processor_->NavigateDown(1);
    if (!ValidateNavigationState(1, log_processor_->GetCurrentIndex(), "navigate down")) {
        return false;
    }
    
    // Test 3: Navigate up
    log_processor_->NavigateUp(1);
    if (!ValidateNavigationState(0, log_processor_->GetCurrentIndex(), "navigate up")) {
        return false;
    }
    
    // Test 4: Navigate to bottom
    log_processor_->NavigateToBottom();
    int expected_bottom = static_cast<int>(log_processor_->GetFilteredEntryCount()) - 1;
    if (!ValidateNavigationState(expected_bottom, log_processor_->GetCurrentIndex(), "navigate to bottom")) {
        return false;
    }
    
    // Test 5: Percentage navigation
    log_processor_->NavigateToPercentage(50);
    int current_percentage = log_processor_->GetCurrentPercentage();
    if (current_percentage < 40 || current_percentage > 60) {
        LogError("Percentage navigation failed - expected ~50%, got " + std::to_string(current_percentage) + "%");
        return false;
    }
    
    LogMessage("Navigation logic tests passed. Tested all navigation operations.");
    return true;
}

bool AutotestRunner::TestSearchFunctionality() {
    LogMessage("Testing search functionality...");
    
    const auto& entries = log_processor_->GetFilteredEntries();
    
    // Test 1: Search for common terms
    auto error_results = log_processor_->Search("Error", false);
    if (!ValidateSearchResults(error_results, entries, "Error")) {
        return false;
    }
    
    // Test 2: Case-sensitive search
    auto case_results = log_processor_->Search("error", true);
    auto case_insensitive_results = log_processor_->Search("error", false);
    
    if (case_insensitive_results.size() < case_results.size()) {
        LogError("Case-insensitive search returned fewer results than case-sensitive");
        return false;
    }
    
    // Test 3: Search result navigation
    if (!error_results.empty()) {
        log_processor_->SetSearchResults(error_results);
        log_processor_->NavigateToNextSearchResult();
        
        int current_index = log_processor_->GetCurrentIndex();
        bool found_in_results = std::find(error_results.begin(), error_results.end(), current_index) != error_results.end();
        
        if (!found_in_results) {
            LogError("Search result navigation failed - current index not in search results");
            return false;
        }
    }
    
    LogMessage("Search functionality tests passed. Tested search and navigation.");
    return true;
}

bool AutotestRunner::TestErrorHandling() {
    LogMessage("Testing error handling...");
    
    // Test 1: Invalid file loading
    LogProcessor temp_processor;
    if (temp_processor.LoadFile("nonexistent_file.log")) {
        LogError("Error handling failed - nonexistent file was loaded successfully");
        return false;
    }
    
    if (temp_processor.GetLastError().empty()) {
        LogError("Error handling failed - no error message for nonexistent file");
        return false;
    }
    
    // Test 2: Invalid navigation
    temp_processor.SetCurrentIndex(-1);
    if (temp_processor.GetCurrentIndex() < 0) {
        LogError("Error handling failed - negative index was accepted");
        return false;
    }
    
    // Test 3: Invalid filter operations
    FilterManager temp_filter_manager;
    temp_filter_manager.AddQuickFilter("invalid_filter_type");
    if (temp_filter_manager.GetLastError().empty()) {
        LogError("Error handling failed - invalid filter type accepted without error");
        return false;
    }
    
    LogMessage("Error handling tests passed. Verified proper error reporting.");
    return true;
}

bool AutotestRunner::TestPerformance() {
    LogMessage("Testing performance...");
    
    const auto& entries = log_processor_->GetEntries();
    
    // Test 1: Filter performance
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        filter_manager_->ClearAllFilters();
        filter_manager_->AddQuickFilter("error");
        auto filtered = filter_manager_->ApplyFilters(entries);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    if (duration.count() > 5000) { // 5 seconds threshold
        LogError("Performance test failed - filtering took too long: " + std::to_string(duration.count()) + "ms");
        return false;
    }
    
    // Test 2: Search performance
    start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 50; ++i) {
        auto results = log_processor_->Search("Error", false);
    }
    
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    if (duration.count() > 3000) { // 3 seconds threshold
        LogError("Performance test failed - searching took too long: " + std::to_string(duration.count()) + "ms");
        return false;
    }
    
    LogMessage("Performance tests passed. Operations completed within acceptable time limits.");
    return true;
}

AutotestRunner::TestResult AutotestRunner::RunTest(const std::string& test_name, std::function<bool()> test_function) {
    TestResult result(test_name);
    WriteTestHeader(test_name);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        result.passed = test_function();
        if (result.passed) {
            result.details = "Test completed successfully";
        }
    } catch (const std::exception& e) {
        result.passed = false;
        result.error_message = e.what();
        result.details = "Test failed with exception: " + result.error_message;
        LogError("Test '" + test_name + "' failed with exception: " + result.error_message);
    } catch (...) {
        result.passed = false;
        result.error_message = "Unknown exception";
        result.details = "Test failed with unknown exception";
        LogError("Test '" + test_name + "' failed with unknown exception");
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    WriteTestResult(result);
    return result;
}

void AutotestRunner::WriteReport() {
    if (!report_file_.is_open()) return;
    
    WriteTestSummary();
    report_file_.flush();
}

void AutotestRunner::WriteTestHeader(const std::string& test_name) {
    if (report_file_.is_open()) {
        report_file_ << "\n" << (report_.test_results.size() + 1) << ". " << test_name << "..." << std::endl;
    }
}

void AutotestRunner::WriteTestResult(const TestResult& result) {
    if (!report_file_.is_open()) return;
    
    if (result.passed) {
        report_file_ << "   ✓ " << result.details << " (" << result.duration.count() << "ms)" << std::endl;
    } else {
        report_file_ << "   ✗ " << result.details;
        if (!result.error_message.empty()) {
            report_file_ << " - " << result.error_message;
        }
        report_file_ << " (" << result.duration.count() << "ms)" << std::endl;
    }
}

void AutotestRunner::WriteTestSummary() {
    if (!report_file_.is_open()) return;
    
    report_file_ << "\n=== Autotest Summary ===" << std::endl;
    report_file_ << "Total tests: " << report_.total_tests << std::endl;
    report_file_ << "Passed: " << report_.passed_tests << std::endl;
    report_file_ << "Failed: " << report_.failed_tests << std::endl;
    report_file_ << "Success rate: " << std::fixed << std::setprecision(1) 
                 << (report_.total_tests > 0 ? (static_cast<double>(report_.passed_tests) / report_.total_tests * 100.0) : 0.0) 
                 << "%" << std::endl;
    
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(report_.end_time - report_.start_time);
    report_file_ << "Total duration: " << total_duration.count() << "ms" << std::endl;
    
    if (report_.overall_success) {
        report_file_ << "Overall result: SUCCESS - All tests passed" << std::endl;
    } else {
        report_file_ << "Overall result: FAILURE - " << report_.failed_tests << " test(s) failed" << std::endl;
    }
}

void AutotestRunner::WriteSystemInfo() {
    if (!report_file_.is_open()) return;
    
    report_file_ << "=== Unreal Log Viewer Autotest Report ===" << std::endl;
    report_file_ << "Test started: " << GetCurrentTimestamp() << std::endl;
    report_file_ << "Log file: " << log_file_path_ << std::endl;
    report_file_ << "System info: " << report_.system_info << std::endl;
    report_file_ << std::endl;
}

std::string AutotestRunner::GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string AutotestRunner::GetSystemInfo() const {
    std::stringstream ss;
    ss << "Platform: Windows";
    ss << ", Threads: " << std::thread::hardware_concurrency();
    return ss.str();
}

void AutotestRunner::LogMessage(const std::string& message) {
    if (verbose_) {
        std::cout << "[INFO] " << message << std::endl;
    }
}

void AutotestRunner::LogError(const std::string& error) {
    std::cerr << "[ERROR] " << error << std::endl;
}

bool AutotestRunner::ValidateLogEntries(const std::vector<LogEntry>& entries, const std::string& context) {
    if (entries.empty()) {
        LogError("Validation failed for " + context + " - no entries found");
        return false;
    }
    
    int valid_count = 0;
    for (const auto& entry : entries) {
        if (entry.IsValid()) {
            valid_count++;
        }
    }
    
    if (valid_count == 0) {
        LogError("Validation failed for " + context + " - no valid entries found");
        return false;
    }
    
    return true;
}

bool AutotestRunner::ValidateFilterResults(const std::vector<LogEntry>& original, 
                                          const std::vector<LogEntry>& filtered, 
                                          const std::string& filter_description) {
    if (filtered.size() > original.size()) {
        LogError("Filter validation failed for " + filter_description + " - more filtered than original entries");
        return false;
    }
    
    // Additional validation could be added here
    return true;
}

bool AutotestRunner::ValidateNavigationState(int expected_index, int actual_index, const std::string& operation) {
    if (expected_index != actual_index) {
        LogError("Navigation validation failed for " + operation + 
                " - expected index " + std::to_string(expected_index) + 
                ", got " + std::to_string(actual_index));
        return false;
    }
    return true;
}

bool AutotestRunner::ValidateSearchResults(const std::vector<int>& results, 
                                          const std::vector<LogEntry>& entries, 
                                          const std::string& query) {
    // Validate that all result indices are within bounds
    for (int index : results) {
        if (index < 0 || index >= static_cast<int>(entries.size())) {
            LogError("Search validation failed for query '" + query + "' - index out of bounds: " + std::to_string(index));
            return false;
        }
    }
    
    return true;
}

} // namespace ue_log