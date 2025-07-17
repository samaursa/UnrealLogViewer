#pragma once

#include "../../macros.h"
#include <string>
#include <optional>

namespace ue_log {
    
    enum class LogEntryType {
        Structured,      // [timestamp][frame]Logger: Level: message
        SemiStructured,  // [timestamp][frame]Logger: message (no level)
        Unstructured     // Logger: Level: message (no timestamp/frame)
    };
    
    class LogEntry {
        CK_GENERATED_BODY(LogEntry);
        
    private:
        LogEntryType entry_type;
        std::optional<std::string> timestamp;
        std::optional<int> frame_number;
        std::string logger_name;
        std::optional<std::string> log_level;
        std::string message;
        std::string raw_line;
        size_t line_number;
        
    public:
        // Default constructor
        LogEntry() : entry_type(LogEntryType::Unstructured), timestamp(std::nullopt), 
                    frame_number(std::nullopt), logger_name(""), log_level(std::nullopt), 
                    message(""), raw_line(""), line_number(0) {}
        
        // Main constructor
        LogEntry(const std::string& logger, const std::string& msg, const std::string& raw)
            : entry_type(LogEntryType::Unstructured), timestamp(std::nullopt), 
              frame_number(std::nullopt), logger_name(logger), log_level(std::nullopt), 
              message(msg), raw_line(raw), line_number(0) {}
        
        // Full constructor for structured entries
        LogEntry(LogEntryType type, const std::optional<std::string>& ts, 
                const std::optional<int>& frame, const std::string& logger, 
                const std::optional<std::string>& level, const std::string& msg, 
                const std::string& raw, size_t line_num)
            : entry_type(type), timestamp(ts), frame_number(frame), 
              logger_name(logger), log_level(level), message(msg), 
              raw_line(raw), line_number(line_num) {}
        
        // Properties using macros
        CK_PROPERTY(entry_type);
        CK_PROPERTY(timestamp);
        CK_PROPERTY(frame_number);
        CK_PROPERTY(logger_name);
        CK_PROPERTY(log_level);
        CK_PROPERTY(message);
        CK_PROPERTY(raw_line);
        CK_PROPERTY(line_number);
        
        // Validation and utility methods
        bool IsValid() const;
        bool IsStructured() const { return entry_type == LogEntryType::Structured; }
        bool IsSemiStructured() const { return entry_type == LogEntryType::SemiStructured; }
        bool IsUnstructured() const { return entry_type == LogEntryType::Unstructured; }
        bool HasTimestamp() const { return timestamp.has_value(); }
        bool HasFrameNumber() const { return frame_number.has_value(); }
        bool HasLogLevel() const { return log_level.has_value(); }
        
        // Display methods
        std::string GetDisplayTimestamp() const;
        std::string GetDisplayLogLevel() const;
        std::string ToString() const;
        
        // Equality operator for testing
        bool operator==(const LogEntry& other) const;
        CK_DECL_AND_DEF_OPERATOR_NOT_EQUAL(LogEntry);
    };
    
} // namespace ue_log