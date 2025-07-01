# Unreal Engine Log Viewer Requirements

## Project Overview
Enhance the existing FTXUI-based terminal log viewer for Unreal Engine logs. The current application can read log files but needs full log parsing, filtering, real-time tailing, and advanced UI features.

## Core Functionality Requirements

### 1. Log Structure Parsing
- Parse Unreal Engine log format into structured columns:
  - **Timestamp**: Extract and display log timestamp
  - **Frame Number**: Extract frame number if present
  - **Logger Name**: Extract logger/category name (e.g., LogTemp, LogBlueprintUserMessages, etc.)
  - **Log Level**: Extract log level (Log, Warning, Error, etc.)
  - **Message**: The actual log message content
- Display in a table/column format with proper alignment
- Handle varying log formats and malformed entries gracefully
- Support both UE4 and UE5 log formats

### 2. Real-time Log Tailing
- Implement file watching to detect new log entries
- Automatically scroll to show new entries as they arrive
- Allow toggling between tailing mode and manual scrolling
- Maintain performance with large log files during real-time updates
- Handle log file rotation/replacement

### 3. Multi-level Filtering System
- **Primary Filters**: Top-level filters that can be created, edited, and deleted
- **Nested Subfilters**: Each filter can contain subfilters with unlimited nesting depth
- **Filter Types**:
  - Text matching (contains, exact match, regex)
  - Logger name filtering
  - Log level filtering
  - Time range filtering
  - Frame number range filtering
- **Filter Logic**: Support AND/OR operations between filters and subfilters
- **Filter Persistence**: Save and load filter configurations

### 4. Visual Highlighting System
- Assign unique highlight colors to each primary filter
- Subfilters inherit parent color with slight variations (brightness/saturation)
- Support multiple color schemes for different terminal capabilities
- Allow customization of highlight colors
- Show visual indicators for active/inactive filters

### 5. Filter Management UI
- **Filter Panel**: Dedicated UI panel for managing filters
- **Filter Tree View**: Hierarchical display of filters and subfilters
- **Toggle Controls**: Individual on/off switches for each filter
- **Filter Editing**: In-place editing of filter criteria
- **Filter Statistics**: Show match counts for each filter

## User Interface Requirements

### 6. Layout Structure
- **Main Log View**: Primary scrollable table showing parsed log entries
- **Filter Panel**: Collapsible side panel or bottom panel for filter management
- **Status Bar**: Show current file, filter status, entry counts, tailing status
- **Command Bar**: Input area for quick commands and search

### 7. Navigation Controls
- **Keyboard Shortcuts**:
  - Arrow keys for navigation
  - Page Up/Down for bulk scrolling
  - Home/End for jumping to start/end
  - Tab to switch between panels
  - Enter to toggle filter selection
  - Space to toggle filter on/off
  - Escape to cancel operations
- **Search Functionality**: Quick search with highlight
- **Jump to Line**: Direct navigation to specific line numbers or timestamps

### 8. Performance Considerations
- Implement virtual scrolling for large log files
- Lazy loading of log entries
- Efficient regex compilation and caching
- Background processing for file watching
- Memory management for long-running sessions

## Technical Implementation Notes

### 9. File Handling
- Support for large log files (GB+)
- Handle locked files (logs currently being written)
- Detect and handle file encoding (UTF-8, UTF-16, etc.)
- Support compressed log files (.gz, .zip)

### 10. Configuration System
- Configuration file for user preferences
- Configurable key bindings
- Customizable color schemes
- Persistent filter sets
- Window layout preferences

### 11. Error Handling
- Graceful handling of corrupted log files
- User-friendly error messages
- Recovery from parsing errors
- Validation of filter expressions

## User Experience Features

### 12. Usability Enhancements
- **Context Menus**: Right-click actions for common operations
- **Help System**: Built-in help with keyboard shortcuts
- **Undo/Redo**: For filter operations
- **Export Functionality**: Export filtered results to file
- **Statistics View**: Summary of log levels, timeframes, etc.

### 13. Advanced Features
- **Bookmarks**: Mark important log entries
- **Notes**: Add user notes to specific log entries
- **Session Management**: Save and restore viewer sessions
- **Multiple File Support**: View multiple log files simultaneously
- **Log Comparison**: Compare two log files side by side

## Implementation Priority
1. **High Priority**: Log parsing, basic filtering, real-time tailing
2. **Medium Priority**: Multi-level filters, visual highlighting, filter management UI
3. **Low Priority**: Advanced features, export functionality, session management

## Testing Requirements
- Test with various Unreal Engine log formats
- Performance testing with large files (1GB+)
- Real-time performance testing
- Filter performance with complex nested structures
- Memory usage optimization validation

## Dependencies
- Continue using FTXUI for terminal UI
- Consider additional libraries for file watching (filesystem, inotify)
- Regex library for pattern matching
- Configuration file handling (JSON, YAML, or INI)

## Success Criteria
- Handle 1GB+ log files smoothly
- Real-time tailing with <100ms latency
- Support 10+ concurrent filters with subfilters
- Intuitive keyboard-driven interface
- Stable performance during extended sessions