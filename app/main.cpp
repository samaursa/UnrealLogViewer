#include "../macros.h"
#include "../lib/ui/main_window.h"
#include "../lib/config/config_manager.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <string>
#include <memory>

using namespace ftxui;
using namespace ue_log;

int main(int argc, char* argv[]) {
    try {
        // Create configuration manager
        auto config_manager = std::make_unique<ConfigManager>();
        
        // Try to load configuration (placeholder - will be implemented later)
        std::cout << "Using default configuration" << std::endl;
        
        // Create main window
        auto main_window = std::make_unique<MainWindow>(config_manager.get());
        
        // Initialize the main window
        main_window->Initialize();
        
        // Load initial file if provided
        if (argc > 1) {
            std::string file_path = argv[1];
            std::cout << "Loading log file: " << file_path << std::endl;
            
            if (!main_window->LoadLogFile(file_path)) {
                std::cerr << "Error loading file: " << main_window->GetLastError() << std::endl;
                // Continue anyway - user can load file from UI
            }
        }
        
        // Create FTXUI screen and run the application
        auto screen = ScreenInteractive::Fullscreen();
        
        // Set up exit callback
        bool should_exit = false;
        main_window->SetExitCallback([&]() {
            should_exit = true;
            screen.Exit();
        });
        
        // Create the FTXUI component
        auto component = main_window->CreateFTXUIComponent();
        
        // Run the UI loop
        std::cout << "Starting UI loop..." << std::endl;
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