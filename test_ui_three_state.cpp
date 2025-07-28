#include "lib/ui/filter_panel.h"
#include "lib/filter_engine/filter_engine.h"
#include "lib/filter_engine/filter.h"
#include "lib/config/config_manager.h"
#include <iostream>
#include <memory>

using namespace ue_log;

int main() {
    std::cout << "Testing FilterPanel UI three-state functionality...\n";
    
    // Create a filter engine and add some test filters
    FilterEngine filter_engine;
    
    // Create test filters
    auto filter1 = std::make_unique<Filter>("TestFilter1", FilterType::TextContains, "Error");
    auto filter2 = std::make_unique<Filter>("TestFilter2", FilterType::LoggerName, "LogTemp");
    auto filter3 = std::make_unique<Filter>("TestFilter3", FilterType::LogLevel, "Warning");
    
    // Set different states
    filter1->SetFilterState(FilterState::INCLUDE);
    filter2->SetFilterState(FilterState::EXCLUDE);
    filter3->SetFilterState(FilterState::DISABLED);
    
    // Add filters to engine
    filter_engine.AddFilter(std::move(filter1));
    filter_engine.AddFilter(std::move(filter2));
    filter_engine.AddFilter(std::move(filter3));
    
    // Create filter panel
    ConfigManager config_manager;
    FilterPanel filter_panel(&filter_engine, &config_manager);
    filter_panel.Initialize();
    filter_panel.RefreshFilters();
    
    // Test that we can get the filters and their states
    const auto& filters = filter_engine.Get_primary_filters();
    
    std::cout << "Filter states:\n";
    for (size_t i = 0; i < filters.size(); ++i) {
        const auto& filter = filters[i];
        std::string state_name;
        switch (filter->GetFilterState()) {
            case FilterState::INCLUDE: state_name = "INCLUDE"; break;
            case FilterState::EXCLUDE: state_name = "EXCLUDE"; break;
            case FilterState::DISABLED: state_name = "DISABLED"; break;
        }
        std::cout << "  " << filter->Get_name() << ": " << state_name << "\n";
    }
    
    // Test cycling through states
    std::cout << "\nTesting state cycling for TestFilter1:\n";
    auto* test_filter = const_cast<Filter*>(filters[0].get());
    
    for (int i = 0; i < 4; ++i) {
        std::string state_name;
        switch (test_filter->GetFilterState()) {
            case FilterState::INCLUDE: state_name = "INCLUDE"; break;
            case FilterState::EXCLUDE: state_name = "EXCLUDE"; break;
            case FilterState::DISABLED: state_name = "DISABLED"; break;
        }
        std::cout << "  Cycle " << i << ": " << state_name << "\n";
        test_filter->CycleFilterState();
    }
    
    // Test the UI functionality by simulating toggle operations
    std::cout << "\nTesting UI toggle functionality:\n";
    filter_panel.SetSelectedFilterIndex(0);
    
    // Simulate pressing space to toggle the filter
    std::cout << "  Before toggle: " << (test_filter->GetFilterState() == FilterState::INCLUDE ? "INCLUDE" : 
                                        test_filter->GetFilterState() == FilterState::EXCLUDE ? "EXCLUDE" : "DISABLED") << "\n";
    
    filter_panel.ToggleSelectedFilter();
    
    std::cout << "  After toggle: " << (test_filter->GetFilterState() == FilterState::INCLUDE ? "INCLUDE" : 
                                       test_filter->GetFilterState() == FilterState::EXCLUDE ? "EXCLUDE" : "DISABLED") << "\n";
    
    std::cout << "\nAll UI tests completed successfully!\n";
    return 0;
}