#include "../macros.h"
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"
#include "../lib/core/autotest_runner.h"

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
        std::string log_file_path;
        bool autotest_mode = false;
        std::string autotest_output = "autotest_report.txt";
        
        // Add positional argument for log file
        app.add_option("logfile", log_file_path, "Path to the log file to open")
           ->check(CLI::ExistingFile);
        
        // Add autotest option
        app.add_flag("--autotest", autotest_mode, 
                     "Run in autotest mode - loads file, performs tests, and exits");
        
        // Add autotest output option
        app.add_option("--autotest-output", autotest_output,
                       "Output file for autotest report (default: autotest_report.txt)")
           ->needs("--autotest");
        
        // Parse command line arguments
        try {
            app.parse(argc, argv);
        } catch (const CLI::ParseError& e) {
            return app.exit(e);
        }
        
        // Handle autotest mode first (before creating UI components)
        if (autotest_mode) {
            if (log_file_path.empty()) {
                std::cerr << "Error: Log file path is required for autotest mode" << std::endl;
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
        
        // Normal interactive mode
        std::cout << "Starting Unreal Log Viewer..." << std::endl;
        
        // Create configuration manager and main window for interactive mode
        auto config_manager = std::make_unique<ConfigManager>();
        auto main_window = std::make_unique<MainWindow>(config_manager.get());
        
        // Initialize the main window
        main_window->Initialize();
        
        // Load initial file if provided
        if (!log_file_path.empty()) {
            std::cout << "Loading log file: " << log_file_path << std::endl;
            
            if (!main_window->LoadLogFile(log_file_path)) {
                std::cerr << "Warning: Failed to load file: " << main_window->GetLastError() << std::endl;
                std::cerr << "You can load a file from the UI or specify a different file." << std::endl;
            }
        } else {
            std::cout << "No log file specified. You can load a file from the UI." << std::endl;
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