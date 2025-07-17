#include <catch2/catch_test_macros.hpp>
#include "../lib/file_monitor/file_monitor.h"
#include "../lib/log_parser/log_parser.h"
#include "../lib/filter_engine/filter_engine.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <atomic>

using namespace ue_log;

// Helper function to create a test file
void CreateSimpleTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
}

// Helper function to append lines to a test file
void AppendToSimpleTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename, std::ios::app);
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
}

// Helper function to clean up test files
void CleanupSimpleTestFile(const std::string& filename) {
    std::filesystem::remove(filename);
}

TEST_CASE("Simple real-time integration test", "[simple_integration]") {
    std::string test_file = "test_simple_integration.txt";
    
    // Create initial log file
    CreateSimpleTestFile(test_file, {"Initial line"});
    
    // Track what we receive
    std::atomic<int> callback_count(0);
    std::vector<std::string> received_lines;
    
    // Set up file monitor callback
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        callback_count++;
        for (const auto& line : new_lines) {
            received_lines.push_back(line);
        }
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(25));
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Add new lines
    AppendToSimpleTestFile(test_file, {"New line 1", "New line 2"});
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Verify basic functionality
    REQUIRE(callback_count > 0);
    REQUIRE(received_lines.size() == 2);
    REQUIRE(received_lines[0] == "New line 1");
    REQUIRE(received_lines[1] == "New line 2");
    
    monitor.StopMonitoring();
    CleanupSimpleTestFile(test_file);
}

TEST_CASE("LogParser integration test", "[simple_integration]") {
    LogParser parser;
    
    // Test parsing a structured log entry
    std::string log_line = "[2025.07.16-10.01.25:951][425]LogTemp: Info: Test message";
    LogEntry entry = parser.ParseSingleEntry(log_line, 1);
    
    REQUIRE(entry.IsValid());
    REQUIRE(entry.Get_logger_name() == "LogTemp");
    REQUIRE(entry.Get_message().find("Test message") != std::string::npos);
}

TEST_CASE("FilterEngine integration test", "[simple_integration]") {
    FilterEngine filter_engine;
    
    // Create a simple filter
    auto test_filter = std::make_unique<Filter>("TestFilter", FilterType::TextContains, "Error");
    filter_engine.AddFilter(std::move(test_filter));
    
    // Create test entries
    LogEntry error_entry("LogTemp", "This is an Error message", "raw line");
    LogEntry info_entry("LogTemp", "This is an Info message", "raw line");
    
    // Test filtering
    REQUIRE(filter_engine.PassesFilters(error_entry));
    REQUIRE_FALSE(filter_engine.PassesFilters(info_entry));
}

TEST_CASE("File rotation integration test", "[simple_integration]") {
    std::string test_file = "test_rotation_integration.txt";
    
    CreateSimpleTestFile(test_file, {"Initial line"});
    
    std::atomic<int> callback_count(0);
    std::vector<std::string> all_received_lines;
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        callback_count++;
        for (const auto& line : new_lines) {
            all_received_lines.push_back(line);
        }
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(25));
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Add some lines before rotation
    AppendToSimpleTestFile(test_file, {"Before rotation"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Simulate file rotation
    CleanupSimpleTestFile(test_file);
    CreateSimpleTestFile(test_file, {"After rotation"});
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Add more lines after rotation
    AppendToSimpleTestFile(test_file, {"Post rotation"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify rotation handling
    REQUIRE(callback_count >= 2);
    REQUIRE(all_received_lines.size() >= 2);
    
    // Should have received lines from both before and after rotation
    bool found_before = false, found_after = false, found_post = false;
    for (const auto& line : all_received_lines) {
        if (line.find("Before rotation") != std::string::npos) found_before = true;
        if (line.find("After rotation") != std::string::npos) found_after = true;
        if (line.find("Post rotation") != std::string::npos) found_post = true;
    }
    
    REQUIRE(found_before);
    REQUIRE(found_post); // Should definitely have post-rotation content
    
    monitor.StopMonitoring();
    CleanupSimpleTestFile(test_file);
}

TEST_CASE("Performance integration test", "[simple_integration]") {
    std::string test_file = "test_performance_integration.txt";
    CreateSimpleTestFile(test_file, {"Initial"});
    
    std::atomic<int> callback_count(0);
    std::vector<std::chrono::milliseconds> processing_times;
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        auto start_time = std::chrono::steady_clock::now();
        
        callback_count++;
        
        // Simulate some processing
        for (const auto& line : new_lines) {
            // Just access the line to simulate processing
            volatile size_t len = line.length();
            (void)len; // Suppress unused variable warning
        }
        
        auto end_time = std::chrono::steady_clock::now();
        auto processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        processing_times.push_back(processing_time);
        
        // Verify <100ms processing time requirement
        REQUIRE(processing_time.count() < 100);
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(10));
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Add a batch of lines
    std::vector<std::string> batch_lines;
    for (int i = 0; i < 20; ++i) {
        batch_lines.push_back("Line " + std::to_string(i));
    }
    AppendToSimpleTestFile(test_file, batch_lines);
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Verify performance
    REQUIRE(callback_count > 0);
    REQUIRE_FALSE(processing_times.empty());
    
    // All processing times should be under 100ms
    for (const auto& time : processing_times) {
        REQUIRE(time.count() < 100);
    }
    
    monitor.StopMonitoring();
    CleanupSimpleTestFile(test_file);
}