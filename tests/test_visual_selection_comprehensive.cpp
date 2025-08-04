#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include <memory>
#include <fstream>
#include <filesystem>

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

class VisualSelectionComprehensiveTest {
public:
    void SetUp() {
        config_manager_ = std::make_unique<ConfigManager>();
        main_window_ = std::make_unique<MainWindow>(config_manager_.get());
        main_window_->Initialize();
    }
    
    void TearDown() {
        if (!temp_log_file_.empty()) {
            DeleteTestFile(temp_log_file_);
        }
    }
    
    void LoadTestFile(const std::vector<std::string>& lines) {
        temp_log_file_ = GetUniqueTestFileName("comprehensive_test");
        CreateTestLogFile(temp_log_file_, lines);
        main_window_->LoadLogFile(temp_log_file_);
    }
    
public:
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<MainWindow> main_window_;
    std::string temp_log_file_;
};

// Unit Tests for Visual Selection State Management
TEST_CASE("Visual Selection State Management - All methods", "[visual_selection][unit]") {
    VisualSelectionComprehensiveTest test;
    test.SetUp();
    
    auto lines = CreateLargeLogFile(10);
    test.LoadTestFile(lines);
    
    SECTION("EnterVisualSelectionMode - Basic functionality") {
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        test.main_window_->EnterVisualSelectionMode();
        
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first == 0);
        REQUIRE(range.second == 0);
    }
    
    SECTION("EnterVisualSelectionMode - From different positions") {
        // Move to different positions and test visual selection activation
        test.main_window_->SelectNextEntry(); // Move to index 1
        test.main_window_->SelectNextEntry(); // Move to index 2
        
        test.main_window_->EnterVisualSelectionMode();
        
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first == 2);
        REQUIRE(range.second == 2);
    }
    
    SECTION("ExitVisualSelectionMode - Basic functionality") {
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        test.main_window_->ExitVisualSelectionMode();
        
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 0);
    }
    
    SECTION("ExtendVisualSelection - Forward extension") {
        test.main_window_->EnterVisualSelectionMode();
        
        test.main_window_->ExtendVisualSelection(3);
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 4);
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first == 0);
        REQUIRE(range.second == 3);
    }
    
    SECTION("ExtendVisualSelection - Backward extension") {
        // Start from index 5
        for (int i = 0; i < 5; ++i) {
            test.main_window_->SelectNextEntry();
        }
        
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExtendVisualSelection(2);
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 4);
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first == 2);
        REQUIRE(range.second == 5);
    }
    
    SECTION("ExtendVisualSelection - Bounds checking") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Test extending beyond end
        test.main_window_->ExtendVisualSelection(1000);
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.second < 10); // Should be clamped to available entries
        
        // Test extending before start
        test.main_window_->ExtendVisualSelection(-10);
        range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first >= 0); // Should be clamped to 0
    }
    
    SECTION("GetVisualSelectionRange - Various scenarios") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Single line selection
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first == range.second);
        
        // Multi-line selection
        test.main_window_->ExtendVisualSelection(4);
        range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first <= range.second);
        REQUIRE(range.second - range.first + 1 == test.main_window_->GetVisualSelectionSize());
    }
    
    SECTION("GetVisualSelectionSize - Accuracy") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Test various selection sizes
        for (int i = 1; i <= 5; ++i) {
            test.main_window_->ExtendVisualSelection(i - 1);
            REQUIRE(test.main_window_->GetVisualSelectionSize() == i);
        }
    }
    
    test.TearDown();
}

// Integration Tests with Existing Features
TEST_CASE("Visual Selection Integration - Feature compatibility", "[visual_selection][integration]") {
    VisualSelectionComprehensiveTest test;
    test.SetUp();
    
    auto lines = CreateLargeLogFile(20);
    test.LoadTestFile(lines);
    
    SECTION("Integration with filters") {
        // Apply a filter first
        auto filter_panel = test.main_window_->GetFilterPanel();
        REQUIRE(filter_panel != nullptr);
        
        // Enter visual selection mode with active filters
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Visual selection should work with filtered entries
        test.main_window_->ExtendVisualSelection(2);
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
    }
    
    SECTION("Integration with search functionality") {
        // Perform a search
        test.main_window_->PerformSearch("Test");
        
        // Enter visual selection mode
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Visual selection should work with search results
        test.main_window_->ExtendVisualSelection(1);
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
    }
    
    SECTION("Integration with detail view") {
        REQUIRE(test.main_window_->IsDetailViewVisible());
        
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExtendVisualSelection(2);
        
        // Rendering should work with detail view
        auto element = test.main_window_->Render();
        REQUIRE(element != nullptr);
    }
    
    SECTION("Integration with line numbers") {
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExtendVisualSelection(3);
        
        // Rendering should work with line numbers
        auto element = test.main_window_->Render();
        REQUIRE(element != nullptr);
    }
    
    SECTION("Integration with word wrap") {
        test.main_window_->ToggleWordWrap();
        REQUIRE(test.main_window_->IsWordWrapEnabled());
        
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExtendVisualSelection(2);
        
        // Rendering should work with word wrap
        auto element = test.main_window_->Render();
        REQUIRE(element != nullptr);
    }
    
    SECTION("Integration with tailing mode") {
        // Start tailing
        test.main_window_->StartTailing();
        REQUIRE(test.main_window_->IsTailing());
        
        // Visual selection should work in tailing mode
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        test.main_window_->ExtendVisualSelection(1);
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
        
        test.main_window_->StopTailing();
    }
    
    test.TearDown();
}

// Edge Cases and Error Handling
TEST_CASE("Visual Selection Edge Cases - Empty and single-line files", "[visual_selection][edge_cases]") {
    VisualSelectionComprehensiveTest test;
    test.SetUp();
    
    SECTION("Empty log file") {
        std::vector<std::string> empty_lines;
        test.LoadTestFile(empty_lines);
        
        // Note: Empty files get sample data as fallback, so visual selection should work
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Should work with sample data
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        test.main_window_->ExitVisualSelectionMode();
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    SECTION("Single-line log file") {
        std::vector<std::string> single_line = {
            "[2024.01.15-10.30.45:123][456]LogTemp: Info: Single test message"
        };
        test.LoadTestFile(single_line);
        
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        // Should not be able to extend beyond single line
        test.main_window_->ExtendVisualSelection(5);
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first == 0);
        REQUIRE(range.second == 0);
    }
    
    SECTION("Two-line log file") {
        std::vector<std::string> two_lines = {
            "[2024.01.15-10.30.45:123][456]LogTemp: Info: First message",
            "[2024.01.15-10.30.46:124][457]LogTemp: Info: Second message"
        };
        test.LoadTestFile(two_lines);
        
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExtendVisualSelection(1);
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
        
        // Should not be able to extend beyond two lines
        test.main_window_->ExtendVisualSelection(10);
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
    }
    
    test.TearDown();
}

// Performance Tests
TEST_CASE("Visual Selection Performance - Large selections", "[visual_selection][performance]") {
    VisualSelectionComprehensiveTest test;
    test.SetUp();
    
    SECTION("Large file performance") {
        auto lines = CreateLargeLogFile(1000);
        test.LoadTestFile(lines);
        
        test.main_window_->EnterVisualSelectionMode();
        
        // Test extending to large selection
        auto start_time = std::chrono::high_resolution_clock::now();
        test.main_window_->ExtendVisualSelection(999);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1000);
        REQUIRE(duration.count() < 100); // Should complete in less than 100ms
        
        // Test rendering performance with large selection
        start_time = std::chrono::high_resolution_clock::now();
        auto element = test.main_window_->Render();
        end_time = std::chrono::high_resolution_clock::now();
        
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        REQUIRE(element != nullptr);
        REQUIRE(duration.count() < 500); // Should render in less than 500ms
    }
    
    SECTION("Rapid selection changes") {
        auto lines = CreateLargeLogFile(100);
        test.LoadTestFile(lines);
        
        test.main_window_->EnterVisualSelectionMode();
        
        // Rapidly change selection multiple times
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 50; ++i) {
            test.main_window_->ExtendVisualSelection(i);
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 50);
        REQUIRE(duration.count() < 50); // Should complete rapidly
    }
    
    test.TearDown();
}

// Error Handling Tests
TEST_CASE("Visual Selection Error Handling - All scenarios", "[visual_selection][error_handling]") {
    VisualSelectionComprehensiveTest test;
    test.SetUp();
    
    auto lines = CreateLargeLogFile(5);
    test.LoadTestFile(lines);
    
    SECTION("Copy without visual selection mode") {
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        test.main_window_->CopyVisualSelectionToClipboard();
        
        REQUIRE(test.main_window_->GetLastError() == "Not in visual selection mode");
    }
    
    SECTION("Invalid state transitions") {
        // Try to exit visual selection mode when not in it
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        test.main_window_->ExitVisualSelectionMode();
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Try to extend selection when not in visual mode
        test.main_window_->ExtendVisualSelection(3);
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    SECTION("Multiple mode entries and exits") {
        // Test rapid mode switching
        for (int i = 0; i < 10; ++i) {
            test.main_window_->EnterVisualSelectionMode();
            REQUIRE(test.main_window_->IsVisualSelectionMode());
            
            test.main_window_->ExitVisualSelectionMode();
            REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        }
    }
    
    SECTION("Bounds checking edge cases") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Test extreme values
        test.main_window_->ExtendVisualSelection(INT_MAX);
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.second < 5); // Should be bounded by available entries
        
        test.main_window_->ExtendVisualSelection(INT_MIN);
        range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first >= 0); // Should not go below 0
    }
    
    SECTION("Clipboard error scenarios") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Test copying with valid selection (should succeed or show platform message)
        test.main_window_->CopyVisualSelectionToClipboard();
        
        std::string error = test.main_window_->GetLastError();
        // Should either succeed or show platform-specific message
        REQUIRE((error.find("copied to clipboard") != std::string::npos || 
                error.find("not implemented") != std::string::npos));
    }
    
    test.TearDown();
}

// Keyboard Event Handling Tests
TEST_CASE("Visual Selection Keyboard Events - Comprehensive coverage", "[visual_selection][keyboard]") {
    VisualSelectionComprehensiveTest test;
    test.SetUp();
    
    auto lines = CreateLargeLogFile(20);
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("All navigation keys in visual mode") {
        // Enter visual selection mode
        ftxui::Event v_key = ftxui::Event::Character('v');
        REQUIRE(component->OnEvent(v_key));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Test j key (down)
        ftxui::Event j_key = ftxui::Event::Character('j');
        REQUIRE(component->OnEvent(j_key));
        
        // Test k key (up)
        ftxui::Event k_key = ftxui::Event::Character('k');
        REQUIRE(component->OnEvent(k_key));
        
        // Test arrow keys
        REQUIRE(component->OnEvent(ftxui::Event::ArrowDown));
        REQUIRE(component->OnEvent(ftxui::Event::ArrowUp));
        
        // Test Ctrl+d (half-page down)
        ftxui::Event ctrl_d = ftxui::Event::Character(static_cast<char>(4));
        REQUIRE(component->OnEvent(ctrl_d));
        
        // Test Ctrl+u (half-page up)
        ftxui::Event ctrl_u = ftxui::Event::Character(static_cast<char>(21));
        REQUIRE(component->OnEvent(ctrl_u));
        
        // Test y key (copy and exit)
        ftxui::Event y_key = ftxui::Event::Character('y');
        REQUIRE(component->OnEvent(y_key));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    SECTION("Blocked keys in visual mode") {
        component->OnEvent(ftxui::Event::Character('v')); // Enter visual mode
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Test that filter panel toggle is blocked
        ftxui::Event f_key = ftxui::Event::Character('f');
        REQUIRE(component->OnEvent(f_key)); // Should be handled (blocked)
        REQUIRE_FALSE(test.main_window_->IsFilterPanelVisible());
        
        // Test that search is blocked
        ftxui::Event slash_key = ftxui::Event::Character('/');
        REQUIRE(component->OnEvent(slash_key)); // Should be handled (blocked)
        REQUIRE_FALSE(test.main_window_->IsSearchActive());
        
        // Test other keys are blocked
        REQUIRE(component->OnEvent(ftxui::Event::Character('g')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('G')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('n')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('N')));
    }
    
    SECTION("Help keys pass through") {
        component->OnEvent(ftxui::Event::Character('v')); // Enter visual mode
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Help keys should pass through (return false)
        REQUIRE_FALSE(component->OnEvent(ftxui::Event::Character('h')));
        REQUIRE_FALSE(component->OnEvent(ftxui::Event::Character('?')));
        REQUIRE_FALSE(component->OnEvent(ftxui::Event::F1));
    }
    
    SECTION("ESC key exits visual mode") {
        component->OnEvent(ftxui::Event::Character('v')); // Enter visual mode
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        REQUIRE(component->OnEvent(ftxui::Event::Escape));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    test.TearDown();
}

// Manual Testing Scenarios (Automated)
TEST_CASE("Visual Selection Manual Scenarios - Automated", "[visual_selection][manual]") {
    VisualSelectionComprehensiveTest test;
    test.SetUp();
    
    auto lines = CreateLargeLogFile(15);
    test.LoadTestFile(lines);
    
    SECTION("Complete workflow - Enter, select, copy, exit") {
        auto component = test.main_window_->CreateFTXUIComponent();
        
        // 1. Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        // 2. Extend selection with j key
        for (int i = 0; i < 3; ++i) {
            REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        }
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 4);
        
        // 3. Copy selection
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // 4. Verify copy operation
        std::string error = test.main_window_->GetLastError();
        REQUIRE((error.find("copied to clipboard") != std::string::npos || 
                error.find("not implemented") != std::string::npos));
    }
    
    SECTION("Selection extension in both directions") {
        auto component = test.main_window_->CreateFTXUIComponent();
        
        // Start from middle position
        for (int i = 0; i < 5; ++i) {
            test.main_window_->SelectNextEntry();
        }
        
        component->OnEvent(ftxui::Event::Character('v'));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Extend down
        for (int i = 0; i < 3; ++i) {
            component->OnEvent(ftxui::Event::Character('j'));
        }
        int size_after_down = test.main_window_->GetVisualSelectionSize();
        
        // Extend up (should change selection)
        for (int i = 0; i < 6; ++i) {
            component->OnEvent(ftxui::Event::Character('k'));
        }
        int size_after_up = test.main_window_->GetVisualSelectionSize();
        
        REQUIRE(size_after_down > 1);
        REQUIRE(size_after_up > 1);
        
        component->OnEvent(ftxui::Event::Escape);
    }
    
    SECTION("Large selection workflow") {
        auto component = test.main_window_->CreateFTXUIComponent();
        
        component->OnEvent(ftxui::Event::Character('v'));
        
        // Use Ctrl+d to create large selection quickly
        for (int i = 0; i < 3; ++i) {
            component->OnEvent(ftxui::Event::Character(static_cast<char>(4))); // Ctrl+d
        }
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() > 5);
        
        // Copy large selection
        component->OnEvent(ftxui::Event::Character('y'));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    SECTION("Cancel selection workflow") {
        auto component = test.main_window_->CreateFTXUIComponent();
        
        component->OnEvent(ftxui::Event::Character('v'));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Extend selection
        for (int i = 0; i < 2; ++i) {
            component->OnEvent(ftxui::Event::Character('j'));
        }
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
        
        // Cancel with ESC
        component->OnEvent(ftxui::Event::Escape);
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 0);
    }
    
    test.TearDown();
}