#include "log_parser.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace ue_log {
    
    LogParser::LogParser() 
        : memory_map(nullptr), current_line_number(0), current_file_path("") {
        // Initialize regex patterns
        InitializeRegexPatterns();
    }
    
    LogParser::~LogParser() {
        UnloadFile();
    }
    
    Result LogParser::LoadFile(const std::string& file_path) {
        // Unload any existing file first
        UnloadFile();
        
        // Check if file exists
        if (!std::filesystem::exists(file_path)) {
            return Result::Error(ErrorCode::FileNotFound, "File does not exist: " + file_path);
        }
        
        // Check if file is readable
        std::ifstream test_file(file_path);
        if (!test_file.is_open()) {
            return Result::Error(ErrorCode::FileAccessDenied, "Cannot open file for reading: " + file_path);
        }
        test_file.close();
        
        try {
            // Check if file is empty
            std::uintmax_t file_size = std::filesystem::file_size(file_path);
            
            // Create memory-mapped file
            memory_map = std::make_unique<mio::mmap_source>();
            
            if (file_size > 0) {
                std::error_code error;
                memory_map->map(file_path, error);
                
                if (error) {
                    memory_map.reset();
                    return Result::Error(ErrorCode::FileLocked, "Failed to memory-map file: " + error.message());
                }
            }
            // For empty files, we don't need to map anything
            
            current_file_path = file_path;
            current_line_number = 0;
            parsed_entries.clear();
            
            return Result::Success();
            
        } catch (const std::exception& e) {
            memory_map.reset();
            return Result::Error(ErrorCode::UnknownError, "Exception while loading file: " + std::string(e.what()));
        }
    }
    
    void LogParser::UnloadFile() {
        if (memory_map) {
            memory_map->unmap();
            memory_map.reset();
        }
        ResetState();
    }
    
    bool LogParser::IsFileLoaded() const {
        return memory_map && (!current_file_path.empty());
    }
    
    size_t LogParser::GetFileSize() const {
        if (!IsFileLoaded()) {
            return 0;
        }
        
        // For empty files, return 0
        if (!memory_map->is_mapped()) {
            return 0;
        }
        
        return memory_map->size();
    }
    
    Result LogParser::InitializeRegexPatterns() {
        try {
            // Structured pattern: [timestamp][frame]Logger: Level: message
            // Example: [2025.07.16-10.01.25:951][425]CkAbility: Trace: [PIE-ID -2] Discovered and Adding Cue
            // Frame can have leading/trailing spaces: [  0], [425], etc.
            structured_pattern = std::regex(R"(\[([^\]]+)\]\[\s*(\d+)\s*\]([^:]+):\s*([^:]+):\s*(.+))");
            
            // Semi-structured pattern: [timestamp][frame]Logger: message (no level)
            // Example: [2025.07.16-10.01.25:951][425]LogTemp: Some message without level
            // Frame can have leading/trailing spaces: [  0], [425], etc.
            semi_structured_pattern = std::regex(R"(\[([^\]]+)\]\[\s*(\d+)\s*\]([^:]+):\s*(.+))");
            
            // Unstructured pattern: Logger: Level: message (no timestamp/frame)
            // Example: LogCsvProfiler: Display: Metadata set : cpu="AuthenticAMD|AMD Ryzen 9 7900X 12-Core Processor"
            unstructured_pattern = std::regex(R"(([^:]+):\s*([^:]+):\s*(.+))");
            
            return Result::Success();
            
        } catch (const std::regex_error& e) {
            return Result::Error(ErrorCode::InvalidRegexPattern, "Failed to compile regex patterns: " + std::string(e.what()));
        }
    }
    
    LogEntryType LogParser::DetectEntryType(const std::string& line) {
        if (IsStructuredFormat(line)) {
            return LogEntryType::Structured;
        } else if (IsSemiStructuredFormat(line)) {
            return LogEntryType::SemiStructured;
        } else if (IsUnstructuredFormat(line)) {
            return LogEntryType::Unstructured;
        }
        
        // Default to unstructured if no pattern matches
        return LogEntryType::Unstructured;
    }
    
    bool LogParser::IsStructuredFormat(const std::string& line) {
        if (!std::regex_match(line, structured_pattern)) {
            return false;
        }
        
        // Additional validation: check if it contains a valid Unreal log level
        std::smatch match;
        if (std::regex_match(line, match, structured_pattern)) {
            std::string potential_level = match[4].str();
            // Only consider it structured if it has a valid Unreal log level
            return potential_level == "Display" || potential_level == "Warning" || 
                   potential_level == "Error" || potential_level == "Verbose" || 
                   potential_level == "VeryVerbose" || potential_level == "Trace";
        }
        
        return false;
    }
    
    bool LogParser::IsSemiStructuredFormat(const std::string& line) {
        // Check semi-structured but make sure it's not structured
        return std::regex_match(line, semi_structured_pattern) && !IsStructuredFormat(line);
    }
    
    bool LogParser::IsUnstructuredFormat(const std::string& line) {
        // Check unstructured but make sure it's not structured or semi-structured
        return std::regex_match(line, unstructured_pattern) && 
               !IsStructuredFormat(line) && 
               !IsSemiStructuredFormat(line);
    }
    
    LogEntry LogParser::ParseSingleEntry(const std::string& line, size_t line_number) {
        // Check if this is a continuation line (no timestamp)
        if (!HasTimestamp(line)) {
            // This is a continuation line - create a minimal entry with no logger/timestamp
            return LogEntry(
                LogEntryType::Unstructured,
                std::nullopt,  // No timestamp
                std::nullopt,  // No frame number
                "",            // Empty logger name for continuation lines
                std::nullopt,  // No log level
                line,          // Use the entire line as message
                line,          // Raw line
                line_number
            );
        }
        
        LogEntryType type = DetectEntryType(line);
        
        // Use specific parsing method based on entry type
        switch (type) {
            case LogEntryType::Structured:
                return ParseStructuredEntry(line, line_number);
            case LogEntryType::SemiStructured:
                return ParseSemiStructuredEntry(line, line_number);
            case LogEntryType::Unstructured:
                return ParseUnstructuredEntry(line, line_number);
            default:
                // Fallback to unstructured parsing
                return ParseUnstructuredEntry(line, line_number);
        }
    }
    
    LogEntry LogParser::ParseStructuredEntry(const std::string& line, size_t line_number) {
        std::smatch match;
        
        if (std::regex_match(line, match, structured_pattern)) {
            // Extract components using regex groups
            // Pattern: \[([^\]]+)\]\[\s*(\d+)\s*\]([^:]+):\s*([^:]+):\s*(.+)
            // Groups: 1=timestamp, 2=frame, 3=logger, 4=potential_level, 5=message
            
            std::string timestamp = match[1].str();
            int frame_number = std::stoi(match[2].str());
            std::string logger_name = match[3].str();
            std::string potential_level = match[4].str();
            std::string message = match[5].str();
            
            // Validate if the potential level is actually a valid Unreal log level
            std::optional<std::string> validated_level = ExtractLogLevel(line, LogEntryType::Structured);
            
            // If the potential level is not valid, treat it as part of the message
            if (!validated_level.has_value()) {
                // Reconstruct message to include the invalid "level"
                message = potential_level + ": " + message;
            }
            
            return LogEntry(
                LogEntryType::Structured,
                timestamp,
                frame_number,
                logger_name,
                validated_level,
                message,
                line,
                line_number
            );
        }
        
        // Fallback if regex doesn't match (shouldn't happen if DetectEntryType worked correctly)
        return ParseUnstructuredEntry(line, line_number);
    }
    
    LogEntry LogParser::ParseSemiStructuredEntry(const std::string& line, size_t line_number) {
        std::smatch match;
        
        if (std::regex_match(line, match, semi_structured_pattern)) {
            // Extract components using regex groups
            // Pattern: \[([^\]]+)\]\[(\d+)\]([^:]+):\s*(.+)
            // Groups: 1=timestamp, 2=frame, 3=logger, 4=message
            
            std::string timestamp = match[1].str();
            int frame_number = std::stoi(match[2].str());
            std::string logger_name = match[3].str();
            std::string message = match[4].str();
            
            return LogEntry(
                LogEntryType::SemiStructured,
                timestamp,
                frame_number,
                logger_name,
                std::nullopt, // No log level
                message,
                line,
                line_number
            );
        }
        
        // Fallback if regex doesn't match
        return ParseUnstructuredEntry(line, line_number);
    }
    
    LogEntry LogParser::ParseUnstructuredEntry(const std::string& line, size_t line_number) {
        std::smatch match;
        
        if (std::regex_match(line, match, unstructured_pattern)) {
            // Extract components using regex groups
            // Pattern: ([^:]+):\s*([^:]+):\s*(.+)
            // Groups: 1=logger, 2=potential_level, 3=message
            
            std::string logger_name = match[1].str();
            std::string potential_level = match[2].str();
            std::string message = match[3].str();
            
            // Validate if the potential level is actually a valid Unreal log level
            std::optional<std::string> validated_level = ExtractLogLevel(line, LogEntryType::Unstructured);
            
            // If the potential level is not valid, treat it as part of the message
            if (!validated_level.has_value()) {
                // Reconstruct message to include the invalid "level"
                message = potential_level + ": " + message;
            }
            
            return LogEntry(
                LogEntryType::Unstructured,
                std::nullopt, // No timestamp
                std::nullopt, // No frame number
                logger_name,
                validated_level,
                message,
                line,
                line_number
            );
        }
        
        // Final fallback for lines that don't match any pattern
        // Create a basic entry with the logger name as the first part before colon
        std::string logger_name = "Unknown";
        std::string message = line;
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            logger_name = line.substr(0, colon_pos);
            if (colon_pos < line.length() - 1) {
                message = line.substr(colon_pos + 1);
                // Trim leading whitespace
                size_t start = message.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    message = message.substr(start);
                }
            }
        }
        
        return LogEntry(
            LogEntryType::Unstructured,
            std::nullopt,
            std::nullopt,
            logger_name,
            std::nullopt,
            message,
            line,
            line_number
        );
    }
    
    std::string LogParser::ExtractTimestamp(const std::string& line) {
        std::smatch match;
        
        // Try structured pattern first
        if (std::regex_match(line, match, structured_pattern)) {
            return match[1].str();
        }
        
        // Try semi-structured pattern
        if (std::regex_match(line, match, semi_structured_pattern)) {
            return match[1].str();
        }
        
        return ""; // No timestamp found
    }
    
    std::optional<int> LogParser::ExtractFrameNumber(const std::string& line) {
        std::smatch match;
        
        // Try structured pattern first
        if (std::regex_match(line, match, structured_pattern)) {
            return std::stoi(match[2].str());
        }
        
        // Try semi-structured pattern
        if (std::regex_match(line, match, semi_structured_pattern)) {
            return std::stoi(match[2].str());
        }
        
        return std::nullopt; // No frame number found
    }
    
    std::string LogParser::ExtractLoggerName(const std::string& line, LogEntryType type) {
        std::smatch match;
        
        switch (type) {
            case LogEntryType::Structured:
                if (std::regex_match(line, match, structured_pattern)) {
                    return match[3].str();
                }
                break;
            case LogEntryType::SemiStructured:
                if (std::regex_match(line, match, semi_structured_pattern)) {
                    return match[3].str();
                }
                break;
            case LogEntryType::Unstructured:
                if (std::regex_match(line, match, unstructured_pattern)) {
                    return match[1].str();
                }
                break;
        }
        
        // Fallback: extract first part before colon
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            return line.substr(0, colon_pos);
        }
        
        return "Unknown";
    }
    
    std::optional<std::string> LogParser::ExtractLogLevel(const std::string& line, LogEntryType type) {
        std::smatch match;
        
        // Helper function to validate if a string is a valid Unreal Engine log level
        auto isValidUnrealLogLevel = [](const std::string& level) -> bool {
            return level == "Display" || level == "Warning" || level == "Error" || 
                   level == "Verbose" || level == "VeryVerbose" || level == "Trace";
        };
        
        switch (type) {
            case LogEntryType::Structured:
                if (std::regex_match(line, match, structured_pattern)) {
                    std::string potential_level = match[4].str();
                    // Only return if it's a valid Unreal log level
                    if (isValidUnrealLogLevel(potential_level)) {
                        return potential_level;
                    }
                }
                break;
            case LogEntryType::SemiStructured:
                // Semi-structured entries don't have log levels
                return std::nullopt;
            case LogEntryType::Unstructured:
                if (std::regex_match(line, match, unstructured_pattern)) {
                    std::string potential_level = match[2].str();
                    // Only return if it's a valid Unreal log level
                    if (isValidUnrealLogLevel(potential_level)) {
                        return potential_level;
                    }
                }
                break;
        }
        
        return std::nullopt;
    }
    
    std::string LogParser::ExtractMessage(const std::string& line, LogEntryType type) {
        std::smatch match;
        
        switch (type) {
            case LogEntryType::Structured:
                if (std::regex_match(line, match, structured_pattern)) {
                    return match[5].str();
                }
                break;
            case LogEntryType::SemiStructured:
                if (std::regex_match(line, match, semi_structured_pattern)) {
                    return match[4].str();
                }
                break;
            case LogEntryType::Unstructured:
                if (std::regex_match(line, match, unstructured_pattern)) {
                    return match[3].str();
                }
                break;
        }
        
        // Fallback: extract everything after the last colon
        size_t last_colon = line.rfind(':');
        if (last_colon != std::string::npos && last_colon < line.length() - 1) {
            std::string message = line.substr(last_colon + 1);
            // Trim leading whitespace
            size_t start = message.find_first_not_of(" \t");
            if (start != std::string::npos) {
                message = message.substr(start);
            }
            return message;
        }
        
        return line; // Return the whole line if no colon found
    }
    
    std::vector<LogEntry> LogParser::ParseEntries(size_t start_offset) {
        std::vector<LogEntry> entries;
        
        if (!IsFileLoaded()) {
            return entries;
        }
        
        std::vector<std::string> lines = SplitIntoLines(start_offset);
        size_t line_number = current_line_number;
        
        // Pre-allocate memory for entries - now 1:1 with lines
        entries.reserve(lines.size());
        
        // TEMPORARY: Treat every line as a separate entry (disable multiline parsing)
        for (size_t i = 0; i < lines.size(); i++) {
            const std::string& line = lines[i];
            
            if (!line.empty()) {
                // Parse every non-empty line as a separate entry
                LogEntry entry = ParseSingleEntry(line, line_number + i + 1);
                entries.push_back(entry);
            }
        }
        
        // Update parsed entries
        parsed_entries.reserve(parsed_entries.size() + entries.size());
        parsed_entries.insert(parsed_entries.end(), entries.begin(), entries.end());
        current_line_number = line_number + lines.size();
        
        return entries;
    }
    
    std::vector<std::string> LogParser::SplitIntoLines(size_t start_offset, size_t max_lines) {
        std::vector<std::string> lines;
        
        if (!IsFileLoaded()) {
            return lines;
        }
        
        // Handle empty files
        if (!memory_map->is_mapped()) {
            return lines;
        }
        
        const char* data = memory_map->data();
        size_t size = memory_map->size();
        
        if (start_offset >= size) {
            return lines;
        }
        
        // Pre-allocate memory for better performance
        // Estimate lines based on average line length (assume ~100 chars per line)
        size_t estimated_lines = (size - start_offset) / 100;
        if (max_lines > 0) {
            estimated_lines = std::min(estimated_lines, max_lines);
        }
        lines.reserve(estimated_lines);
        
        // Use more efficient line splitting with string_view-like approach
        const char* line_start = data + start_offset;
        const char* current = line_start;
        const char* end = data + size;
        size_t lines_read = 0;
        
        while (current < end && (max_lines == 0 || lines_read < max_lines)) {
            // Find end of line
            const char* line_end = current;
            while (line_end < end && *line_end != '\n' && *line_end != '\r') {
                ++line_end;
            }
            
            // Create line if not empty
            if (line_end > current) {
                lines.emplace_back(current, line_end - current);
                lines_read++;
            }
            
            // Skip line ending characters
            current = line_end;
            if (current < end && *current == '\r') {
                ++current;
            }
            if (current < end && *current == '\n') {
                ++current;
            }
        }
        
        return lines;
    }
    
    size_t LogParser::GetTotalLineCount() {
        if (!IsFileLoaded()) {
            return 0;
        }
        
        // Handle empty files
        if (!memory_map->is_mapped()) {
            return 0;
        }
        
        const char* data = memory_map->data();
        size_t size = memory_map->size();
        size_t line_count = 0;
        
        for (size_t i = 0; i < size; ++i) {
            if (data[i] == '\n') {
                line_count++;
            }
        }
        
        // If file doesn't end with newline, count the last line
        if (size > 0 && data[size - 1] != '\n') {
            line_count++;
        }
        
        return line_count;
    }
    
    bool LogParser::IsValid() const {
        return IsFileLoaded() && !current_file_path.empty();
    }
    
    void LogParser::ResetState() {
        current_file_path.clear();
        current_line_number = 0;
        parsed_entries.clear();
    }
    
    std::string LogParser::GetLineFromOffset(size_t offset, size_t& next_offset) {
        if (!IsFileLoaded() || offset >= memory_map->size()) {
            next_offset = offset;
            return "";
        }
        
        const char* data = memory_map->data();
        size_t size = memory_map->size();
        std::string line;
        
        for (size_t i = offset; i < size; ++i) {
            char c = data[i];
            
            if (c == '\n') {
                next_offset = i + 1;
                return line;
            } else if (c != '\r') {  // Skip carriage return
                line += c;
            }
        }
        
        next_offset = size;
        return line;
    }
    
    bool LogParser::IsValidLogLine(const std::string& line) {
        // A valid log line should not be empty and should contain at least one colon
        return !line.empty() && 
               line.find(':') != std::string::npos &&
               line.length() > 3;  // Minimum reasonable log line length
    }
    
    bool LogParser::HasTimestamp(const std::string& line) {
        // Check if line starts with a timestamp pattern like [2024.09.30-14.56.10:293]
        // This is a simple check - if it starts with '[' and contains a timestamp-like pattern
        if (line.empty() || line[0] != '[') {
            return false;
        }
        
        // Look for the closing bracket of the timestamp
        size_t close_bracket = line.find(']');
        if (close_bracket == std::string::npos || close_bracket < 10) {
            return false;
        }
        
        // Extract the potential timestamp part
        std::string timestamp_part = line.substr(1, close_bracket - 1);
        
        // Check if it contains timestamp-like patterns (digits, dots, colons, dashes)
        // A valid timestamp should have at least: YYYY.MM.DD-HH.MM.SS:mmm format
        std::regex timestamp_pattern(R"(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})");
        return std::regex_search(timestamp_part, timestamp_pattern);
    }
    
} // namespace ue_log