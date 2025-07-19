#include "filter_panel.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>

namespace ue_log {

using namespace ftxui;

class FilterPanelComponent : public ComponentBase {
public:
    FilterPanelComponent(FilterPanel* parent) : parent_(parent) {}
    
    Element Render() override {
        return parent_->Render();
    }
    
    bool OnEvent(Event event) override {
        if (!parent_->IsFocused()) {
            return false;
        }
        
        // Handle Tab key to return focus to main window
        if (event == Event::Tab) {
            parent_->SetFocus(false);
            return true;
        }
        
        // Handle keyboard navigation
        if (event == Event::ArrowUp || event == Event::Character('k')) {
            parent_->NavigateUp();
            return true;
        }
        if (event == Event::ArrowDown || event == Event::Character('j')) {
            parent_->NavigateDown();
            return true;
        }
        if (event == Event::Character(' ')) {
            parent_->ToggleSelectedFilter();
            return true;
        }
        if (event == Event::Return || event == Event::Character('\n')) {
            parent_->ToggleExpansion();
            return true;
        }
        
        return false;
    }
    
private:
    FilterPanel* parent_;
};

FilterPanel::FilterPanel(FilterEngine* filter_engine, ConfigManager* config_manager)
    : filter_engine_(filter_engine), config_manager_(config_manager) {
}

void FilterPanel::Initialize() {
    // Refresh filters from engine if available
    if (filter_engine_) {
        RefreshFilters();
    }
    
    // Create FTXUI component
    component_ = Make<FilterPanelComponent>(this);
}

ftxui::Element FilterPanel::Render() const {
    if (!is_visible_) {
        return text("");
    }
    
    Element content = RenderFilterTree();
    Element controls = RenderFilterControls();
    
    Element window_element = window(text(GetTitle()), vbox({
        content | flex,
        separator(),
        controls
    }));
    
    // Add visual focus indicator
    if (is_focused_) {
        // Filter panel has focus - add bright border
        window_element = window_element | border;
    } else {
        // Filter panel doesn't have focus - add dim border
        window_element = window_element | border | dim;
    }
    
    return window_element;
}

ftxui::Component FilterPanel::CreateFTXUIComponent() {
    if (!component_) {
        Initialize();
    }
    return component_;
}

void FilterPanel::RefreshFilters() {
    if (!filter_engine_) {
        display_items_.clear();
        return;
    }
    
    BuildDisplayItems();
    
    // Update selection if it's out of bounds
    if (selected_filter_index_ >= static_cast<int>(display_items_.size())) {
        selected_filter_index_ = display_items_.empty() ? -1 : static_cast<int>(display_items_.size()) - 1;
    }
}

void FilterPanel::SetSelectedFilterIndex(int index) {
    if (display_items_.empty()) {
        selected_filter_index_ = -1;
        return;
    }
    
    // Clamp index to valid range
    if (index < 0) {
        selected_filter_index_ = 0;
    } else if (index >= static_cast<int>(display_items_.size())) {
        selected_filter_index_ = static_cast<int>(display_items_.size()) - 1;
    } else {
        selected_filter_index_ = index;
    }
}

const Filter* FilterPanel::GetSelectedFilter() const {
    if (selected_filter_index_ < 0 || selected_filter_index_ >= static_cast<int>(display_items_.size())) {
        return nullptr;
    }
    return display_items_[selected_filter_index_].filter;
}

void FilterPanel::ToggleSelectedFilter() {
    // Handle hierarchical filters (contextual filters)
    if (current_filter_expression_ && !current_filter_expression_->IsEmpty()) {
        const auto& conditions = current_filter_expression_->GetConditions();
        if (selected_filter_index_ >= 0 && selected_filter_index_ < static_cast<int>(conditions.size())) {
            // Toggle the selected condition
            auto* condition = conditions[selected_filter_index_].get();
            condition->Request_is_active_(!condition->Get_is_active_());
            
            // Trigger filter update
            if (filters_changed_callback_) {
                filters_changed_callback_();
            }
        }
        return;
    }
    
    // Handle traditional filters
    const Filter* filter = GetSelectedFilter();
    if (!filter || !filter_engine_) {
        return;
    }
    
    // Find the filter in the engine and toggle it
    const auto& filters = filter_engine_->Get_primary_filters();
    for (size_t i = 0; i < filters.size(); ++i) {
        if (filters[i].get() == filter) {
            // Toggle the filter's active state
            const_cast<Filter*>(filter)->Request_is_active(!filter->Get_is_active());
            RefreshFilters();
            if (filters_changed_callback_) {
                filters_changed_callback_();
            }
            break;
        }
    }
}

void FilterPanel::NavigateUp() {
    // Handle hierarchical filters (contextual filters)
    if (current_filter_expression_ && !current_filter_expression_->IsEmpty()) {
        const auto& conditions = current_filter_expression_->GetConditions();
        if (conditions.empty()) {
            return;
        }
        
        int new_index = selected_filter_index_ - 1;
        if (new_index < 0) {
            new_index = static_cast<int>(conditions.size()) - 1;
        }
        selected_filter_index_ = new_index;
        return;
    }
    
    // Handle traditional filters
    if (display_items_.empty()) {
        return;
    }
    
    int new_index = selected_filter_index_ - 1;
    if (new_index < 0) {
        new_index = static_cast<int>(display_items_.size()) - 1;
    }
    SetSelectedFilterIndex(new_index);
}

void FilterPanel::NavigateDown() {
    // Handle hierarchical filters (contextual filters)
    if (current_filter_expression_ && !current_filter_expression_->IsEmpty()) {
        const auto& conditions = current_filter_expression_->GetConditions();
        if (conditions.empty()) {
            return;
        }
        
        int new_index = selected_filter_index_ + 1;
        if (new_index >= static_cast<int>(conditions.size())) {
            new_index = 0;
        }
        selected_filter_index_ = new_index;
        return;
    }
    
    // Handle traditional filters
    if (display_items_.empty()) {
        return;
    }
    
    int new_index = selected_filter_index_ + 1;
    if (new_index >= static_cast<int>(display_items_.size())) {
        new_index = 0;
    }
    SetSelectedFilterIndex(new_index);
}

void FilterPanel::ToggleExpansion() {
    if (selected_filter_index_ < 0 || selected_filter_index_ >= static_cast<int>(display_items_.size())) {
        return;
    }
    
    // For now, this is a placeholder since we don't have hierarchical filters yet
    // In the future, this would expand/collapse filter groups
}

void FilterPanel::DeleteSelectedFilter() {
    // Handle hierarchical filters (contextual filters)
    if (current_filter_expression_ && !current_filter_expression_->IsEmpty()) {
        const auto& conditions = current_filter_expression_->GetConditions();
        if (selected_filter_index_ >= 0 && selected_filter_index_ < static_cast<int>(conditions.size())) {
            // Remove the selected condition
            const_cast<FilterExpression*>(current_filter_expression_)->RemoveCondition(selected_filter_index_);
            
            // Adjust selection if needed
            if (selected_filter_index_ >= static_cast<int>(current_filter_expression_->GetConditions().size())) {
                selected_filter_index_ = static_cast<int>(current_filter_expression_->GetConditions().size()) - 1;
            }
            
            // Trigger filter update
            if (filters_changed_callback_) {
                filters_changed_callback_();
            }
        }
        return;
    }
    
    // Handle traditional filters
    const Filter* filter = GetSelectedFilter();
    if (!filter || !filter_engine_) {
        return;
    }
    
    // Find and remove the filter from the engine
    const auto& filters = filter_engine_->Get_primary_filters();
    for (size_t i = 0; i < filters.size(); ++i) {
        if (filters[i].get() == filter) {
            filter_engine_->RemoveFilter(filter->Get_name());
            RefreshFilters();
            if (filters_changed_callback_) {
                filters_changed_callback_();
            }
            break;
        }
    }
}

void FilterPanel::BuildDisplayItems() {
    display_items_.clear();
    
    if (!filter_engine_) {
        return;
    }
    
    const auto& filters = filter_engine_->Get_primary_filters();
    for (const auto& filter : filters) {
        FilterDisplayItem item;
        item.filter = filter.get();
        item.depth = 0;
        item.is_expanded = true;
        item.is_last_child = true;
        item.display_text = filter->Get_name();
        display_items_.push_back(item);
    }
}

void FilterPanel::BuildDisplayItemsRecursive(const Filter* filter, int depth, bool is_last, 
                                           const std::string& parent_prefix) {
    // Placeholder for future hierarchical filter support
    // For now, all filters are at depth 0
}

ftxui::Element FilterPanel::RenderFilterTree() const {
    // Show hierarchical filter expression if it has conditions (contextual filters)
    if (current_filter_expression_ && !current_filter_expression_->IsEmpty()) {
        std::vector<Element> rows;
        
        // Show the current filter expression
        rows.push_back(text("Current Filter:") | bold);
        
        // Show each condition on a separate line with checkboxes and selection
        const auto& conditions = current_filter_expression_->GetConditions();
        for (size_t i = 0; i < conditions.size(); ++i) {
            // Create checkbox indicator
            std::string checkbox = conditions[i]->Get_is_active_() ? "[✓]" : "[ ]";
            
            // Create condition text
            std::string condition_text = checkbox + " " + std::to_string(i + 1) + ". " + conditions[i]->ToString();
            
            // Create the row element
            Element row = text(condition_text);
            
            // Apply selection highlighting
            bool is_selected = (static_cast<int>(i) == selected_filter_index_);
            if (is_selected) {
                row = row | inverted;
            }
            
            // Apply color based on active state
            if (conditions[i]->Get_is_active_()) {
                row = row | color(Color::Green);
            } else {
                row = row | dim;
            }
            
            rows.push_back(row);
        }
        
        if (conditions.empty()) {
            rows.push_back(text("No conditions") | dim);
        }
        
        rows.push_back(separator());
        
        // Show filter statistics
        size_t total_conditions = current_filter_expression_->GetTotalConditions();
        std::string stats = "Total Conditions: " + std::to_string(total_conditions);
        rows.push_back(text(stats) | dim);
        
        // Show operator
        std::string op = (current_filter_expression_->Get_operator_() == FilterOperator::And) ? "AND" : "OR";
        rows.push_back(text("Logic: " + op) | dim);
        
        return vbox(rows);
    }
    
    // Fall back to traditional filters if no hierarchical filters
    if (!display_items_.empty()) {
        std::vector<Element> rows;
        
        for (size_t i = 0; i < display_items_.size(); ++i) {
            bool is_selected = (static_cast<int>(i) == selected_filter_index_);
            Element row = RenderFilterItem(display_items_[i], is_selected);
            rows.push_back(row);
        }
        
        return vbox(rows);
    }
    
    // Show empty state if no filters at all
    return vbox({
        text("No filters defined") | center,
        text("Press 'C' on a log entry to create contextual filters") | center | dim,
        text("") | flex
    });
}

ftxui::Element FilterPanel::RenderFilterItem(const FilterDisplayItem& item, bool is_selected) const {
    const Filter* filter = item.filter;
    if (!filter) {
        return text("Invalid filter");
    }
    
    // Create the filter status indicator
    std::string status = filter->Get_is_active() ? "[✓]" : "[ ]";
    
    // Create the filter type indicator
    std::string type_str = GetFilterTypeString(filter->Get_type());
    
    // Create the main content
    Element content = hbox({
        text(status) | size(WIDTH, EQUAL, 4),
        text(type_str) | size(WIDTH, EQUAL, 12),
        text(filter->Get_name()) | size(WIDTH, EQUAL, 20),
        text(filter->Get_criteria()) | flex
    });
    
    // Apply styling based on state
    if (is_selected) {
        content = content | inverted;
    }
    
    if (!filter->Get_is_active()) {
        content = content | dim;
    }
    
    return content;
}

ftxui::Element FilterPanel::RenderFilterControls() const {
    return hbox({
        text("[↑↓] Navigate") | size(WIDTH, EQUAL, 15),
        text("[Space] Toggle") | size(WIDTH, EQUAL, 15),
        text("[Enter] Expand") | flex
    }) | dim;
}

std::string FilterPanel::GetFilterTypeString(FilterType type) const {
    switch (type) {
        case FilterType::TextContains: return "Text";
        case FilterType::TextExact: return "Exact";
        case FilterType::TextRegex: return "Regex";
        case FilterType::LoggerName: return "Logger";
        case FilterType::LogLevel: return "Level";
        case FilterType::TimeRange: return "Time";
        case FilterType::FrameRange: return "Frame";
        default: return "Unknown";
    }
}

std::string FilterPanel::GetTreePrefix(int depth, bool is_last, bool has_children, bool is_expanded) const {
    // Placeholder for future hierarchical display
    return "";
}

ftxui::Color FilterPanel::GetFilterColor(const Filter* filter) const {
    if (!filter) {
        return Color::Default;
    }
    
    if (!filter->Get_is_active()) {
        return Color::GrayDark;
    }
    
    switch (filter->Get_type()) {
        case FilterType::TextContains:
        case FilterType::TextExact:
        case FilterType::TextRegex:
            return Color::Blue;
        case FilterType::LoggerName:
            return Color::Green;
        case FilterType::LogLevel:
            return Color::Yellow;
        case FilterType::TimeRange:
        case FilterType::FrameRange:
            return Color::Magenta;
        default:
            return Color::Default;
    }
}

} // namespace ue_log