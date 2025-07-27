#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include "../lib/common/unreal_project_utils.h"

class UnrealProjectUtilsTest {
public:
    void SetUp() {
        // Create a temporary test directory
        test_dir = std::filesystem::temp_directory_path() / "unreal_utils_test";
        std::filesystem::create_directories(test_dir);
        
        // Create test subdirectories
        saved_logs_dir = test_dir / "Saved" / "Logs";
        logs_dir = test_dir / "Logs";
        empty_dir = test_dir / "empty";
        
        std::filesystem::create_directories(saved_logs_dir);
        std::filesystem::create_directories(logs_dir);
        std::filesystem::create_directories(empty_dir);
    }
    
    void TearDown() {
        // Clean up test directory
        std::error_code ec;
        std::filesystem::remove_all(test_dir, ec);
    }
    
    void CreateTestLogFile(const std::filesystem::path& dir, const std::string& filename, const std::string& content = "test log content") {
        std::filesystem::path file_path = dir / filename;
        std::ofstream file(file_path);
        file << content;
        file.close();
    }
    
    void CreateTestLogFileWithDelay(const std::filesystem::path& dir, const std::string& filename, int delay_ms = 100) {
        if (delay_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
        CreateTestLogFile(dir, filename);
    }
    
    std::filesystem::path test_dir;
    std::filesystem::path saved_logs_dir;
    std::filesystem::path logs_dir;
    std::filesystem::path empty_dir;
};

TEST_CASE("FindSavedLogsDirectory with current directory", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    // Test with existing Saved/Logs directory (created in SetUp)
    std::string result = ue_log::unreal_utils::FindSavedLogsDirectory(test.test_dir.string());
    REQUIRE(result == test.saved_logs_dir.string());
    
    // Test with no Saved/Logs directory
    std::filesystem::remove_all(test.saved_logs_dir);
    std::filesystem::remove_all(test.logs_dir);
    result = ue_log::unreal_utils::FindSavedLogsDirectory(test.test_dir.string());
    REQUIRE(result.empty());
    
    test.TearDown();
}

TEST_CASE("FindSavedLogsDirectory with specific directory", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    test.CreateTestLogFile(test.saved_logs_dir, "test.log");
    
    std::string result = ue_log::unreal_utils::FindSavedLogsDirectory(test.test_dir.string());
    REQUIRE(result == test.saved_logs_dir.string());
    
    // Test with non-existent directory
    result = ue_log::unreal_utils::FindSavedLogsDirectory("/non/existent/path");
    REQUIRE(result.empty());
    
    test.TearDown();
}

TEST_CASE("FindSavedLogsDirectory fallback to Logs directory", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    // Remove Saved/Logs but keep Logs
    std::filesystem::remove_all(test.saved_logs_dir);
    test.CreateTestLogFile(test.logs_dir, "test.log");
    
    std::string result = ue_log::unreal_utils::FindSavedLogsDirectory(test.test_dir.string());
    REQUIRE(result == test.logs_dir.string());
    
    test.TearDown();
}

TEST_CASE("FindSavedLogsDirectory when already in Logs directory", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    test.CreateTestLogFile(test.saved_logs_dir, "test.log");
    
    std::string result = ue_log::unreal_utils::FindSavedLogsDirectory(test.saved_logs_dir.string());
    REQUIRE(result == test.saved_logs_dir.string());
    
    test.TearDown();
}

TEST_CASE("ContainsLogFiles with log files present", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    test.CreateTestLogFile(test.saved_logs_dir, "test1.log");
    test.CreateTestLogFile(test.saved_logs_dir, "test2.log");
    test.CreateTestLogFile(test.saved_logs_dir, "not_a_log.txt");
    
    bool result = ue_log::unreal_utils::ContainsLogFiles(test.saved_logs_dir.string());
    REQUIRE(result == true);
    
    test.TearDown();
}

TEST_CASE("ContainsLogFiles with no log files", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    test.CreateTestLogFile(test.saved_logs_dir, "not_a_log.txt");
    test.CreateTestLogFile(test.saved_logs_dir, "another.dat");
    
    bool result = ue_log::unreal_utils::ContainsLogFiles(test.saved_logs_dir.string());
    REQUIRE(result == false);
    
    test.TearDown();
}

TEST_CASE("ContainsLogFiles with empty directory", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    bool result = ue_log::unreal_utils::ContainsLogFiles(test.empty_dir.string());
    REQUIRE(result == false);
    
    test.TearDown();
}

TEST_CASE("ContainsLogFiles with invalid directory", "[unreal_utils]") {
    bool result = ue_log::unreal_utils::ContainsLogFiles("/non/existent/path");
    REQUIRE(result == false);
}

TEST_CASE("GetLogFiles sorted by modification time", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    // Create files with delays to ensure different modification times
    test.CreateTestLogFile(test.saved_logs_dir, "oldest.log");
    test.CreateTestLogFileWithDelay(test.saved_logs_dir, "middle.log", 50);
    test.CreateTestLogFileWithDelay(test.saved_logs_dir, "newest.log", 50);
    
    // Add a non-log file to ensure it's filtered out
    test.CreateTestLogFile(test.saved_logs_dir, "not_a_log.txt");
    
    auto result = ue_log::unreal_utils::GetLogFiles(test.saved_logs_dir.string());
    
    REQUIRE(result.size() == 3);
    
    // Files should be sorted by modification time (newest first)
    REQUIRE(result[0].find("newest.log") != std::string::npos);
    REQUIRE(result[1].find("middle.log") != std::string::npos);
    REQUIRE(result[2].find("oldest.log") != std::string::npos);
    
    test.TearDown();
}

TEST_CASE("GetLogFiles with empty directory", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    auto result = ue_log::unreal_utils::GetLogFiles(test.empty_dir.string());
    REQUIRE(result.empty());
    
    test.TearDown();
}

TEST_CASE("GetLogFiles with invalid directory", "[unreal_utils]") {
    auto result = ue_log::unreal_utils::GetLogFiles("/non/existent/path");
    REQUIRE(result.empty());
}

TEST_CASE("ValidateDirectoryPath with valid directory", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    bool result = ue_log::unreal_utils::ValidateDirectoryPath(test.test_dir.string());
    REQUIRE(result == true);
    
    test.TearDown();
}

TEST_CASE("ValidateDirectoryPath with invalid directory", "[unreal_utils]") {
    bool result = ue_log::unreal_utils::ValidateDirectoryPath("/non/existent/path");
    REQUIRE(result == false);
}

TEST_CASE("ValidateDirectoryPath with file instead of directory", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    std::filesystem::path file_path = test.test_dir / "test_file.txt";
    std::ofstream file(file_path);
    file << "test content";
    file.close();
    
    bool result = ue_log::unreal_utils::ValidateDirectoryPath(file_path.string());
    REQUIRE(result == false);
    
    test.TearDown();
}

TEST_CASE("GetFileMetadata with valid file", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    std::filesystem::path file_path = test.test_dir / "test.log";
    std::string content = "This is test log content with some length";
    std::ofstream file(file_path);
    file << content;
    file.close();
    
    auto [size, mod_time] = ue_log::unreal_utils::GetFileMetadata(file_path.string());
    
    REQUIRE(size > 0);
    REQUIRE(mod_time != std::filesystem::file_time_type{});
    
    test.TearDown();
}

TEST_CASE("GetFileMetadata with non-existent file", "[unreal_utils]") {
    auto [size, mod_time] = ue_log::unreal_utils::GetFileMetadata("/non/existent/file.log");
    
    REQUIRE(size == 0);
    REQUIRE(mod_time == std::filesystem::file_time_type{});
}

TEST_CASE("GetFileMetadata with directory instead of file", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    auto [size, mod_time] = ue_log::unreal_utils::GetFileMetadata(test.test_dir.string());
    
    REQUIRE(size == 0);
    REQUIRE(mod_time == std::filesystem::file_time_type{});
    
    test.TearDown();
}

TEST_CASE("Integration test: Complete workflow", "[unreal_utils]") {
    UnrealProjectUtilsTest test;
    test.SetUp();
    
    // Create a realistic Unreal project structure
    test.CreateTestLogFile(test.saved_logs_dir, "MyProject.log");
    test.CreateTestLogFileWithDelay(test.saved_logs_dir, "MyProject-2024.01.01-12.00.00.log", 50);
    test.CreateTestLogFileWithDelay(test.saved_logs_dir, "MyProject-2024.01.01-13.00.00.log", 50);
    
    // 1. Find the Saved/Logs directory
    std::string logs_path = ue_log::unreal_utils::FindSavedLogsDirectory(test.test_dir.string());
    REQUIRE(!logs_path.empty());
    REQUIRE(logs_path == test.saved_logs_dir.string());
    
    // 2. Verify it contains log files
    bool has_logs = ue_log::unreal_utils::ContainsLogFiles(logs_path);
    REQUIRE(has_logs == true);
    
    // 3. Get all log files sorted by modification time
    auto log_files = ue_log::unreal_utils::GetLogFiles(logs_path);
    REQUIRE(log_files.size() == 3);
    
    // 4. Verify the newest file is first
    REQUIRE(log_files[0].find("13.00.00") != std::string::npos);
    
    // 5. Get metadata for the newest file
    auto [size, mod_time] = ue_log::unreal_utils::GetFileMetadata(log_files[0]);
    REQUIRE(size > 0);
    REQUIRE(mod_time != std::filesystem::file_time_type{});
    
    test.TearDown();
}