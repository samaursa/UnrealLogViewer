#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file
    std::ofstream test_file("test_tailing.log");
    test_file << "[2024-01-01-12.00.00:000][  0]LogTemp: Test message 1\n";
    test_file << "[2024-01-01-12.00.01:000][  1]LogTemp: Test message 2\n";
    test_file.close();
    
    // Create MainWindow and test tailing functionality
    MainWindow window;
    window.Initialize();
    
    // Test 1: StartTailing without file should fail
    std::cout << "Test 1: StartTailing without file loaded..." << std::endl;
    bool result = window.StartTailing();
    if (!result) {
        std::cout << "✓ PASS: StartTailing correctly failed without file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
    } else {
        std::cout << "✗ FAIL: StartTailing should have failed without file" << std::endl;
        return 1;
    }
    
    // Test 2: Load file and test StartTailing
    std::cout << "\nTest 2: StartTailing with file loaded..." << std::endl;
    if (window.LoadLogFile("test_tailing.log")) {
        std::cout << "✓ File loaded successfully" << std::endl;
        
        result = window.StartTailing();
        if (result) {
            std::cout << "✓ PASS: StartTailing succeeded with file loaded" << std::endl;
            std::cout << "  Status: " << window.GetLastError() << std::endl;
            std::cout << "  IsTailing: " << (window.IsTailing() ? "true" : "false") << std::endl;
        } else {
            std::cout << "✗ FAIL: StartTailing should have succeeded with file loaded" << std::endl;
            std::cout << "  Error: " << window.GetLastError() << std::endl;
            return 1;
        }
    } else {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    // Test 3: Test StopTailing
    std::cout << "\nTest 3: StopTailing..." << std::endl;
    window.StopTailing();
    if (!window.IsTailing()) {
        std::cout << "✓ PASS: StopTailing correctly stopped tailing" << std::endl;
        std::cout << "  Status: " << window.GetLastError() << std::endl;
    } else {
        std::cout << "✗ FAIL: StopTailing should have stopped tailing" << std::endl;
        return 1;
    }
    
    // Clean up
    std::remove("test_tailing.log");
    
    std::cout << "\n✓ All tests passed!" << std::endl;
    return 0;
}