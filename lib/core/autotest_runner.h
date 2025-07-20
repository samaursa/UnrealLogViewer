#pragma once

#include "log_processor.h"
#include "filter_manager.h"
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>

namespace ue_log {

/**
 * Comprehensive autotest framework for headless testing of log viewer functionality.
 * Runs tests without UI dependencies and generates detailed reports.
 */
class AutotestRunner {
public:
    struct TestResult {
        std::string test_name;
        bool passed;
        std::string details;
        std::chrono::milliseconds duration;
        std::string error_message;
        
        TestResult(const std::string& name) 
            : test_name(name), passed(false), duration(0) {}
    };
    
    struct AutotestReport {
        std::string log_file_path;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point end_time;
        std::vector<TestResult> test_results;
        int total_tests;
        int passed_tests;
        int failed_tests;
        bool overall_success;
        std::string system_info;
        
        AutotestReport() : total_tests(0), passed_tests(0), failed_tests(0), overall_success(false) {}
    };
    
    AutotestRunner(const std::string& log_file_path, const std::string& output_file_path);
    ~AutotestRunner();
    
    // Main test execution
    bool RunAllTests();
    
    // Individual test methods
    bool TestFileLoading();
    bool TestLogParsing();
    bool TestFilterFunctionality();
    bool TestNavigationLogic();
    bool TestSearchFunctionality();
    bool TestErrorHandling();
    bool TestPerformance();
    
    // Report generation
    void WriteReport();
    const AutotestReport& GetReport() const { return report_; }
    
    // Configuration
    void SetVerbose(bool verbose) { verbose_ = verbose; }
    void SetPerformanceTestEnabled(bool enabled) { performance_test_enabled_ = enabled; }
    
private:
    std::string log_file_path_;
    std::string output_file_path_;
    std::unique_ptr<LogProcessor> log_processor_;
    std::unique_ptr<FilterManager> filter_manager_;
    AutotestReport report_;
    std::ofstream report_file_;
    bool verbose_;
    bool performance_test_enabled_;
    
    // Test execution helpers
    TestResult RunTest(const std::string& test_name, std::function<bool()> test_function);
    void WriteTestHeader(const std::string& test_name);
    void WriteTestResult(const TestResult& result);
    void WriteTestSummary();
    void WriteSystemInfo();
    
    // Utility methods
    std::string GetCurrentTimestamp() const;
    std::string GetSystemInfo() const;
    void LogMessage(const std::string& message);
    void LogError(const std::string& error);
    
    // Test validation helpers
    bool ValidateLogEntries(const std::vector<LogEntry>& entries, const std::string& context);
    bool ValidateFilterResults(const std::vector<LogEntry>& original, 
                              const std::vector<LogEntry>& filtered, 
                              const std::string& filter_description);
    bool ValidateNavigationState(int expected_index, int actual_index, const std::string& operation);
    bool ValidateSearchResults(const std::vector<int>& results, 
                              const std::vector<LogEntry>& entries, 
                              const std::string& query);
};

} // namespace ue_log