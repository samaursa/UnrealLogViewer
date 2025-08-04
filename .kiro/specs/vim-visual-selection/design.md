# Design Document

## Overview

This document outlines the design for implementing vim-like visual selection mode in the Unreal Log Viewer. The feature will extend the existing keyboard handling system to support multi-line selection with visual feedback, building upon the current single-line copy functionality and selection highlighting system.

## Architecture

### Core Components

The visual selection functionality will be integrated into the existing `MainWindow` class, leveraging the current event handling system and visual theme management. The implementation will add new state management for visual selection mode while maintaining compatibility with existing features.

```
┌─────────────────────────────────────────────────────────────┐
│                        MainWindow                           │
├─────────────────────────────────────────────────────────────┤
│  Visual Selection State:                                    │
│  - visual_selection_mode_: bool                             │
│  - visual_selection_start_: int                             │
│  - visual_selection_end_: int                               │
│  - visual_selection_anchor_: int                            │
├─────────────────────────────────────────────────────────────┤
│  Event Handling:                                            │
│  - HandleVisualSelectionKeyboard()                          │
│  - EnterVisualSelectionMode()                               │
│  - ExitVisualSelectionMode()                                │
│  - ExtendVisualSelection()                                  │
├─────────────────────────────────────────────────────────────┤
│  Rendering:                                                 │
│  - RenderVisualSelectionHighlight()                         │
│  - UpdateVisualSelectionStatusBar()                         │
├─────────────────────────────────────────────────────────────┤
│  Clipboard Integration:                                     │
│  - CopyVisualSelectionToClipboard()                         │
└─────────────────────────────────────────────────────────────┘
```

### Integration Points

- **Event System**: Extends the existing `OnEvent` handler in `MainWindowComponent`
- **Visual Theme Manager**: Uses existing color system for selection highlighting
- **Log Entry Renderer**: Extends current highlighting system for multi-line selection
- **Status Bar**: Integrates with existing status display system
- **Clipboard**: Builds upon existing `CopyCurrentLineToClipboard()` functionality

## Components and Interfaces

### Visual Selection State Management

```cpp
class MainWindow {
private:
    // Visual selection state
    bool visual_selection_mode_ = false;
    int visual_selection_start_ = -1;  // Starting line index
    int visual_selection_end_ = -1;    // Current end line index
    int visual_selection_anchor_ = -1; // Fixed anchor point for selection
    
public:
    // Visual selection mode control
    auto EnterVisualSelectionMode() -> void;
    auto ExitVisualSelectionMode() -> void;
    auto IsVisualSelectionMode() const -> bool;
    
    // Selection manipulation
    auto ExtendVisualSelection(int new_end_index) -> void;
    auto GetVisualSelectionRange() const -> std::pair<int, int>;
    auto GetVisualSelectionSize() const -> int;
    
    // Clipboard operations
    auto CopyVisualSelectionToClipboard() -> void;
};
```

### Event Handling Extension

The existing keyboard event handling in `MainWindowComponent::OnEvent` will be extended to support visual selection mode:

```cpp
bool OnEvent(Event event) override {
    // Check if we're in visual selection mode first
    if (parent_->IsVisualSelectionMode()) {
        return HandleVisualSelectionEvent(event);
    }
    
    // Handle visual selection mode activation
    if (event == Event::Character('v')) {
        parent_->EnterVisualSelectionMode();
        return true;
    }
    
    // Existing event handling continues...
}

bool HandleVisualSelectionEvent(Event event) {
    // ESC - Exit visual selection mode
    if (event == Event::Escape) {
        parent_->ExitVisualSelectionMode();
        return true;
    }
    
    // y - Copy selection and exit
    if (event == Event::Character('y')) {
        parent_->CopyVisualSelectionToClipboard();
        parent_->ExitVisualSelectionMode();
        return true;
    }
    
    // Navigation keys - extend selection
    if (event == Event::Character('j')) {
        parent_->ExtendVisualSelection(parent_->GetSelectedEntryIndex() + 1);
        return true;
    }
    
    if (event == Event::Character('k')) {
        parent_->ExtendVisualSelection(parent_->GetSelectedEntryIndex() - 1);
        return true;
    }
    
    // Ctrl+d/u for half-page navigation
    if (event == Event::CtrlD) {
        int half_page = std::max(1, parent_->GetVisibleHeight() / 2);
        parent_->ExtendVisualSelection(parent_->GetSelectedEntryIndex() + half_page);
        return true;
    }
    
    if (event == Event::CtrlU) {
        int half_page = std::max(1, parent_->GetVisibleHeight() / 2);
        parent_->ExtendVisualSelection(parent_->GetSelectedEntryIndex() - half_page);
        return true;
    }
    
    // Allow help dialog access
    if (event == Event::Character('h') || event == Event::Character('?')) {
        // Delegate to normal help handling
        return false;
    }
    
    // Block other keys in visual selection mode
    return true;
}
```

### Visual Highlighting System

The visual selection highlighting will integrate with the existing `LogEntryRenderer` and `VisualThemeManager`:

```cpp
class LogEntryRenderer {
public:
    // Extended rendering method for visual selection
    auto RenderLogEntryWithVisualSelection(
        const LogEntry& entry, 
        bool is_selected,
        bool is_visual_selected,
        int relative_line_number) const -> ftxui::Element;
        
private:
    auto ApplyVisualSelectionHighlight(
        ftxui::Element element,
        bool is_visual_selected) const -> ftxui::Element;
};

class VisualThemeManager {
public:
    // New color for visual selection highlighting
    auto GetVisualSelectionColor() const -> ftxui::Color;
    auto GetVisualSelectionBackgroundColor() const -> ftxui::Color;
};
```

### Status Bar Integration

The status bar will be extended to show visual selection information:

```cpp
auto MainWindow::RenderStatusBar() const -> ftxui::Element {
    // Existing status bar elements...
    
    if (visual_selection_mode_) {
        int selection_size = GetVisualSelectionSize();
        std::string visual_status = "VISUAL: " + std::to_string(selection_size) + 
                                   (selection_size == 1 ? " line selected" : " lines selected");
        
        status_elements.push_back(
            text(visual_status) | 
            color(_Visual_Theme_Manager_->GetAccentColor()) | 
            bold
        );
    }
    
    // Continue with existing status bar rendering...
}
```

## Data Models

### Visual Selection State

```cpp
struct VisualSelectionState {
    bool active = false;
    int anchor_index = -1;      // Fixed starting point
    int current_index = -1;     // Current cursor position
    
    // Helper methods
    auto GetStartIndex() const -> int {
        return std::min(anchor_index, current_index);
    }
    
    auto GetEndIndex() const -> int {
        return std::max(anchor_index, current_index);
    }
    
    auto GetSize() const -> int {
        if (!active || anchor_index == -1 || current_index == -1) {
            return 0;
        }
        return std::abs(current_index - anchor_index) + 1;
    }
    
    auto Contains(int index) const -> bool {
        if (!active) return false;
        return index >= GetStartIndex() && index <= GetEndIndex();
    }
};
```

## Error Handling

### Visual Selection Mode Errors

- **Invalid Selection Range**: Ensure selection indices are within bounds of filtered entries
- **Empty Selection**: Handle cases where no lines are available for selection
- **Clipboard Failures**: Graceful handling of clipboard access errors with user feedback

### Error Recovery

```cpp
auto MainWindow::ExtendVisualSelection(int new_end_index) -> void {
    // Bounds checking
    if (new_end_index < 0) {
        new_end_index = 0;
    }
    if (new_end_index >= static_cast<int>(filtered_entries_.size())) {
        new_end_index = static_cast<int>(filtered_entries_.size()) - 1;
    }
    
    // Update selection state
    visual_selection_end_ = new_end_index;
    selected_entry_index_ = new_end_index;
    
    // Ensure visibility
    EnsureSelectionVisible();
}
```

## Testing Strategy

### Unit Tests

- **Visual Selection State Management**: Test mode activation, deactivation, and state transitions
- **Selection Range Calculation**: Test boundary conditions and range calculations
- **Keyboard Event Handling**: Test all visual selection keyboard shortcuts
- **Clipboard Integration**: Test multi-line copying functionality

### Integration Tests

- **Visual Highlighting**: Test selection highlighting with different themes
- **Status Bar Updates**: Test visual selection status display
- **Navigation Integration**: Test interaction with existing navigation features
- **Filter Compatibility**: Test visual selection with active filters

### Manual Testing Scenarios

1. **Basic Visual Selection**: Enter visual mode, select multiple lines, copy and exit
2. **Navigation Testing**: Test all navigation keys (j/k, Ctrl+d/u) in visual mode
3. **Boundary Testing**: Test selection at beginning and end of log file
4. **Mode Switching**: Test entering/exiting visual mode multiple times
5. **Integration Testing**: Test visual selection with filters, search, and detail view

## Implementation Phases

### Phase 1: Core Visual Selection State
- Add visual selection state variables to MainWindow
- Implement basic mode activation/deactivation
- Add keyboard event handling for 'v' and 'ESC'

### Phase 2: Selection Navigation
- Implement selection extension with j/k navigation
- Add Ctrl+d/u half-page navigation
- Implement bounds checking and auto-scroll

### Phase 3: Visual Feedback
- Extend LogEntryRenderer for visual selection highlighting
- Add visual selection colors to VisualThemeManager
- Update status bar to show selection information

### Phase 4: Clipboard Integration
- Implement multi-line clipboard copying
- Add error handling and user feedback
- Test clipboard functionality across platforms

### Phase 5: Integration and Polish
- Ensure compatibility with existing features
- Add comprehensive testing
- Performance optimization for large selections

## Performance Considerations

- **Large Selections**: Optimize rendering for selections spanning thousands of lines
- **Real-time Updates**: Efficient highlighting updates during selection extension
- **Memory Usage**: Minimize memory overhead for selection state tracking
- **Scroll Performance**: Smooth auto-scrolling during selection extension

## Security Considerations

- **Clipboard Access**: Proper error handling for clipboard access failures
- **Memory Management**: Safe handling of large text selections in clipboard operations
- **Input Validation**: Validate selection indices to prevent buffer overflows