# Design Document

## Overview

This design document outlines the implementation approach for enhancing the visual quality of life aspects of the Unreal Log Viewer. The improvements focus on better visual formatting, enhanced readability, improved column spacing, relative line numbering for navigation, logger badge system, and making warnings/errors more prominent.

The design leverages the existing FTXUI framework and extends the current MainWindow rendering system to provide a more polished and user-friendly interface.

## Architecture

### Visual Enhancement System

The visual improvements will be implemented through several interconnected systems:

1. **Enhanced Rendering Pipeline**: Extend the existing `MainWindow::Render()` method with new visual formatting capabilities
2. **Color Management System**: Centralized color scheme management for consistent theming
3. **Logger Badge System**: Dynamic color assignment and badge rendering for logger categories
4. **Line Number Display System**: Vim-style relative line numbering with absolute positioning in status/detail areas
5. **Visual Hierarchy System**: Enhanced styling for different log levels and content types

### Component Integration

The visual improvements will integrate with existing components:

- **MainWindow**: Primary rendering orchestration and layout management
- **LogEntry**: Data source for visual elements (logger, level, message, etc.)
- **FilterPanel**: Coordinate visual consistency with main display
- **StatusBar**: Display absolute line numbers and visual feedback

## Components and Interfaces

### 1. Visual Theme Manager

```cpp
class VisualThemeManager {
public:
    // Color scheme management
    ftxui::Color GetLoggerColor(const std::string& logger_name);
    ftxui::Color GetLogLevelColor(const std::string& log_level);
    ftxui::Color GetBackgroundColor();
    ftxui::Color GetTextColor();
    ftxui::Color GetHighlightColor();
    
    // Visual style configuration
    struct ColumnSpacing {
        int line_number_width = 6;
        int timestamp_width = 12;
        int frame_width = 8;
        int logger_width = 20;
        int level_width = 8;
        int padding = 2;
    };
    
    ColumnSpacing GetColumnSpacing() const;
    void SetColumnSpacing(const ColumnSpacing& spacing);
    
private:
    std::unordered_map<std::string, ftxui::Color> logger_colors_;
    std::vector<ftxui::Color> available_colors_;
    size_t next_color_index_ = 0;
    
    void InitializeColorPalette();
    ftxui::Color AssignLoggerColor(const std::string& logger_name);
};
```

### 2. Enhanced Log Entry Renderer

```cpp
class LogEntryRenderer {
public:
    LogEntryRenderer(VisualThemeManager* theme_manager);
    
    // Main rendering methods
    ftxui::Element RenderLogEntry(const LogEntry& entry, bool is_selected, 
                                 int relative_line_number = 0) const;
    ftxui::Element RenderTableHeader() const;
    
    // Component rendering methods
    ftxui::Element RenderLineNumber(int relative_number, bool is_current) const;
    ftxui::Element RenderTimestamp(const LogEntry& entry) const;
    ftxui::Element RenderFrameNumber(const LogEntry& entry) const;
    ftxui::Element RenderLoggerBadge(const LogEntry& entry) const;
    ftxui::Element RenderLogLevel(const LogEntry& entry) const;
    ftxui::Element RenderMessage(const LogEntry& entry, bool is_selected) const;
    
    // Configuration
    void SetWordWrapEnabled(bool enabled) { word_wrap_enabled_ = enabled; }
    void SetShowLineNumbers(bool show) { show_line_numbers_ = show; }
    
private:
    VisualThemeManager* theme_manager_;
    bool word_wrap_enabled_ = false;
    bool show_line_numbers_ = true;
    
    // Helper methods
    ftxui::Element CreateBadgeElement(const std::string& text, ftxui::Color color) const;
    ftxui::Element ApplyLogLevelStyling(ftxui::Element element, const std::string& level) const;
    std::string TruncateText(const std::string& text, int max_width) const;
};
```

### 3. Relative Line Number System

```cpp
class RelativeLineNumberSystem {
public:
    struct LineNumberInfo {
        int absolute_number;
        int relative_number;
        bool is_current_line;
    };
    
    // Calculate relative line numbers for visible entries
    std::vector<LineNumberInfo> CalculateRelativeNumbers(
        int current_selection, int visible_start, int visible_count) const;
    
    // Vim-style navigation support
    bool HandleNavigationInput(const std::string& input, int& jump_distance, char& direction);
    
private:
    // Helper methods for vim-style parsing
    bool ParseJumpCommand(const std::string& input, int& count, char& command);
};
```

### 4. Enhanced Status Bar System

```cpp
class EnhancedStatusBar {
public:
    EnhancedStatusBar(VisualThemeManager* theme_manager);
    
    // Rendering methods
    ftxui::Element RenderMainStatusBar(const MainWindow* window) const;
    ftxui::Element RenderSearchStatusBar(const MainWindow* window) const;
    
    // Status information display
    ftxui::Element RenderAbsoluteLineInfo(int current_line, int total_lines) const;
    ftxui::Element RenderFileInfo(const std::string& file_path, size_t entry_count) const;
    ftxui::Element RenderFilterInfo(const FilterEngine* filter_engine) const;
    
private:
    VisualThemeManager* theme_manager_;
    
    // Helper methods
    ftxui::Element CreateStatusSegment(const std::string& label, const std::string& value) const;
};
```

## Data Models

### Color Palette Configuration

```cpp
struct ColorPalette {
    // Logger badge colors (cycling through these)
    std::vector<ftxui::Color> logger_colors = {
        ftxui::Color::Blue,
        ftxui::Color::Green,
        ftxui::Color::Cyan,
        ftxui::Color::Magenta,
        ftxui::Color::Yellow,
        ftxui::Color::BlueLight,
        ftxui::Color::GreenLight,
        ftxui::Color::CyanLight,
        ftxui::Color::MagentaLight
    };
    
    // Log level colors
    ftxui::Color error_color = ftxui::Color::RedLight;
    ftxui::Color warning_color = ftxui::Color::YellowLight;
    ftxui::Color info_color = ftxui::Color::White;
    ftxui::Color debug_color = ftxui::Color::GrayLight;
    
    // UI element colors
    ftxui::Color background_color = ftxui::Color::Black;
    ftxui::Color text_color = ftxui::Color::White;
    ftxui::Color highlight_color = ftxui::Color::Blue;
    ftxui::Color selection_color = ftxui::Color::BlueLight;
};
```

### Visual Layout Configuration

```cpp
struct VisualLayoutConfig {
    // Column widths and spacing
    struct ColumnLayout {
        int line_number_width = 6;      // "  123 " or " +12 " for relative
        int timestamp_width = 12;       // "12:34:56.789"
        int frame_width = 8;           // " [12345]"
        int logger_badge_width = 20;   // "[LoggerName    ]"
        int level_width = 8;           // " ERROR  "
        int column_padding = 2;        // Spaces between columns
    };
    
    ColumnLayout columns;
    
    // Visual styling options
    bool use_badges_for_loggers = true;
    bool highlight_errors_warnings = true;
    bool show_relative_line_numbers = true;
    bool use_color_coding = true;
    
    // Filter interface improvements
    std::string filter_expand_text = "Press [Enter] to expand filter options, [Tab] to focus filter panel";
};
```

## Error Handling

### Visual Rendering Error Handling

1. **Color Assignment Failures**: Fall back to default colors if custom color assignment fails
2. **Text Rendering Issues**: Gracefully handle text that's too long or contains special characters
3. **Layout Calculation Errors**: Provide minimum viable layout if optimal calculations fail
4. **Theme Loading Failures**: Fall back to built-in default theme

### Graceful Degradation

- If color support is not available, fall back to text-based differentiation
- If terminal width is too narrow, prioritize essential columns and truncate others
- If relative line number calculation fails, show absolute numbers as fallback

## Testing Strategy

### Unit Tests

1. **VisualThemeManager Tests**
   - Color assignment consistency
   - Logger color cycling behavior
   - Theme configuration loading/saving

2. **LogEntryRenderer Tests**
   - Individual component rendering (badges, line numbers, etc.)
   - Layout calculation accuracy
   - Text truncation and wrapping behavior

3. **RelativeLineNumberSystem Tests**
   - Relative number calculation accuracy
   - Vim-style navigation command parsing
   - Edge cases (first/last entries, empty lists)

### Integration Tests

1. **Visual Layout Tests**
   - Complete log table rendering with various entry types
   - Column alignment and spacing consistency
   - Responsive behavior with different terminal sizes

2. **Color System Tests**
   - Logger badge color consistency across sessions
   - Log level color application
   - Theme switching behavior

### Visual Regression Tests

1. **Rendering Consistency Tests**
   - Compare rendered output before and after changes
   - Verify visual hierarchy is maintained
   - Ensure accessibility standards are met

### Manual Testing Scenarios

1. **User Experience Tests**
   - Load logs with many different logger names
   - Test relative line number navigation
   - Verify error/warning prominence
   - Test filter interface clarity improvements

## Implementation Phases

### Phase 1: Core Visual Infrastructure
- Implement VisualThemeManager
- Create basic color palette system
- Extend MainWindow rendering pipeline

### Phase 2: Logger Badge System
- Implement dynamic logger color assignment
- Create badge rendering components
- Integrate with existing log table rendering

### Phase 3: Line Number System
- Implement RelativeLineNumberSystem
- Update MainWindow to use relative numbers
- Add absolute line display to status bar

### Phase 4: Enhanced Visual Hierarchy
- Implement error/warning highlighting
- Improve column spacing and alignment
- Add visual polish and consistency

### Phase 5: Filter Interface Improvements
- Update filter panel visual consistency
- Improve filter status messaging
- Add helpful tooltips and descriptions

## Performance Considerations

### Rendering Optimization
- Cache color assignments for loggers to avoid repeated calculations
- Use efficient text truncation algorithms
- Minimize FTXUI element creation overhead

### Memory Management
- Limit logger color cache size to prevent unbounded growth
- Use string interning for repeated logger names
- Efficient storage of visual configuration data

### Responsive Design
- Implement adaptive column widths based on terminal size
- Provide graceful degradation for narrow terminals
- Optimize rendering for different screen sizes