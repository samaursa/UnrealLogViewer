#include "../macros.h"
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <filesystem>

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
        
        // Create configuration manager
        auto config_manager = std::make_unique<ConfigManager>();
        
        // Create main window
        auto main_window = std::make_unique<MainWindow>(config_manager.get());
        
        // Initialize the main window
        main_window->Initialize();
        
        // Handle autotest mode
        if (autotest_mode) {
            if (log_file_path.empty()) {
                std::cerr << "Error: Log file path is required for autotest mode" << std::endl;
                return 1;
            }
            
            std::cout << "Running autotest mode..." << std::endl;
            std::cout << "Log file: " << log_file_path << std::endl;
            std::cout << "Output report: " << autotest_output << std::endl;
            
            bool success = main_window->RunAutotest(log_file_path, autotest_output);
            
            if (success) {
                std::cout << "Autotest completed successfully. Report written to: " << autotest_output << std::endl;
                return 0;
            } else {
                std::cerr << "Autotest failed. Check the report file for details: " << autotest_output << std::endl;
                return 1;
            }
        }
        
        // Normal interactive mode
        std::cout << "Starting Unreal Log Viewer..." << std::endl;
        
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