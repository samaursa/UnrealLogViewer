#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include "../lib/log_parser/log_parser.h"
#include <memory>
#include <sstream>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace ue_log;

class VisualSelectionClipboardTest {
public:
    void SetUp() {
        config_manager_ = std::make_unique<ConfigManager>();
        main_window_ = std::make_unique<MainWindow>(config_manager_.get());
        main_window_->Initialize();
        
        // Create sample log entries for testing
        CreateSampleLogEntries();
    }
    
    void CreateSampleLogEntries() {
        // Create a temporary log file with sample entries
        std::stringstream log_content;
        log_content << "[2023-01-01 10:00:00.000] [Frame: 1] [LogTemp] Info: First log entry\n";
        log_content << "[2023-01-01 10:00:01.000] [Frame: 2] [LogTemp] Warning: Second log entry\n";
        log_content << "[2023-01-01 10:00:02.000] [Frame: 3] [LogTemp] Error: Third log entry\n";
        log_content << "[2023-01-01 10:00:03.000] [Frame: 4] [LogTemp] Info: Fourth log entry\n";
        log_content << "[2023-01-01 10:00:04.000] [Frame: 5] [LogTemp] Info: Fifth log entry\n";
        
        // Write to temporary file
        temp_log_file_ = "test_visual_selection_clipboard.log";
        std::ofstream file(temp_log_file_);
        file << log_content.str();
        file.close();
        
        // Load the file
        main_window_->LoadLogFile(temp_log_file_);
    }
    
    void TearDown() {
        // Clean up temporary file
        if (!temp_log_file_.empty()) {
            std::remove(temp_log_file_.c_str());
        }
    }
    
    // Helper method to get clipboard content (Windows only)
    std::string GetClipboardContent() {
        #ifdef _WIN32
        if (OpenClipboard(nullptr)) {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData) {
                char* pszText = static_cast<char*>(GlobalLock(hData));
                if (pszText) {
                    std::string result(pszText);
                    GlobalUnlock(hData);
                    CloseClipboard();
                    return result;
                }
            }
            CloseClipboard();
        }
        #endif
        return "";
    }
    
public:
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<MainWindow> main_window_;
    std::string temp_log_file_;
};

TEST_CASE("Visual Selection Clipboard - Copy single line", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Enter visual selection mode (should start with single line selected)
    test.main_window_->EnterVisualSelectionMode();
    REQUIRE(test.main_window_->IsVisualSelectionMode());
    REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
    
    // Copy to clipboard
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Check that operation succeeded (no error message about failure)
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error.find("1 line copied to clipboard") != std::string::npos);
    
    #ifdef _WIN32
    // Verify clipboard content
    std::string clipboard_content = test.GetClipboardContent();
    REQUIRE_FALSE(clipboard_content.empty());
    REQUIRE(clipboard_content.find("First log entry") != std::string::npos);
    #endif
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Copy multiple lines", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Enter visual selection mode and extend to multiple lines
    test.main_window_->EnterVisualSelectionMode();
    test.main_window_->ExtendVisualSelection(2); // Select entries 0, 1, 2
    
    REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
    
    // Copy to clipboard
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Check that operation succeeded
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error.find("3 lines copied to clipboard") != std::string::npos);
    
    #ifdef _WIN32
    // Verify clipboard content contains all three lines
    std::string clipboard_content = test.GetClipboardContent();
    REQUIRE_FALSE(clipboard_content.empty());
    REQUIRE(clipboard_content.find("First log entry") != std::string::npos);
    REQUIRE(clipboard_content.find("Second log entry") != std::string::npos);
    REQUIRE(clipboard_content.find("Third log entry") != std::string::npos);
    
    // Check that lines are separated by line breaks
    REQUIRE(clipboard_content.find("\r\n") != std::string::npos);
    #endif
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Copy all lines", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Enter visual selection mode and select all entries
    test.main_window_->EnterVisualSelectionMode();
    int total_entries = static_cast<int>(test.main_window_->GetDisplayedEntries().size());
    test.main_window_->ExtendVisualSelection(total_entries - 1);
    
    REQUIRE(test.main_window_->GetVisualSelectionSize() == total_entries);
    
    // Copy to clipboard
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Check that operation succeeded
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error.find("5 lines copied to clipboard") != std::string::npos);
    
    #ifdef _WIN32
    // Verify clipboard content contains all lines
    std::string clipboard_content = test.GetClipboardContent();
    REQUIRE_FALSE(clipboard_content.empty());
    REQUIRE(clipboard_content.find("First log entry") != std::string::npos);
    REQUIRE(clipboard_content.find("Second log entry") != std::string::npos);
    REQUIRE(clipboard_content.find("Third log entry") != std::string::npos);
    REQUIRE(clipboard_content.find("Fourth log entry") != std::string::npos);
    REQUIRE(clipboard_content.find("Fifth log entry") != std::string::npos);
    #endif
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Copy backward selection", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Move to entry 3 and then select backward to entry 1
    test.main_window_->SelectNextEntry(); // Move to index 1
    test.main_window_->SelectNextEntry(); // Move to index 2
    test.main_window_->SelectNextEntry(); // Move to index 3
    
    test.main_window_->EnterVisualSelectionMode(); // Anchor at index 3
    test.main_window_->ExtendVisualSelection(1); // Extend to index 1
    
    REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
    
    // Copy to clipboard
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Check that operation succeeded
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error.find("3 lines copied to clipboard") != std::string::npos);
    
    #ifdef _WIN32
    // Verify clipboard content contains the correct lines in order
    std::string clipboard_content = test.GetClipboardContent();
    REQUIRE_FALSE(clipboard_content.empty());
    REQUIRE(clipboard_content.find("Second log entry") != std::string::npos);
    REQUIRE(clipboard_content.find("Third log entry") != std::string::npos);
    REQUIRE(clipboard_content.find("Fourth log entry") != std::string::npos);
    #endif
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Error handling - Not in visual mode", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Try to copy without being in visual selection mode
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Should get an error message
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error == "Not in visual selection mode");
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Error handling - Invalid range", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Enter visual selection mode but corrupt the state
    test.main_window_->EnterVisualSelectionMode();
    
    // Exit and re-enter to test invalid state handling
    test.main_window_->ExitVisualSelectionMode();
    
    // Try to copy with invalid state
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Should get an error message
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error == "Not in visual selection mode");
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Raw line format preservation", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Enter visual selection mode and select first line
    test.main_window_->EnterVisualSelectionMode();
    
    // Copy to clipboard
    test.main_window_->CopyVisualSelectionToClipboard();
    
    #ifdef _WIN32
    // Verify that the raw line format is preserved
    std::string clipboard_content = test.GetClipboardContent();
    REQUIRE_FALSE(clipboard_content.empty());
    
    // Should contain the full raw line with timestamp, frame, logger, level, and message
    REQUIRE(clipboard_content.find("[2023-01-01 10:00:00.000]") != std::string::npos);
    REQUIRE(clipboard_content.find("[Frame: 1]") != std::string::npos);
    REQUIRE(clipboard_content.find("[LogTemp]") != std::string::npos);
    REQUIRE(clipboard_content.find("Info:") != std::string::npos);
    REQUIRE(clipboard_content.find("First log entry") != std::string::npos);
    #endif
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Line break formatting", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Enter visual selection mode and select two lines
    test.main_window_->EnterVisualSelectionMode();
    test.main_window_->ExtendVisualSelection(1); // Select entries 0 and 1
    
    REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
    
    // Copy to clipboard
    test.main_window_->CopyVisualSelectionToClipboard();
    
    #ifdef _WIN32
    // Verify proper line break formatting
    std::string clipboard_content = test.GetClipboardContent();
    REQUIRE_FALSE(clipboard_content.empty());
    
    // Should contain Windows line endings between lines
    REQUIRE(clipboard_content.find("\r\n") != std::string::npos);
    
    // Should not end with a line break (last line shouldn't have trailing \r\n)
    bool ends_with_crlf = (clipboard_content.size() >= 2 && 
                          clipboard_content.substr(clipboard_content.size() - 2) == "\r\n");
    REQUIRE_FALSE(ends_with_crlf);
    #endif
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Large selection performance", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Test copying all available entries (should be efficient)
    test.main_window_->EnterVisualSelectionMode();
    int total_entries = static_cast<int>(test.main_window_->GetDisplayedEntries().size());
    test.main_window_->ExtendVisualSelection(total_entries - 1);
    
    // Copy should complete without issues
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Check that operation succeeded
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error.find("lines copied to clipboard") != std::string::npos);
    
    test.TearDown();
}

TEST_CASE("Visual Selection Clipboard - Empty file handling", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    
    // Set up with empty config but don't create sample entries
    test.config_manager_ = std::make_unique<ConfigManager>();
    test.main_window_ = std::make_unique<MainWindow>(test.config_manager_.get());
    test.main_window_->Initialize();
    
    // Try to enter visual selection mode with no entries
    test.main_window_->EnterVisualSelectionMode();
    
    // Should not be in visual selection mode due to no entries
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    
    // Try to copy
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Should get appropriate error
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error == "Not in visual selection mode");
}

#ifndef _WIN32
TEST_CASE("Visual Selection Clipboard - Non-Windows platform handling", "[visual_selection][clipboard]") {
    VisualSelectionClipboardTest test;
    test.SetUp();
    
    // Enter visual selection mode
    test.main_window_->EnterVisualSelectionMode();
    
    // Try to copy (should show platform not supported message)
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Should get platform error message
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error == "Clipboard functionality not implemented for this platform");
    
    test.TearDown();
}
#endif