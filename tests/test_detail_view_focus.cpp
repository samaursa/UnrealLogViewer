#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include <fstream>
#include <filesystem>

using namespace ue_log;

TEST_CASE("Detail View Focus and Sizing", "[ui][detail_view][focus]") {
    ConfigManager config_manager;
    MainWindow window(&config_manager);
    window.Initialize();
    
    SECTION("Detail view focus state management") {
        // Initially detail view should not be focused
        REQUIRE_FALSE(window.IsDetailViewFocused());
        
        // Focus detail view
        window.FocusDetailView();
        REQUIRE(window.IsDetailViewFocused());
        
        // Unfocus detail view
        window.UnfocusDetailView();
        REQUIRE_FALSE(window.IsDetailViewFocused());
    }
    
    SECTION("Detail view with multi-line content") {
        // Create a test log file with multi-line entries
        std::string test_file = "test_detail_view.log";
        std::ofstream file(test_file);
        file << "[2024.09.30-14.56.10:293][ 12]LogTest: Error: Main error message\n";
        file << "Line 2 of error details\n";
        file << "Line 3 of error details\n";
        file << "Line 4 of error details\n";
        file << "Line 5 of error details\n";
        file.close();
        
        // Load the file
        bool loaded = window.LoadLogFile(test_file);
        REQUIRE(loaded);
        
        // Should have 1 entry (grouped)
        auto entries = window.GetDisplayedEntries();
        REQUIRE(entries.size() == 1);
        
        // The entry should contain multi-line content
        std::string message = entries[0].Get_message();
        REQUIRE(message.find("Main error message") != std::string::npos);
        REQUIRE(message.find("Line 2 of error details") != std::string::npos);
        REQUIRE(message.find("Line 5 of error details") != std::string::npos);
        
        // Select the entry
        window.SelectNextEntry();
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        // Test detail view navigation
        window.FocusDetailView();
        REQUIRE(window.IsDetailViewFocused());
        
        // Test scrolling (should not crash)
        window.DetailViewScrollDown(2);
        window.DetailViewScrollUp(1);
        window.DetailViewScrollToTop();
        window.DetailViewScrollToBottom();
        
        // Clean up
        window.UnfocusDetailView();
        try {
            std::filesystem::remove(test_file);
        } catch (...) {
            // Ignore cleanup errors
        }
    }
    
    SECTION("Detail view rendering with focus states") {
        // Create a simple test entry
        std::string test_file = "test_detail_render.log";
        std::ofstream file(test_file);
        file << "[2024.09.30-14.56.10:293][ 12]LogTest: Info: Simple message\n";
        file.close();
        
        window.LoadLogFile(test_file);
        window.SelectNextEntry();
        
        // Test rendering in both focus states
        window.UnfocusDetailView();
        auto unfocused_render = window.Render();
        REQUIRE(unfocused_render != nullptr);
        
        window.FocusDetailView();
        auto focused_render = window.Render();
        REQUIRE(focused_render != nullptr);
        
        // Both should render without crashing
        REQUIRE(true);
        
        // Clean up
        try {
            std::filesystem::remove(test_file);
        } catch (...) {
            // Ignore cleanup errors
        }
    }
}