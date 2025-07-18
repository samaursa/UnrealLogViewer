#pragma once

#include "component.h"
#include "../filter_engine/filter_expression.h"
#include "../config/config_manager.h"
#include "../../macros.h"
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace ue_log {

/**
 * UI component for building hierarchical filter expressions
 */
class FilterBuilder : public Component {
    CK_GENERATED_BODY(FilterBuilder);

public:
    /**
     * Constructor.
     * @param config_manager Pointer to the configuration manager.
     */
    explicit FilterBuilder(ConfigManager* config_manager = nullptr);
    
    /**
     * Destructor.
     */
    ~FilterBuilder() override = default;
    
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
    std::string GetTitle() const override { return "Filter Builder"; }
    
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
     * Check if the filter builder is visible.
     * @return True if visible, false otherwise.
     */
    bool IsVisible() const { return is_visible_; }
    
    /**
     * Set the visibility of the filter builder.
     * @param visible True to show, false to hide.
     */
    void SetVisible(bool visible) { is_visible_ = visible; }
    
    /**
     * Get the current filter expression.
     * @return Reference to the current filter expression.
     */
    const FilterExpression& GetFilterExpression() const { return *filter_expression_; }
    
    /**
     * Set the filter expression to edit.
     * @param expression The filter expression to edit.
     */
    void SetFilterExpression(std::unique_ptr<FilterExpression> expression);
    
    /**
     * Clear the current filter expression.
     */
    void ClearFilter();
    
    /**
     * Add a condition from a quick search.
     * @param search_text The search text to convert to a condition.
     */
    void AddConditionFromSearch(const std::string& search_text);
    
    /**
     * Add contextual conditions from a log entry.
     * @param entry The log entry to create conditions from.
     */
    void AddContextualConditions(const LogEntry& entry);
    
    /**
     * Set the callback to be called when the filter is applied.
     * @param callback Function to call when filter is applied.
     */
    void SetFilterAppliedCallback(std::function<void(const FilterExpression&)> callback) {
        filter_applied_callback_ = std::move(callback);
    }
    
    /**
     * Set the callback to be called when the filter builder is closed.
     * @param callback Function to call when builder is closed.
     */
    void SetClosedCallback(std::function<void()> callback) {
        closed_callback_ = std::move(callback);
    }

    // Navigation and editing
    void NavigateUp();
    void NavigateDown();
    void AddCondition();
    void EditSelectedItem();
    void DeleteSelectedItem();
    void ToggleSelectedItem();
    void GroupSelectedItems();
    void ToggleOperator();
    void ApplyFilter();
    void Cancel();

private:
    // FTXUI component
    ftxui::Component component_;
    
    // Dependencies
    ConfigManager* config_manager_ = nullptr;
    
    // State
    bool is_focused_ = false;
    bool is_visible_ = false;
    int selected_item_index_ = -1;
    
    // Filter expression being edited
    std::unique_ptr<FilterExpression> filter_expression_;
    
    // UI state for condition editing
    bool show_condition_dialog_ = false;
    bool show_promotion_dialog_ = false;
    std::string promotion_search_text_;
    
    // Display items for the expression tree
    struct ExpressionTreeItem {
        enum Type { Condition, Expression, Operator };
        Type type;
        void* data; // Points to FilterCondition* or FilterExpression*
        int depth;
        bool is_last_child;
        std::string display_text;
    };
    
    std::vector<ExpressionTreeItem> tree_items_;
    
    // Callbacks
    std::function<void(const FilterExpression&)> filter_applied_callback_;
    std::function<void()> closed_callback_;
    
    // Helper methods
    void BuildTreeItems();
    void BuildTreeItemsRecursive(const FilterExpression* expr, int depth, bool is_last);
    ftxui::Element RenderExpressionTree() const;
    ftxui::Element RenderTreeItem(const ExpressionTreeItem& item, bool is_selected) const;
    ftxui::Element RenderCurrentExpression() const;
    ftxui::Element RenderControls() const;
    ftxui::Element RenderConditionDialog() const;
    ftxui::Element RenderPromotionDialog() const;
    
    std::string GetTreePrefix(int depth, bool is_last) const;
    void UpdateTreeItems();
};

/**
 * Dialog for creating/editing filter conditions
 */
class ConditionDialog {
public:
    struct ConditionData {
        FilterConditionType type = FilterConditionType::MessageContains;
        std::string value;
        bool is_valid = false;
    };
    
    static ftxui::Component CreateDialog(ConditionData& data, std::function<void()> on_save, std::function<void()> on_cancel);
    static std::vector<std::string> GetFieldNames();
    static std::vector<std::string> GetOperatorNames(FilterConditionType type);
    static FilterConditionType GetConditionTypeFromStrings(const std::string& field, const std::string& operator_name);
};

} // namespace ue_log