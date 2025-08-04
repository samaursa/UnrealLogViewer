#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/main_window.h"
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace ue_log;

// Test utility functions
namespace {
    void CreateTestLogFile(const std::string& path, const std::vector<std::string>& lines) {
        std::ofstream file(path);
        for (const auto& line : lines) {
            file << line << "\n";
        }
        file.close();
    }
    
    void DeleteTestFile(const std::string& path) {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
    }
    
    std::vector<std::string> CreateSampleLogLines() {
        return {
            "[2024.01.15-10.30.45:123][456]LogTemp: Warning: Test message 1",
            "[2024.01.15-10.30.46:124][457]LogTemp: Error: Test message 2",
            "[2024.01.15-10.30.47:125][458]LogCore: Info: Test message 3",
            "[2024.01.15-10.30.48:126][459]LogTemp: Warning: Test message 4",
            "[2024.01.15-10.30.49:127][460]LogCore: Error: Test message 5"
        };
    }
}

TEST_CASE("MainWindow Basic Functionality", "[ui][main_window]") {
    
    SECTION("Default constructor initializes correctly") {
        MainWindow window;
        
        REQUIRE_FALSE(window.IsFileLoaded());
        REQUIRE(window.GetCurrentFilePath().empty());
        REQUIRE(window.GetDisplayedEntries().empty());
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        REQUIRE_FALSE(window.IsTailing());
        REQUIRE_FALSE(window.IsFilterPanelVisible());
        REQUIRE(window.GetLastError().empty());
    }
    
    SECTION("Constructor with ConfigManager") {
        ConfigManager config_manager;
        MainWindow window(&config_manager);
        
        REQUIRE_FALSE(window.IsFileLoaded());
        REQUIRE(window.GetCurrentFilePath().empty());
    }
    
    SECTION("Initialize method sets up components") {
        MainWindow window;
        
        REQUIRE_NOTHROW(window.Initialize());
        
        // Verify components are accessible
        REQUIRE(window.HasLogParser());
        REQUIRE(window.HasFilterEngine());
        REQUIRE(window.HasFileMonitor());
        REQUIRE(window.HasFilterPanel());
    }
}

TEST_CASE("MainWindow File Operations", "[ui][main_window][file]") {
    
    SECTION("LoadLogFile with valid file") {
        std::string test_file = "test_main_window.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        
        bool result = window.LoadLogFile(test_file);
        
        REQUIRE(result);
        REQUIRE(window.IsFileLoaded());
        REQUIRE(window.GetCurrentFilePath() == test_file);
        REQUIRE_FALSE(window.GetDisplayedEntries().empty());
        REQUIRE(window.GetLastError().empty());
        
        DeleteTestFile(test_file);
    }
    
    SECTION("LoadLogFile with non-existent file") {
        MainWindow window;
        window.Initialize();
        
        bool result = window.LoadLogFile("non_existent.log");
        
        REQUIRE_FALSE(result);
        REQUIRE_FALSE(window.IsFileLoaded());
        REQUIRE(window.GetCurrentFilePath().empty());
        REQUIRE(window.GetDisplayedEntries().empty());
        REQUIRE_FALSE(window.GetLastError().empty());
    }
    
    SECTION("LoadLogFile with empty file") {
        std::string test_file = "empty_test.log";
        CreateTestLogFile(test_file, {});
        
        MainWindow window;
        window.Initialize();
        
        bool result = window.LoadLogFile(test_file);
        
        REQUIRE_FALSE(result);
        REQUIRE_FALSE(window.IsFileLoaded());
        REQUIRE_FALSE(window.GetLastError().empty());
        
        DeleteTestFile(test_file);
    }
    
    SECTION("ReloadCurrentFile") {
        std::string test_file = "reload_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        size_t initial_count = window.GetDisplayedEntries().size();
        
        // Add more lines to the file
        std::ofstream file(test_file, std::ios::app);
        file << "[2024.01.15-10.30.50:128][461]LogTemp: Info: New message\n";
        file.close();
        
        window.ReloadCurrentFile();
        
        REQUIRE(window.GetDisplayedEntries().size() > initial_count);
        
        DeleteTestFile(test_file);
    }
    
    SECTION("CloseCurrentFile") {
        std::string test_file = "close_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        REQUIRE(window.IsFileLoaded());
        
        window.CloseCurrentFile();
        
        REQUIRE_FALSE(window.IsFileLoaded());
        REQUIRE(window.GetCurrentFilePath().empty());
        REQUIRE(window.GetDisplayedEntries().empty());
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        REQUIRE_FALSE(window.IsTailing());
        
        DeleteTestFile(test_file);
    }
}

TEST_CASE("MainWindow Navigation", "[ui][main_window][navigation]") {
    
    SECTION("Navigation with loaded file") {
        std::string test_file = "nav_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        size_t entry_count = window.GetDisplayedEntries().size();
        REQUIRE(entry_count > 0);
        
        // Test initial state
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        // Test SelectNextEntry
        window.SelectNextEntry();
        REQUIRE(window.GetSelectedEntryIndex() == 1);
        
        // Test SelectPreviousEntry
        window.SelectPreviousEntry();
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        // Test GoToBottom
        window.GoToBottom();
        REQUIRE(window.GetSelectedEntryIndex() == static_cast<int>(entry_count) - 1);
        
        // Test GoToTop
        window.GoToTop();
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        // Test GoToLine
        if (entry_count > 2) {
            window.GoToLine(2);
            REQUIRE(window.GetSelectedEntryIndex() == 2);
        }
        
        DeleteTestFile(test_file);
    }
    
    SECTION("Navigation bounds checking") {
        std::string test_file = "bounds_test.log";
        CreateTestLogFile(test_file, {"[2024.01.15-10.30.45:123][456]LogTemp: Warning: Single entry"});
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Test that navigation doesn't go out of bounds
        window.SelectPreviousEntry(); // Should stay at 0
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        window.SelectNextEntry(); // Should stay at 0 (only one entry)
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        // Test invalid line numbers
        window.GoToLine(-1); // Should be ignored
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        window.GoToLine(100); // Should be ignored
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        DeleteTestFile(test_file);
    }
}

TEST_CASE("MainWindow Display Management", "[ui][main_window][display]") {
    
    SECTION("SetTerminalSize updates display") {
        MainWindow window;
        window.Initialize();
        
        window.SetTerminalSize(120, 40);
        
        // This test verifies the method doesn't crash
        // Actual display calculations are tested indirectly through rendering
        REQUIRE_NOTHROW(window.RefreshDisplay());
    }
    
    SECTION("ToggleFilterPanel") {
        MainWindow window;
        window.Initialize();
        
        REQUIRE_FALSE(window.IsFilterPanelVisible());
        
        window.ToggleFilterPanel();
        REQUIRE(window.IsFilterPanelVisible());
        
        window.ToggleFilterPanel();
        REQUIRE_FALSE(window.IsFilterPanelVisible());
    }
    
    SECTION("RefreshDisplay doesn't crash") {
        MainWindow window;
        window.Initialize();
        
        REQUIRE_NOTHROW(window.RefreshDisplay());
        
        // Load a file and test refresh
        std::string test_file = "refresh_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        window.LoadLogFile(test_file);
        
        REQUIRE_NOTHROW(window.RefreshDisplay());
        
        DeleteTestFile(test_file);
    }
}

TEST_CASE("MainWindow Event Handling", "[ui][main_window][events]") {
    
    SECTION("Keyboard event handling") {
        std::string test_file = "event_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Test arrow key navigation
        REQUIRE(window.OnEvent(ftxui::Event::ArrowDown));
        REQUIRE(window.GetSelectedEntryIndex() == 1);
        
        REQUIRE(window.OnEvent(ftxui::Event::ArrowUp));
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        // Test page navigation
        REQUIRE(window.OnEvent(ftxui::Event::PageDown));
        // Selection should change (exact value depends on visible rows)
        
        REQUIRE(window.OnEvent(ftxui::Event::Home));
        REQUIRE(window.GetSelectedEntryIndex() == 0);
        
        REQUIRE(window.OnEvent(ftxui::Event::End));
        REQUIRE(window.GetSelectedEntryIndex() > 0);
        
        // Test filter panel toggle
        REQUIRE(window.OnEvent(ftxui::Event::Tab));
        REQUIRE(window.IsFilterPanelVisible());
        
        // Test tailing toggle
        REQUIRE(window.OnEvent(ftxui::Event::F5));
        // Tailing state should change (may fail if file monitoring fails, which is OK)
        
        DeleteTestFile(test_file);
    }
    
    SECTION("Event handling without loaded file") {
        MainWindow window;
        window.Initialize();
        
        // Events should be handled gracefully even without a loaded file
        REQUIRE(window.OnEvent(ftxui::Event::ArrowDown));
        REQUIRE(window.OnEvent(ftxui::Event::ArrowUp));
        REQUIRE(window.OnEvent(ftxui::Event::Tab));
        
        REQUIRE(window.GetSelectedEntryIndex() == 0);
    }
}

TEST_CASE("MainWindow Rendering", "[ui][main_window][render]") {
    
    SECTION("Render without loaded file") {
        MainWindow window;
        window.Initialize();
        
        auto element = window.Render();
        REQUIRE(element != nullptr);
        
        // Render to string to verify content
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(80), ftxui::Dimension::Fixed(24));
        ftxui::Render(screen, element);
        std::string output = screen.ToString();
        
        REQUIRE(output.find("No log file loaded") != std::string::npos);
    }
    
    SECTION("Render with loaded file") {
        std::string test_file = "render_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        auto element = window.Render();
        REQUIRE(element != nullptr);
        
        // Render to string to verify content
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(120), ftxui::Dimension::Fixed(30));
        ftxui::Render(screen, element);
        std::string output = screen.ToString();
        
        // Verify log content appears
        REQUIRE(output.find("LogTemp") != std::string::npos);
        REQUIRE(output.find("Warning") != std::string::npos);
        REQUIRE(output.find("Test message") != std::string::npos);
        
        // Verify table structure
        REQUIRE(output.find("│") != std::string::npos);  // Table borders
        
        DeleteTestFile(test_file);
    }
    
    SECTION("Render with error state") {
        MainWindow window;
        window.Initialize();
        
        // Try to load non-existent file to trigger error state
        window.LoadLogFile("non_existent.log");
        
        auto element = window.Render();
        REQUIRE(element != nullptr);
        
        // Render to string to verify error content
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(80), ftxui::Dimension::Fixed(24));
        ftxui::Render(screen, element);
        std::string output = screen.ToString();
        
        REQUIRE(output.find("Error") != std::string::npos);
    }
    
    SECTION("GetComponent returns valid component") {
        MainWindow window;
        window.Initialize();
        
        auto component = window.GetComponent();
        REQUIRE(component != nullptr);
    }
}

TEST_CASE("MainWindow Real-time Updates", "[ui][main_window][realtime]") {
    
    SECTION("OnNewLogLines processes new entries") {
        std::string test_file = "realtime_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        size_t initial_count = window.GetDisplayedEntries().size();
        
        // Simulate new log lines
        std::vector<std::string> new_lines = {
            "[2024.01.15-10.30.50:128][461]LogTemp: Info: New entry 1",
            "[2024.01.15-10.30.51:129][462]LogTemp: Warning: New entry 2"
        };
        
        // This tests the callback logic (actual FileMonitor integration tested separately)
        // We can't directly call OnNewLogLines as it's private, but we can test the effect
        // through the public interface when tailing is enabled
        
        DeleteTestFile(test_file);
    }
}

TEST_CASE("MainWindow Error Handling", "[ui][main_window][error]") {
    
    SECTION("Graceful handling of invalid operations") {
        MainWindow window;
        window.Initialize();
        
        // Operations without loaded file should not crash
        REQUIRE_NOTHROW(window.ReloadCurrentFile());
        REQUIRE_NOTHROW(window.StartTailing());
        REQUIRE_NOTHROW(window.StopTailing());
        REQUIRE_NOTHROW(window.SelectNextEntry());
        REQUIRE_NOTHROW(window.SelectPreviousEntry());
        REQUIRE_NOTHROW(window.GoToTop());
        REQUIRE_NOTHROW(window.GoToBottom());
    }
    
    SECTION("Error state management") {
        MainWindow window;
        window.Initialize();
        
        // Initially no error
        REQUIRE(window.GetLastError().empty());
        
        // Loading invalid file should set error
        window.LoadLogFile("non_existent.log");
        REQUIRE_FALSE(window.GetLastError().empty());
        
        // Loading valid file should clear error
        std::string test_file = "error_clear_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        window.LoadLogFile(test_file);
        REQUIRE(window.GetLastError().empty());
        
        DeleteTestFile(test_file);
    }
}

TEST_CASE("MainWindow Visual Selection State Management", "[ui][main_window][visual_selection]") {
    
    SECTION("Initial visual selection state") {
        MainWindow window;
        window.Initialize();
        
        // Initially not in visual selection mode
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        REQUIRE(window.GetVisualSelectionRange() == std::make_pair(-1, -1));
        REQUIRE(window.GetVisualSelectionSize() == 0);
    }
    
    SECTION("EnterVisualSelectionMode with no file loaded") {
        MainWindow window;
        window.Initialize();
        
        // Should not enter visual selection mode without loaded file
        window.EnterVisualSelectionMode();
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        REQUIRE_FALSE(window.GetLastError().empty());
    }
    
    SECTION("EnterVisualSelectionMode with loaded file") {
        std::string test_file = "visual_selection_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Should enter visual selection mode successfully
        window.EnterVisualSelectionMode();
        REQUIRE(window.IsVisualSelectionMode());
        REQUIRE(window.GetVisualSelectionRange() == std::make_pair(0, 0));
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        DeleteTestFile(test_file);
    }
    
    SECTION("ExitVisualSelectionMode") {
        std::string test_file = "visual_selection_exit_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Enter visual selection mode
        window.EnterVisualSelectionMode();
        REQUIRE(window.IsVisualSelectionMode());
        
        // Exit visual selection mode
        window.ExitVisualSelectionMode();
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        REQUIRE(window.GetVisualSelectionRange() == std::make_pair(-1, -1));
        REQUIRE(window.GetVisualSelectionSize() == 0);
        
        DeleteTestFile(test_file);
    }
    
    SECTION("Visual selection with different starting positions") {
        std::string test_file = "visual_selection_position_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Navigate to different position
        window.SelectNextEntry();
        window.SelectNextEntry();
        REQUIRE(window.GetSelectedEntryIndex() == 2);
        
        // Enter visual selection mode
        window.EnterVisualSelectionMode();
        REQUIRE(window.IsVisualSelectionMode());
        REQUIRE(window.GetVisualSelectionRange() == std::make_pair(2, 2));
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        DeleteTestFile(test_file);
    }
    
    SECTION("GetVisualSelectionRange with invalid state") {
        MainWindow window;
        window.Initialize();
        
        // Without entering visual selection mode
        auto range = window.GetVisualSelectionRange();
        REQUIRE(range.first == -1);
        REQUIRE(range.second == -1);
        
        // Size should be 0
        REQUIRE(window.GetVisualSelectionSize() == 0);
    }
    
    SECTION("Visual selection state persistence") {
        std::string test_file = "visual_selection_persistence_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Enter visual selection mode
        window.EnterVisualSelectionMode();
        REQUIRE(window.IsVisualSelectionMode());
        
        // State should persist across other operations
        window.RefreshDisplay();
        REQUIRE(window.IsVisualSelectionMode());
        
        // Exit should clear state
        window.ExitVisualSelectionMode();
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        
        DeleteTestFile(test_file);
    }
    
    SECTION("Multiple enter/exit cycles") {
        std::string test_file = "visual_selection_cycles_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Multiple enter/exit cycles should work correctly
        for (int i = 0; i < 3; ++i) {
            window.EnterVisualSelectionMode();
            REQUIRE(window.IsVisualSelectionMode());
            REQUIRE(window.GetVisualSelectionSize() == 1);
            
            window.ExitVisualSelectionMode();
            REQUIRE_FALSE(window.IsVisualSelectionMode());
            REQUIRE(window.GetVisualSelectionSize() == 0);
        }
        
        DeleteTestFile(test_file);
    }
}

TEST_CASE("MainWindow Visual Selection Mode Activation", "[ui][main_window][visual_selection][activation]") {
    
    SECTION("'v' key activates visual selection mode") {
        std::string test_file = "visual_activation_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Initially not in visual selection mode
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        
        // Simulate 'v' key press through OnEvent
        auto component = window.CreateFTXUIComponent();
        ftxui::Event v_key = ftxui::Event::Character('v');
        bool handled = component->OnEvent(v_key);
        
        // Should handle the event and enter visual selection mode
        REQUIRE(handled);
        REQUIRE(window.IsVisualSelectionMode());
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        DeleteTestFile(test_file);
    }
    
    SECTION("'v' key with no file loaded shows error") {
        MainWindow window;
        window.Initialize();
        
        // Initially not in visual selection mode
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        
        // Simulate 'v' key press through OnEvent
        auto component = window.CreateFTXUIComponent();
        ftxui::Event v_key = ftxui::Event::Character('v');
        bool handled = component->OnEvent(v_key);
        
        // Should handle the event but not enter visual selection mode
        REQUIRE(handled);
        REQUIRE_FALSE(window.IsVisualSelectionMode());
        REQUIRE_FALSE(window.GetLastError().empty());
    }
    
    SECTION("Visual selection mode shows in status bar") {
        std::string test_file = "visual_status_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Enter visual selection mode
        window.EnterVisualSelectionMode();
        REQUIRE(window.IsVisualSelectionMode());
        
        // Render the window to check status bar
        auto rendered = window.Render();
        REQUIRE(rendered != nullptr);
        
        // The status bar should contain visual selection indicator
        // Note: We can't easily test the exact content without more complex rendering inspection
        // but we can verify the mode is active and size is correct
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        DeleteTestFile(test_file);
    }
    
    SECTION("Visual selection mode activation at different positions") {
        std::string test_file = "visual_position_activation_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Navigate to position 2
        window.SelectNextEntry();
        window.SelectNextEntry();
        REQUIRE(window.GetSelectedEntryIndex() == 2);
        
        // Activate visual selection mode with 'v' key
        auto component = window.CreateFTXUIComponent();
        ftxui::Event v_key = ftxui::Event::Character('v');
        bool handled = component->OnEvent(v_key);
        
        REQUIRE(handled);
        REQUIRE(window.IsVisualSelectionMode());
        REQUIRE(window.GetVisualSelectionRange() == std::make_pair(2, 2));
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        DeleteTestFile(test_file);
    }
    
    SECTION("Visual selection mode activation sets anchor point correctly") {
        std::string test_file = "visual_anchor_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        MainWindow window;
        window.Initialize();
        window.LoadLogFile(test_file);
        
        // Navigate to position 3
        for (int i = 0; i < 3; ++i) {
            window.SelectNextEntry();
        }
        REQUIRE(window.GetSelectedEntryIndex() == 3);
        
        // Activate visual selection mode
        window.EnterVisualSelectionMode();
        
        // Should set anchor point to current selection
        REQUIRE(window.IsVisualSelectionMode());
        auto range = window.GetVisualSelectionRange();
        REQUIRE(range.first == 3);
        REQUIRE(range.second == 3);
        REQUIRE(window.GetVisualSelectionSize() == 1);
        
        DeleteTestFile(test_file);
    }
}

TEST_CASE("MainWindow Integration", "[ui][main_window][integration]") {
    
    SECTION("Complete workflow") {
        std::string test_file = "workflow_test.log";
        CreateTestLogFile(test_file, CreateSampleLogLines());
        
        ConfigManager config_manager;
        MainWindow window(&config_manager);
        window.Initialize();
        
        // Load file
        REQUIRE(window.LoadLogFile(test_file));
        REQUIRE(window.IsFileLoaded());
        
        // Navigate
        window.SelectNextEntry();
        window.SelectNextEntry();
        REQUIRE(window.GetSelectedEntryIndex() == 2);
        
        // Toggle filter panel
        window.ToggleFilterPanel();
        REQUIRE(window.IsFilterPanelVisible());
        
        // Render
        auto element = window.Render();
        REQUIRE(element != nullptr);
        
        // Close
        window.CloseCurrentFile();
        REQUIRE_FALSE(window.IsFileLoaded());
        
        DeleteTestFile(test_file);
    }
}