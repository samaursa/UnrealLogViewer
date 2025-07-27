#include "lib/ui/file_browser.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

using namespace ue_log;
using namespace ftxui;

int main() {
    // Create a test directory with some log files
    std::string test_dir = "demo_logs";
    std::filesystem::create_directory(test_dir);
    
    // Create some test log files
    std::ofstream file1(test_dir + "/application.log");
    file1 << "Application started\nLoading configuration...\nReady to process requests\n";
    file1.close();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::ofstream file2(test_dir + "/error.log");
    file2 << "ERROR: Failed to connect to database\nERROR: Timeout occurred\n";
    file2.close();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::ofstream file3(test_dir + "/debug.log");
    file3 << "DEBUG: Processing request 1\nDEBUG: Processing request 2\nDEBUG: Processing request 3\n";
    file3.close();
    
    // Create FileBrowser and initialize
    FileBrowser browser(test_dir);
    browser.Initialize();
    browser.SetFocus(true);
    
    std::cout << "File Browser Demo - Use j/k to navigate, Ctrl+u/d for half-page, q to quit\n";
    std::cout << "Press any key to start...\n";
    std::cin.get();
    
    auto screen = ScreenInteractive::Fullscreen();
    
    auto component = CatchEvent(browser.CreateFTXUIComponent(), [&](Event event) {
        if (event == Event::Character('q')) {
            screen.ExitLoopClosure()();
            return true;
        }
        if (event == Event::Return) {
            std::string selected = browser.GetSelectedFilePath();
            if (!selected.empty()) {
                screen.ExitLoopClosure()();
                std::cout << "\nSelected file: " << selected << std::endl;
            }
            return true;
        }
        return false;
    });
    
    screen.Loop(component);
    
    // Cleanup
    std::filesystem::remove_all(test_dir);
    
    return 0;
}