#pragma once

#include "component.h"
#include "../filter_engine/filter_engine.h"
#include "../filter_engine/filter_expression.h"
#include "../config/config_manager.h"
#include "../../macros.h"
#include <memory>
#include <string>
#include <functional>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace ue_log {

/**
 * Component for managing filters.
 * Provides hierarchical filter tree view with toggle controls and navigation.
 */
class FilterPanel : public Component {
    CK_GENERATED_BODY(FilterPanel);

public:
    /**
     * Constructor.
     * @param filter_engine Pointer to the filter engine.
     * @param config_manager Pointer to the configuration manager.
     */
    FilterPanel(FilterEngine* filter_engine = nullptr, ConfigManager* config_manager = nullptr);
    
    /**
     * Destructor.
     */
    ~FilterPanel() override = default;
    
    /**
     * Initialize the component.
     */
    void Initialize() override;
    
    /**
     * Render the component to an FTXUI Element.
     * @return FTXUI Element representing this component.
     */
    ftxui::Element Render() const override;
    
    /**
     * Create an FTXUI Component for this component.
     * @return FTXUI Component that can be used in the FTXUI framework.
     */
    ftxui::Component CreateFTXUIComponent() override;
    
    /**
     * Get the title of this component.
     * @return Title string.
     */
    std::string GetTitle() const override { return "Filters"; }
    
    /**
     * Check if the component is focused.
     * @return True if the component is focused, false otherwise.
     */
    bool IsFocused() const override { return is_focused_; }
    
    /**
     * Set the focus state of the component.
     * @param focused True to focus the component, false to unfocus.
     */
    void SetFocus(bool focused) override { is_focused_ = focused; }
    
    /**
     * Set the filter engine to use.
     * @param filter_engine Pointer to the filter engine.
     */
    void SetFilterEngine(FilterEngine* filter_engine) { filter_engine_ = filter_engine; }
    
    /**
     * Set the current filter expression to display.
     * @param expression Pointer to the filter expression.
     */
    void SetCurrentFilterExpression(const FilterExpression* expression) { current_filter_expression_ = expression; }
    
    /**
     * Refresh the filter list from the filter engine.
     */
    void RefreshFilters();
    
    /**
     * Get the currently selected filter index.
     * @return Index of the selected filter, or -1 if no filter is selected.
     */
    int GetSelectedFilterIndex() const { return selected_filter_index_; }
    
    /**
     * Set the selected filter index.
     * @param index Index of the filter to select.
     */
    void SetSelectedFilterIndex(int index);
    
    /**
     * Get the currently selected filter.
     * @return Pointer to the selected filter, or nullptr if no filter is selected.
     */
    const Filter* GetSelectedFilter() const;
    
    /**
     * Toggle the selected filter on/off.
     */
    void ToggleSelectedFilter();
    
    /**
     * Navigate up in the filter list.
     */
    void NavigateUp();
    
    /**
     * Navigate down in the filter list.
     */
    void NavigateDown();
    /**
     * Expand/collapse the selected filter (if it has sub-filters).
     */
    void ToggleExpansion();
    
    /**
     * Delete the selected filter.
     */
    void DeleteSelectedFilter();
    
    /**
     * Set the callback to be called when filters are changed.
     * @param callback Function to call when filters are changed.
     */
    void SetFiltersChangedCallback(std::function<void()> callback) {
        filters_changed_callback_ = std::move(callback);
    }
    
    /**
     * Check if the filter panel is visible.
     * @return True if visible, false otherwise.
     */
    bool IsVisible() const { return is_visible_; }
    
    /**
     * Set the visibility of the filter panel.
     * @param visible True to show, false to hide.
     */
    void SetVisible(bool visible) { is_visible_ = visible; }

private:
    // FTXUI component
    ftxui::Component component_;
    
    // Dependencies
    FilterEngine* filter_engine_ = nullptr;
    ConfigManager* config_manager_ = nullptr;
    const FilterExpression* current_filter_expression_ = nullptr;
    
    // State
    bool is_focused_ = false;
    bool is_visible_ = true;
    int selected_filter_index_ = -1;
    
    // Filter display state
    struct FilterDisplayItem {
        const Filter* filter;
        int depth;
        bool is_expanded;
        bool is_last_child;
        std::string display_text;
    };
    
    std::vector<FilterDisplayItem> display_items_;
    std::vector<bool> expansion_state_;
    
    // Callbacks
    std::function<void()> filters_changed_callback_;
    
    // Helper methods
    void BuildDisplayItems();
    void BuildDisplayItemsRecursive(const Filter* filter, int depth, bool is_last, 
                                   const std::string& parent_prefix);
    ftxui::Element RenderFilterTree() const;
    ftxui::Element RenderFilterItem(const FilterDisplayItem& item, bool is_selected) const;
    ftxui::Element RenderFilterControls() const;
    std::string GetFilterTypeString(FilterType type) const;
    std::string GetTreePrefix(int depth, bool is_last, bool has_children, bool is_expanded) const;
    ftxui::Color GetFilterColor(const Filter* filter) const;
};

} // namespace ue_log