# Design Document

## Overview

This design document outlines the implementation approach for enhanced search and filtering functionality in the Unreal Log Viewer. The design focuses on providing intuitive keyboard shortcuts for column-based filtering, streamlined search-to-filter promotion, and clear visual feedback through numbered column headers.

## Architecture

### Component Integration
The enhanced search and filtering features will integrate with existing components:
- **MainWindow**: Event handling for new keyboard shortcuts
- **FilterEngine**: Filter creation and management
- **FilterPanel**: Display and management of created filters
- **Search System**: Enhanced search-to-filter promotion

### Key Design Principles
1. **Keyboard-First Interface**: All new features accessible via keyboard shortcuts
2. **Context-Aware Behavior**: Different behavior based on search state and selected entry
3. **Visual Clarity**: Clear numbering and feedback for user actions
4. **Non-Destructive**: New shortcuts don't interfere with existing functionality

## Components and Interfaces

### 1. Enhanced Column Header Rendering

**Purpose**: Display numbered column headers for quick reference

**Implementation**:
```cpp
// Update RenderTableHeader() method
ftxui::Element RenderTableHeader() const {
    std::vector<Element> header_elements;
    
    int column_index = 0;
    if (show_line_numbers_) {
        header_elements.push_back(text("[^" + std::to_string(column_index++) + "] Line") | bold);
    }
    
    header_elements.push_back(text("[^" + std::to_string(column_index++) + "] Timestamp") | bold);
    header_elements.push_back(text("[^" + std::to_string(column_index++) + "] Frame") | bold);
    header_elements.push_back(text("[^" + std::to_string(column_index++) + "] Logger") | bold);
    header_elements.push_back(text("[^" + std::to_string(column_index++) + "] Level") | bold);
    header_elements.push_back(text("[^" + std::to_string(column_index++) + "] Message") | bold | flex);
    
    return hbox(header_elements) | inverted;
}
```

### 2. Keyboard Event Handler Enhancement

**Purpose**: Handle new Ctrl+N keyboard shortcuts

**Implementation**:
```cpp
// In MainWindowComponent::OnEvent()
bool OnEvent(Event event) override {
    // Handle Ctrl+Number shortcuts
    if (event.is_character() && event.character().length() == 1) {
        char ch = event.character()[0];
        
        // Check for Ctrl+Number (ASCII values 1-6 for Ctrl+1 to Ctrl+6)
        if (ch >= 1 && ch <= 6) {
            int column_number = ch;
            
            if (parent_->IsSearchActive()) {
                // Search is active - create search-to-filter promotion
                parent_->PromoteSearchToColumnFilter(column_number);
            } else {
                // No search - create direct column filter
                parent_->CreateDirectColumnFilter(column_number);
            }
            return true;
        }
    }
    
    // ... existing event handling
}
```

### 3. Search-to-Filter Promotion System

**Purpose**: Convert active searches into column-specific filters

**New Methods**:
```cpp
class MainWindow {
    // Enhanced search promotion with column targeting
    void PromoteSearchToColumnFilter(int column_number);
    void CreateFilterFromSearchAndColumn(FilterConditionType type, const std::string& search_term);
    
    // Updated search promotion dialog
    void ShowEnhancedSearchPromotionDialog();
    ftxui::Element RenderEnhancedSearchPromotionDialog() const;
};
```

**Column Mapping**:
- Column 0: Any field contains (if search active) / Line number (if no search)
- Column 1: Timestamp after / Message contains
- Column 2: Frame after / Logger contains  
- Column 3: Logger equals / Level equals
- Column 4: Level equals
- Column 5: Message contains

### 4. Direct Column Filter Creation

**Purpose**: Create filters based on currently selected entry values

**New Methods**:
```cpp
class MainWindow {
    void CreateDirectColumnFilter(int column_number);
    void CreateTimestampAfterFilter(const LogEntry& entry);
    void CreateFrameAfterFilter(const LogEntry& entry);
    void CreateLoggerEqualsFilter(const LogEntry& entry);
    void CreateLevelEqualsFilter(const LogEntry& entry);
    void CreateMessageContainsFilter(const LogEntry& entry);
};
```

**Implementation Strategy**:
1. Check if an entry is selected
2. Extract the appropriate field value from the selected entry
3. Create the corresponding filter condition
4. Add filter to the filter engine
5. Apply filters and update display
6. Show status message

## Data Models

### Enhanced Filter Condition Types
Extend existing FilterConditionType enum if needed:
```cpp
enum class FilterConditionType {
    // Existing types...
    TimestampAfter,
    FrameAfter,
    LoggerEquals,
    LoggerContains,
    LogLevelEquals,
    MessageContains,
    AnyFieldContains
};
```

### Column Index Mapping
```cpp
enum class ColumnIndex {
    Line = 0,        // Only if line numbers enabled
    Timestamp = 1,   // Adjusted based on line number visibility
    Frame = 2,
    Logger = 3,
    Level = 4,
    Message = 5
};
```

## Error Handling

### Input Validation
1. **Selected Entry Validation**: Check if entry exists and has required field
2. **Search State Validation**: Verify search is active when needed
3. **Field Value Validation**: Ensure extracted values are valid for filter creation

### Error Messages
- "No entry selected for column filter"
- "Selected entry has no [field name] value"
- "Cannot create filter: invalid [field name] value"
- "Search must be active to promote to filter"

### Graceful Degradation
- If field is missing, show helpful error message
- If filter creation fails, maintain current state
- Preserve existing search/filter state on errors

## Testing Strategy

### Unit Tests
1. **Column Index Calculation**: Test correct mapping with/without line numbers
2. **Filter Creation**: Test each column filter type creation
3. **Search Promotion**: Test search-to-filter conversion for each column
4. **Error Handling**: Test error conditions and messages

### Integration Tests
1. **Keyboard Shortcuts**: Test all Ctrl+N combinations
2. **State Management**: Test behavior in different application states
3. **Filter Application**: Test that created filters work correctly
4. **UI Updates**: Test that status messages and displays update properly

### User Acceptance Tests
1. **Workflow Testing**: Test complete search-to-filter workflows
2. **Keyboard Navigation**: Test all keyboard shortcuts work as expected
3. **Visual Feedback**: Test that numbered headers and messages are clear
4. **Error Scenarios**: Test user experience with invalid operations

## Implementation Notes

### Performance Considerations
- Column index calculation should be cached or computed efficiently
- Filter creation should be fast to maintain responsive UI
- Status message updates should not block UI rendering

### Accessibility
- Keyboard shortcuts should be discoverable through help system
- Error messages should be clear and actionable
- Visual indicators should be distinguishable

### Extensibility
- Column mapping system should be easily extensible for new columns
- Filter types should be easily added without major refactoring
- Keyboard shortcut system should support future enhancements