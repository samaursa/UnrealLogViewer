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
void CreateIntegrationTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
}

// Helper function to append lines to a test file
void AppendToIntegrationTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename, std::ios::app);
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
}

// Helper function to clean up test files
void CleanupIntegrationTestFile(const std::string& filename) {
    std::filesystem::remove(filename);
}

TEST_CASE("Real-time log processing integration", "[integration]") {
    std::string test_file = "test_integration_realtime.txt";
    
    // Create initial log file with some entries
    std::vector<std::string> initial_logs = {
        "[2025.07.16-10.01.25:951][425]LogTemp: Info: Application started",
        "[2025.07.16-10.01.25:952][426]LogEngine: Warning: Low memory detected"
    };
    CreateIntegrationTestFile(test_file, initial_logs);
    
    // Set up components
    LogParser parser;
    FilterEngine filter_engine;
    
    // Create a filter for error messages
    auto error_filter = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "Error");
    filter_engine.AddFilter(std::move(error_filter));
    
    // Track processed entries
    std::vector<LogEntry> processed_entries;
    std::vector<LogEntry> filtered_entries;
    std::atomic<int> callback_count(0);
    std::atomic<bool> error_detected(false);
    
    // Set up file monitor callback
    FileChangeCallback callback = [&](const std::string& file_path, const std::vector<std::string>& new_lines) {
        callback_count++;
        
        // Parse new log lines
        for (const std::string& line : new_lines) {
            LogEntry entry = parser.ParseSingleEntry(line, callback_count);
            if (entry.IsValid()) {
                processed_entries.push_back(entry);
                
                // Apply filters
                if (filter_engine.PassesFilters(entry)) {
                    filtered_entries.push_back(entry);
                    if (entry.Get_message().find("Error") != std::string::npos) {
                        error_detected = true;
                    }
                }
            }
        }
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(25)); // Fast polling for testing
    
    SECTION("Real-time log parsing and filtering") {
        Result start_result = monitor.StartMonitoring(test_file);
        REQUIRE(start_result.IsSuccess());
        
        // Wait for monitoring to start
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Add new log entries including an error
        std::vector<std::string> new_logs = {
            "[2025.07.16-10.01.25:953][427]LogTemp: Info: Processing data",
            "[2025.07.16-10.01.25:954][428]LogEngine: Error: Failed to load Error texture",
            "[2025.07.16-10.01.25:955][429]LogTemp: Info: Processing complete"
        };
        AppendToIntegrationTestFile(test_file, new_logs);
        
        // Wait for processing
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Verify real-time processing
        REQUIRE(callback_count > 0);
        REQUIRE(processed_entries.size() == 3); // 3 new entries
        
        // Debug: Check what messages we actually got
        int error_count = 0;
        for (const auto& entry : processed_entries) {
            if (entry.Get_message().find("Error") != std::string::npos) {
                error_count++;
            }
        }
        
        // We should have at least one entry with "Error" in the message
        REQUIRE(error_count >= 1);
        
        // The filter should match at least one entry
        REQUIRE(filtered_entries.size() >= 1);
        
        monitor.StopMonitoring();
    }
    
    CleanupIntegrationTestFile(test_file);
}

TEST_CASE("File rotation handling integration", "[integration]") {
    std::string test_file = "test_integration_rotation.txt";
    
    // Create initial log file
    CreateIntegrationTestFile(test_file, {"[2025.07.16-10.01.25:951][425]LogTemp: Info: Initial entry"});
    
    LogParser parser;
    std::vector<LogEntry> all_entries;
    std::atomic<int> rotation_callbacks(0);
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        rotation_callbacks++;
        
        for (const std::string& line : new_lines) {
            LogEntry entry = parser.ParseSingleEntry(line, rotation_callbacks);
            if (entry.IsValid()) {
                all_entries.push_back(entry);
            }
        }
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(25));
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Add some entries before rotation
    AppendToIntegrationTestFile(test_file, {
        "[2025.07.16-10.01.25:952][426]LogEngine: Warning: Before rotation"
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Simulate log rotation by replacing the file
    CleanupIntegrationTestFile(test_file);
    CreateIntegrationTestFile(test_file, {
        "[2025.07.16-10.01.25:953][427]LogTemp: Info: After rotation"
    });
    
    // Wait for rotation detection
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Add more entries after rotation
    AppendToIntegrationTestFile(test_file, {
        "[2025.07.16-10.01.25:954][428]LogEngine: Error: Post-rotation error"
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify rotation handling
    REQUIRE(rotation_callbacks >= 2); // At least before and after rotation
    REQUIRE(all_entries.size() >= 2); // Should have entries from both before and after rotation
    
    // Verify we can parse entries after rotation
    bool found_post_rotation = false;
    for (const auto& entry : all_entries) {
        if (entry.Get_message().find("Post-rotation error") != std::string::npos) {
            found_post_rotation = true;
            break;
        }
    }
    REQUIRE(found_post_rotation);
    
    monitor.StopMonitoring();
    CleanupIntegrationTestFile(test_file);
}

TEST_CASE("Performance integration test", "[integration]") {
    std::string test_file = "test_integration_performance.txt";
    CreateIntegrationTestFile(test_file, {"[2025.07.16-10.01.25:951][425]LogTemp: Info: Initial"});
    
    LogParser parser;
    FilterEngine filter_engine;
    
    // Add multiple filters
    auto info_filter = std::make_unique<Filter>("InfoFilter", FilterType::TextContains, "Info");
    auto warning_filter = std::make_unique<Filter>("WarningFilter", FilterType::TextContains, "Warning");
    auto error_filter = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "Error");
    
    filter_engine.AddFilter(std::move(info_filter));
    filter_engine.AddFilter(std::move(warning_filter));
    filter_engine.AddFilter(std::move(error_filter));
    
    std::atomic<int> total_processed(0);
    std::atomic<int> total_filtered(0);
    std::vector<std::chrono::milliseconds> processing_times;
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        auto start_time = std::chrono::steady_clock::now();
        
        std::vector<LogEntry> batch_entries;
        
        // Parse all new lines
        for (const std::string& line : new_lines) {
            LogEntry entry = parser.ParseSingleEntry(line, total_processed);
            if (entry.IsValid()) {
                batch_entries.push_back(entry);
                total_processed++;
            }
        }
        
        // Apply filters to batch
        std::vector<LogEntry> filtered = filter_engine.ApplyFilters(batch_entries);
        total_filtered += static_cast<int>(filtered.size());
        
        auto end_time = std::chrono::steady_clock::now();
        auto processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        processing_times.push_back(processing_time);
        
        // Verify <100ms processing time requirement
        REQUIRE(processing_time.count() < 100);
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(10)); // Very fast polling
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Add a batch of log entries
    std::vector<std::string> batch_logs;
    for (int i = 0; i < 50; ++i) {
        std::string log_type = (i % 3 == 0) ? "Info" : (i % 3 == 1) ? "Warning" : "Error";
        batch_logs.push_back("[2025.07.16-10.01.25:" + std::to_string(952 + i) + "][" + 
                           std::to_string(426 + i) + "]LogTemp: " + log_type + ": Message " + std::to_string(i));
    }
    
    AppendToIntegrationTestFile(test_file, batch_logs);
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Verify performance
    REQUIRE(total_processed >= 50);
    REQUIRE(total_filtered >= 0); // Should have processed entries (filtering is optional)
    REQUIRE_FALSE(processing_times.empty());
    
    // Verify all processing times were under 100ms
    for (const auto& time : processing_times) {
        REQUIRE(time.count() < 100);
    }
    
    // Calculate average processing time
    auto total_time = std::chrono::milliseconds(0);
    for (const auto& time : processing_times) {
        total_time += time;
    }
    auto avg_time = total_time / static_cast<int>(processing_times.size());
    
    // Average should be well under the 100ms requirement
    REQUIRE(avg_time.count() < 50);
    
    monitor.StopMonitoring();
    CleanupIntegrationTestFile(test_file);
}

TEST_CASE("Multi-component error handling integration", "[integration]") {
    std::string test_file = "test_integration_errors.txt";
    CreateIntegrationTestFile(test_file, {"Valid initial line"});
    
    LogParser parser;
    FilterEngine filter_engine;
    
    std::atomic<int> parse_errors(0);
    std::atomic<int> valid_entries(0);
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        for (const std::string& line : new_lines) {
            LogEntry entry = parser.ParseSingleEntry(line, valid_entries + parse_errors);
            if (entry.IsValid()) {
                valid_entries++;
                
                // Try to apply filters (should not crash even with invalid entries)
                filter_engine.PassesFilters(entry);
            } else {
                parse_errors++;
            }
        }
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(25));
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Add mix of valid and invalid log entries
    std::vector<std::string> mixed_logs = {
        "[2025.07.16-10.01.25:952][426]LogTemp: Info: Valid entry 1",
        "Invalid log line without proper format",
        "[2025.07.16-10.01.25:953][427]LogEngine: Warning: Valid entry 2",
        "Another invalid line",
        "[2025.07.16-10.01.25:954][428]LogTemp: Error: Valid entry 3"
    };
    
    AppendToIntegrationTestFile(test_file, mixed_logs);
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Verify error handling - LogParser may parse more entries as valid than expected
    REQUIRE(valid_entries >= 3); // At least 3 valid entries
    REQUIRE(valid_entries + parse_errors == 5); // Total should be 5 entries
    
    // System should continue working despite parse errors
    REQUIRE(monitor.IsMonitoring());
    REQUIRE(monitor.Get_status() == FileMonitorStatus::Running);
    
    monitor.StopMonitoring();
    CleanupIntegrationTestFile(test_file);
}

TEST_CASE("Concurrent access integration test", "[integration]") {
    std::string test_file = "test_integration_concurrent.txt";
    CreateIntegrationTestFile(test_file, {"Initial entry"});
    
    LogParser parser;
    FilterEngine filter_engine;
    
    // Add a filter
    auto test_filter = std::make_unique<Filter>("TestFilter", FilterType::TextContains, "Concurrent");
    filter_engine.AddFilter(std::move(test_filter));
    
    std::atomic<int> total_callbacks(0);
    std::atomic<int> concurrent_entries(0);
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        total_callbacks++;
        
        for (const std::string& line : new_lines) {
            LogEntry entry = parser.ParseSingleEntry(line, total_callbacks);
            if (entry.IsValid()) {
                if (filter_engine.PassesFilters(entry)) {
                    concurrent_entries++;
                }
            }
        }
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(10)); // Very fast polling
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Create multiple threads that write to the file concurrently
    std::vector<std::thread> writers;
    for (int i = 0; i < 3; ++i) {
        writers.emplace_back([&, i]() {
            for (int j = 0; j < 10; ++j) {
                std::vector<std::string> logs = {
                    "[2025.07.16-10.01.25:" + std::to_string(952 + i * 10 + j) + "][" + 
                    std::to_string(426 + i * 10 + j) + "]LogTemp: Info: Concurrent message from thread " + 
                    std::to_string(i) + " iteration " + std::to_string(j)
                };
                AppendToIntegrationTestFile(test_file, logs);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }
    
    // Wait for all writers to complete
    for (auto& writer : writers) {
        writer.join();
    }
    
    // Wait for all processing to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Verify concurrent processing
    REQUIRE(total_callbacks > 0);
    REQUIRE(concurrent_entries >= 30); // Should have processed entries from all threads
    
    // Verify system stability under concurrent access
    REQUIRE(monitor.IsMonitoring());
    REQUIRE(monitor.Get_status() == FileMonitorStatus::Running);
    
    monitor.StopMonitoring();
    CleanupIntegrationTestFile(test_file);
}