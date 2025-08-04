#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace ue_log;

int main() {
    // Create a sample log file
    std::ofstream log_file("clipboard_demo.log");
    log_file << "[2023-01-01 10:00:00.000] [Frame: 1] [LogTemp] Info: First log entry\n";
    log_file << "[2023-01-01 10:00:01.000] [Frame: 2] [LogTemp] Warning: Second log entry\n";
    log_file << "[2023-01-01 10:00:02.000] [Frame: 3] [LogTemp] Error: Third log entry\n";
    log_file << "[2023-01-01 10:00:03.000] [Frame: 4] [LogTemp] Info: Fourth log entry\n";
    log_file << "[2023-01-01 10:00:04.000] [Frame: 5] [LogTemp] Info: Fifth log entry\n";
    log_file.close();
    
    // Create main window and load the file
    ConfigManager config_manager;
    MainWindow main_window(&config_manager);
    main_window.Initialize();
    
    if (!main_window.LoadLogFile("clipboard_demo.log")) {
        std::cout << "Failed to load log file: " << main_window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Loaded " << main_window.GetDisplayedEntries().size() << " log entries" << std::endl;
    
    // Test 1: Single line copy
    std::cout << "\n=== Test 1: Single line copy ===" << std::endl;
    main_window.EnterVisualSelectionMode();
    if (main_window.IsVisualSelectionMode()) {
        std::cout << "Entered visual selection mode" << std::endl;
        std::cout << "Selection size: " << main_window.GetVisualSelectionSize() << std::endl;
        
        main_window.CopyVisualSelectionToClipboard();
        std::cout << "Copy result: " << main_window.GetLastError() << std::endl;
        
        main_window.ExitVisualSelectionMode();
    }
    
    // Test 2: Multi-line copy
    std::cout << "\n=== Test 2: Multi-line copy ===" << std::endl;
    main_window.EnterVisualSelectionMode();
    if (main_window.IsVisualSelectionMode()) {
        main_window.ExtendVisualSelection(2); // Select entries 0, 1, 2
        std::cout << "Extended selection to " << main_window.GetVisualSelectionSize() << " lines" << std::endl;
        
        auto range = main_window.GetVisualSelectionRange();
        std::cout << "Selection range: [" << range.first << ", " << range.second << "]" << std::endl;
        
        main_window.CopyVisualSelectionToClipboard();
        std::cout << "Copy result: " << main_window.GetLastError() << std::endl;
        
        main_window.ExitVisualSelectionMode();
    }
    
    // Test 3: Error handling - copy without visual selection
    std::cout << "\n=== Test 3: Error handling ===" << std::endl;
    main_window.CopyVisualSelectionToClipboard();
    std::cout << "Copy without visual selection: " << main_window.GetLastError() << std::endl;
    
    // Clean up
    std::remove("clipboard_demo.log");
    
    std::cout << "\nDemo completed successfully!" << std::endl;
    return 0;
}