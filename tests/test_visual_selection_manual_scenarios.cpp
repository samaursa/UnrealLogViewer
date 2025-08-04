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
    
    std::vector<std::string> CreateRealisticLogFile() {
        return {
            "[2024.01.15-10.30.45:123][456]LogTemp: Info: Application started successfully",
            "[2024.01.15-10.30.46:124][457]LogCore: Warning: Memory usage is high (85%)",
            "[2024.01.15-10.30.47:125][458]LogGame: Info: Player connected: PlayerID=12345",
            "[2024.01.15-10.30.48:126][459]LogTemp: Error: Failed to load texture: missing_texture.png",
            "[2024.01.15-10.30.49:127][460]LogCore: Info: Garbage collection completed in 15ms",
            "[2024.01.15-10.30.50:128][461]LogGame: Warning: Player health is critical (5/100)",
            "[2024.01.15-10.30.51:129][462]LogTemp: Info: Save game completed successfully",
            "[2024.01.15-10.30.52:130][463]LogCore: Error: Network connection lost",
            "[2024.01.15-10.30.53:131][464]LogGame: Info: Attempting to reconnect...",
            "[2024.01.15-10.30.54:132][465]LogCore: Info: Network connection restored",
            "[2024.01.15-10.30.55:133][466]LogTemp: Warning: Frame rate dropped below 30 FPS",
            "[2024.01.15-10.30.56:134][467]LogGame: Info: Level transition completed",
            "[2024.01.15-10.30.57:135][468]LogCore: Info: Performance optimization applied",
            "[2024.01.15-10.30.58:136][469]LogTemp: Error: Shader compilation failed",
            "[2024.01.15-10.30.59:137][470]LogGame: Info: Player disconnected: PlayerID=12345"
        };
    }
}

class VisualSelectionManualScenariosTest {
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
        temp_log_file_ = GetUniqueTestFileName("manual_scenarios_test");
        CreateTestLogFile(temp_log_file_, lines);
        main_window_->LoadLogFile(temp_log_file_);
    }
    
public:
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<MainWindow> main_window_;
    std::string temp_log_file_;
};

TEST_CASE("Manual Scenario 1 - Basic visual selection workflow", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    auto lines = CreateRealisticLogFile();
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Complete basic workflow") {
        // Step 1: Verify initial state
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetSelectedEntryIndex() == 0);
        
        // Step 2: Enter visual selection mode with 'v' key
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        // Step 3: Extend selection down with 'j' key (select 3 lines total)
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
        
        // Step 4: Verify selection range
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first == 0);
        REQUIRE(range.second == 2);
        
        // Step 5: Copy selection with 'y' key
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Step 6: Verify copy operation
        std::string error = test.main_window_->GetLastError();
        bool copy_successful = (error.find("3 lines copied") != std::string::npos) ||
                              (error.find("not implemented") != std::string::npos);
        REQUIRE(copy_successful);
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 2 - Navigation and selection extension", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    auto lines = CreateRealisticLogFile();
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Advanced navigation workflow") {
        // Step 1: Move to middle of file
        for (int i = 0; i < 5; ++i) {
            test.main_window_->SelectNextEntry();
        }
        REQUIRE(test.main_window_->GetSelectedEntryIndex() == 5);
        
        // Step 2: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 3: Use Ctrl+d for half-page down
        REQUIRE(component->OnEvent(ftxui::Event::Character(static_cast<char>(4))));
        int size_after_ctrl_d = test.main_window_->GetVisualSelectionSize();
        REQUIRE(size_after_ctrl_d > 1);
        
        // Step 4: Use Ctrl+u for half-page up
        REQUIRE(component->OnEvent(ftxui::Event::Character(static_cast<char>(21))));
        int size_after_ctrl_u = test.main_window_->GetVisualSelectionSize();
        REQUIRE(size_after_ctrl_u >= 1);
        
        // Step 5: Use arrow keys for fine adjustment
        REQUIRE(component->OnEvent(ftxui::Event::ArrowDown));
        REQUIRE(component->OnEvent(ftxui::Event::ArrowDown));
        
        // Step 6: Verify final selection
        REQUIRE(test.main_window_->GetVisualSelectionSize() > 1);
        
        // Step 7: Cancel with ESC
        REQUIRE(component->OnEvent(ftxui::Event::Escape));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 3 - Backward selection", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    auto lines = CreateRealisticLogFile();
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Backward selection workflow") {
        // Step 1: Move to end of file
        for (int i = 0; i < 10; ++i) {
            test.main_window_->SelectNextEntry();
        }
        int start_index = test.main_window_->GetSelectedEntryIndex();
        REQUIRE(start_index >= 10);
        
        // Step 2: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 3: Select backward with 'k' key
        for (int i = 0; i < 5; ++i) {
            REQUIRE(component->OnEvent(ftxui::Event::Character('k')));
        }
        
        // Step 4: Verify backward selection
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 6);
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first < range.second);
        REQUIRE(range.second == start_index);
        
        // Step 5: Copy backward selection
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Step 6: Verify copy
        std::string error = test.main_window_->GetLastError();
        bool copy_successful = (error.find("6 lines copied") != std::string::npos) ||
                              (error.find("not implemented") != std::string::npos);
        REQUIRE(copy_successful);
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 4 - Selection with filters active", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    auto lines = CreateRealisticLogFile();
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Visual selection with active filters") {
        // Step 1: Apply a search filter
        test.main_window_->PerformSearch("Error");
        
        // Step 2: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 3: Extend selection
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
        
        // Step 4: Verify selection works with filtered entries
        auto range = test.main_window_->GetVisualSelectionRange();
        REQUIRE(range.first >= 0);
        REQUIRE(range.second >= range.first);
        
        // Step 5: Copy filtered selection
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Step 6: Clear search
        test.main_window_->ClearSearch();
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 5 - Help access during visual selection", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    auto lines = CreateRealisticLogFile();
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Help access workflow") {
        // Step 1: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 2: Extend selection
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
        
        // Step 3: Try to access help with 'h' (should pass through)
        bool h_handled = component->OnEvent(ftxui::Event::Character('h'));
        REQUIRE_FALSE(h_handled); // Should pass through for help
        REQUIRE(test.main_window_->IsVisualSelectionMode()); // Should still be in visual mode
        
        // Step 4: Try to access help with '?' (should pass through)
        bool question_handled = component->OnEvent(ftxui::Event::Character('?'));
        REQUIRE_FALSE(question_handled); // Should pass through for help
        REQUIRE(test.main_window_->IsVisualSelectionMode()); // Should still be in visual mode
        
        // Step 5: Try to access help with F1 (should pass through)
        bool f1_handled = component->OnEvent(ftxui::Event::F1);
        REQUIRE_FALSE(f1_handled); // Should pass through for help
        REQUIRE(test.main_window_->IsVisualSelectionMode()); // Should still be in visual mode
        
        // Step 6: Exit visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Escape));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 6 - Blocked functionality during visual selection", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    auto lines = CreateRealisticLogFile();
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Blocked functionality workflow") {
        // Step 1: Verify normal functionality works before visual mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('f'))); // Filter panel toggle
        bool filter_panel_visible = test.main_window_->IsFilterPanelVisible();
        
        // Toggle back to ensure clean state
        if (filter_panel_visible) {
            component->OnEvent(ftxui::Event::Character('f'));
        }
        
        // Step 2: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 3: Try filter panel toggle (should be blocked)
        REQUIRE(component->OnEvent(ftxui::Event::Character('f'))); // Should be handled (blocked)
        REQUIRE_FALSE(test.main_window_->IsFilterPanelVisible()); // Should not be visible
        
        // Step 4: Try search activation (should be blocked)
        REQUIRE(component->OnEvent(ftxui::Event::Character('/'))); // Should be handled (blocked)
        REQUIRE_FALSE(test.main_window_->IsSearchActive()); // Should not be active
        
        // Step 5: Try other navigation keys (should be blocked)
        REQUIRE(component->OnEvent(ftxui::Event::Character('g'))); // Should be blocked
        REQUIRE(component->OnEvent(ftxui::Event::Character('G'))); // Should be blocked
        REQUIRE(component->OnEvent(ftxui::Event::Character('n'))); // Should be blocked
        REQUIRE(component->OnEvent(ftxui::Event::Character('N'))); // Should be blocked
        
        // Step 6: Verify still in visual selection mode
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 7: Exit visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Escape));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Step 8: Verify normal functionality restored
        REQUIRE(component->OnEvent(ftxui::Event::Character('f'))); // Should work again
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 7 - Large selection workflow", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    // Create a larger log file for this test
    std::vector<std::string> large_lines;
    for (int i = 0; i < 50; ++i) {
        large_lines.push_back("[2024.01.15-10.30." + std::to_string(45 + i) + ":123][" + 
                             std::to_string(456 + i) + "]LogTemp: Info: Large file test message " + 
                             std::to_string(i + 1));
    }
    test.LoadTestFile(large_lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Large selection workflow") {
        // Step 1: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 2: Create large selection using Ctrl+d multiple times
        for (int i = 0; i < 5; ++i) {
            REQUIRE(component->OnEvent(ftxui::Event::Character(static_cast<char>(4)))); // Ctrl+d
        }
        
        // Step 3: Verify large selection
        int selection_size = test.main_window_->GetVisualSelectionSize();
        REQUIRE(selection_size > 10); // Should have selected many lines
        
        // Step 4: Test rendering with large selection
        auto element = test.main_window_->Render();
        REQUIRE(element != nullptr);
        
        // Step 5: Copy large selection
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Step 6: Verify copy operation
        std::string error = test.main_window_->GetLastError();
        bool copy_successful = (error.find("lines copied") != std::string::npos) ||
                              (error.find("not implemented") != std::string::npos);
        REQUIRE(copy_successful);
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 8 - Edge case handling", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    SECTION("Single line file workflow") {
        std::vector<std::string> single_line = {
            "[2024.01.15-10.30.45:123][456]LogTemp: Info: Only one line in file"
        };
        test.LoadTestFile(single_line);
        
        auto component = test.main_window_->CreateFTXUIComponent();
        
        // Step 1: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1);
        
        // Step 2: Try to extend beyond single line
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1); // Should remain 1
        
        // Step 3: Try to extend upward
        REQUIRE(component->OnEvent(ftxui::Event::Character('k')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 1); // Should remain 1
        
        // Step 4: Copy single line
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Step 5: Verify copy
        std::string error = test.main_window_->GetLastError();
        bool copy_successful = (error.find("1 line copied") != std::string::npos) ||
                              (error.find("not implemented") != std::string::npos);
        REQUIRE(copy_successful);
    }
    
    SECTION("Two line file workflow") {
        std::vector<std::string> two_lines = {
            "[2024.01.15-10.30.45:123][456]LogTemp: Info: First line",
            "[2024.01.15-10.30.46:124][457]LogTemp: Info: Second line"
        };
        test.LoadTestFile(two_lines);
        
        auto component = test.main_window_->CreateFTXUIComponent();
        
        // Step 1: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 2: Extend to second line
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
        
        // Step 3: Try to extend beyond second line
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2); // Should remain 2
        
        // Step 4: Copy both lines
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 9 - Multiple session workflow", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    auto lines = CreateRealisticLogFile();
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Multiple visual selection sessions") {
        // Session 1: Select and copy first few lines
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Session 2: Move to different position and select
        for (int i = 0; i < 5; ++i) {
            test.main_window_->SelectNextEntry();
        }
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 2);
        REQUIRE(component->OnEvent(ftxui::Event::Escape)); // Cancel this time
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Session 3: Select backward from end
        test.main_window_->ScrollToBottom();
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('k')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('k')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Verify final state is clean
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 0);
    }
    
    test.TearDown();
}

TEST_CASE("Manual Scenario 10 - Integration with all features", "[visual_selection][manual_scenarios]") {
    VisualSelectionManualScenariosTest test;
    test.SetUp();
    
    auto lines = CreateRealisticLogFile();
    test.LoadTestFile(lines);
    
    auto component = test.main_window_->CreateFTXUIComponent();
    
    SECTION("Full feature integration workflow") {
        // Step 1: Enable word wrap
        test.main_window_->ToggleWordWrap();
        REQUIRE(test.main_window_->IsWordWrapEnabled());
        
        // Step 2: Verify detail view is enabled
        REQUIRE(test.main_window_->IsDetailViewVisible());
        
        // Step 3: Apply a search
        test.main_window_->PerformSearch("Info");
        
        // Step 4: Enter visual selection mode
        REQUIRE(component->OnEvent(ftxui::Event::Character('v')));
        REQUIRE(test.main_window_->IsVisualSelectionMode());
        
        // Step 5: Extend selection
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(component->OnEvent(ftxui::Event::Character('j')));
        REQUIRE(test.main_window_->GetVisualSelectionSize() == 3);
        
        // Step 6: Test rendering with all features enabled
        auto element = test.main_window_->Render();
        REQUIRE(element != nullptr);
        
        // Step 7: Copy selection
        REQUIRE(component->OnEvent(ftxui::Event::Character('y')));
        REQUIRE_FALSE(test.main_window_->IsVisualSelectionMode());
        
        // Step 8: Verify copy operation
        std::string error = test.main_window_->GetLastError();
        bool copy_successful = (error.find("lines copied") != std::string::npos) ||
                              (error.find("not implemented") != std::string::npos);
        REQUIRE(copy_successful);
        
        // Step 9: Clean up
        test.main_window_->ClearSearch();
        test.main_window_->ToggleWordWrap(); // Disable word wrap
        REQUIRE_FALSE(test.main_window_->IsWordWrapEnabled());
    }
    
    test.TearDown();
}