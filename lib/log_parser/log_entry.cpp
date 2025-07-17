#include "log_entry.h"
#include <sstream>

namespace ue_log {
    
    bool LogEntry::IsValid() const {
        // A log entry is valid if it has at least a logger name and message
        return !logger_name.empty() && !message.empty() && !raw_line.empty();
    }
    
    std::string LogEntry::GetDisplayTimestamp() const {
        if (HasTimestamp()) {
            return timestamp.value();
        }
        return "N/A";
    }
    
    std::string LogEntry::GetDisplayLogLevel() const {
        if (HasLogLevel()) {
            return log_level.value();
        }
        return "Info"; // Default to Info if no level specified
    }
    
    std::string LogEntry::ToString() const {
        std::ostringstream oss;
        
        // Add timestamp if available
        if (HasTimestamp()) {
            oss << "[" << timestamp.value() << "]";
        }
        
        // Add frame number if available
        if (HasFrameNumber()) {
            oss << "[" << frame_number.value() << "]";
        }
        
        // Add logger name
        oss << logger_name << ": ";
        
        // Add log level if available
        if (HasLogLevel()) {
            oss << log_level.value() << ": ";
        }
        
        // Add message
        oss << message;
        
        return oss.str();
    }
    
    bool LogEntry::operator==(const LogEntry& other) const {
        return entry_type == other.entry_type &&
               timestamp == other.timestamp &&
               frame_number == other.frame_number &&
               logger_name == other.logger_name &&
               log_level == other.log_level &&
               message == other.message &&
               raw_line == other.raw_line &&
               line_number == other.line_number;
    }
    
} // namespace ue_log