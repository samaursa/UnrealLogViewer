#include <catch2/catch_test_macros.hpp>
#include "../lib/common/result.h"

using namespace ue_log;

TEST_CASE("Result default constructor creates success result", "[result]") {
    Result result;
    
    REQUIRE(result.IsSuccess());
    REQUIRE_FALSE(result.IsError());
    REQUIRE(result.Get_error_code() == ErrorCode::Success);
    REQUIRE(result.Get_error_message().empty());
    REQUIRE(result.Get_is_success() == true);
}

TEST_CASE("Result static Success factory method", "[result]") {
    Result result = Result::Success();
    
    REQUIRE(result.IsSuccess());
    REQUIRE_FALSE(result.IsError());
    REQUIRE(result.Get_error_code() == ErrorCode::Success);
    REQUIRE(result.Get_error_message().empty());
}

TEST_CASE("Result static Error factory method", "[result]") {
    Result result = Result::Error(ErrorCode::FileNotFound, "test.log not found");
    
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.IsError());
    REQUIRE(result.Get_error_code() == ErrorCode::FileNotFound);
    REQUIRE(result.Get_error_message() == "test.log not found");
}

TEST_CASE("Result constructor with error code and message", "[result]") {
    Result result(ErrorCode::InvalidLogFormat, "Malformed log entry on line 42");
    
    REQUIRE_FALSE(result.IsSuccess());
    REQUIRE(result.IsError());
    REQUIRE(result.Get_error_code() == ErrorCode::InvalidLogFormat);
    REQUIRE(result.Get_error_message() == "Malformed log entry on line 42");
}

TEST_CASE("Result constructor with Success error code", "[result]") {
    Result result(ErrorCode::Success, "");
    
    REQUIRE(result.IsSuccess());
    REQUIRE_FALSE(result.IsError());
    REQUIRE(result.Get_error_code() == ErrorCode::Success);
}

TEST_CASE("Result GetErrorCodeDescription returns correct descriptions", "[result]") {
    SECTION("Success") {
        Result result = Result::Success();
        REQUIRE(result.GetErrorCodeDescription() == "Success");
    }
    
    SECTION("FileNotFound") {
        Result result = Result::Error(ErrorCode::FileNotFound, "");
        REQUIRE(result.GetErrorCodeDescription() == "File not found");
    }
    
    SECTION("FileAccessDenied") {
        Result result = Result::Error(ErrorCode::FileAccessDenied, "");
        REQUIRE(result.GetErrorCodeDescription() == "File access denied");
    }
    
    SECTION("FileLocked") {
        Result result = Result::Error(ErrorCode::FileLocked, "");
        REQUIRE(result.GetErrorCodeDescription() == "File is locked");
    }
    
    SECTION("InvalidLogFormat") {
        Result result = Result::Error(ErrorCode::InvalidLogFormat, "");
        REQUIRE(result.GetErrorCodeDescription() == "Invalid log format");
    }
    
    SECTION("InvalidRegexPattern") {
        Result result = Result::Error(ErrorCode::InvalidRegexPattern, "");
        REQUIRE(result.GetErrorCodeDescription() == "Invalid regex pattern");
    }
    
    SECTION("OutOfMemory") {
        Result result = Result::Error(ErrorCode::OutOfMemory, "");
        REQUIRE(result.GetErrorCodeDescription() == "Out of memory");
    }
    
    SECTION("ConfigurationError") {
        Result result = Result::Error(ErrorCode::ConfigurationError, "");
        REQUIRE(result.GetErrorCodeDescription() == "Configuration error");
    }
    
    SECTION("UnknownError") {
        Result result = Result::Error(ErrorCode::UnknownError, "");
        REQUIRE(result.GetErrorCodeDescription() == "Unknown error");
    }
}

TEST_CASE("Result GetFullErrorDescription formats correctly", "[result]") {
    SECTION("Success result") {
        Result result = Result::Success();
        REQUIRE(result.GetFullErrorDescription() == "Success");
    }
    
    SECTION("Error with message") {
        Result result = Result::Error(ErrorCode::FileNotFound, "Could not open test.log");
        REQUIRE(result.GetFullErrorDescription() == "File not found: Could not open test.log");
    }
    
    SECTION("Error without message") {
        Result result = Result::Error(ErrorCode::OutOfMemory, "");
        REQUIRE(result.GetFullErrorDescription() == "Out of memory");
    }
    
    SECTION("Error with empty message") {
        Result result(ErrorCode::InvalidRegexPattern, "");
        REQUIRE(result.GetFullErrorDescription() == "Invalid regex pattern");
    }
}

TEST_CASE("Result equality operator works correctly", "[result]") {
    Result success1 = Result::Success();
    Result success2 = Result::Success();
    Result error1 = Result::Error(ErrorCode::FileNotFound, "test.log");
    Result error2 = Result::Error(ErrorCode::FileNotFound, "test.log");
    Result error3 = Result::Error(ErrorCode::FileNotFound, "other.log");
    Result error4 = Result::Error(ErrorCode::FileLocked, "test.log");
    
    REQUIRE(success1 == success2);
    REQUIRE(error1 == error2);
    REQUIRE_FALSE(success1 == error1);
    REQUIRE_FALSE(error1 == error3); // Different message
    REQUIRE_FALSE(error1 == error4); // Different error code
    
    // Test inequality operator
    REQUIRE(success1 != error1);
    REQUIRE(error1 != error3);
    REQUIRE(error1 != error4);
}

TEST_CASE("Result handles all ErrorCode enum values", "[result]") {
    // Test that we can create Results with all error codes
    std::vector<ErrorCode> all_codes = {
        ErrorCode::Success,
        ErrorCode::FileNotFound,
        ErrorCode::FileAccessDenied,
        ErrorCode::FileLocked,
        ErrorCode::InvalidLogFormat,
        ErrorCode::InvalidRegexPattern,
        ErrorCode::OutOfMemory,
        ErrorCode::ConfigurationError,
        ErrorCode::UnknownError
    };
    
    for (ErrorCode code : all_codes) {
        Result result(code, "test message");
        REQUIRE(result.Get_error_code() == code);
        REQUIRE_FALSE(result.GetErrorCodeDescription().empty());
        
        if (code == ErrorCode::Success) {
            REQUIRE(result.IsSuccess());
        } else {
            REQUIRE(result.IsError());
        }
    }
}

TEST_CASE("Result can be used in typical error handling patterns", "[result]") {
    SECTION("Function returning success") {
        auto test_function = []() -> Result {
            return Result::Success();
        };
        
        Result result = test_function();
        REQUIRE(result.IsSuccess());
    }
    
    SECTION("Function returning error") {
        auto test_function = []() -> Result {
            return Result::Error(ErrorCode::FileNotFound, "File does not exist");
        };
        
        Result result = test_function();
        REQUIRE(result.IsError());
        REQUIRE(result.Get_error_code() == ErrorCode::FileNotFound);
        REQUIRE(result.Get_error_message() == "File does not exist");
    }
    
    SECTION("Chaining error checks") {
        auto step1 = []() -> Result { return Result::Success(); };
        auto step2 = []() -> Result { return Result::Error(ErrorCode::InvalidLogFormat, "Parse failed"); };
        
        Result result1 = step1();
        if (result1.IsSuccess()) {
            Result result2 = step2();
            REQUIRE(result2.IsError());
            REQUIRE(result2.Get_error_code() == ErrorCode::InvalidLogFormat);
        }
    }
}