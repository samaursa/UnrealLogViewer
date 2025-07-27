#include "../macros.h"
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include "../lib/core/autotest_runner.h"
#include "../lib/common/unreal_project_utils.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/animation.hpp>
#include <ftxui/dom/elements.hpp>
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include <thread>
#include <atomic>
#include <chrono>

using namespace ftxui;
using namespace ue_log;

int main(int argc, char* argv[]) {
    try {
        // Set up CLI11 command line parser
        CLI::App app{"Unreal Log Viewer - A terminal-based log file viewer for Unreal Engine logs"};
        app.set_version_flag("--version", "1.0.0");
        
        // Command line options
        std::string input_path;
        bool autotest_mode = false;
        std::string autotest_output = "autotest_report.txt";
        bool test_mode = false;
        
        // Add positional argument for log file or directory
        app.add_option("path", input_path, 
                       "Path to log file or directory containing log files. "
                       "If no path is provided, will attempt to find Saved/Logs directory in current location.")
           ->check([](const std::string& path) -> std::string {
               if (path.empty()) {
                   return "";  // Allow empty path for auto-detection
               }
               if (!std::filesystem::exists(path)) {
                   return "Path does not exist: " + path;
               }
               
               // Check if it's a file or directory
               std::error_code ec;
               if (std::filesystem::is_regular_file(path, ec)) {
                   // For files, check if it's a .log file
                   if (path.size() < 4 || path.substr(path.size() - 4) != ".log") {
                       return "File must have .log extension: " + path;
                   }
               } else if (std::filesystem::is_directory(path, ec)) {
                   // For directories, check if it's accessible
                   if (!ue_log::unreal_utils::ValidateDirectoryPath(path)) {
                       return "Directory is not accessible: " + path;
                   }
               } else {
                   return "Path must be a file or directory: " + path;
               }
               
               return "";
           });
        
        // Add autotest option
        app.add_flag("--autotest", autotest_mode, 
                     "Run in autotest mode - loads file, performs tests, and exits");
        
        // Add autotest output option
        app.add_option("--autotest-output", autotest_output,
                       "Output file for autotest report (default: autotest_report.txt)")
           ->needs("--autotest");
        
        // Add test mode option
        app.add_flag("--test", test_mode,
                     "Run in test mode - loads file and exits after 5 seconds");
        
        // Parse command line arguments
        try {
            app.parse(argc, argv);
        } catch (const CLI::ParseError& e) {
            return app.exit(e);
        }
        
        // Handle autotest mode first (before creating UI components)
        if (autotest_mode) {
            std::string log_file_path = input_path;
            
            // For autotest mode, we need a specific log file, not a directory
            if (input_path.empty()) {
                std::cerr << "Error: Log file path is required for autotest mode" << std::endl;
                return 1;
            }
            
            // If input_path is a directory, we can't run autotest
            std::error_code ec;
            if (std::filesystem::is_directory(input_path, ec)) {
                std::cerr << "Error: Autotest mode requires a specific log file, not a directory" << std::endl;
                return 1;
            }
            
            std::cout << "Running comprehensive autotest mode..." << std::endl;
            std::cout << "Log file: " << log_file_path << std::endl;
            std::cout << "Output report: " << autotest_output << std::endl;
            
            // Create and run the new AutotestRunner with timeout protection
            AutotestRunner autotest_runner(log_file_path, autotest_output);
            autotest_runner.SetVerbose(true); // Enable verbose output for user feedback
            
            // Run autotest with timeout (30 seconds)
            std::atomic<bool> test_completed{false};
            std::atomic<bool> test_success{false};
            
            std::thread autotest_thread([&]() {
                try {
                    test_success = autotest_runner.RunAllTests();
                    test_completed = true;
                } catch (...) {
                    test_success = false;
                    test_completed = true;
                }
            });
            
            // Wait for completion or timeout
            const int timeout_seconds = 30;
            for (int i = 0; i < timeout_seconds * 10; ++i) {
                if (test_completed) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            if (!test_completed) {
                std::cerr << "Autotest timed out after " << timeout_seconds << " seconds. Terminating..." << std::endl;
                // Force terminate the thread (not ideal but necessary for stuck processes)
                std::terminate();
            }
            
            autotest_thread.join();
            
            if (test_success) {
                std::cout << "Autotest completed successfully. Report written to: " << autotest_output << std::endl;
                return 0;
            } else {
                std::cerr << "Autotest failed. Check the report file for details: " << autotest_output << std::endl;
                return 1;
            }
        }
        
        // Handle test mode
        if (test_mode) {
            std::cout << "Running in test mode - will exit after 5 seconds..." << std::endl;
            
            // Create configuration manager and main window for test mode
            auto config_manager = std::make_unique<ConfigManager>();
            auto main_window = std::make_unique<MainWindow>(config_manager.get());
            
            // Initialize the main window
            main_window->Initialize();
            
            // Load the log file if provided
            if (!input_path.empty()) {
                std::error_code ec;
                if (std::filesystem::is_regular_file(input_path, ec)) {
                    std::cout << "Loading log file: " << input_path << std::endl;
                    if (!main_window->LoadLogFile(input_path)) {
                        std::cerr << "Failed to load file: " << main_window->GetLastError() << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "Test mode requires a valid log file path" << std::endl;
                    return 1;
                }
            }
            
            // Create FTXUI screen
            auto screen = ScreenInteractive::Fullscreen();
            
            // Set up exit callback
            main_window->SetExitCallback([&]() {
                screen.Exit();
            });
            
            // Set up refresh callback
            main_window->SetRefreshCallback([&]() {
                ftxui::animation::RequestAnimationFrame();
            });
            
            // Create the FTXUI component
            auto component = main_window->CreateFTXUIComponent();
            
            // Start a timer thread to exit after 5 seconds
            std::thread timer_thread([&]() {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                std::cout << "\nTest mode timeout reached - exiting..." << std::endl;
                screen.Exit();
            });
            timer_thread.detach();
            
            // Run the UI loop
            std::cout << "Starting UI loop in test mode..." << std::endl;
            screen.Loop(component);
            
            std::cout << "Test mode completed." << std::endl;
            return 0;
        }
        
        // Normal interactive mode
        std::cout << "Starting Unreal Log Viewer..." << std::endl;
        
        // Create configuration manager and main window for interactive mode
        auto config_manager = std::make_unique<ConfigManager>();
        auto main_window = std::make_unique<MainWindow>(config_manager.get());
        
        // Initialize the main window
        main_window->Initialize();
        
        // Determine what to do based on input_path
        std::string resolved_path = input_path;
        bool is_directory = false;
        
        if (input_path.empty()) {
            // No path provided - try to auto-detect Saved/Logs directory
            std::cout << "No path specified. Attempting to find Saved/Logs directory..." << std::endl;
            resolved_path = ue_log::unreal_utils::FindSavedLogsDirectory();
            
            if (!resolved_path.empty()) {
                std::cout << "Found Saved/Logs directory: " << resolved_path << std::endl;
                is_directory = true;
            } else {
                std::cout << "No Saved/Logs directory found. You can load a file from the UI." << std::endl;
            }
        } else {
            // Path was provided - determine if it's a file or directory
            std::error_code ec;
            if (std::filesystem::is_directory(input_path, ec)) {
                is_directory = true;
                std::cout << "Directory specified: " << input_path << std::endl;
            } else if (std::filesystem::is_regular_file(input_path, ec)) {
                is_directory = false;
                std::cout << "File specified: " << input_path << std::endl;
            }
        }
        
        // Handle the resolved path
        if (!resolved_path.empty()) {
            if (is_directory) {
                // Check if directory contains log files
                if (ue_log::unreal_utils::ContainsLogFiles(resolved_path)) {
                    std::cout << "Directory contains log files. Starting in file browser mode..." << std::endl;
                    // TODO: This will be implemented in task 6 - for now, show message
                    std::cout << "File browser mode not yet implemented. You can load a file from the UI." << std::endl;
                } else {
                    std::cerr << "Warning: Directory contains no .log files: " << resolved_path << std::endl;
                    std::cerr << "You can load a file from the UI or specify a different directory." << std::endl;
                }
            } else {
                // It's a file - load it directly
                std::cout << "Loading log file: " << resolved_path << std::endl;
                
                if (!main_window->LoadLogFile(resolved_path)) {
                    std::cerr << "Warning: Failed to load file: " << main_window->GetLastError() << std::endl;
                    std::cerr << "You can load a file from the UI or specify a different file." << std::endl;
                }
            }
        }
        
        // Create FTXUI screen and run the application
        auto screen = ScreenInteractive::Fullscreen();
        
        // Set up exit callback
        main_window->SetExitCallback([&]() {
            screen.Exit();
        });
        
        // Set up refresh callback to wake up the UI when new log entries arrive
        main_window->SetRefreshCallback([&]() {
            // Request animation frame to wake up the UI loop and trigger a refresh
            // This ensures the UI updates immediately when new log entries are detected,
            // even when the terminal window is not in focus
            ftxui::animation::RequestAnimationFrame();
        });
        
        // Create the FTXUI component
        auto component = main_window->CreateFTXUIComponent();
        
        // Run the UI loop
        std::cout << "Starting UI loop... (Press 'q' to quit, 'h' for help)" << std::endl;
        screen.Loop(component);
        
        std::cout << "Unreal Log Viewer exiting..." << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
}