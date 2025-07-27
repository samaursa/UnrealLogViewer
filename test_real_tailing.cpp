#include "lib/ui/main_window.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

int main() {
    try {
        // Create a test log file with properly formatted Unreal log entries
        std::string test_file = "test_real_tailing.log";
        std::ofstream file(test_file);
        
        // Create properly formatted Unreal log entries
        file << "[2024.09.30-14.22.24:342][  0]LogInit: Display: Initial entry 1\n";
        file << "[2024.09.30-14.22.24:343][  1]LogWindows: Display: Initial entry 2\n";
        file << "[2024.09.30-14.22.24:344][  2]LogCore: Warning: Initial entry 3\n";
        file.close();
        
        // Create MainWindow instance
        ue_log::ConfigManager config;
        ue_log::MainWindow window(&config);
        window.Initialize();
        
        // Load the test file
        if (!window.LoadLogFile(test_file)) {
            std::cerr << "Failed to load test file" << std::endl;
            return 1;
        }
        
        std::cout << "Initial entries loaded: " << window.GetDisplayedEntries().size() << std::endl;
        
        // Start tailing
        if (!window.StartTailing()) {
            std::cerr << "Failed to start tailing" << std::endl;
            return 1;
        }
        
        std::cout << "Tailing started. Entries: " << window.GetDisplayedEntries().size() << std::endl;
        
        // Add one properly formatted new line
        std::ofstream append_file(test_file, std::ios::app);
        append_file << "[2024.09.30-14.22.24:345][  3]LogCore: Display: New entry 4\n";
        append_file.close();
        
        // Wait for file monitor to detect the change
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "After adding 1 line - Entries: " << window.GetDisplayedEntries().size() << std::endl;
        
        // Add another properly formatted line
        std::ofstream append_file2(test_file, std::ios::app);
        append_file2 << "[2024.09.30-14.22.24:346][  4]LogCore: Display: New entry 5\n";
        append_file2.close();
        
        // Wait for file monitor to detect the change
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "After adding 2nd line - Entries: " << window.GetDisplayedEntries().size() << std::endl;
        
        // Expected: 3 initial + 1 + 1 = 5 entries
        int expected = 5;
        int actual = static_cast<int>(window.GetDisplayedEntries().size());
        
        if (actual == expected) {
            std::cout << "✓ PASS: Correct number of entries (" << expected << ")" << std::endl;
        } else {
            std::cout << "✗ FAIL: Expected " << expected << " entries, got " << actual << std::endl;
            
            // If we got way more than expected, it suggests the bug is present
            if (actual > expected * 2) {
                std::cout << "  This suggests the full file re-read bug is present!" << std::endl;
            }
        }
        
        // Clean up
        std::filesystem::remove(test_file);
        
        return (actual == expected) ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}