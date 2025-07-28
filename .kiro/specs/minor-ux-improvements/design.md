# Design Document

## Overview

This design document outlines the implementation approach for two minor but impactful user experience improvements to the Unreal Log Viewer:

1. **Enhanced Unreal Project Detection**: Automatically detect and use `Saved/Logs` directories when launching with folder arguments
2. **Three-State Filter System**: Extend the current binary filter system to support include, exclude, and disabled states

Both improvements build upon existing functionality and require minimal architectural changes while providing significant usability enhancements.

## Architecture

### Enhanced Unreal Project Detection

The current system already has robust Unreal project detection in `unreal_project_utils.cpp`, but it only applies when no path is provided. The enhancement will extend this logic to also apply when a folder path is explicitly provided as an argument.

**Current Flow:**
```
User provides folder → Use folder directly → Scan for .log files
```

**Enhanced Flow:**
```
User provides folder → Check for Saved/Logs subfolder → Use Saved/Logs if found, otherwise use original folder → Scan for .log files
```

### Three-State Filter System

The current filter system uses a simple boolean `is_active` field. The enhancement will extend this to support three distinct states while maintaining backward compatibility.

**Current State:**
- `bool is_active` (true/false)

**Enhanced State:**
- `enum class FilterState { INCLUDE, EXCLUDE, DISABLED }`

## Components and Interfaces

### 1. Enhanced Unreal Project Detection

#### Modified Components:
- **`app/main.cpp`**: Update command line argument processing logic
- **`lib/common/unreal_project_utils.h/cpp`**: Add new utility function for folder argument processing

#### New Interface:
```cpp
namespace ue_log::unreal_utils {
    // New function to handle folder arguments with Unreal project detection
    std::pair<std::string, std::string> ProcessFolderArgument(const std::string& folder_path);
}
```

### 2. Three-State Filter System

#### Modified Components:
- **`lib/filter_engine/filter.h/cpp`**: Add FilterState enum and update filter logic
- **`lib/ui/filter_panel.h/cpp`**: Update UI rendering and interaction handling
- **`lib/filter_engine/filter_engine.cpp`**: Update filter application logic

#### New Interfaces:
```cpp
enum class FilterState {
    INCLUDE,    // Show only entries that match (green checkmark)
    EXCLUDE,    // Hide entries that match (red negative sign)
    DISABLED    // Ignore filter completely (greyed out)
};

class Filter {
    // Replace is_active with filter_state
    FilterState filter_state = FilterState::INCLUDE;
    
    // New methods
    FilterState GetFilterState() const;
    void SetFilterState(FilterState state);
    void CycleFilterState(); // INCLUDE → EXCLUDE → DISABLED → INCLUDE
    
    // Updated matching logic
    bool ShouldInclude(const LogEntry& entry) const;
    bool ShouldExclude(const LogEntry& entry) const;
};
```

## Data Models

### FilterState Enum
```cpp
enum class FilterState {
    INCLUDE = 0,    // Default state - include matching entries
    EXCLUDE = 1,    // Exclude matching entries  
    DISABLED = 2    // Filter is disabled
};
```

### Filter Class Updates
```cpp
class Filter {
private:
    FilterState filter_state_ = FilterState::INCLUDE;  // Replaces is_active
    
public:
    // State management
    FilterState GetFilterState() const { return filter_state_; }
    void SetFilterState(FilterState state) { filter_state_ = state; }
    void CycleFilterState();
    
    // Backward compatibility
    bool IsActive() const { return filter_state_ != FilterState::DISABLED; }
    void SetActive(bool active) { 
        filter_state_ = active ? FilterState::INCLUDE : FilterState::DISABLED; 
    }
};
```

## Error Handling

### Enhanced Unreal Project Detection
- **Invalid folder paths**: Existing error handling in `unreal_project_utils` will catch and report these
- **Permission issues**: Existing filesystem error handling will manage access problems
- **Missing Saved/Logs**: Graceful fallback to original folder path with informative logging

### Three-State Filter System
- **Invalid state transitions**: Enum validation ensures only valid states are set
- **Serialization compatibility**: Backward compatibility maintained by mapping old boolean values
- **UI state consistency**: Filter panel will validate state before rendering

## Testing Strategy

### Unit Tests

#### Enhanced Unreal Project Detection
```cpp
// Test cases for ProcessFolderArgument function
TEST(UnrealProjectUtils, ProcessFolderArgument_WithSavedLogs) {
    // Test folder with Saved/Logs subfolder
}

TEST(UnrealProjectUtils, ProcessFolderArgument_WithoutSavedLogs) {
    // Test folder without Saved/Logs subfolder
}

TEST(UnrealProjectUtils, ProcessFolderArgument_InvalidPath) {
    // Test invalid folder paths
}
```

#### Three-State Filter System
```cpp
// Test cases for FilterState functionality
TEST(Filter, FilterState_CycleStates) {
    // Test state cycling: INCLUDE → EXCLUDE → DISABLED → INCLUDE
}

TEST(Filter, FilterState_MatchingLogic) {
    // Test include/exclude logic with sample log entries
}

TEST(Filter, FilterState_BackwardCompatibility) {
    // Test that old boolean API still works
}
```

### Integration Tests

#### End-to-End Folder Detection
```cpp
TEST(MainWindow, LaunchWithUnrealProjectFolder) {
    // Test launching with Unreal project root folder
    // Verify Saved/Logs is automatically detected and used
}
```

#### Filter State UI Integration
```cpp
TEST(FilterPanel, ThreeStateFilterInteraction) {
    // Test clicking through filter states in UI
    // Verify visual indicators match filter states
}
```

### Manual Testing Scenarios

1. **Folder Detection Testing**:
   - Launch with various folder types (Unreal project root, regular folder, Saved/Logs directly)
   - Verify correct path resolution and user feedback

2. **Filter State Testing**:
   - Create filters and cycle through all three states
   - Verify visual indicators (green checkmark, red negative, greyed out)
   - Test filter combinations with mixed states

## Implementation Notes

### Backward Compatibility
- The `is_active` boolean API will be maintained for existing code
- Filter serialization will handle both old and new formats
- UI will gracefully handle filters created with the old system

### Performance Considerations
- Filter state checking is a simple enum comparison (no performance impact)
- Folder detection adds one additional filesystem check per launch (minimal impact)
- Existing regex caching and filter optimization remains unchanged

### Visual Design
- **INCLUDE state**: Green background with white checkmark (✓)
- **EXCLUDE state**: Red background with white negative sign (−)  
- **DISABLED state**: Grey background with dimmed text

### User Experience Flow
1. **Filter Creation**: New filters default to INCLUDE state (current behavior)
2. **State Cycling**: Single click cycles through states in logical order
3. **Visual Feedback**: Clear, distinct visual indicators for each state
4. **Tooltips**: Hover text explains current state and next action