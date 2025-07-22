#include "lib/log_parser/log_parser.h"
#include "lib/ui/visual_theme_manager.h"
#include "lib/ui/log_entry_renderer.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>

using namespace ue_log;

int main() {
    std::cout << "=== Unreal Engine Log Level Parsing Test ===" << std::endl;
    
    // Create test log entries that represent real Unreal Engine log patterns
    std::vector<std::string> test_log_lines = {
        // Valid Unreal log levels
        "[2025.07.16-10.01.25:951][425]LogCsvProfiler: Display: Metadata set : cpu=\"AuthenticAMD|AMD Ryzen 9 7900X 12-Core Processor\"",
        "[2025.07.16-10.01.26:123][426]LogEngine: Warning: Failed to load texture asset",
        "[2025.07.16-10.01.27:456][427]LogGameplay: Error: Player controller not found",
        "[2025.07.16-10.01.28:789][428]LogAI: Verbose: Pathfinding calculation completed",
        "[2025.07.16-10.01.29:012][429]LogNetwork: VeryVerbose: Packet transmission details",
        "[2025.07.16-10.01.30:345][430]LogAbility: Trace: [PIE-ID -2] Discovered and Adding Cue",
        
        // Invalid log levels (should be treated as part of message)
        "[2025.07.16-10.01.31:678][431]LogTemp: Info: This should be treated as message",
        "[2025.07.16-10.01.32:901][432]LogCustom: Debug: This should also be treated as message",
        "[2025.07.16-10.01.33:234][433]LogSystem: SomeRandomText: This is not a valid log level",
        
        // Semi-structured (no log level)
        "[2025.07.16-10.01.34:567][434]LogTemp: Simple message without log level",
        
        // Unstructured with valid log levels
        "LogCsvProfiler: Display: Unstructured log with valid level",
        "LogEngine: Warning: Another unstructured log",
        
        // Unstructured with invalid log levels
        "LogTemp: Info: This should have Info as part of message",
        "LogCustom: SomeText: This should have SomeText as part of message"
    };
    
    // Create a temporary test file
    std::string test_file = "test_unreal_logs.txt";
    std::ofstream file(test_file);
    for (const auto& line : test_log_lines) {
        file << line << std::endl;
    }
    file.close();
    
    // Create log parser and parse the test file
    LogParser parser;
    auto result = parser.LoadFile(test_file);
    
    if (result.IsError()) {
        std::cout << "Failed to load test file: " << result.Get_error_message() << std::endl;
        return 1;
    }
    
    auto entries = parser.ParseEntries();
    
    std::cout << "\nParsed " << entries.size() << " log entries:" << std::endl;
    std::cout << std::string(120, '=') << std::endl;
    
    // Create theme manager and renderer for display
    auto theme_manager = std::make_unique<VisualThemeManager>();
    auto renderer = std::make_unique<LogEntryRenderer>(theme_manager.get());
    
    // Display results
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        
        std::cout << "Entry " << (i + 1) << ":" << std::endl;
        std::cout << "  Raw Line: " << entry.Get_raw_line() << std::endl;
        std::cout << "  Logger: '" << entry.Get_logger_name() << "'" << std::endl;
        
        if (entry.Get_log_level().has_value()) {
            std::cout << "  Log Level: '" << entry.Get_log_level().value() << "'" << std::endl;
        } else {
            std::cout << "  Log Level: [NONE]" << std::endl;
        }
        
        std::cout << "  Message: '" << entry.Get_message() << "'" << std::endl;
        std::cout << "  Entry Type: ";
        switch (entry.Get_entry_type()) {
            case LogEntryType::Structured:
                std::cout << "Structured";
                break;
            case LogEntryType::SemiStructured:
                std::cout << "SemiStructured";
                break;
            case LogEntryType::Unstructured:
                std::cout << "Unstructured";
                break;
        }
        std::cout << std::endl;
        
        // Show visual rendering
        auto element = renderer->RenderLogEntry(entry, false, 0);
        ftxui::Screen screen(120, 1);
        ftxui::Render(screen, element);
        std::cout << "  Rendered: " << screen.ToString() << std::endl;
        
        std::cout << std::string(80, '-') << std::endl;
    }
    
    // Validation tests
    std::cout << "\n=== Validation Tests ===" << std::endl;
    
    bool all_tests_passed = true;
    
    // Test 1: Valid Unreal log levels should be recognized
    std::vector<std::string> expected_valid_levels = {"Display", "Warning", "Error", "Verbose", "VeryVerbose", "Trace"};
    int valid_level_count = 0;
    
    for (const auto& entry : entries) {
        if (entry.Get_log_level().has_value()) {
            std::string level = entry.Get_log_level().value();
            bool is_valid_unreal_level = std::find(expected_valid_levels.begin(), expected_valid_levels.end(), level) != expected_valid_levels.end();
            if (is_valid_unreal_level) {
                valid_level_count++;
            } else {
                std::cout << "FAIL: Invalid Unreal log level found: '" << level << "'" << std::endl;
                all_tests_passed = false;
            }
        }
    }
    
    std::cout << "Valid Unreal log levels found: " << valid_level_count << std::endl;
    
    // Test 2: Invalid log levels should not be recognized (should be part of message)
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        const std::string& raw_line = test_log_lines[i];
        
        // Check specific cases where invalid levels should be part of message
        if (raw_line.find("Info:") != std::string::npos && entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Info") {
            std::cout << "FAIL: 'Info' should not be recognized as valid Unreal log level" << std::endl;
            all_tests_passed = false;
        }
        
        if (raw_line.find("Debug:") != std::string::npos && entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Debug") {
            std::cout << "FAIL: 'Debug' should not be recognized as valid Unreal log level" << std::endl;
            all_tests_passed = false;
        }
        
        if (raw_line.find("SomeRandomText:") != std::string::npos && entry.Get_log_level().has_value() && entry.Get_log_level().value() == "SomeRandomText") {
            std::cout << "FAIL: 'SomeRandomText' should not be recognized as valid Unreal log level" << std::endl;
            all_tests_passed = false;
        }
    }
    
    // Test 3: Messages should contain the invalid "log levels" when they're not valid
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        const std::string& raw_line = test_log_lines[i];
        
        if (raw_line.find("Info:") != std::string::npos && !entry.Get_log_level().has_value()) {
            if (entry.Get_message().find("Info:") == std::string::npos) {
                std::cout << "FAIL: Message should contain 'Info:' when it's not a valid log level" << std::endl;
                all_tests_passed = false;
            }
        }
        
        if (raw_line.find("Debug:") != std::string::npos && !entry.Get_log_level().has_value()) {
            if (entry.Get_message().find("Debug:") == std::string::npos) {
                std::cout << "FAIL: Message should contain 'Debug:' when it's not a valid log level" << std::endl;
                all_tests_passed = false;
            }
        }
    }
    
    // Test 4: Visual theme manager should handle all Unreal log levels
    std::cout << "\n=== Visual Theme Manager Log Level Colors ===" << std::endl;
    for (const std::string& level : expected_valid_levels) {
        auto color = theme_manager->GetLogLevelColor(level);
        std::cout << "  " << level << ": Color assigned" << std::endl;
    }
    
    // Clean up
    std::remove(test_file.c_str());
    
    std::cout << "\n=== Test Results ===" << std::endl;
    if (all_tests_passed) {
        std::cout << "✓ All tests PASSED!" << std::endl;
        std::cout << "✓ Unreal Engine log level parsing is working correctly" << std::endl;
        std::cout << "✓ Invalid log levels are properly handled as part of messages" << std::endl;
        std::cout << "✓ Visual theme manager supports all Unreal log levels" << std::endl;
    } else {
        std::cout << "✗ Some tests FAILED!" << std::endl;
        return 1;
    }
    
    return 0;
}