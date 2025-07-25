#include "lib/ui/main_window.h"
#include <iostream>
#include <fstream>

using namespace ue_log;

int main() {
    // Create a test log file
    std::ofstream test_file("test_tailing_nav.log");
    test_file << "[2024-01-01-12.00.00:000][  0]LogTemp: Test message 1\n";
    test_file << "[2024-01-01-12.00.01:000][  1]LogTemp: Test message 2\n";
    test_file << "[2024-01-01-12.00.02:000][  2]LogTemp: Test message 3\n";
    test_file << "[2024-01-01-12.00.03:000][  3]LogTemp: Test message 4\n";
    test_file << "[2024-01-01-12.00.04:000][  4]LogTemp: Test message 5\n";
    test_file.close();
    
    // Create MainWindow instance
    MainWindow window;
    window.Initialize();
    
    // Load the test file
    if (!window.LoadLogFile("test_tailing_nav.log")) {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "✓ File loaded successfully" << std::endl;
    std::cout << "Initial tailing state: " << (window.IsTailing() ? "true" : "false") << std::endl;
    
    // Test 1: j key navigation cancels tailing
    std::cout << "\nTest 1: j key navigation cancels tailing..." << std::endl;
    if (!window.StartTailing()) {
        std::cout << "✗ FAIL: Could not start tailing" << std::endl;
        return 1;
    }
    std::cout << "✓ Tailing started: " << (window.IsTailing() ? "true" : "false") << std::endl;
    
    window.SelectNextEntry();
    if (!window.IsTailing()) {
        std::cout << "✓ PASS: j navigation correctly stopped tailing" << std::endl;
    } else {
        std::cout << "✗ FAIL: j navigation should have stopped tailing" << std::endl;
        return 1;
    }
    
    // Test 2: k key navigation cancels tailing
    std::cout << "\nTest 2: k key navigation cancels tailing..." << std::endl;
    if (!window.StartTailing()) {
        std::cout << "✗ FAIL: Could not restart tailing" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "✓ Tailing restarted: " << (window.IsTailing() ? "true" : "false") << std::endl;
    
    window.SelectPreviousEntry();
    if (!window.IsTailing()) {
        std::cout << "✓ PASS: k navigation correctly stopped tailing" << std::endl;
    } else {
        std::cout << "✗ FAIL: k navigation should have stopped tailing" << std::endl;
        return 1;
    }
    
    // Test 3: Ctrl+d navigation cancels tailing
    std::cout << "\nTest 3: Ctrl+d navigation cancels tailing..." << std::endl;
    if (!window.StartTailing()) {
        std::cout << "✗ FAIL: Could not restart tailing" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "✓ Tailing restarted: " << (window.IsTailing() ? "true" : "false") << std::endl;
    
    window.HalfPageDown();
    if (!window.IsTailing()) {
        std::cout << "✓ PASS: Ctrl+d navigation correctly stopped tailing" << std::endl;
    } else {
        std::cout << "✗ FAIL: Ctrl+d navigation should have stopped tailing" << std::endl;
        return 1;
    }
    
    // Test 4: Ctrl+u navigation cancels tailing
    std::cout << "\nTest 4: Ctrl+u navigation cancels tailing..." << std::endl;
    if (!window.StartTailing()) {
        std::cout << "✗ FAIL: Could not restart tailing" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "✓ Tailing restarted: " << (window.IsTailing() ? "true" : "false") << std::endl;
    
    window.HalfPageUp();
    if (!window.IsTailing()) {
        std::cout << "✓ PASS: Ctrl+u navigation correctly stopped tailing" << std::endl;
    } else {
        std::cout << "✗ FAIL: Ctrl+u navigation should have stopped tailing" << std::endl;
        return 1;
    }
    
    // Clean up
    std::remove("test_tailing_nav.log");
    
    std::cout << "\n✓ All navigation cancellation tests passed!" << std::endl;
    return 0;
}