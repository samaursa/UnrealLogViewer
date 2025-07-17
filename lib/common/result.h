#pragma once

#include "../../macros.h"
#include <string>

namespace ue_log {
    
    enum class ErrorCode {
        Success,
        FileNotFound,
        FileAccessDenied,
        FileLocked,
        InvalidLogFormat,
        InvalidRegexPattern,
        OutOfMemory,
        ConfigurationError,
        UnknownError
    };
    
    class Result {
        CK_GENERATED_BODY(Result);
        
    private:
        ErrorCode error_code;
        std::string error_message;
        bool is_success;
        
    public:
        // Default constructor for success
        Result() : error_code(ErrorCode::Success), error_message(""), is_success(true) {}
        
        // Constructor for error cases
        Result(ErrorCode code, const std::string& message)
            : error_code(code), error_message(message), is_success(code == ErrorCode::Success) {}
        
        // Properties using macros
        CK_PROPERTY_GET(error_code);
        CK_PROPERTY_GET(error_message);
        CK_PROPERTY_GET(is_success);
        
        // Static factory methods
        static Result Success();
        static Result Error(ErrorCode code, const std::string& message);
        
        // Convenience methods
        bool IsSuccess() const { return is_success; }
        bool IsError() const { return !is_success; }
        
        // Get human-readable error code description
        std::string GetErrorCodeDescription() const;
        
        // Get full error description (code + message)
        std::string GetFullErrorDescription() const;
        
        // Equality operator for testing
        bool operator==(const Result& other) const;
        CK_DECL_AND_DEF_OPERATOR_NOT_EQUAL(Result);
    };
    
} // namespace ue_log