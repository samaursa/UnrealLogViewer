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
}

class VisualSelectionErrorHandlingTest {
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
        temp_log_file_ = GetUniqueTestFileName("error_handling_test");
        CreateTestLogFile(temp_log_file_, lines);
        main_window_->LoadLogFile(temp_log_file_);
    }
    
public:
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<MainWindow> main_window_;
    std::string temp_log_file_;
};

TEST_CASE("Visual Selection Error Handling - State validation", "[visual_selection][error_handling]") {
    VisualSelectionErrorHandlingTest test;
    test.SetUp();
    
    std::vector<std::string> lines = {
        "[2024.01.15-10.30.45:123][456]LogTemp: Info: Test message 1",
        "[2024.01.15-10.30.46:124][457]LogTemp: Info: Test message 2",
        "[2024.01.15-10.30.47:125][458]LogTemp: Info: Test message 3"
    };
    test.LoadTestFile(lines);
    
    SECTION("Copy without visual selection mode active") {
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        test.main_window_->CopyVisualSelectionToClipboard();
        
        REQUIRE(test.main_window_->GetLastError() == "Not in visual selection mode");
    }
    
    SECTION("GetVisualSelectionRange when not in visual mode") {
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        auto range = test.main_window_->GetVisualSelectionRange();
        
        // Should return invalid range
        REQUIRE(range.first == -1);
        REQUIRE(range.second == -1);
    }
    
    SECTION("GetVisualSelectionSize when not in visual mode") {
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        int size = test.main_window_->GetVisualSelectionSize();
        
        REQUIRE(size == 0);
    }
    
    SECTION("ExtendVisualSelection when not in visual mode") {
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Should not crash or change state
        test.main_window_->ExtendVisualSelection(5);
        
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 0);
    }
    
    SECTION("Multiple ExitVisualSelectionMode calls") {
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        test.main_window_->ExitVisualSelectionMode();
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Should not crash on second call
        test.main_window_->ExitVisualSelectionMode();
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Error Handling - Boundary conditions", "[visual_selection][error_handling]") {
    VisualSelectionErrorHandlingTest test;
    test.SetUp();
    
    std::vector<std::string> lines = {
        "[2024.01.15-10.30.45:123][456]LogTemp: Info: Test message 1",
        "[2024.01.15-10.30.46:124][457]LogTemp: Info: Test message 2"
    };
    test.LoadTestFile(lines);
    
    SECTION("ExtendVisualSelection beyond file bounds") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Try to extend far beyond available entries
        test.main_window_->ExtendVisualSelection(1000);
        
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.second <= 1); // Should be clamped to last available entry
        
        int size = test.main_window_->GetVisualSelectionSize();
        REQUIRE(size <= 2); // Should not exceed available entries
    }
    
    SECTION("ExtendVisualSelection to negative indices") {
        // Start from second entry
        test.main_window_->SelectNextEntry();
        test.main_window_->EnterVisualSelectionMode();
        
        // Try to extend to negative index
        test.main_window_->ExtendVisualSelection(-10);
        
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first >= 0); // Should be clamped to 0
        REQUIRE(range.second >= 0);
    }
    
    SECTION("ExtendVisualSelection with extreme values") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Test with INT_MAX
        test.main_window_->ExtendVisualSelection(INT_MAX);
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.second < INT_MAX);
        REQUIRE(range.second <= 1);
        
        // Test with INT_MIN
        test.main_window_->ExtendVisualSelection(INT_MIN);
        range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first >= 0);
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Error Handling - Empty file scenarios", "[visual_selection][error_handling]") {
    VisualSelectionErrorHandlingTest test;
    test.SetUp();
    
    SECTION("Empty log file") {
        std::vector<std::string> empty_lines;
        test.LoadTestFile(empty_lines);
        
        // Note: Empty files get sample data as fallback, so visual selection should work
        // This tests the fallback behavior
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Should be able to work with sample data
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first >= 0);
        REQUIRE(range.second >= 0);
        
        // Should be able to copy
        test.main_window_->CopyVisualSelectionToClipboard();
        std::string error = test.main_window_->GetLastError();
        bool copy_successful = (error.find("1 line copied") != std::string::npos) ||
                              (error.find("not implemented") != std::string::npos);
        REQUIRE(copy_successful);
        
        test.main_window_->ExitVisualSelectionMode();
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    SECTION("No file loaded") {
        // Don't load any file
        
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        test.main_window_->CopyVisualSelectionToClipboard();
        REQUIRE(test.main_window_->GetLastError() == "Not in visual selection mode");
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Error Handling - Clipboard error scenarios", "[visual_selection][error_handling]") {
    VisualSelectionErrorHandlingTest test;
    test.SetUp();
    
    std::vector<std::string> lines = {
        "[2024.01.15-10.30.45:123][456]LogTemp: Info: Test message 1",
        "[2024.01.15-10.30.46:124][457]LogTemp: Info: Test message 2",
        "[2024.01.15-10.30.47:125][458]LogTemp: Info: Test message 3"
    };
    test.LoadTestFile(lines);
    
    SECTION("Clipboard copy with valid selection") {
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExtendVisualSelection(1);
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
        
        test.main_window_->CopyVisualSelectionToClipboard();
        
        std::string error = test.main_window_->GetLastError();
        
        // Should either succeed or show platform-specific error
        bool valid_result = (error.find("copied to clipboard") != std::string::npos) ||
                           (error.find("not implemented") != std::string::npos) ||
                           (error.find("Clipboard functionality") != std::string::npos);
        
        REQUIRE(valid_result);
    }
    
    SECTION("Clipboard copy with single line selection") {
        test.main_window_->EnterVisualSelectionMode();
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        test.main_window_->CopyVisualSelectionToClipboard();
        
        std::string error = test.main_window_->GetLastError();
        
        // Should handle single line copy
        bool valid_result = (error.find("1 line copied") != std::string::npos) ||
                           (error.find("not implemented") != std::string::npos) ||
                           (error.find("Clipboard functionality") != std::string::npos);
        
        REQUIRE(valid_result);
    }
    
    SECTION("Clipboard copy with maximum selection") {
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExtendVisualSelection(2); // Select all 3 lines
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
        
        test.main_window_->CopyVisualSelectionToClipboard();
        
        std::string error = test.main_window_->GetLastError();
        
        // Should handle maximum selection
        bool valid_result = (error.find("3 lines copied") != std::string::npos) ||
                           (error.find("not implemented") != std::string::npos) ||
                           (error.find("Clipboard functionality") != std::string::npos);
        
        REQUIRE(valid_result);
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Error Handling - State corruption recovery", "[visual_selection][error_handling]") {
    VisualSelectionErrorHandlingTest test;
    test.SetUp();
    
    std::vector<std::string> lines = {
        "[2024.01.15-10.30.45:123][456]LogTemp: Info: Test message 1",
        "[2024.01.15-10.30.46:124][457]LogTemp: Info: Test message 2",
        "[2024.01.15-10.30.47:125][458]LogTemp: Info: Test message 3"
    };
    test.LoadTestFile(lines);
    
    SECTION("Rapid mode switching") {
        // Test rapid enter/exit cycles
        for (int i = 0; i < 20; ++i) {
            test.main_window_->EnterVisualSelectionMode();
            REQUIRE(test.main_window_->IsVisualSelectionMode());
            
            if (i % 2 == 0) {
                test.main_window_->ExtendVisualSelection(1);
            }
            
            test.main_window_->ExitVisualSelectionMode();
            REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        }
        
        // Final state should be clean
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 0);
    }
    
    SECTION("Multiple EnterVisualSelectionMode calls") {
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        int initial_size = test.main_window_->GetVisualSelectionSize();
        
        // Second call should not change state
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == initial_size);
    }
    
    SECTION("ExtendVisualSelection with same index") {
        test.main_window_->EnterVisualSelectionMode();
        
        int initial_size = test.main_window_->GetVisualSelectionSize();
        auto initial_range = test.main_window_->GetVisualSelectionRange();
        
        // Extend to same position
        test.main_window_->ExtendVisualSelection(0);
        
        REQUIRE(test.main_window_->GetVisualSelectionSize() == initial_size);
        auto new_range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(new_range.first == initial_range.first);
        REQUIRE(new_range.second == initial_range.second);
    }
    
    SECTION("Operations after file reload") {
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Reload file (should reset visual selection state)
        test.main_window_->ReloadLogFile();
        
        // Visual selection mode should be reset
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Should be able to enter visual selection mode again
        test.main_window_->EnterVisualSelectionMode();
        REQUIRE(test.main_window_->IsVisualSelectionMode());
    }
    
    test.TearDown();
}

TEST_CASE("Visual Selection Error Handling - Rendering robustness", "[visual_selection][error_handling]") {
    VisualSelectionErrorHandlingTest test;
    test.SetUp();
    
    std::vector<std::string> lines = {
        "[2024.01.15-10.30.45:123][456]LogTemp: Info: Test message 1",
        "[2024.01.15-10.30.46:124][457]LogTemp: Info: Test message 2"
    };
    test.LoadTestFile(lines);
    
    SECTION("Render with invalid visual selection state") {
        // Force invalid state for testing
        test.main_window_->EnterVisualSelectionMode();
        test.main_window_->ExitVisualSelectionMode();
        
        // Rendering should not crash
        auto element = test.main_window_->Render();
        REQUIRE(element != nullptr);
    }
    
    SECTION("Render during state transitions") {
        // Render while entering visual selection mode
        test.main_window_->EnterVisualSelectionMode();
        auto element1 = test.main_window_->Render();
        REQUIRE(element1 != nullptr);
        
        // Render while extending selection
        test.main_window_->ExtendVisualSelection(1);
        auto element2 = test.main_window_->Render();
        REQUIRE(element2 != nullptr);
        
        // Render while exiting visual selection mode
        test.main_window_->ExitVisualSelectionMode();
        auto element3 = test.main_window_->Render();
        REQUIRE(element3 != nullptr);
    }
    
    SECTION("Render with extreme selection sizes") {
        test.main_window_->EnterVisualSelectionMode();
        
        // Try to create very large selection
        test.main_window_->ExtendVisualSelection(1000);
        
        // Rendering should handle large selections gracefully
        auto element = test.main_window_->Render();
        REQUIRE(element != nullptr);
    }
    
    test.TearDown();
}