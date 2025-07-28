#include "lib/filter_engine/filter_engine.h"
#include "lib/log_parser/log_entry.h"
#include <iostream>

using namespace ue_log;

int main() {
    // Create test log entries
    LogEntry entry1;
    entry1.Request_message("This is an error message");
    entry1.Request_logger_name("TestLogger");
    
    LogEntry entry2;
    entry2.Request_message("This is a warning message");
    entry2.Request_logger_name("TestLogger");
    
    LogEntry entry3;
    entry3.Request_message("This is an info message");
    entry3.Request_logger_name("TestLogger");
    
    // Create filter engine
    FilterEngine engine;
    
    // Create an exclude filter for "error"
    auto filter = std::make_unique<Filter>("ErrorFilter", FilterType::TextContains, "error");
    filter->SetFilterState(FilterState::EXCLUDE);
    
    std::cout << "Filter state: " << static_cast<int>(filter->GetFilterState()) << std::endl;
    std::cout << "Filter should exclude entry1: " << filter->ShouldExclude(entry1) << std::endl;
    std::cout << "Filter should exclude entry2: " << filter->ShouldExclude(entry2) << std::endl;
    std::cout << "Filter should exclude entry3: " << filter->ShouldExclude(entry3) << std::endl;
    
    engine.AddFilter(std::move(filter));
    
    // Test PassesFilters
    std::cout << "\nPassesFilters results:" << std::endl;
    std::cout << "Entry1 (error): " << engine.PassesFilters(entry1) << std::endl;
    std::cout << "Entry2 (warning): " << engine.PassesFilters(entry2) << std::endl;
    std::cout << "Entry3 (info): " << engine.PassesFilters(entry3) << std::endl;
    
    // Test ApplyFilters
    std::vector<LogEntry> entries = {entry1, entry2, entry3};
    auto filtered = engine.ApplyFilters(entries);
    
    std::cout << "\nApplyFilters results:" << std::endl;
    std::cout << "Original entries: " << entries.size() << std::endl;
    std::cout << "Filtered entries: " << filtered.size() << std::endl;
    
    for (const auto& entry : filtered) {
        std::cout << "  - " << entry.Get_message() << std::endl;
    }
    
    return 0;
}