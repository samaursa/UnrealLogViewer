#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include <memory>
#include <sstream>
#include <fstream>

using namespace ue_log;

class YKeyCopyTest {
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
        
        // Write to temporary file
        temp_log_file_ = "test_y_key_copy.log";
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
    
public:
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<MainWindow> main_window_;
    std::string temp_log_file_;
};

TEST_CASE("Y Key Copy - Basic functionality", "[visual_selection][y_key][copy]") {
    YKeyCopyTest test;
    test.SetUp();
    
    // Enter visual selection mode
    test.main_window_->EnterVisualSelectionMode();
    REQUIRE(test.main_window_->IsVisualSelectionMode());
    
    // Create FTXUI component for event handling
    auto component = test.main_window_->CreateFTXUIComponent();
    
    // Press 'y' key to copy and exit
    ftxui::Event y_key = ftxui::Event::Character('y');
    bool handled = component->OnEvent(y_key);
    
    // Verify the event was handled
    REQUIRE(handled);
    
    // Verify visual selection mode was exited (Requirement 3.6)
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    
    // Verify success message is displayed (Requirement 3.4)
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error.find("1 line copied to clipboard") != std::string::npos);
    
    test.TearDown();
}

TEST_CASE("Y Key Copy - Error handling when not in visual mode", "[visual_selection][y_key][error]") {
    YKeyCopyTest test;
    test.SetUp();
    
    // Ensure we're not in visual selection mode
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    
    // Try to copy directly without being in visual selection mode
    test.main_window_->CopyVisualSelectionToClipboard();
    
    // Should get error message (Requirement 3.5)
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error == "Not in visual selection mode");
    
    test.TearDown();
}

TEST_CASE("Y Key Copy - Preserves success message after exit", "[visual_selection][y_key][message]") {
    YKeyCopyTest test;
    test.SetUp();
    
    // Enter visual selection mode
    test.main_window_->EnterVisualSelectionMode();
    REQUIRE(test.main_window_->IsVisualSelectionMode());
    
    // Create FTXUI component for event handling
    auto component = test.main_window_->CreateFTXUIComponent();
    
    // Press 'y' key to copy and exit
    ftxui::Event y_key = ftxui::Event::Character('y');
    component->OnEvent(y_key);
    
    // Verify visual selection mode was exited
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    
    // Verify success message is preserved (not overwritten by exit message)
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error.find("copied to clipboard") != std::string::npos);
    REQUIRE(last_error.find("Visual selection mode deactivated") == std::string::npos);
    
    test.TearDown();
}

TEST_CASE("Y Key Copy - Event handling in visual selection mode", "[visual_selection][y_key][event]") {
    YKeyCopyTest test;
    test.SetUp();
    
    // Enter visual selection mode
    test.main_window_->EnterVisualSelectionMode();
    REQUIRE(test.main_window_->IsVisualSelectionMode());
    
    // Create FTXUI component for event handling
    auto component = test.main_window_->CreateFTXUIComponent();
    
    // Test that 'y' key is handled in visual selection mode
    ftxui::Event y_key = ftxui::Event::Character('y');
    bool handled = component->OnEvent(y_key);
    
    REQUIRE(handled); // Event should be handled
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode()); // Should exit mode
    
    test.TearDown();
}

TEST_CASE("Y Key Copy - Does not interfere with normal mode", "[visual_selection][y_key][normal_mode]") {
    YKeyCopyTest test;
    test.SetUp();
    
    // Ensure we're not in visual selection mode
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    
    // Create FTXUI component for event handling
    auto component = test.main_window_->CreateFTXUIComponent();
    
    // Press 'y' key in normal mode (should not be handled by visual selection)
    ftxui::Event y_key = ftxui::Event::Character('y');
    bool handled = component->OnEvent(y_key);
    
    // In normal mode, 'y' key should not be handled by visual selection handler
    // (it might be handled by other handlers, but that's not our concern)
    // The important thing is we're still not in visual selection mode
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    
    test.TearDown();
}

TEST_CASE("Y Key Copy - Clipboard operation success message format", "[visual_selection][y_key][message_format]") {
    YKeyCopyTest test;
    test.SetUp();
    
    // Enter visual selection mode
    test.main_window_->EnterVisualSelectionMode();
    REQUIRE(test.main_window_->IsVisualSelectionMode());
    
    // Get initial selection size
    int selection_size = test.main_window_->GetVisualSelectionSize();
    REQUIRE(selection_size == 1); // Should start with single line selected
    
    // Create FTXUI component for event handling
    auto component = test.main_window_->CreateFTXUIComponent();
    
    // Press 'y' key to copy
    ftxui::Event y_key = ftxui::Event::Character('y');
    component->OnEvent(y_key);
    
    // Verify success message format (Requirement 3.4)
    std::string last_error = test.main_window_->GetLastError();
    REQUIRE(last_error == "1 line copied to clipboard");
    
    test.TearDown();
}