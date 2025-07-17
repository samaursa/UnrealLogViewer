#include <catch2/catch_test_macros.hpp>
#include "../lib/log_parser/log_entry.h"
#include "../lib/log_parser/log_parser.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace ue_log;

TEST_CASE("LogEntry default constructor creates valid empty entry", "[log_entry]") {
    LogEntry entry;
    
    // Default constructed entry should not be valid (missing required fields)
    REQUIRE_FALSE(entry.IsValid());
    REQUIRE(entry.Get_logger_name().empty());
    REQUIRE(entry.Get_message().empty());
    REQUIRE(entry.Get_raw_line().empty());
}

TEST_CASE("LogEntry basic constructor with required fields", "[log_entry]") {
    LogEntry entry("LogTemp", "Test message", "LogTemp: Test message");
    
    REQUIRE(entry.IsValid());
    REQUIRE(entry.Get_logger_name() == "LogTemp");
    REQUIRE(entry.Get_message() == "Test message");
    REQUIRE(entry.Get_raw_line() == "LogTemp: Test message");
    REQUIRE(entry.Get_line_number() == 0); // Default value
}

TEST_CASE("LogEntry full constructor for structured entry", "[log_entry]") {
    LogEntry entry(
        LogEntryType::Structured,
        std::string("2025.07.16-10.01.25:951"),
        425,
        "CkAbility",
        std::string("Trace"),
        "[PIE-ID -2] Discovered and Adding Cue",
        "[2025.07.16-10.01.25:951][425]CkAbility: Trace: [PIE-ID -2] Discovered and Adding Cue",
        1
    );
    
    REQUIRE(entry.IsValid());
    REQUIRE(entry.IsStructured());
    REQUIRE_FALSE(entry.IsSemiStructured());
    REQUIRE_FALSE(entry.IsUnstructured());
    
    REQUIRE(entry.HasTimestamp());
    REQUIRE(entry.HasFrameNumber());
    REQUIRE(entry.HasLogLevel());
    
    REQUIRE(entry.Get_timestamp().value() == "2025.07.16-10.01.25:951");
    REQUIRE(entry.Get_frame_number().value() == 425);
    REQUIRE(entry.Get_logger_name() == "CkAbility");
    REQUIRE(entry.Get_log_level().value() == "Trace");
    REQUIRE(entry.Get_message() == "[PIE-ID -2] Discovered and Adding Cue");
    REQUIRE(entry.Get_line_number() == 1);
}

TEST_CASE("LogEntry semi-structured entry (no log level)", "[log_entry]") {
    LogEntry entry(
        LogEntryType::SemiStructured,
        std::string("2025.07.16-10.01.25:951"),
        425,
        "LogTemp",
        std::nullopt, // No log level
        "Some message without level",
        "[2025.07.16-10.01.25:951][425]LogTemp: Some message without level",
        2
    );
    
    REQUIRE(entry.IsValid());
    REQUIRE_FALSE(entry.IsStructured());
    REQUIRE(entry.IsSemiStructured());
    REQUIRE_FALSE(entry.IsUnstructured());
    
    REQUIRE(entry.HasTimestamp());
    REQUIRE(entry.HasFrameNumber());
    REQUIRE_FALSE(entry.HasLogLevel());
    
    REQUIRE(entry.Get_timestamp().value() == "2025.07.16-10.01.25:951");
    REQUIRE(entry.Get_frame_number().value() == 425);
    REQUIRE(entry.Get_logger_name() == "LogTemp");
    REQUIRE_FALSE(entry.Get_log_level().has_value());
    REQUIRE(entry.Get_message() == "Some message without level");
}

TEST_CASE("LogEntry unstructured entry (no timestamp/frame)", "[log_entry]") {
    LogEntry entry(
        LogEntryType::Unstructured,
        std::nullopt, // No timestamp
        std::nullopt, // No frame number
        "LogCsvProfiler",
        std::string("Display"),
        "Metadata set : cpu=\"AuthenticAMD|AMD Ryzen 9 7900X 12-Core Processor\"",
        "LogCsvProfiler: Display: Metadata set : cpu=\"AuthenticAMD|AMD Ryzen 9 7900X 12-Core Processor\"",
        3
    );
    
    REQUIRE(entry.IsValid());
    REQUIRE_FALSE(entry.IsStructured());
    REQUIRE_FALSE(entry.IsSemiStructured());
    REQUIRE(entry.IsUnstructured());
    
    REQUIRE_FALSE(entry.HasTimestamp());
    REQUIRE_FALSE(entry.HasFrameNumber());
    REQUIRE(entry.HasLogLevel());
    
    REQUIRE(entry.Get_logger_name() == "LogCsvProfiler");
    REQUIRE(entry.Get_log_level().value() == "Display");
    REQUIRE(entry.Get_message() == "Metadata set : cpu=\"AuthenticAMD|AMD Ryzen 9 7900X 12-Core Processor\"");
}

TEST_CASE("LogEntry display methods work correctly", "[log_entry]") {
    SECTION("Entry with timestamp") {
        LogEntry entry(
            LogEntryType::Structured,
            std::string("2025.07.16-10.01.25:951"),
            425,
            "LogTemp",
            std::string("Warning"),
            "Test message",
            "raw line",
            1
        );
        
        REQUIRE(entry.GetDisplayTimestamp() == "2025.07.16-10.01.25:951");
        REQUIRE(entry.GetDisplayLogLevel() == "Warning");
    }
    
    SECTION("Entry without timestamp") {
        LogEntry entry(
            LogEntryType::Unstructured,
            std::nullopt,
            std::nullopt,
            "LogTemp",
            std::nullopt,
            "Test message",
            "raw line",
            1
        );
        
        REQUIRE(entry.GetDisplayTimestamp() == "N/A");
        REQUIRE(entry.GetDisplayLogLevel() == "Info"); // Default when no level
    }
}

TEST_CASE("LogEntry ToString method formats correctly", "[log_entry]") {
    SECTION("Fully structured entry") {
        LogEntry entry(
            LogEntryType::Structured,
            std::string("2025.07.16-10.01.25:951"),
            425,
            "CkAbility",
            std::string("Trace"),
            "Test message",
            "raw line",
            1
        );
        
        std::string expected = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message";
        REQUIRE(entry.ToString() == expected);
    }
    
    SECTION("Semi-structured entry (no level)") {
        LogEntry entry(
            LogEntryType::SemiStructured,
            std::string("2025.07.16-10.01.25:951"),
            425,
            "LogTemp",
            std::nullopt,
            "Test message",
            "raw line",
            1
        );
        
        std::string expected = "[2025.07.16-10.01.25:951][425]LogTemp: Test message";
        REQUIRE(entry.ToString() == expected);
    }
    
    SECTION("Unstructured entry") {
        LogEntry entry(
            LogEntryType::Unstructured,
            std::nullopt,
            std::nullopt,
            "LogCsvProfiler",
            std::string("Display"),
            "Metadata set",
            "raw line",
            1
        );
        
        std::string expected = "LogCsvProfiler: Display: Metadata set";
        REQUIRE(entry.ToString() == expected);
    }
}

TEST_CASE("LogEntry equality operator works correctly", "[log_entry]") {
    LogEntry entry1(
        LogEntryType::Structured,
        std::string("2025.07.16-10.01.25:951"),
        425,
        "LogTemp",
        std::string("Warning"),
        "Test message",
        "raw line",
        1
    );
    
    LogEntry entry2(
        LogEntryType::Structured,
        std::string("2025.07.16-10.01.25:951"),
        425,
        "LogTemp",
        std::string("Warning"),
        "Test message",
        "raw line",
        1
    );
    
    LogEntry entry3(
        LogEntryType::Structured,
        std::string("2025.07.16-10.01.25:951"),
        425,
        "LogTemp",
        std::string("Error"), // Different log level
        "Test message",
        "raw line",
        1
    );
    
    REQUIRE(entry1 == entry2);
    REQUIRE_FALSE(entry1 == entry3);
    REQUIRE(entry1 != entry3);
}

TEST_CASE("LogEntry property macros work correctly", "[log_entry]") {
    LogEntry entry("LogTemp", "Test message", "raw line");
    
    // Test property getters
    REQUIRE(entry.Get_logger_name() == "LogTemp");
    REQUIRE(entry.Get_message() == "Test message");
    REQUIRE(entry.Get_raw_line() == "raw line");
    
    // Test property setters (Request methods)
    entry.Request_logger_name("NewLogger");
    entry.Request_message("New message");
    entry.Request_raw_line("new raw line");
    
    REQUIRE(entry.Get_logger_name() == "NewLogger");
    REQUIRE(entry.Get_message() == "New message");
    REQUIRE(entry.Get_raw_line() == "new raw line");
}

// Helper function declarations
std::string CreateTestLogFile(const std::string& filename, const std::vector<std::string>& lines);
void CleanupTestFile(const std::string& filepath);

// Helper function to create a test log file
std::string CreateTestLogFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::string filepath = std::filesystem::temp_directory_path().string() + "/" + filename;
    std::ofstream file(filepath);
    for (const std::string& line : lines) {
        file << line << "\n";
    }
    file.close();
    return filepath;
}

// Helper function to clean up test files
void CleanupTestFile(const std::string& filepath) {
    if (std::filesystem::exists(filepath)) {
        // Add a small delay to ensure file handles are released
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        try {
            std::filesystem::remove(filepath);
        } catch (const std::exception&) {
            // Ignore cleanup errors in tests
        }
    }
}

TEST_CASE("LogParser default constructor initializes correctly", "[log_parser]") {
    LogParser parser;
    
    REQUIRE_FALSE(parser.IsFileLoaded());
    REQUIRE_FALSE(parser.IsValid());
    REQUIRE(parser.Get_current_file_path().empty());
    REQUIRE(parser.Get_current_line_number() == 0);
    REQUIRE(parser.Get_parsed_entries().empty());
    REQUIRE(parser.GetFileSize() == 0);
}

TEST_CASE("LogParser regex patterns initialize correctly", "[log_parser]") {
    LogParser parser;
    
    // Test that regex patterns are initialized (no exceptions thrown)
    Result result = parser.InitializeRegexPatterns();
    REQUIRE(result.IsSuccess());
}

TEST_CASE("LogParser file loading works correctly", "[log_parser]") {
    LogParser parser;
    
    SECTION("Loading non-existent file returns error") {
        Result result = parser.LoadFile("non_existent_file.log");
        REQUIRE(result.IsError());
        REQUIRE(result.Get_error_code() == ErrorCode::FileNotFound);
        REQUIRE_FALSE(parser.IsFileLoaded());
    }
    
    SECTION("Loading valid file succeeds") {
        std::vector<std::string> test_lines = {
            "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message 1",
            "LogCsvProfiler: Display: Test message 2",
            "[2025.07.16-10.01.26:123][426]LogTemp: Test message 3"
        };
        
        std::string test_file = CreateTestLogFile("test_load.log", test_lines);
        
        Result result = parser.LoadFile(test_file);
        REQUIRE(result.IsSuccess());
        REQUIRE(parser.IsFileLoaded());
        REQUIRE(parser.IsValid());
        REQUIRE(parser.Get_current_file_path() == test_file);
        REQUIRE(parser.GetFileSize() > 0);
        
        CleanupTestFile(test_file);
    }
    
    SECTION("Loading file twice unloads previous file") {
        std::vector<std::string> test_lines1 = {"LogTemp: Info: First file"};
        std::vector<std::string> test_lines2 = {"LogTemp: Info: Second file"};
        
        std::string test_file1 = CreateTestLogFile("test1.log", test_lines1);
        std::string test_file2 = CreateTestLogFile("test2.log", test_lines2);
        
        Result result1 = parser.LoadFile(test_file1);
        REQUIRE(result1.IsSuccess());
        REQUIRE(parser.Get_current_file_path() == test_file1);
        
        Result result2 = parser.LoadFile(test_file2);
        REQUIRE(result2.IsSuccess());
        REQUIRE(parser.Get_current_file_path() == test_file2);
        
        CleanupTestFile(test_file1);
        CleanupTestFile(test_file2);
    }
}

TEST_CASE("LogParser file unloading works correctly", "[log_parser]") {
    LogParser parser;
    std::vector<std::string> test_lines = {"LogTemp: Info: Test message"};
    std::string test_file = CreateTestLogFile("test_unload.log", test_lines);
    
    // Load file
    Result result = parser.LoadFile(test_file);
    REQUIRE(result.IsSuccess());
    REQUIRE(parser.IsFileLoaded());
    
    // Unload file
    parser.UnloadFile();
    REQUIRE_FALSE(parser.IsFileLoaded());
    REQUIRE_FALSE(parser.IsValid());
    REQUIRE(parser.Get_current_file_path().empty());
    REQUIRE(parser.Get_current_line_number() == 0);
    REQUIRE(parser.GetFileSize() == 0);
    
    CleanupTestFile(test_file);
}

TEST_CASE("LogParser entry type detection works correctly", "[log_parser]") {
    LogParser parser;
    
    SECTION("Structured format detection") {
        std::string structured_line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message";
        REQUIRE(parser.IsStructuredFormat(structured_line));
        REQUIRE_FALSE(parser.IsSemiStructuredFormat(structured_line));
        REQUIRE_FALSE(parser.IsUnstructuredFormat(structured_line));
        REQUIRE(parser.DetectEntryType(structured_line) == LogEntryType::Structured);
    }
    
    SECTION("Semi-structured format detection") {
        std::string semi_structured_line = "[2025.07.16-10.01.25:951][425]LogTemp: Test message without level";
        REQUIRE_FALSE(parser.IsStructuredFormat(semi_structured_line));
        REQUIRE(parser.IsSemiStructuredFormat(semi_structured_line));
        REQUIRE_FALSE(parser.IsUnstructuredFormat(semi_structured_line));
        REQUIRE(parser.DetectEntryType(semi_structured_line) == LogEntryType::SemiStructured);
    }
    
    SECTION("Unstructured format detection") {
        std::string unstructured_line = "LogCsvProfiler: Display: Metadata set";
        REQUIRE_FALSE(parser.IsStructuredFormat(unstructured_line));
        REQUIRE_FALSE(parser.IsSemiStructuredFormat(unstructured_line));
        REQUIRE(parser.IsUnstructuredFormat(unstructured_line));
        REQUIRE(parser.DetectEntryType(unstructured_line) == LogEntryType::Unstructured);
    }
    
    SECTION("Invalid format defaults to unstructured") {
        std::string invalid_line = "This is not a valid log line";
        REQUIRE(parser.DetectEntryType(invalid_line) == LogEntryType::Unstructured);
    }
}

TEST_CASE("LogParser line splitting works correctly", "[log_parser]") {
    LogParser parser;
    std::vector<std::string> test_lines = {
        "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Line 1",
        "LogCsvProfiler: Display: Line 2",
        "[2025.07.16-10.01.26:123][426]LogTemp: Line 3",
        "LogTemp: Info: Line 4"
    };
    
    std::string test_file = CreateTestLogFile("test_split.log", test_lines);
    
    Result result = parser.LoadFile(test_file);
    REQUIRE(result.IsSuccess());
    
    SECTION("Split all lines") {
        std::vector<std::string> lines = parser.SplitIntoLines();
        REQUIRE(lines.size() == 4);
        REQUIRE(lines[0].find("Line 1") != std::string::npos);
        REQUIRE(lines[1].find("Line 2") != std::string::npos);
        REQUIRE(lines[2].find("Line 3") != std::string::npos);
        REQUIRE(lines[3].find("Line 4") != std::string::npos);
    }
    
    SECTION("Split with max lines limit") {
        std::vector<std::string> lines = parser.SplitIntoLines(0, 2);
        REQUIRE(lines.size() <= 2);  // Should be at most 2 lines
        if (lines.size() >= 1) {
            REQUIRE(lines[0].find("Line 1") != std::string::npos);
        }
        if (lines.size() >= 2) {
            REQUIRE(lines[1].find("Line 2") != std::string::npos);
        }
    }
    
    SECTION("Get total line count") {
        size_t line_count = parser.GetTotalLineCount();
        REQUIRE(line_count == 4);
    }
    
    CleanupTestFile(test_file);
}

TEST_CASE("LogParser single entry parsing works correctly", "[log_parser]") {
    LogParser parser;
    
    SECTION("Parse structured entry") {
        std::string line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message";
        LogEntry entry = parser.ParseSingleEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Structured);
        REQUIRE(entry.Get_raw_line() == line);
        REQUIRE(entry.Get_line_number() == 1);
        REQUIRE(entry.Get_logger_name() == "CkAbility");
        REQUIRE(entry.Get_message() == "Test message");
    }
    
    SECTION("Parse semi-structured entry") {
        std::string line = "[2025.07.16-10.01.25:951][425]LogTemp: Test message without level";
        LogEntry entry = parser.ParseSingleEntry(line, 2);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::SemiStructured);
        REQUIRE(entry.Get_raw_line() == line);
        REQUIRE(entry.Get_line_number() == 2);
        REQUIRE(entry.Get_logger_name() == "LogTemp");
        REQUIRE(entry.Get_message() == "Test message without level");
    }
    
    SECTION("Parse unstructured entry") {
        std::string line = "LogCsvProfiler: Display: Metadata set";
        LogEntry entry = parser.ParseSingleEntry(line, 3);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Unstructured);
        REQUIRE(entry.Get_raw_line() == line);
        REQUIRE(entry.Get_line_number() == 3);
        REQUIRE(entry.Get_logger_name() == "LogCsvProfiler");
        REQUIRE(entry.Get_message() == "Metadata set");
    }
}

TEST_CASE("LogParser batch entry parsing works correctly", "[log_parser]") {
    LogParser parser;
    std::vector<std::string> test_lines = {
        "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Structured message",
        "LogCsvProfiler: Display: Unstructured message",
        "[2025.07.16-10.01.26:123][426]LogTemp: Semi-structured message",
        "Invalid line without colon",  // This should be filtered out
        "LogTemp: Info: Another valid message"
    };
    
    std::string test_file = CreateTestLogFile("test_batch.log", test_lines);
    
    Result result = parser.LoadFile(test_file);
    REQUIRE(result.IsSuccess());
    
    std::vector<LogEntry> entries = parser.ParseEntries();
    
    // Should parse 4 valid entries (invalid line filtered out)
    REQUIRE(entries.size() == 4);
    
    // Check first entry (structured)
    REQUIRE(entries[0].Get_entry_type() == LogEntryType::Structured);
    REQUIRE(entries[0].Get_logger_name() == "CkAbility");
    REQUIRE(entries[0].Get_message() == "Structured message");
    
    // Check second entry (unstructured)
    REQUIRE(entries[1].Get_entry_type() == LogEntryType::Unstructured);
    REQUIRE(entries[1].Get_logger_name() == "LogCsvProfiler");
    REQUIRE(entries[1].Get_message() == "Unstructured message");
    
    // Check third entry (semi-structured)
    REQUIRE(entries[2].Get_entry_type() == LogEntryType::SemiStructured);
    REQUIRE(entries[2].Get_logger_name() == "LogTemp");
    REQUIRE(entries[2].Get_message() == "Semi-structured message");
    
    // Check fourth entry
    REQUIRE(entries[3].Get_logger_name() == "LogTemp");
    REQUIRE(entries[3].Get_message() == "Another valid message");
    
    // Check that parsed entries are stored
    REQUIRE(parser.Get_parsed_entries().size() == 4);
    
    CleanupTestFile(test_file);
}

TEST_CASE("LogParser handles edge cases correctly", "[log_parser]") {
    LogParser parser;
    
    SECTION("Empty file") {
        std::vector<std::string> empty_lines = {};
        std::string test_file = CreateTestLogFile("empty.log", empty_lines);
        
        Result result = parser.LoadFile(test_file);
        REQUIRE(result.IsSuccess());
        
        std::vector<LogEntry> entries = parser.ParseEntries();
        REQUIRE(entries.empty());
        REQUIRE(parser.GetTotalLineCount() == 0);
        
        CleanupTestFile(test_file);
    }
    
    SECTION("File with only invalid lines") {
        std::vector<std::string> invalid_lines = {
            "This is not a log line",
            "Neither is this",
            "No colons here"
        };
        std::string test_file = CreateTestLogFile("invalid.log", invalid_lines);
        
        Result result = parser.LoadFile(test_file);
        REQUIRE(result.IsSuccess());
        
        std::vector<LogEntry> entries = parser.ParseEntries();
        REQUIRE(entries.empty());  // All lines should be filtered out
        
        CleanupTestFile(test_file);
    }
    
    SECTION("File with mixed line endings") {
        std::string test_file = std::filesystem::temp_directory_path().string() + "/mixed_endings.log";
        std::ofstream file(test_file, std::ios::binary);
        file << "LogTemp: Info: Line with LF\n";
        file << "LogTemp: Info: Line with CRLF\r\n";
        file << "LogTemp: Info: Line without ending";
        file.close();
        
        Result result = parser.LoadFile(test_file);
        REQUIRE(result.IsSuccess());
        
        std::vector<std::string> lines = parser.SplitIntoLines();
        REQUIRE(lines.size() == 3);
        REQUIRE(lines[0].find("Line with LF") != std::string::npos);
        REQUIRE(lines[1].find("Line with CRLF") != std::string::npos);
        REQUIRE(lines[2].find("Line without ending") != std::string::npos);
        
        CleanupTestFile(test_file);
    }
}

TEST_CASE("LogParser memory management works correctly", "[log_parser]") {
    SECTION("Destructor unloads file properly") {
        std::vector<std::string> test_lines = {"LogTemp: Info: Test message"};
        std::string test_file = CreateTestLogFile("destructor_test.log", test_lines);
        
        {
            LogParser parser;
            Result result = parser.LoadFile(test_file);
            REQUIRE(result.IsSuccess());
            REQUIRE(parser.IsFileLoaded());
        } // parser goes out of scope here
        
        // File should still exist (destructor shouldn't delete the actual file)
        REQUIRE(std::filesystem::exists(test_file));
        
        CleanupTestFile(test_file);
    }
    
    SECTION("Multiple load/unload cycles") {
        LogParser parser;
        std::vector<std::string> test_lines = {"LogTemp: Info: Test message"};
        
        for (int i = 0; i < 3; ++i) {
            std::string test_file = CreateTestLogFile("cycle_test_" + std::to_string(i) + ".log", test_lines);
            
            Result result = parser.LoadFile(test_file);
            REQUIRE(result.IsSuccess());
            REQUIRE(parser.IsFileLoaded());
            
            parser.UnloadFile();
            REQUIRE_FALSE(parser.IsFileLoaded());
            
            CleanupTestFile(test_file);
        }
    }
}

// Tests for detailed parsing methods (Task 3.2)

TEST_CASE("LogParser ParseStructuredEntry works correctly", "[log_parser]") {
    LogParser parser;
    
    SECTION("Parse complete structured entry") {
        std::string line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: [PIE-ID -2] Discovered and Adding Cue";
        LogEntry entry = parser.ParseStructuredEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Structured);
        REQUIRE(entry.Get_timestamp().value() == "2025.07.16-10.01.25:951");
        REQUIRE(entry.Get_frame_number().value() == 425);
        REQUIRE(entry.Get_logger_name() == "CkAbility");
        REQUIRE(entry.Get_log_level().value() == "Trace");
        REQUIRE(entry.Get_message() == "[PIE-ID -2] Discovered and Adding Cue");
        REQUIRE(entry.Get_raw_line() == line);
        REQUIRE(entry.Get_line_number() == 1);
    }
    
    SECTION("Parse structured entry with different log level") {
        std::string line = "[2025.07.16-10.02.38:625][457]LogStall: Warning: Shutdown complete.";
        LogEntry entry = parser.ParseStructuredEntry(line, 2);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Structured);
        REQUIRE(entry.Get_timestamp().value() == "2025.07.16-10.02.38:625");
        REQUIRE(entry.Get_frame_number().value() == 457);
        REQUIRE(entry.Get_logger_name() == "LogStall");
        REQUIRE(entry.Get_log_level().value() == "Warning");
        REQUIRE(entry.Get_message() == "Shutdown complete.");
    }
    
    SECTION("Fallback for malformed structured entry") {
        std::string line = "Not a structured entry";
        LogEntry entry = parser.ParseStructuredEntry(line, 3);
        
        // Should fallback to unstructured parsing
        REQUIRE(entry.Get_entry_type() == LogEntryType::Unstructured);
        REQUIRE(entry.Get_raw_line() == line);
    }
}

TEST_CASE("LogParser ParseSemiStructuredEntry works correctly", "[log_parser]") {
    LogParser parser;
    
    SECTION("Parse complete semi-structured entry") {
        std::string line = "[2025.07.16-10.01.25:951][425]LogTemp: Some message without level";
        LogEntry entry = parser.ParseSemiStructuredEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::SemiStructured);
        REQUIRE(entry.Get_timestamp().value() == "2025.07.16-10.01.25:951");
        REQUIRE(entry.Get_frame_number().value() == 425);
        REQUIRE(entry.Get_logger_name() == "LogTemp");
        REQUIRE_FALSE(entry.Get_log_level().has_value());
        REQUIRE(entry.Get_message() == "Some message without level");
        REQUIRE(entry.Get_raw_line() == line);
        REQUIRE(entry.Get_line_number() == 1);
    }
    
    SECTION("Parse semi-structured entry with complex message") {
        std::string line = "[2025.07.16-10.01.37:305][743]LogDerivedDataCache: Maintenance finished in +00:00:00.000";
        LogEntry entry = parser.ParseSemiStructuredEntry(line, 2);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::SemiStructured);
        REQUIRE(entry.Get_timestamp().value() == "2025.07.16-10.01.37:305");
        REQUIRE(entry.Get_frame_number().value() == 743);
        REQUIRE(entry.Get_logger_name() == "LogDerivedDataCache");
        REQUIRE_FALSE(entry.Get_log_level().has_value());
        REQUIRE(entry.Get_message() == "Maintenance finished in +00:00:00.000");
    }
}

TEST_CASE("LogParser ParseUnstructuredEntry works correctly", "[log_parser]") {
    LogParser parser;
    
    SECTION("Parse complete unstructured entry") {
        std::string line = "LogCsvProfiler: Display: Metadata set : cpu=\"AuthenticAMD|AMD Ryzen 9 7900X 12-Core Processor\"";
        LogEntry entry = parser.ParseUnstructuredEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Unstructured);
        REQUIRE_FALSE(entry.Get_timestamp().has_value());
        REQUIRE_FALSE(entry.Get_frame_number().has_value());
        REQUIRE(entry.Get_logger_name() == "LogCsvProfiler");
        REQUIRE(entry.Get_log_level().value() == "Display");
        REQUIRE(entry.Get_message() == "Metadata set : cpu=\"AuthenticAMD|AMD Ryzen 9 7900X 12-Core Processor\"");
        REQUIRE(entry.Get_raw_line() == line);
        REQUIRE(entry.Get_line_number() == 1);
    }
    
    SECTION("Parse unstructured entry with different log level") {
        std::string line = "LogEngine: Error: Failed to initialize graphics device";
        LogEntry entry = parser.ParseUnstructuredEntry(line, 2);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Unstructured);
        REQUIRE(entry.Get_logger_name() == "LogEngine");
        REQUIRE(entry.Get_log_level().value() == "Error");
        REQUIRE(entry.Get_message() == "Failed to initialize graphics device");
    }
    
    SECTION("Fallback for malformed unstructured entry") {
        std::string line = "SimpleLogger: Basic message";
        LogEntry entry = parser.ParseUnstructuredEntry(line, 3);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Unstructured);
        REQUIRE(entry.Get_logger_name() == "SimpleLogger");
        REQUIRE_FALSE(entry.Get_log_level().has_value());
        REQUIRE(entry.Get_message() == "Basic message");
    }
    
    SECTION("Final fallback for completely malformed entry") {
        std::string line = "No colons in this line at all";
        LogEntry entry = parser.ParseUnstructuredEntry(line, 4);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Unstructured);
        REQUIRE(entry.Get_logger_name() == "Unknown");
        REQUIRE_FALSE(entry.Get_log_level().has_value());
        REQUIRE(entry.Get_message() == line);
    }
}

TEST_CASE("LogParser component extraction methods work correctly", "[log_parser]") {
    LogParser parser;
    
    SECTION("ExtractTimestamp") {
        std::string structured_line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message";
        std::string semi_structured_line = "[2025.07.16-10.01.26:123][426]LogTemp: Test message";
        std::string unstructured_line = "LogCsvProfiler: Display: Test message";
        
        REQUIRE(parser.ExtractTimestamp(structured_line) == "2025.07.16-10.01.25:951");
        REQUIRE(parser.ExtractTimestamp(semi_structured_line) == "2025.07.16-10.01.26:123");
        REQUIRE(parser.ExtractTimestamp(unstructured_line) == "");
    }
    
    SECTION("ExtractFrameNumber") {
        std::string structured_line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message";
        std::string semi_structured_line = "[2025.07.16-10.01.26:123][999]LogTemp: Test message";
        std::string unstructured_line = "LogCsvProfiler: Display: Test message";
        
        REQUIRE(parser.ExtractFrameNumber(structured_line).value() == 425);
        REQUIRE(parser.ExtractFrameNumber(semi_structured_line).value() == 999);
        REQUIRE_FALSE(parser.ExtractFrameNumber(unstructured_line).has_value());
    }
    
    SECTION("ExtractLoggerName") {
        std::string structured_line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message";
        std::string semi_structured_line = "[2025.07.16-10.01.26:123][426]LogTemp: Test message";
        std::string unstructured_line = "LogCsvProfiler: Display: Test message";
        
        REQUIRE(parser.ExtractLoggerName(structured_line, LogEntryType::Structured) == "CkAbility");
        REQUIRE(parser.ExtractLoggerName(semi_structured_line, LogEntryType::SemiStructured) == "LogTemp");
        REQUIRE(parser.ExtractLoggerName(unstructured_line, LogEntryType::Unstructured) == "LogCsvProfiler");
    }
    
    SECTION("ExtractLogLevel") {
        std::string structured_line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message";
        std::string semi_structured_line = "[2025.07.16-10.01.26:123][426]LogTemp: Test message";
        std::string unstructured_line = "LogCsvProfiler: Display: Test message";
        
        REQUIRE(parser.ExtractLogLevel(structured_line, LogEntryType::Structured).value() == "Trace");
        REQUIRE_FALSE(parser.ExtractLogLevel(semi_structured_line, LogEntryType::SemiStructured).has_value());
        REQUIRE(parser.ExtractLogLevel(unstructured_line, LogEntryType::Unstructured).value() == "Display");
    }
    
    SECTION("ExtractMessage") {
        std::string structured_line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Complex test message";
        std::string semi_structured_line = "[2025.07.16-10.01.26:123][426]LogTemp: Simple test message";
        std::string unstructured_line = "LogCsvProfiler: Display: Metadata message";
        
        REQUIRE(parser.ExtractMessage(structured_line, LogEntryType::Structured) == "Complex test message");
        REQUIRE(parser.ExtractMessage(semi_structured_line, LogEntryType::SemiStructured) == "Simple test message");
        REQUIRE(parser.ExtractMessage(unstructured_line, LogEntryType::Unstructured) == "Metadata message");
    }
}

TEST_CASE("LogParser enhanced ParseSingleEntry uses correct parsing methods", "[log_parser]") {
    LogParser parser;
    
    SECTION("Structured entry uses ParseStructuredEntry") {
        std::string line = "[2025.07.16-10.01.25:951][425]CkAbility: Trace: Test message";
        LogEntry entry = parser.ParseSingleEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Structured);
        REQUIRE(entry.HasTimestamp());
        REQUIRE(entry.HasFrameNumber());
        REQUIRE(entry.HasLogLevel());
        REQUIRE(entry.Get_timestamp().value() == "2025.07.16-10.01.25:951");
        REQUIRE(entry.Get_frame_number().value() == 425);
        REQUIRE(entry.Get_logger_name() == "CkAbility");
        REQUIRE(entry.Get_log_level().value() == "Trace");
        REQUIRE(entry.Get_message() == "Test message");
    }
    
    SECTION("Semi-structured entry uses ParseSemiStructuredEntry") {
        std::string line = "[2025.07.16-10.01.26:123][426]LogTemp: Test message without level";
        LogEntry entry = parser.ParseSingleEntry(line, 2);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::SemiStructured);
        REQUIRE(entry.HasTimestamp());
        REQUIRE(entry.HasFrameNumber());
        REQUIRE_FALSE(entry.HasLogLevel());
        REQUIRE(entry.Get_timestamp().value() == "2025.07.16-10.01.26:123");
        REQUIRE(entry.Get_frame_number().value() == 426);
        REQUIRE(entry.Get_logger_name() == "LogTemp");
        REQUIRE(entry.Get_message() == "Test message without level");
    }
    
    SECTION("Unstructured entry uses ParseUnstructuredEntry") {
        std::string line = "LogCsvProfiler: Display: Metadata set";
        LogEntry entry = parser.ParseSingleEntry(line, 3);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Unstructured);
        REQUIRE_FALSE(entry.HasTimestamp());
        REQUIRE_FALSE(entry.HasFrameNumber());
        REQUIRE(entry.HasLogLevel());
        REQUIRE(entry.Get_logger_name() == "LogCsvProfiler");
        REQUIRE(entry.Get_log_level().value() == "Display");
        REQUIRE(entry.Get_message() == "Metadata set");
    }
}

TEST_CASE("LogParser enhanced batch parsing with real Unreal log examples", "[log_parser]") {
    LogParser parser;
    std::vector<std::string> real_log_lines = {
        "LogCsvProfiler: Display: Metadata set : cpu=\"AuthenticAMD|AMD Ryzen 9 7900X 12-Core Processor\"",
        "LogCsvProfiler: Display: Metadata set : pgoenabled=\"0\"",
        "[2025.07.16-10.01.25:951][425]CkAbility: Trace: [PIE-ID -2] Discovered and Adding Cue [Default__JumpAbilityCue_CkTests_BP_C] with Name [Ck.Gyms.Ability.Cue.JumpTakeoff]",
        "[2025.07.16-10.01.37:305][743]LogDerivedDataCache: ../../../Engine/DerivedDataCache: Maintenance finished in +00:00:00.000 and deleted 0 files",
        "[2025.07.16-10.02.35:495][457]LogUObjectHash: Compacting FUObjectHashTables data took   1.33ms",
        "[2025.07.16-10.02.38:559][457]LogSlate: Window 'Save Content' being destroyed",
        "[2025.07.16-10.02.38:625][457]LogStall: Shutdown...",
        "[2025.07.16-10.02.38:707][457]LogWorld: UWorld::CleanupWorld for World_2, bSessionEnded=true, bCleanupResources=true"
    };
    
    std::string test_file = CreateTestLogFile("real_unreal_logs.log", real_log_lines);
    
    Result result = parser.LoadFile(test_file);
    REQUIRE(result.IsSuccess());
    
    std::vector<LogEntry> entries = parser.ParseEntries();
    
    // Should parse all 8 entries
    REQUIRE(entries.size() == 8);
    
    // Check first entry (unstructured)
    REQUIRE(entries[0].Get_entry_type() == LogEntryType::Unstructured);
    REQUIRE(entries[0].Get_logger_name() == "LogCsvProfiler");
    REQUIRE(entries[0].Get_log_level().value() == "Display");
    REQUIRE(entries[0].Get_message().find("cpu=") != std::string::npos);
    
    // Check third entry (structured with complex message)
    REQUIRE(entries[2].Get_entry_type() == LogEntryType::Structured);
    REQUIRE(entries[2].Get_timestamp().value() == "2025.07.16-10.01.25:951");
    REQUIRE(entries[2].Get_frame_number().value() == 425);
    REQUIRE(entries[2].Get_logger_name() == "CkAbility");
    REQUIRE(entries[2].Get_log_level().value() == "Trace");
    REQUIRE(entries[2].Get_message().find("PIE-ID -2") != std::string::npos);
    
    // Check fourth entry (structured - has path as log level)
    REQUIRE(entries[3].Get_entry_type() == LogEntryType::Structured);
    REQUIRE(entries[3].Get_timestamp().value() == "2025.07.16-10.01.37:305");
    REQUIRE(entries[3].Get_frame_number().value() == 743);
    REQUIRE(entries[3].Get_logger_name() == "LogDerivedDataCache");
    REQUIRE(entries[3].Get_log_level().value() == "../../../Engine/DerivedDataCache");
    REQUIRE(entries[3].Get_message().find("Maintenance finished") != std::string::npos);
    
    // Check last entry (structured - has method name as log level)
    REQUIRE(entries[7].Get_entry_type() == LogEntryType::Structured);
    REQUIRE(entries[7].Get_logger_name() == "LogWorld");
    REQUIRE(entries[7].Get_log_level().value() == "UWorld");
    REQUIRE(entries[7].Get_message().find("CleanupWorld") != std::string::npos);
    
    CleanupTestFile(test_file);
}

TEST_CASE("LogParser handles edge cases in detailed parsing", "[log_parser]") {
    LogParser parser;
    
    SECTION("Structured entry with special characters in message") {
        std::string line = "[2025.07.16-10.01.25:951][425]LogTemp: Warning: Message with \"quotes\" and [brackets] and : colons";
        LogEntry entry = parser.ParseSingleEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Structured);
        REQUIRE(entry.Get_logger_name() == "LogTemp");
        REQUIRE(entry.Get_log_level().value() == "Warning");
        REQUIRE(entry.Get_message() == "Message with \"quotes\" and [brackets] and : colons");
    }
    
    SECTION("Frame number with leading zeros") {
        std::string line = "[2025.07.16-10.01.25:951][0042]LogTemp: Info: Test message";
        LogEntry entry = parser.ParseSingleEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Structured);
        REQUIRE(entry.Get_frame_number().value() == 42);
    }
    
    SECTION("Logger name with special characters") {
        std::string line = "[2025.07.16-10.01.25:951][425]Log_Special-Name123: Trace: Test message";
        LogEntry entry = parser.ParseSingleEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Structured);
        REQUIRE(entry.Get_logger_name() == "Log_Special-Name123");
    }
    
    SECTION("Empty message") {
        std::string line = "[2025.07.16-10.01.25:951][425]LogTemp: Info: ";
        LogEntry entry = parser.ParseSingleEntry(line, 1);
        
        REQUIRE(entry.Get_entry_type() == LogEntryType::Structured);
        REQUIRE(entry.Get_message() == " ");
    }
}