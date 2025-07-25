#include "lib/ui/main_window.h"
#include <iostream>
#include <ftxui/component/event.hpp>

using namespace ue_log;
using namespace ftxui;

int main() {
    std::cout << "Testing 'G' key handler for tailing..." << std::endl;
    
    MainWindow window;
    window.Initialize();
    
    // Test 1: 'G' key without file should not start tailing
    std::cout << "\nTest 1: 'G' key without file loaded..." << std::endl;
    
    // Create a MainWindowComponent to test event handling
    class TestMainWindowComponent : public ComponentBase {
    public:
        TestMainWindowComponent(MainWindow* parent) : parent_(parent) {}
        
        Element Render() override {
            return parent_->Render();
        }
        
        bool OnEvent(Event event) override {
            // This is a copy of the OnEvent logic from MainWindowComponent
            if (event == Event::Character('G')) {
                // Only start tailing if not already active (no additional effect)
                if (!parent_->IsTailing()) {
                    parent_->StartTailing();
                }
                return true;
            }
            return false;
        }
        
    private:
        MainWindow* parent_;
    };
    
    auto component = Make<TestMainWindowComponent>(&window);
    
    // Simulate 'G' key press without file
    Event g_event = Event::Character('G');
    bool handled = component->OnEvent(g_event);
    
    if (handled && !window.IsTailing()) {
        std::cout << "✓ PASS: 'G' key handled but tailing not started without file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
    } else if (handled && window.IsTailing()) {
        std::cout << "✗ FAIL: 'G' key should not start tailing without file" << std::endl;
        return 1;
    } else {
        std::cout << "✗ FAIL: 'G' key was not handled" << std::endl;
        return 1;
    }
    
    // Test 2: Load file and test 'G' key
    std::cout << "\nTest 2: 'G' key with file loaded..." << std::endl;
    
    if (window.LoadLogFile("test_tailing.log")) {
        std::cout << "✓ File loaded successfully" << std::endl;
        
        // Simulate 'G' key press with file
        handled = component->OnEvent(g_event);
        
        if (handled && window.IsTailing()) {
            std::cout << "✓ PASS: 'G' key started tailing with file loaded" << std::endl;
            std::cout << "  Status: " << window.GetLastError() << std::endl;
            std::cout << "  IsTailing: " << (window.IsTailing() ? "true" : "false") << std::endl;
        } else {
            std::cout << "✗ FAIL: 'G' key should have started tailing with file loaded" << std::endl;
            std::cout << "  Handled: " << (handled ? "true" : "false") << std::endl;
            std::cout << "  IsTailing: " << (window.IsTailing() ? "true" : "false") << std::endl;
            std::cout << "  Error: " << window.GetLastError() << std::endl;
            return 1;
        }
        
        // Test 3: Press 'G' again (should have no additional effect)
        std::cout << "\nTest 3: 'G' key when already tailing..." << std::endl;
        
        handled = component->OnEvent(g_event);
        
        if (handled && window.IsTailing()) {
            std::cout << "✓ PASS: 'G' key handled but no additional effect when already tailing" << std::endl;
            std::cout << "  Status: " << window.GetLastError() << std::endl;
        } else {
            std::cout << "✗ FAIL: 'G' key should be handled and tailing should remain active" << std::endl;
            return 1;
        }
        
    } else {
        std::cout << "✗ FAIL: Could not load test file" << std::endl;
        std::cout << "  Error: " << window.GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "\n✓ All tests passed!" << std::endl;
    return 0;
}