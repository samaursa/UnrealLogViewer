#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include "../lib/log_parser/log_parser.h"
#include <memory>
#include <sstream>
#include <fstream>

using namespace ue_log;

class VisualSelectionRenderingTest {
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
        temp_log_file_ = "test_visual_selection_rendering.log";
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

TEST_CASE("Visual Selection Rendering - Normal rendering without visual selection", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Test that normal rendering works when not in visual selection mode
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    
    // Render should work without throwing
    auto element = test.main_window_->Render();
    REQUIRE(element != nullptr);
    
    test.TearDown();
}

TEST_CASE("Visual Selection Rendering - Rendering with visual selection", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Enter visual selection mode
    test.main_window_->EnterVisualSelectionMode();
    REQUIRE(test.main_window_->IsVisualSelectionMode());
    
    // Extend selection to include multiple entries
    test.main_window_->ExtendVisualSelection(2); // Select entries 0, 1, 2
    
    auto range = test.main_window_->GetVisualSelectionRange();
    REQUIRE(range.first == 0);
    REQUIRE(range.second == 2);
    REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
    
    // Render should work with visual selection
    auto element = test.main_window_->Render();
    REQUIRE(element != nullptr);
    
    test.TearDown();
}

TEST_CASE("Visual Selection Rendering - Range calculation", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Test visual selection range calculation
    test.main_window_->EnterVisualSelectionMode();
    
    // Test forward selection (anchor to higher index)
    test.main_window_->ExtendVisualSelection(3);
    auto range = test.main_window_->GetVisualSelectionRange();
    REQUIRE(range.first == 0); // Should be min of anchor and end
    REQUIRE(range.second == 3); // Should be max of anchor and end
    
    // Test backward selection (anchor to lower index)
    test.main_window_->ExitVisualSelectionMode();
    
    // Debug: Check initial state
    int initial_index = test.main_window_->GetSelectedEntryIndex();
    INFO("Initial selected index: " << initial_index);
    
    // Move to a specific index (let's use index 2)
    test.main_window_->SelectNextEntry(); // Move to index 1
    test.main_window_->SelectNextEntry(); // Move to index 2
    
    int current_index = test.main_window_->GetSelectedEntryIndex();
    INFO("Current selected index after moves: " << current_index);
    
    test.main_window_->EnterVisualSelectionMode(); // Anchor at current index
    test.main_window_->ExtendVisualSelection(0); // Extend to index 0
    
    range = test.main_window_->GetVisualSelectionRange();
    INFO("Actual range: (" << range.first << ", " << range.second << ")");
    INFO("Selection size: " << test.main_window_->GetVisualSelectionSize());
    INFO("Total entries: " << test.main_window_->GetDisplayedEntries().size());
    
    // The range should be from 0 to the anchor index
    REQUIRE(range.first == 0); // Should be min of anchor and end
    REQUIRE(range.second == current_index); // Should be max of anchor and end
    
    test.TearDown();
}

TEST_CASE("Visual Selection Rendering - Large selection efficiency", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Test that large visual selections are handled efficiently
    test.main_window_->EnterVisualSelectionMode();
    
    // Create a large selection (all entries)
    int total_entries = static_cast<int>(test.main_window_->GetDisplayedEntries().size());
    test.main_window_->ExtendVisualSelection(total_entries - 1);
    
    auto range = test.main_window_->GetVisualSelectionRange();
    REQUIRE(range.first == 0);
    REQUIRE(range.second == total_entries - 1);
    REQUIRE(test.main_window_->GetVisualSelectionSize() == total_entries);
    
    // Render should still work efficiently
    auto element = test.main_window_->Render();
    REQUIRE(element != nullptr);
    
    test.TearDown();
}

TEST_CASE("Visual Selection Rendering - Status bar display", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Test that status bar shows visual selection information
    test.main_window_->EnterVisualSelectionMode();
    test.main_window_->ExtendVisualSelection(2);
    
    // Render and check that visual selection info is displayed
    auto element = test.main_window_->Render();
    REQUIRE(element != nullptr);
    
    // Check that visual selection size is correct
    REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
    
    test.TearDown();
}

TEST_CASE("Visual Selection Rendering - With line numbers", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Test visual selection rendering with line numbers enabled
    test.main_window_->EnterVisualSelectionMode();
    test.main_window_->ExtendVisualSelection(1);
    
    // Render should work with line numbers
    auto element = test.main_window_->Render();
    REQUIRE(element != nullptr);
    
    test.TearDown();
}

TEST_CASE("Visual Selection Rendering - With detail view", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Test visual selection rendering with detail view enabled
    REQUIRE(test.main_window_->IsDetailViewVisible()); // Should be enabled by default
    
    test.main_window_->EnterVisualSelectionMode();
    test.main_window_->ExtendVisualSelection(1);
    
    // Render should work with detail view
    auto element = test.main_window_->Render();
    REQUIRE(element != nullptr);
    
    test.TearDown();
}

TEST_CASE("Visual Selection Rendering - Exit clears highlighting", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Test that exiting visual selection mode clears highlighting
    test.main_window_->EnterVisualSelectionMode();
    test.main_window_->ExtendVisualSelection(2);
    
    REQUIRE(test.main_window_->IsVisualSelectionMode());
    REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
    
    // Exit visual selection mode
    test.main_window_->ExitVisualSelectionMode();
    
    REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    REQUIRE(test.main_window_->GetVisualSelectionSize() == 0);
    
    // Render should work normally after exit
    auto element = test.main_window_->Render();
    REQUIRE(element != nullptr);
    
    test.TearDown();
}

TEST_CASE("Visual Selection Rendering - Bounds checking", "[visual_selection][rendering]") {
    VisualSelectionRenderingTest test;
    test.SetUp();
    
    // Test that visual selection handles bounds correctly
    test.main_window_->EnterVisualSelectionMode();
    
    int total_entries = static_cast<int>(test.main_window_->GetDisplayedEntries().size());
    
    // Test extending beyond bounds
    test.main_window_->ExtendVisualSelection(total_entries + 10); // Beyond end
    
    auto range = test.main_window_->GetVisualSelectionRange();
    REQUIRE(range.first >= 0);
    REQUIRE(range.second < total_entries);
    
    // Test extending before bounds
    test.main_window_->ExtendVisualSelection(-10); // Before start
    
    range = test.main_window_->GetVisualSelectionRange();
    REQUIRE(range.first >= 0);
    REQUIRE(range.second < total_entries);
    
    test.TearDown();
}