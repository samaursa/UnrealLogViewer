#include "lib/filter_engine/filter.h"
#include "lib/log_parser/log_entry.h"
#include <iostream>
#include <cassert>

using namespace ue_log;

int main() {
    std::cout << "Testing three-state filter functionality...\n";
    
    // Create a test log entry
    LogEntry entry;
    entry.Request_message("Test message 1");
    entry.Request_logger_name("LogTemp");
    entry.Request_log_level("Display");
    
    // Create a filter
    Filter filter("TestFilter", FilterType::TextContains, "Test");
    
    // Test initial state (should be INCLUDE)
    assert(filter.GetFilterState() == FilterState::INCLUDE);
    assert(filter.IsActive() == true);
    assert(filter.ShouldInclude(entry) == true);
    assert(filter.ShouldExclude(entry) == false);
    std::cout << "✓ Initial state: INCLUDE\n";
    
    // Cycle to EXCLUDE state
    filter.CycleFilterState();
    assert(filter.GetFilterState() == FilterState::EXCLUDE);
    assert(filter.IsActive() == true);
    assert(filter.ShouldInclude(entry) == false);
    assert(filter.ShouldExclude(entry) == true);
    std::cout << "✓ Cycled to: EXCLUDE\n";
    
    // Cycle to DISABLED state
    filter.CycleFilterState();
    assert(filter.GetFilterState() == FilterState::DISABLED);
    assert(filter.IsActive() == false);
    assert(filter.ShouldInclude(entry) == false);
    assert(filter.ShouldExclude(entry) == false);
    std::cout << "✓ Cycled to: DISABLED\n";
    
    // Cycle back to INCLUDE state
    filter.CycleFilterState();
    assert(filter.GetFilterState() == FilterState::INCLUDE);
    assert(filter.IsActive() == true);
    assert(filter.ShouldInclude(entry) == true);
    assert(filter.ShouldExclude(entry) == false);
    std::cout << "✓ Cycled back to: INCLUDE\n";
    
    // Test backward compatibility
    filter.SetActive(false);
    assert(filter.GetFilterState() == FilterState::DISABLED);
    assert(filter.IsActive() == false);
    std::cout << "✓ Backward compatibility: SetActive(false)\n";
    
    filter.SetActive(true);
    assert(filter.GetFilterState() == FilterState::INCLUDE);
    assert(filter.IsActive() == true);
    std::cout << "✓ Backward compatibility: SetActive(true)\n";
    
    // Test direct state setting
    filter.SetFilterState(FilterState::EXCLUDE);
    assert(filter.GetFilterState() == FilterState::EXCLUDE);
    assert(filter.IsActive() == true);
    std::cout << "✓ Direct state setting: EXCLUDE\n";
    
    std::cout << "\nAll tests passed! Three-state filter functionality is working correctly.\n";
    return 0;
}