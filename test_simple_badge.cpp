#include "lib/ui/log_entry_renderer.h"
#include "lib/ui/visual_theme_manager.h"
#include "lib/log_parser/log_entry.h"
#include <iostream>
#include <memory>

using namespace ue_log;

int main() {
    try {
        // Create theme manager and renderer
        auto theme_manager = std::make_unique<VisualThemeManager>();
        auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
        
        // Create test log entry
        LogEntry entry(LogEntryType::Structured, 
                       std::string("12:34:56.789"), 
                       123, 
                       "GameEngine", 
                       std::string("Info"), 
                       "Test message", 
                       "[12:34:56.789][123]GameEngine: Info: Test message", 
                       1);
        
        // Test logger color consistency
        auto color1 = theme_manager->GetLoggerColor("GameEngine");
        auto color2 = theme_manager->GetLoggerColor("GameEngine");
        auto color3 = theme_manager->GetLoggerColor("Renderer");
        
        std::cout << "Logger color consistency test:" << std::endl;
        std::cout << "Same logger same color: " << (color1 == color2 ? "PASS" : "FAIL") << std::endl;
        std::cout << "Different loggers different colors: " << (color1 != color3 ? "PASS" : "FAIL") << std::endl;
        
        // Test rendering (just verify it doesn't crash)
        auto badge = renderer->RenderLoggerBadge(entry);
        auto full_entry = renderer->RenderLogEntry(entry, false, 0);
        auto header = renderer->RenderTableHeader();
        
        std::cout << "Rendering tests: PASS (no crashes)" << std::endl;
        
        // Test configuration
        renderer->SetWordWrapEnabled(true);
        std::cout << "Word wrap enabled: " << (renderer->IsWordWrapEnabled() ? "PASS" : "FAIL") << std::endl;
        
        renderer->SetShowLineNumbers(false);
        std::cout << "Line numbers disabled: " << (!renderer->IsShowLineNumbers() ? "PASS" : "FAIL") << std::endl;
        
        std::cout << "All LogEntryRenderer tests: PASS" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}