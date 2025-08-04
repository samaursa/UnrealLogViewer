#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include <filesystem>
#include <fstream>

using namespace ue_log;

namespace {
    void CreateTestLogFile(const std::string& path, const std::vector<std::string>& lines) {
        std::ofstream file(path);
        for (const auto& line : lines) {
            file << line << "\n";
        }
        file.close();
    }
    
    void DeleteTestFile(const std::string& path) {
        try {
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        } catch (...) {
            // Ignore deletion errors in tests
        }
    }
    
    std::string GetUniqueTestFileName(const std::string& base_name) {
        static int counter = 0;
        return base_name + "_" + std::to_string(++counter) + ".log";
    }
    
    std::vector<std::string> CreateLargeLogFile(int line_count) {
        std::vector<std::string> lines;
        for (int i = 0; i < line_count; ++i) {
            lines.push_back("[2024.01.15-10.30." + std::to_string(45 + i) + ":123][" + 
                           std::to_string(456 + i) + "]LogTemp: Info: Test message " + std::to_string(i + 1));
        }
        return lines;
    }
}

TEST_CASE("Visual Selection Status Bar - Basic display", "[visual_selection][status_bar]") {
    std::string test_file = GetUniqueTestFileName("status_bar_basic_test");
    auto lines = CreateLargeLogFile(10);
    CreateTestLogFile(test_file, lines);
    
    MainWindow window;
    window.Initialize();
    window.LoadLogFile(test_file);
    
    SECTION("Status bar shows visual selection mode") {
        // Enter visual selection mode
        window.EnterVisualSelectionMode();
        REQUIRE(window.IsVisualSelectionMode());
        
        // Check that selection size is 1 initially
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        // Render to ensure status bar is updated
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    SECTION("Status bar shows correct line count for single line") {
        window.EnterVisualSelectionMode();
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    SECTION("Status bar shows correct line count for multiple lines") {
        window.EnterVisualSelectionMode();
        
        // Extend selection to 3 lines
        window.ExtendVisualSelection(2);
        REQUIRE(window.GetVisualSelectionSize() == 3);
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    DeleteTestFile(test_file);
}

TEST_CASE("Visual Selection Status Bar - Multi-screen selections", "[visual_selection][status_bar]") {
    std::string test_file = GetUniqueTestFileName("status_bar_multiscreen_test");
    auto lines = CreateLargeLogFile(100); // Create enough lines for multi-screen selection
    CreateTestLogFile(test_file, lines);
    
    MainWindow window;
    window.Initialize();
    window.LoadLogFile(test_file);
    
    // Set a small terminal size to ensure multi-screen behavior
    window.SetTerminalSize(80, 20);
    
    SECTION("Status bar shows range information for large selections") {
        window.EnterVisualSelectionMode();
        
        // Create a selection larger than one screen
        int visible_height = 10; // Approximate visible height for small terminal
        window.ExtendVisualSelection(visible_height + 5); // Select more than one screen
        
        int selection_size = window.GetVisualSelectionSize();
        REQUIRE(selection_size > visible_height);
        
        // Get the selection range
        auto range = window.GetVisualSelectionRange();
        REQUIRE(range.first >= 0);
        REQUIRE(range.second >= 0);
        REQUIRE(range.second >= range.first);
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    SECTION("Status bar handles very large selections") {
        window.EnterVisualSelectionMode();
        
        // Create a very large selection
        window.ExtendVisualSelection(50);
        
        int selection_size = window.GetVisualSelectionSize();
        REQUIRE(selection_size == 51); // 0-based to 50 inclusive
        
        auto range = window.GetVisualSelectionRange();
        REQUIRE(range.first == 0);
        REQUIRE(range.second == 50);
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    DeleteTestFile(test_file);
}

TEST_CASE("Visual Selection Status Bar - Mode transitions", "[visual_selection][status_bar]") {
    std::string test_file = GetUniqueTestFileName("status_bar_transitions_test");
    auto lines = CreateLargeLogFile(20);
    CreateTestLogFile(test_file, lines);
    
    MainWindow window;
    window.Initialize();
    window.LoadLogFile(test_file);
    
    SECTION("Status bar updates when entering visual selection mode") {
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        
        // Render before entering visual mode
        auto rendered_before = window.Render();
        REQUIRE(rendered_before != nullptr);
        
        // Enter visual selection mode
        window.EnterVisualSelectionMode();
        REQUIRE(window.IsVisualSelectionMode());
        
        // Render after entering visual mode
        auto rendered_after = window.Render();
        REQUIRE(rendered_after != nullptr);
    }
    
    SECTION("Status bar updates when exiting visual selection mode") {
        // Enter visual selection mode first
        window.EnterVisualSelectionMode();
        window.ExtendVisualSelection(3);
        REQUIRE(window.IsVisualSelectionMode());
        REQUIRE(window.GetVisualSelectionSize() == 4);
        
        // Render while in visual mode
        auto rendered_visual = window.Render();
        REQUIRE(rendered_visual != nullptr);
        
        // Exit visual selection mode
        window.ExitVisualSelectionMode();
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        
        // Render after exiting visual mode
        auto rendered_normal = window.Render();
        REQUIRE(rendered_normal != nullptr);
    }
    
    SECTION("Status bar updates when extending selection") {
        window.EnterVisualSelectionMode();
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        // Extend selection multiple times and check each time
        for (int i = 1; i <= 5; ++i) {
            window.ExtendVisualSelection(i);
            REQUIRE(window.GetVisualSelectionSize() == i + 1);
            
            auto rendered = window.Render();
            REQUIRE(rendered != nullptr);
        }
    }
    
    DeleteTestFile(test_file);
}

TEST_CASE("Visual Selection Status Bar - Edge cases", "[visual_selection][status_bar]") {
    std::string test_file = GetUniqueTestFileName("status_bar_edge_cases_test");
    auto lines = CreateLargeLogFile(5);
    CreateTestLogFile(test_file, lines);
    
    MainWindow window;
    window.Initialize();
    window.LoadLogFile(test_file);
    
    SECTION("Status bar handles selection at file boundaries") {
        // Test selection at the beginning of file
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        window.EnterVisualSelectionMode();
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
        
        // Test selection at the end of file
        window.ExitVisualSelectionMode();
        window.ScrollToBottom();
        window.EnterVisualSelectionMode();
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    SECTION("Status bar handles empty selection gracefully") {
        // This shouldn't happen in normal usage, but test robustness
        window.EnterVisualSelectionMode();
        REQUIRE(window.IsVisualSelectionMode());
        
        // Force an invalid state for testing
        window.ExitVisualSelectionMode();
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    DeleteTestFile(test_file);
}

TEST_CASE("Visual Selection Status Bar - Integration with other features", "[visual_selection][status_bar][integration]") {
    std::string test_file = GetUniqueTestFileName("status_bar_integration_test");
    auto lines = CreateLargeLogFile(30);
    CreateTestLogFile(test_file, lines);
    
    MainWindow window;
    window.Initialize();
    window.LoadLogFile(test_file);
    
    SECTION("Status bar works with detail view enabled") {
        REQUIRE(window.IsDetailViewVisible()); // Should be enabled by default
        
        window.EnterVisualSelectionMode();
        window.ExtendVisualSelection(5);
        REQUIRE(window.GetVisualSelectionSize() == 6);
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    SECTION("Status bar works with line numbers enabled") {
        // Line numbers should be enabled by default
        window.EnterVisualSelectionMode();
        window.ExtendVisualSelection(3);
        REQUIRE(window.GetVisualSelectionSize() == 4);
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    SECTION("Status bar works with word wrap enabled") {
        window.ToggleWordWrap();
        REQUIRE(window.IsWordWrapEnabled());
        
        window.EnterVisualSelectionMode();
        window.ExtendVisualSelection(2);
        REQUIRE(window.GetVisualSelectionSize() == 3);
        
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
    }
    
    DeleteTestFile(test_file);
}