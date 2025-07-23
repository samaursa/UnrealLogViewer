#include <catch2/catch_test_macros.hpp>
#include "../lib/log_parser/log_parser.h"
#include <fstream>
#include <filesystem>

using namespace ue_log;

TEST_CASE("LogParser Multi-line Parsing", "[log_parser][multiline]") {
    LogParser parser;
    
    SECTION("Groups continuation lines with previous entry") {
        // Create a test log file with multi-line entries
        std::string test_file = "test_multiline_parsing.log";
        std::ofstream file(test_file);
        file << "[2024.09.30-14.56.10:293][ 12]LogD3D12RHI: Error: [GPUBreadCrumb]\t3D Queue 0\n";
        file << "Breadcrumbs: > Frame 18010 [Active]\n";
        file << "Breadcrumbs: | BufferPoolCopyOps [Finished]\n";
        file << "Breadcrumbs: | TexturePoolCopyOps [Finished]\n";
        file << "[2024.09.30-14.56.11:294][ 13]LogEngine: Display: Another log entry\n";
        file << "Some continuation line\n";
        file << "Another continuation line\n";
        file << "[2024.09.30-14.56.12:295][ 14]LogCore: Warning: Final entry\n";
        file.close();
        
        // Parse the file
        auto result = parser.LoadFile(test_file);
        REQUIRE(result.IsSuccess());
        
        auto entries = parser.ParseEntries();
        
        // Should have 3 entries (not 8 individual lines)
        REQUIRE(entries.size() == 3);
        
        // First entry should have multi-line message
        REQUIRE(entries[0].Get_logger_name() == "LogD3D12RHI");
        REQUIRE(entries[0].Get_log_level().value() == "Error");
        std::string first_message = entries[0].Get_message();
        REQUIRE(first_message.find("GPUBreadCrumb") != std::string::npos);
        REQUIRE(first_message.find("Breadcrumbs: > Frame 18010") != std::string::npos);
        REQUIRE(first_message.find("BufferPoolCopyOps") != std::string::npos);
        REQUIRE(first_message.find("TexturePoolCopyOps") != std::string::npos);
        
        // Second entry should have multi-line message
        REQUIRE(entries[1].Get_logger_name() == "LogEngine");
        REQUIRE(entries[1].Get_log_level().value() == "Display");
        std::string second_message = entries[1].Get_message();
        REQUIRE(second_message.find("Another log entry") != std::string::npos);
        REQUIRE(second_message.find("Some continuation line") != std::string::npos);
        REQUIRE(second_message.find("Another continuation line") != std::string::npos);
        
        // Third entry should be single line
        REQUIRE(entries[2].Get_logger_name() == "LogCore");
        REQUIRE(entries[2].Get_log_level().value() == "Warning");
        REQUIRE(entries[2].Get_message() == "Final entry");
        
        // Clean up - ignore errors if file is locked
        try {
            std::filesystem::remove(test_file);
        } catch (...) {
            // Ignore cleanup errors
        }
    }
    
    SECTION("Handles empty continuation lines correctly") {
        std::string test_file = "test_empty_continuation.log";
        std::ofstream file(test_file);
        file << "[2024.09.30-14.56.10:293][ 12]LogTest: Info: Main message\n";
        file << "\n"; // Empty line
        file << "    \n"; // Line with only spaces
        file << "Continuation after empty lines\n";
        file << "[2024.09.30-14.56.11:294][ 13]LogTest: Info: Next entry\n";
        file.close();
        
        auto result = parser.LoadFile(test_file);
        REQUIRE(result.IsSuccess());
        
        auto entries = parser.ParseEntries();
        REQUIRE(entries.size() == 2);
        
        // First entry should include the continuation line but skip empty lines
        std::string first_message = entries[0].Get_message();
        REQUIRE(first_message.find("Main message") != std::string::npos);
        REQUIRE(first_message.find("Continuation after empty lines") != std::string::npos);
        
        try {
            std::filesystem::remove(test_file);
        } catch (...) {
            // Ignore cleanup errors
        }
    }
    
    SECTION("Single line entries work as before") {
        std::string test_file = "test_single_lines.log";
        std::ofstream file(test_file);
        file << "[2024.09.30-14.56.10:293][ 12]LogTest: Info: First entry\n";
        file << "[2024.09.30-14.56.11:294][ 13]LogTest: Warning: Second entry\n";
        file << "[2024.09.30-14.56.12:295][ 14]LogTest: Error: Third entry\n";
        file.close();
        
        auto result = parser.LoadFile(test_file);
        REQUIRE(result.IsSuccess());
        
        auto entries = parser.ParseEntries();
        REQUIRE(entries.size() == 3);
        
        // Messages might include the log level prefix depending on parsing
        REQUIRE(entries[0].Get_message().find("First entry") != std::string::npos);
        REQUIRE(entries[1].Get_message().find("Second entry") != std::string::npos);
        REQUIRE(entries[2].Get_message().find("Third entry") != std::string::npos);
        
        try {
            std::filesystem::remove(test_file);
        } catch (...) {
            // Ignore cleanup errors
        }
    }
}