#include "result.h"
#include <sstream>

namespace ue_log {
    
    Result Result::Success() {
        return Result();
    }
    
    Result Result::Error(ErrorCode code, const std::string& message) {
        return Result(code, message);
    }
    
    std::string Result::GetErrorCodeDescription() const {
        switch (error_code) {
            case ErrorCode::Success:
                return "Success";
            case ErrorCode::FileNotFound:
                return "File not found";
            case ErrorCode::FileAccessDenied:
                return "File access denied";
            case ErrorCode::FileLocked:
                return "File is locked";
            case ErrorCode::InvalidLogFormat:
                return "Invalid log format";
            case ErrorCode::InvalidRegexPattern:
                return "Invalid regex pattern";
            case ErrorCode::OutOfMemory:
                return "Out of memory";
            case ErrorCode::ConfigurationError:
                return "Configuration error";
            case ErrorCode::UnknownError:
                return "Unknown error";
            default:
                return "Unrecognized error code";
        }
    }
    
    std::string Result::GetFullErrorDescription() const {
        if (IsSuccess()) {
            return "Success";
        }
        
        std::ostringstream oss;
        oss << GetErrorCodeDescription();
        
        if (!error_message.empty()) {
            oss << ": " << error_message;
        }
        
        return oss.str();
    }
    
    bool Result::operator==(const Result& other) const {
        return error_code == other.error_code &&
               error_message == other.error_message &&
               is_success == other.is_success;
    }
    
} // namespace ue_log