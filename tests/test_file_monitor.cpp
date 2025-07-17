#include <catch2/catch_test_macros.hpp>
#include "../lib/file_monitor/file_monitor.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace ue_log;

// Helper function to create a test file
void CreateMonitorTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
}

// Helper function to append lines to a test file
void AppendToMonitorTestFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename, std::ios::app);
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
}

// Helper function to clean up test files
void CleanupMonitorTestFile(const std::string& filename) {
    std::filesystem::remove(filename);
}

TEST_CASE("FileMonitor default constructor initializes correctly", "[file_monitor]") {
    FileMonitor monitor;
    
    REQUIRE(monitor.Get_monitored_file_path().empty());
    REQUIRE(monitor.Get_status() == FileMonitorStatus::Stopped);
    REQUIRE(monitor.Get_poll_interval().count() == 100); // Default 100ms
    REQUIRE(monitor.Get_total_lines_processed() == 0);
    REQUIRE(monitor.Get_total_callbacks_triggered() == 0);
    REQUIRE_FALSE(monitor.IsMonitoring());
}

TEST_CASE("FileMonitor constructor with callback works correctly", "[file_monitor]") {
    bool callback_called = false;
    std::vector<std::string> received_lines;
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        callback_called = true;
        received_lines = new_lines;
    };
    
    FileMonitor monitor(callback);
    
    REQUIRE(monitor.Get_status() == FileMonitorStatus::Stopped);
    REQUIRE_FALSE(monitor.IsMonitoring());
}

TEST_CASE("FileMonitor configuration methods work correctly", "[file_monitor]") {
    FileMonitor monitor;
    
    SECTION("Set poll interval") {
        monitor.SetPollInterval(std::chrono::milliseconds(50));
        REQUIRE(monitor.Get_poll_interval().count() == 50);
        
        // Invalid interval should be ignored
        monitor.SetPollInterval(std::chrono::milliseconds(0));
        REQUIRE(monitor.Get_poll_interval().count() == 50); // Should remain unchanged
    }
    
    SECTION("Set callback") {
        bool callback_called = false;
        FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>&) {
            callback_called = true;
        };
        
        monitor.SetCallback(callback);
        // We can't directly test if callback is set, but we can test it through monitoring
    }
}

TEST_CASE("FileMonitor start/stop functionality works correctly", "[file_monitor]") {
    std::string test_file = "test_monitor_start_stop.txt";
    CreateMonitorTestFile(test_file, {"initial line"});
    
    bool callback_called = false;
    std::vector<std::string> received_lines;
    std::string received_file_path;
    
    FileChangeCallback callback = [&](const std::string& file_path, const std::vector<std::string>& new_lines) {
        callback_called = true;
        received_file_path = file_path;
        received_lines = new_lines;
    };
    
    FileMonitor monitor(callback);
    
    SECTION("Start monitoring with valid file") {
        Result result = monitor.StartMonitoring(test_file);
        if (result.IsError()) {
            INFO("StartMonitoring failed with error: " << result.Get_error_message());
        }
        REQUIRE(result.IsSuccess());
        REQUIRE(monitor.IsMonitoring());
        REQUIRE(monitor.Get_status() == FileMonitorStatus::Running);
        REQUIRE(monitor.Get_monitored_file_path() == test_file);
        
        // Stop monitoring
        Result stop_result = monitor.StopMonitoring();
        REQUIRE(stop_result.IsSuccess());
        REQUIRE_FALSE(monitor.IsMonitoring());
        REQUIRE(monitor.Get_status() == FileMonitorStatus::Stopped);
    }
    
    SECTION("Start monitoring with non-existent file") {
        Result result = monitor.StartMonitoring("non_existent_file.txt");
        REQUIRE(result.IsError());
        REQUIRE(result.Get_error_message().find("File does not exist") != std::string::npos);
        REQUIRE_FALSE(monitor.IsMonitoring());
    }
    
    SECTION("Start monitoring with empty file path") {
        Result result = monitor.StartMonitoring("");
        REQUIRE(result.IsError());
        REQUIRE(result.Get_error_message().find("File path cannot be empty") != std::string::npos);
        REQUIRE_FALSE(monitor.IsMonitoring());
    }
    
    SECTION("Start monitoring without callback") {
        FileMonitor no_callback_monitor;
        Result result = no_callback_monitor.StartMonitoring(test_file);
        REQUIRE(result.IsError());
        REQUIRE(result.Get_error_message().find("Callback function must be set") != std::string::npos);
    }
    
    SECTION("Start monitoring when already running") {
        Result start_result = monitor.StartMonitoring(test_file);
        REQUIRE(start_result.IsSuccess());
        
        Result second_start = monitor.StartMonitoring(test_file);
        REQUIRE(second_start.IsError());
        REQUIRE(second_start.Get_error_message().find("already running") != std::string::npos);
        
        monitor.StopMonitoring();
    }
    
    CleanupMonitorTestFile(test_file);
}

TEST_CASE("FileMonitor detects file changes correctly", "[file_monitor]") {
    std::string test_file = "test_monitor_changes.txt";
    CreateMonitorTestFile(test_file, {"initial line"});
    
    bool callback_called = false;
    std::vector<std::string> received_lines;
    std::string received_file_path;
    
    FileChangeCallback callback = [&](const std::string& file_path, const std::vector<std::string>& new_lines) {
        callback_called = true;
        received_file_path = file_path;
        received_lines = new_lines;
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(50)); // Fast polling for testing
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait a bit for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Add new lines to the file
    AppendToMonitorTestFile(test_file, {"new line 1", "new line 2"});
    
    // Wait for the monitor to detect changes
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Check that callback was called
    REQUIRE(callback_called);
    REQUIRE(received_file_path == test_file);
    REQUIRE(received_lines.size() == 2);
    REQUIRE(received_lines[0] == "new line 1");
    REQUIRE(received_lines[1] == "new line 2");
    
    // Check statistics
    REQUIRE(monitor.Get_total_lines_processed() >= 2);
    REQUIRE(monitor.Get_total_callbacks_triggered() >= 1);
    
    monitor.StopMonitoring();
    CleanupMonitorTestFile(test_file);
}

TEST_CASE("FileMonitor handles file rotation correctly", "[file_monitor]") {
    std::string test_file = "test_monitor_rotation.txt";
    CreateMonitorTestFile(test_file, {"initial line"});
    
    std::vector<std::vector<std::string>> all_callbacks;
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        all_callbacks.push_back(new_lines);
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(50));
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Add some lines
    AppendToMonitorTestFile(test_file, {"line before rotation"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Simulate file rotation by recreating the file
    CleanupMonitorTestFile(test_file);
    CreateMonitorTestFile(test_file, {"line after rotation"});
    
    // Wait for detection
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Add more lines after rotation
    AppendToMonitorTestFile(test_file, {"another line after rotation"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    REQUIRE(all_callbacks.size() >= 2);
    
    monitor.StopMonitoring();
    CleanupMonitorTestFile(test_file);
}

TEST_CASE("FileMonitor statistics work correctly", "[file_monitor]") {
    std::string test_file = "test_monitor_stats.txt";
    CreateMonitorTestFile(test_file, {"initial line"});
    
    int callback_count = 0;
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>&) {
        callback_count++;
    };
    
    FileMonitor monitor(callback);
    
    SECTION("Initial statistics") {
        REQUIRE(monitor.Get_total_lines_processed() == 0);
        REQUIRE(monitor.Get_total_callbacks_triggered() == 0);
        
        std::string summary = monitor.GetStatisticsSummary();
        REQUIRE(summary.find("Status: Stopped") != std::string::npos);
        REQUIRE(summary.find("Total Lines Processed: 0") != std::string::npos);
        REQUIRE(summary.find("Total Callbacks Triggered: 0") != std::string::npos);
    }
    
    SECTION("Statistics after monitoring") {
        monitor.SetPollInterval(std::chrono::milliseconds(50));
        Result start_result = monitor.StartMonitoring(test_file);
        REQUIRE(start_result.IsSuccess());
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Add lines and wait for processing
        AppendToMonitorTestFile(test_file, {"line 1", "line 2", "line 3"});
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        REQUIRE(monitor.Get_total_lines_processed() >= 3);
        REQUIRE(monitor.Get_total_callbacks_triggered() >= 1);
        
        std::string summary = monitor.GetStatisticsSummary();
        REQUIRE(summary.find("Status: Running") != std::string::npos);
        REQUIRE(summary.find(test_file) != std::string::npos);
        
        monitor.StopMonitoring();
    }
    
    SECTION("Reset statistics") {
        monitor.ResetStatistics();
        REQUIRE(monitor.Get_total_lines_processed() == 0);
        REQUIRE(monitor.Get_total_callbacks_triggered() == 0);
    }
    
    CleanupMonitorTestFile(test_file);
}

TEST_CASE("FileMonitor status management works correctly", "[file_monitor]") {
    FileMonitor monitor;
    
    SECTION("Status string conversion") {
        REQUIRE(monitor.GetStatusString() == "Stopped");
    }
    
    SECTION("Wait for status") {
        // Should immediately return true for current status
        bool result = monitor.WaitForStatus(FileMonitorStatus::Stopped, std::chrono::milliseconds(100));
        REQUIRE(result);
        
        // Should timeout waiting for different status
        bool timeout_result = monitor.WaitForStatus(FileMonitorStatus::Running, std::chrono::milliseconds(50));
        REQUIRE_FALSE(timeout_result);
    }
}

TEST_CASE("FileMonitor thread safety works correctly", "[file_monitor]") {
    std::string test_file = "test_monitor_thread_safety.txt";
    CreateMonitorTestFile(test_file, {"initial line"});
    
    std::atomic<int> callback_count(0);
    std::atomic<int> total_lines(0);
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>& new_lines) {
        callback_count++;
        total_lines += static_cast<int>(new_lines.size());
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(25)); // Very fast polling
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Simulate concurrent file writes
    std::vector<std::thread> writers;
    for (int i = 0; i < 3; ++i) {
        writers.emplace_back([&, i]() {
            for (int j = 0; j < 5; ++j) {
                AppendToMonitorTestFile(test_file, {"thread " + std::to_string(i) + " line " + std::to_string(j)});
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    // Wait for all writers to complete
    for (auto& writer : writers) {
        writer.join();
    }
    
    // Wait for monitor to process all changes
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Verify that all lines were processed
    REQUIRE(callback_count > 0);
    REQUIRE(total_lines >= 10); // Should have processed most lines (allow for timing variations)
    
    monitor.StopMonitoring();
    CleanupMonitorTestFile(test_file);
}

TEST_CASE("FileMonitor performance requirements", "[file_monitor]") {
    std::string test_file = "test_monitor_performance.txt";
    CreateMonitorTestFile(test_file, {"initial line"});
    
    auto start_time = std::chrono::steady_clock::now();
    std::atomic<bool> callback_received(false);
    
    FileChangeCallback callback = [&](const std::string&, const std::vector<std::string>&) {
        auto callback_time = std::chrono::steady_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(callback_time - start_time);
        
        // Verify <100ms latency requirement
        REQUIRE(latency.count() < 100);
        callback_received = true;
    };
    
    FileMonitor monitor(callback);
    monitor.SetPollInterval(std::chrono::milliseconds(10)); // Very fast polling for performance test
    
    Result start_result = monitor.StartMonitoring(test_file);
    REQUIRE(start_result.IsSuccess());
    
    // Wait for monitoring to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Record time and add new content
    start_time = std::chrono::steady_clock::now();
    AppendToMonitorTestFile(test_file, {"performance test line"});
    
    // Wait for callback with timeout
    auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
    while (!callback_received && std::chrono::steady_clock::now() < timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    REQUIRE(callback_received);
    
    monitor.StopMonitoring();
    CleanupMonitorTestFile(test_file);
}