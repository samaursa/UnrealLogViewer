#pragma once

#include "../../macros.h"
#include "../common/result.h"
#include "log_entry.h"
#include <mio/mmap.hpp>
#include <string>
#include <vector>
#include <regex>
#include <memory>

namespace ue_log {
    
    class LogParser {
        CK_GENERATED_BODY(LogParser);
        
    private:
        std::unique_ptr<mio::mmap_source> memory_map;
        std::vector<LogEntry> parsed_entries;
        std::regex structured_pattern;        // [timestamp][frame]Logger: Level: message
        std::regex semi_structured_pattern;   // [timestamp][frame]Logger: message
        std::regex unstructured_pattern;      // Logger: Level: message
        size_t current_line_number;
        std::string current_file_path;
        size_t cached_file_size;
        bool is_file_loaded;
        
    public:
        // Default constructor
        LogParser();
        
        // Destructor
        ~LogParser();
        
        // Properties using macros
        CK_PROPERTY_GET(parsed_entries);
        CK_PROPERTY_GET(current_file_path);
        CK_PROPERTY_GET(current_line_number);
        
        // File operations
        Result LoadFile(const std::string& file_path);
        void UnloadFile();
        bool IsFileLoaded() const;
        size_t GetFileSize() const;
        
        // Basic parsing setup
        Result InitializeRegexPatterns();
        LogEntryType DetectEntryType(const std::string& line);
        bool IsStructuredFormat(const std::string& line);
        bool IsSemiStructuredFormat(const std::string& line);
        bool IsUnstructuredFormat(const std::string& line);
        
        // Detailed parsing methods
        LogEntry ParseSingleEntry(const std::string& line, size_t line_number);
        std::vector<LogEntry> ParseEntries(size_t start_offset = 0);
        
        // Specific parsing methods for each entry type
        LogEntry ParseStructuredEntry(const std::string& line, size_t line_number);
        LogEntry ParseSemiStructuredEntry(const std::string& line, size_t line_number);
        LogEntry ParseUnstructuredEntry(const std::string& line, size_t line_number);
        
        // Component extraction methods
        std::string ExtractTimestamp(const std::string& line);
        std::optional<int> ExtractFrameNumber(const std::string& line);
        std::string ExtractLoggerName(const std::string& line, LogEntryType type);
        std::optional<std::string> ExtractLogLevel(const std::string& line, LogEntryType type);
        std::string ExtractMessage(const std::string& line, LogEntryType type);
        
        // Utility methods
        std::vector<std::string> SplitIntoLines(size_t start_offset = 0, size_t max_lines = 0);
        size_t GetTotalLineCount();
        
        // Validation
        bool IsValid() const;
        
    private:
        // Internal helper methods
        void ResetState();
        std::string GetLineFromOffset(size_t offset, size_t& next_offset);
        bool IsValidLogLine(const std::string& line);
        bool HasTimestamp(const std::string& line);
    };
    
} // namespace ue_log