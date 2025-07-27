#include <catch2/catch_test_macros.hpp>
#include "../lib/ui/file_browser.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace ue_log;

TEST_CASE("FileBrowser basic functionality", "[file_browser]") {
    // Create a temporary directory with test log files
    std::string test_dir = "test_file_browser_temp";
    std::filesystem::create_directory(test_dir);
    
    // Create test log files with different modification times
    std::ofstream file1(test_dir + "/test1.log");
    file1 << "Test log content 1\n";
    file1.close();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::ofstream file2(test_dir + "/test2.log");
    file2 << "Test log content 2\nMore content\n";
    file2.close();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::ofstream file3(test_dir + "/test3.log");
    file3 << "Test log content 3\n";
    file3.close();
    
    // Create a non-log file (should be ignored)
    std::ofstream non_log(test_dir + "/test.txt");
    non_log << "Not a log file\n";
    non_log.close();
    
    SECTION("Constructor and initialization") {
        FileBrowser browser(test_dir);
        browser.Initialize();
        
        REQUIRE(browser.HasFiles());
        REQUIRE(browser.GetTitle() == "File Browser");
    }
    
    SECTION("File scanning and sorting") {
        FileBrowser browser(test_dir);
        browser.Initialize();
        
        // Should have 3 log files (ignoring the .txt file)
        REQUIRE(browser.HasFiles());
        
        // First file should be the most recently modified (test3.log)
        std::string selected_path = browser.GetSelectedFilePath();
        REQUIRE(selected_path.find("test3.log") != std::string::npos);
    }
    
    SECTION("Navigation") {
        FileBrowser browser(test_dir);
        browser.Initialize();
        
        // Start with first file selected
        std::string first_file = browser.GetSelectedFilePath();
        
        // Move to next file
        browser.SelectNext();
        std::string second_file = browser.GetSelectedFilePath();
        REQUIRE(first_file != second_file);
        
        // Move back to previous file
        browser.SelectPrevious();
        std::string back_to_first = browser.GetSelectedFilePath();
        REQUIRE(first_file == back_to_first);
    }
    
    SECTION("Boundary conditions") {
        FileBrowser browser(test_dir);
        browser.Initialize();
        
        // Try to go before first file
        browser.SelectPrevious();
        std::string first_attempt = browser.GetSelectedFilePath();
        
        browser.SelectPrevious();
        std::string second_attempt = browser.GetSelectedFilePath();
        REQUIRE(first_attempt == second_attempt); // Should stay at first file
        
        // Go to last file
        browser.SelectNext();
        browser.SelectNext();
        browser.SelectNext(); // Should be at last file now
        
        std::string last_file = browser.GetSelectedFilePath();
        browser.SelectNext(); // Try to go past last file
        std::string still_last = browser.GetSelectedFilePath();
        REQUIRE(last_file == still_last); // Should stay at last file
    }
    
    SECTION("Empty directory") {
        std::string empty_dir = "test_empty_dir";
        std::filesystem::create_directory(empty_dir);
        
        FileBrowser browser(empty_dir);
        browser.Initialize();
        
        REQUIRE_FALSE(browser.HasFiles());
        REQUIRE(browser.GetSelectedFilePath().empty());
        
        // Cleanup
        std::filesystem::remove_all(empty_dir);
    }
    
    SECTION("Non-existent directory") {
        FileBrowser browser("non_existent_directory");
        browser.Initialize();
        
        REQUIRE_FALSE(browser.HasFiles());
        REQUIRE(browser.GetSelectedFilePath().empty());
    }
    
    // Cleanup
    std::filesystem::remove_all(test_dir);
}

TEST_CASE("FileBrowser vim-style navigation", "[file_browser][navigation]") {
    // Create a temporary directory with multiple test log files
    std::string test_dir = "test_navigation_temp";
    std::filesystem::create_directory(test_dir);
    
    // Create 10 test log files for comprehensive navigation testing
    for (int i = 1; i <= 10; ++i) {
        std::ofstream file(test_dir + "/test" + std::to_string(i) + ".log");
        file << "Test log content " << i << "\n";
        file.close();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    FileBrowser browser(test_dir);
    browser.Initialize();
    
    SECTION("Basic j/k navigation") {
        // Start at first file (index 0)
        std::string initial_file = browser.GetSelectedFilePath();
        
        // Move down with 'j' equivalent
        browser.SelectNext();
        std::string second_file = browser.GetSelectedFilePath();
        REQUIRE(initial_file != second_file);
        
        // Move up with 'k' equivalent
        browser.SelectPrevious();
        std::string back_to_first = browser.GetSelectedFilePath();
        REQUIRE(initial_file == back_to_first);
    }
    
    SECTION("Boundary conditions for j/k navigation") {
        // Test upper boundary (beginning of list)
        std::string first_file = browser.GetSelectedFilePath();
        
        // Try to move up from first position
        browser.SelectPrevious();
        browser.SelectPrevious();
        browser.SelectPrevious();
        
        std::string still_first = browser.GetSelectedFilePath();
        REQUIRE(first_file == still_first);
        
        // Navigate to last file
        for (int i = 0; i < 15; ++i) { // More than enough to reach the end
            browser.SelectNext();
        }
        
        std::string last_file = browser.GetSelectedFilePath();
        
        // Try to move down from last position
        browser.SelectNext();
        browser.SelectNext();
        browser.SelectNext();
        
        std::string still_last = browser.GetSelectedFilePath();
        REQUIRE(last_file == still_last);
    }
    
    SECTION("Half-page navigation (Ctrl+u/Ctrl+d equivalent)") {
        // Start at first file
        std::string initial_file = browser.GetSelectedFilePath();
        
        // Move down half page
        browser.HalfPageDown();
        std::string after_half_down = browser.GetSelectedFilePath();
        REQUIRE(initial_file != after_half_down);
        
        // Move up half page
        browser.HalfPageUp();
        std::string after_half_up = browser.GetSelectedFilePath();
        
        // Should be back to initial or close to it
        REQUIRE(after_half_up == initial_file);
    }
    
    SECTION("Half-page navigation boundary conditions") {
        // Test half-page up from beginning
        std::string first_file = browser.GetSelectedFilePath();
        browser.HalfPageUp();
        std::string still_first = browser.GetSelectedFilePath();
        REQUIRE(first_file == still_first);
        
        // Navigate to end and test half-page down
        for (int i = 0; i < 15; ++i) {
            browser.SelectNext();
        }
        
        std::string last_file = browser.GetSelectedFilePath();
        browser.HalfPageDown();
        std::string still_last = browser.GetSelectedFilePath();
        REQUIRE(last_file == still_last);
    }
    
    SECTION("Full page navigation") {
        std::string initial_file = browser.GetSelectedFilePath();
        
        // Move down full page
        browser.PageDown();
        std::string after_page_down = browser.GetSelectedFilePath();
        REQUIRE(initial_file != after_page_down);
        
        // Move up full page
        browser.PageUp();
        std::string after_page_up = browser.GetSelectedFilePath();
        
        // Should be back to initial or close to it
        REQUIRE(after_page_up == initial_file);
    }
    
    SECTION("Event handling for vim keys") {
        browser.SetFocus(true);
        
        // Test 'j' key event
        ftxui::Event j_event = ftxui::Event::Character('j');
        std::string before_j = browser.GetSelectedFilePath();
        bool handled = browser.OnEvent(j_event);
        std::string after_j = browser.GetSelectedFilePath();
        
        REQUIRE(handled);
        REQUIRE(before_j != after_j);
        
        // Test 'k' key event
        ftxui::Event k_event = ftxui::Event::Character('k');
        bool k_handled = browser.OnEvent(k_event);
        std::string after_k = browser.GetSelectedFilePath();
        
        REQUIRE(k_handled);
        REQUIRE(before_j == after_k);
    }
    
    SECTION("Event handling for Ctrl+u/Ctrl+d") {
        browser.SetFocus(true);
        
        // Test Ctrl+d event (ASCII 4)
        ftxui::Event ctrl_d_event = ftxui::Event::Character(static_cast<char>(4));
        std::string before_ctrl_d = browser.GetSelectedFilePath();
        bool handled = browser.OnEvent(ctrl_d_event);
        std::string after_ctrl_d = browser.GetSelectedFilePath();
        
        REQUIRE(handled);
        REQUIRE(before_ctrl_d != after_ctrl_d);
        
        // Test Ctrl+u event (ASCII 21)
        ftxui::Event ctrl_u_event = ftxui::Event::Character(static_cast<char>(21));
        bool u_handled = browser.OnEvent(ctrl_u_event);
        std::string after_ctrl_u = browser.GetSelectedFilePath();
        
        REQUIRE(u_handled);
        REQUIRE(before_ctrl_d == after_ctrl_u);
    }
    
    SECTION("Event handling when not focused") {
        browser.SetFocus(false);
        
        ftxui::Event j_event = ftxui::Event::Character('j');
        std::string before = browser.GetSelectedFilePath();
        bool handled = browser.OnEvent(j_event);
        std::string after = browser.GetSelectedFilePath();
        
        REQUIRE_FALSE(handled);
        REQUIRE(before == after); // Should not change when not focused
    }
    
    SECTION("Arrow key navigation") {
        browser.SetFocus(true);
        
        // Test arrow down
        ftxui::Event arrow_down = ftxui::Event::ArrowDown;
        std::string before_down = browser.GetSelectedFilePath();
        bool handled = browser.OnEvent(arrow_down);
        std::string after_down = browser.GetSelectedFilePath();
        
        REQUIRE(handled);
        REQUIRE(before_down != after_down);
        
        // Test arrow up
        ftxui::Event arrow_up = ftxui::Event::ArrowUp;
        bool up_handled = browser.OnEvent(arrow_up);
        std::string after_up = browser.GetSelectedFilePath();
        
        REQUIRE(up_handled);
        REQUIRE(before_down == after_up);
    }
    
    // Cleanup
    std::filesystem::remove_all(test_dir);
}

TEST_CASE("FileBrowser navigation with empty file list", "[file_browser][navigation][edge_cases]") {
    std::string empty_dir = "test_empty_navigation";
    std::filesystem::create_directory(empty_dir);
    
    FileBrowser browser(empty_dir);
    browser.Initialize();
    
    SECTION("Navigation methods with empty list") {
        // All navigation methods should handle empty file list gracefully
        REQUIRE_NOTHROW(browser.SelectNext());
        REQUIRE_NOTHROW(browser.SelectPrevious());
        REQUIRE_NOTHROW(browser.HalfPageDown());
        REQUIRE_NOTHROW(browser.HalfPageUp());
        REQUIRE_NOTHROW(browser.PageDown());
        REQUIRE_NOTHROW(browser.PageUp());
        
        // Selected file path should remain empty
        REQUIRE(browser.GetSelectedFilePath().empty());
    }
    
    SECTION("Event handling with empty list") {
        browser.SetFocus(true);
        
        ftxui::Event j_event = ftxui::Event::Character('j');
        bool handled = browser.OnEvent(j_event);
        
        REQUIRE(handled); // Event should still be handled
        REQUIRE(browser.GetSelectedFilePath().empty());
    }
    
    // Cleanup
    std::filesystem::remove_all(empty_dir);
}

TEST_CASE("FileBrowser navigation with single file", "[file_browser][navigation][edge_cases]") {
    std::string single_file_dir = "test_single_file";
    std::filesystem::create_directory(single_file_dir);
    
    // Create single log file
    std::ofstream file(single_file_dir + "/single.log");
    file << "Single log file content\n";
    file.close();
    
    FileBrowser browser(single_file_dir);
    browser.Initialize();
    
    SECTION("Navigation with single file") {
        std::string single_file = browser.GetSelectedFilePath();
        REQUIRE_FALSE(single_file.empty());
        
        // All navigation should keep selection on the same file
        browser.SelectNext();
        REQUIRE(browser.GetSelectedFilePath() == single_file);
        
        browser.SelectPrevious();
        REQUIRE(browser.GetSelectedFilePath() == single_file);
        
        browser.HalfPageDown();
        REQUIRE(browser.GetSelectedFilePath() == single_file);
        
        browser.HalfPageUp();
        REQUIRE(browser.GetSelectedFilePath() == single_file);
        
        browser.PageDown();
        REQUIRE(browser.GetSelectedFilePath() == single_file);
        
        browser.PageUp();
        REQUIRE(browser.GetSelectedFilePath() == single_file);
    }
    
    // Cleanup
    std::filesystem::remove_all(single_file_dir);
}

TEST_CASE("FileBrowser file selection mechanism", "[file_browser][selection]") {
    // Create a temporary directory with test log files
    std::string test_dir = "test_file_selection_temp";
    std::filesystem::create_directory(test_dir);
    
    // Create test log files
    std::ofstream file1(test_dir + "/test1.log");
    file1 << "Test log content 1\n";
    file1.close();
    
    std::ofstream file2(test_dir + "/test2.log");
    file2 << "Test log content 2\n";
    file2.close();
    
    FileBrowser browser(test_dir);
    browser.Initialize();
    
    SECTION("File selection callback") {
        std::string selected_file;
        bool callback_called = false;
        
        // Set up callback
        browser.SetFileSelectionCallback([&](const std::string& file_path) {
            selected_file = file_path;
            callback_called = true;
        });
        
        // Simulate Enter key press
        browser.SetFocus(true);
        ftxui::Event enter_event = ftxui::Event::Return;
        bool handled = browser.OnEvent(enter_event);
        
        REQUIRE(handled);
        REQUIRE(callback_called);
        REQUIRE_FALSE(selected_file.empty());
        REQUIRE(selected_file.find(".log") != std::string::npos);
    }
    
    SECTION("File selection with error handling") {
        std::string error_message;
        bool error_callback_called = false;
        
        // Set up error callback
        browser.SetErrorCallback([&](const std::string& error) {
            error_message = error;
            error_callback_called = true;
        });
        
        // Test with empty file list
        FileBrowser empty_browser("non_existent_dir");
        empty_browser.Initialize();
        empty_browser.SetErrorCallback([&](const std::string& error) {
            error_message = error;
            error_callback_called = true;
        });
        
        empty_browser.LoadSelectedFile();
        
        REQUIRE(error_callback_called);
        REQUIRE_FALSE(error_message.empty());
    }
    
    SECTION("File selection without callback") {
        // Should handle gracefully when no callback is set
        browser.SetFocus(true);
        ftxui::Event enter_event = ftxui::Event::Return;
        
        REQUIRE_NOTHROW(browser.OnEvent(enter_event));
    }
    
    SECTION("File validation during selection") {
        std::string selected_file;
        std::string error_message;
        bool selection_callback_called = false;
        bool error_callback_called = false;
        
        browser.SetFileSelectionCallback([&](const std::string& file_path) {
            selected_file = file_path;
            selection_callback_called = true;
        });
        
        browser.SetErrorCallback([&](const std::string& error) {
            error_message = error;
            error_callback_called = true;
        });
        
        // Delete one of the files to simulate file disappearing
        std::filesystem::remove(test_dir + "/test1.log");
        
        // Try to load the deleted file (it might still be in the browser's file list)
        browser.LoadSelectedFile();
        
        // Should either call selection callback (if file still exists) or error callback
        REQUIRE((selection_callback_called || error_callback_called));
    }
    
    // Cleanup
    std::filesystem::remove_all(test_dir);
}