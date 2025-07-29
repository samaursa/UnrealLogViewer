#include "lib/ui/log_entry_renderer.h"
#include "lib/ui/visual_theme_manager.h"
#include "lib/log_parser/log_entry.h"
#include <iostream>

int main() {
    std::cout << "Testing highlighting bug fix..." << std::endl;
    
    // Create a visual theme manager and log entry renderer
    ue_log::VisualThemeManager theme_manager;
    ue_log::LogEntryRenderer renderer(&theme_manager);
    
    // Create a test log entry with the problematic text
    ue_log::LogEntry test_entry;
    test_entry.Request_message("Property ArrayProperty FCk_Ability_OtherAbilitySettings");
    test_entry.Request_timestamp("2024-01-01 12:00:00");
    test_entry.Request_logger_name("TestLogger");
    test_entry.Request_log_level("Info");
    test_entry.Request_line_number(1);
    
    std::cout << "Original message: '" << test_entry.Get_message() << "'" << std::endl;
    
    // Test case-insensitive search for "Ability"
    std::string search_query = "Ability";
    bool case_sensitive = false;
    
    std::cout << "Search query: '" << search_query << "'" << std::endl;
    std::cout << "Case sensitive: " << (case_sensitive ? "yes" : "no") << std::endl;
    
    // Test the highlighting method
    auto highlighted_element = renderer.RenderMessageWithSearchHighlight(test_entry, false, search_query, case_sensitive);
    
    std::cout << "Highlighting test completed - check visual output in actual application" << std::endl;
    
    // Test another case with exact case match
    search_query = "Property";
    case_sensitive = true;
    
    std::cout << "\nTesting exact case match:" << std::endl;
    std::cout << "Search query: '" << search_query << "'" << std::endl;
    std::cout << "Case sensitive: " << (case_sensitive ? "yes" : "no") << std::endl;
    
    auto highlighted_element2 = renderer.RenderMessageWithSearchHighlight(test_entry, false, search_query, case_sensitive);
    
    std::cout << "Second highlighting test completed" << std::endl;
    
    return 0;
}