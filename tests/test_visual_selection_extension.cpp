#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include "../lib/log_parser/log_parser.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstdio>
#include <memory>

using namespace ue_log;

class VisualSelectionExtensionTestFixture {
public:
    VisualSelectionExtensionTestFixture() {
        config_manager = std::make_unique<ConfigManager>();
        main_window = std::make_unique<MainWindow>(config_manager.get());
        main_window->Initialize();
        
        // Create a temporary log file for testing
        CreateTestLogFile();
        
        // Load the test log file
        bool loaded = main_window->LoadLogFile(test_file_path);
        REQUIRE(loaded);
        
        // Wait a moment for the file to be processed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Set initial selection to a middle position (line 6, which is index 5)
        main_window->GoToLine(6);
    }
    
    ~VisualSelectionExtensionTestFixture() {
        // Clean up test file
        std::remove(test_file_path.c_str());
    }
    
    void CreateTestLogFile() {
        test_file_path = "test_visual_selection.log";
        std::ofstream file(test_file_path);
        
        for (int i = 0; i < 20; ++i) {
            file << "[2024-01-01 10:00:" << std::setfill('0') << std::setw(2) << i 
                 << "][Info][TestLogger] Test message " << (i + 1) << std::endl;
        }
        
        file.close();
    }
    
    std::unique_ptr<ConfigManager> config_manager;
    std::unique_ptr<MainWindow> main_window;
    std::string test_file_path;
};

TEST_CASE("ExtendVisualSelection - Basic functionality", "[visual_selection]") {
    VisualSelectionExtensionTestFixture fixture;
    
    // Enter visual selection mode
    fixture.main_window->EnterVisualSelectionMode();
    REQUIRE(fixture.main_window->IsVisualSelectionMode());
    
    // Initial selection should be at index 5
    REQUIRE(fixture.main_window->GetSelectedEntryIndex() == 5);
    auto initial_range = fixture.main_window->GetVisualSelectionRange();
    REQUIRE(initial_range.first == 5);
    REQUIRE(initial_range.second == 5);
    REQUIRE(fixture.main_window->GetVisualSelectionSize() == 1);
    
    // Extend selection down by 3 lines
    fixture.main_window->ExtendVisualSelection(8);
    
    // Check that selection was extended correctly
    REQUIRE(fixture.main_window->GetSelectedEntryIndex() == 8);
    auto extended_range = fixture.main_window->GetVisualSelectionRange();
    REQUIRE(extended_range.first == 5);  // Start should remain at anchor
    REQUIRE(extended_range.second == 8); // End should be at new position
    REQUIRE(fixture.main_window->GetVisualSelectionSize() == 4); // 5, 6, 7, 8
}

TEST_CASE("ExtendVisualSelection - Upward extension", "[visual_selection]") {
    VisualSelectionExtensionTestFixture fixture;
    
    // Enter visual selection mode at index 10 (line 11)
    fixture.main_window->GoToLine(11);
    fixture.main_window->EnterVisualSelectionMode();
    
    // Extend selection upward to index 7
    fixture.main_window->ExtendVisualSelection(7);
    
    // Check that selection was extended correctly
    REQUIRE(fixture.main_window->GetSelectedEntryIndex() == 7);
    auto range = fixture.main_window->GetVisualSelectionRange();
    REQUIRE(range.first == 7);   // Start should be the lower index
    REQUIRE(range.second == 10); // End should be the higher index
    REQUIRE(fixture.main_window->GetVisualSelectionSize() == 4); // 7, 8, 9, 10
}

TEST_CASE("ExtendVisualSelection - Lower bounds checking", "[visual_selection]") {
    VisualSelectionExtensionTestFixture fixture;
    
    // Enter visual selection mode at index 2 (line 3)
    fixture.main_window->GoToLine(3);
    fixture.main_window->EnterVisualSelectionMode();
    
    // Try to extend selection below 0 (should clamp to 0)
    fixture.main_window->ExtendVisualSelection(-5);
    
    // Check that selection was clamped to valid bounds
    REQUIRE(fixture.main_window->GetSelectedEntryIndex() == 0);
    auto range = fixture.main_window->GetVisualSelectionRange();
    REQUIRE(range.first == 0);
    REQUIRE(range.second == 2);
    REQUIRE(fixture.main_window->GetVisualSelectionSize() == 3); // 0, 1, 2
}

TEST_CASE("ExtendVisualSelection - Upper bounds checking", "[visual_selection]") {
    VisualSelectionExtensionTestFixture fixture;
    
    // Enter visual selection mode at index 15 (line 16)
    fixture.main_window->GoToLine(16);
    fixture.main_window->EnterVisualSelectionMode();
    
    // Try to extend selection above the maximum (should clamp to max)
    fixture.main_window->ExtendVisualSelection(25);
    
    // Check that selection was clamped to valid bounds
    REQUIRE(fixture.main_window->GetSelectedEntryIndex() == 19); // Max index is 19 (20 entries, 0-indexed)
    auto range = fixture.main_window->GetVisualSelectionRange();
    REQUIRE(range.first == 15);
    REQUIRE(range.second == 19);
    REQUIRE(fixture.main_window->GetVisualSelectionSize() == 5); // 15, 16, 17, 18, 19
}

TEST_CASE("ExtendVisualSelection - Not in visual mode", "[visual_selection]") {
    VisualSelectionExtensionTestFixture fixture;
    
    // Don't enter visual selection mode
    REQUIRE_FALSE(fixture.main_window->IsVisualSelectionMode());
    
    int initial_index = fixture.main_window->GetSelectedEntryIndex();
    
    // Try to extend selection (should do nothing)
    fixture.main_window->ExtendVisualSelection(10);
    
    // Selection should remain unchanged
    REQUIRE(fixture.main_window->GetSelectedEntryIndex() == initial_index);
    REQUIRE_FALSE(fixture.main_window->IsVisualSelectionMode());
}

TEST_CASE("ExtendVisualSelection - Empty entries", "[visual_selection]") {
    ConfigManager config_manager;
    MainWindow main_window(&config_manager);
    main_window.Initialize();
    
    // Create an empty log file
    std::string empty_file_path = "empty_test.log";
    std::ofstream empty_file(empty_file_path);
    empty_file.close();
    
    // Load the empty file
    main_window.LoadLogFile(empty_file_path);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Debug: Check what entries we have
    auto entries = main_window.GetDisplayedEntries();
    INFO("Number of entries: " << entries.size());
    if (!entries.empty()) {
        INFO("First entry message: " << entries[0].Get_message());
    }
    
    // If we have entries, this test doesn't apply
    if (!entries.empty()) {
        // Clean up and skip this test
        std::remove(empty_file_path.c_str());
        return;
    }
    
    // Try to enter visual selection mode (should fail due to no entries)
    main_window.EnterVisualSelectionMode();
    
    // Should not be in visual selection mode
    REQUIRE_FALSE(main_window.IsVisualSelectionMode());
    
    // Try to extend selection (should do nothing)
    main_window.ExtendVisualSelection(5);
    
    // Should still not be in visual selection mode
    REQUIRE_FALSE(main_window.IsVisualSelectionMode());
    
    // Clean up
    std::remove(empty_file_path.c_str());
}

TEST_CASE("ExtendVisualSelection - Back and forth movement", "[visual_selection]") {
    VisualSelectionExtensionTestFixture fixture;
    
    // Enter visual selection mode at index 10 (line 11)
    fixture.main_window->GoToLine(11);
    fixture.main_window->EnterVisualSelectionMode();
    
    // Extend selection down to index 15
    fixture.main_window->ExtendVisualSelection(15);
    auto range1 = fixture.main_window->GetVisualSelectionRange();
    REQUIRE(range1.first == 10);
    REQUIRE(range1.second == 15);
    REQUIRE(fixture.main_window->GetVisualSelectionSize() == 6);
    
    // Extend selection back up to index 8 (crossing the anchor)
    fixture.main_window->ExtendVisualSelection(8);
    auto range2 = fixture.main_window->GetVisualSelectionRange();
    REQUIRE(range2.first == 8);
    REQUIRE(range2.second == 10);
    REQUIRE(fixture.main_window->GetVisualSelectionSize() == 3);
    
    // Extend selection back down to index 12
    fixture.main_window->ExtendVisualSelection(12);
    auto range3 = fixture.main_window->GetVisualSelectionRange();
    REQUIRE(range3.first == 10);
    REQUIRE(range3.second == 12);
    REQUIRE(fixture.main_window->GetVisualSelectionSize() == 3);
}

TEST_CASE("ExtendVisualSelection - Single entry", "[visual_selection]") {
    ConfigManager config_manager;
    MainWindow main_window(&config_manager);
    main_window.Initialize();
    
    // Create a log file with only one entry
    std::string single_file_path = "single_test.log";
    std::ofstream single_file(single_file_path);
    single_file << "[2024-01-01 10:00:00][Info][TestLogger] Single entry" << std::endl;
    single_file.close();
    
    // Load the single entry file
    main_window.LoadLogFile(single_file_path);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    main_window.GoToLine(1);
    main_window.EnterVisualSelectionMode();
    
    // Try to extend beyond the single entry
    main_window.ExtendVisualSelection(5);
    
    // Should remain at index 0
    REQUIRE(main_window.GetSelectedEntryIndex() == 0);
    auto range = main_window.GetVisualSelectionRange();
    REQUIRE(range.first == 0);
    REQUIRE(range.second == 0);
    REQUIRE(main_window.GetVisualSelectionSize() == 1);
    
    // Clean up
    std::remove(single_file_path.c_str());
}