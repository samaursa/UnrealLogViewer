# Design Document

## Overview

This feature adds folder browsing capabilities to the Unreal log viewer, allowing users to navigate and select log files from directories. The design introduces a new file browser component that integrates seamlessly with the existing application architecture while providing vim-style navigation and intelligent Unreal Engine project detection.

## Architecture

### High-Level Architecture

The folder browsing feature extends the existing application architecture by:

1. **Command Line Enhancement**: Modifying the CLI11 argument parsing to accept both files and directories
2. **New UI Component**: Adding a `FileBrowser` component that follows the existing `Component` base class pattern
3. **Application State Management**: Extending `MainWindow` to handle file browser mode vs log viewing mode
4. **Unreal Project Detection**: Adding utility functions to detect `Saved/Logs` directories

### Component Integration

```
MainWindow
├── FileBrowser (new)
│   ├── File listing and navigation
│   ├── Vim-style key handling
│   └── File selection logic
├── LogParser (existing)
├── FilterEngine (existing)
└── Other existing components
```

## Components and Interfaces

### 1. FileBrowser Component

**Location**: `lib/ui/file_browser.h` and `lib/ui/file_browser.cpp`

**Responsibilities**:
- Display list of `*.log` files in a directory
- Handle vim-style navigation (j/k, Ctrl+u/d)
- Manage file selection and highlighting
- Provide file metadata (size, modification date)

**Key Methods**:
```cpp
class FileBrowser : public Component {
public:
    FileBrowser(const std::string& directory_path);
    void Initialize() override;
    ftxui::Element Render() const override;
    ftxui::Component CreateFTXUIComponent() override;
    
    // Navigation methods
    void SelectNext();
    void SelectPrevious();
    void PageDown();
    void PageUp();
    void HalfPageDown();
    void HalfPageUp();
    
    // File operations
    std::string GetSelectedFilePath() const;
    bool HasFiles() const;
    void RefreshFileList();
    
    // Event handling
    bool OnEvent(ftxui::Event event);
    
private:
    struct FileInfo {
        std::string filename;
        std::string full_path;
        std::filesystem::file_time_type modified_time;
        std::uintmax_t size;
    };
    
    std::string directory_path_;
    std::vector<FileInfo> files_;
    int selected_index_ = 0;
    int scroll_offset_ = 0;
    
    void ScanDirectory();
    void SortFilesByModificationTime();
    ftxui::Element RenderFileList() const;
    ftxui::Element RenderFileEntry(const FileInfo& file, bool selected) const;
};
```

### 2. Command Line Argument Enhancement

**Location**: `app/main.cpp`

**Changes**:
- Modify CLI11 validation to accept both files and directories
- Add logic to detect if argument is file or directory
- Handle directory path processing

**Implementation**:
```cpp
// Replace CLI::ExistingFile with custom validator
app.add_option("path", input_path, "Path to log file or directory containing log files")
   ->check([](const std::string& path) -> std::string {
       if (std::filesystem::exists(path)) {
           return "";
       }
       return "Path does not exist: " + path;
   });
```

### 3. MainWindow State Management

**Location**: `lib/ui/main_window.h` and `lib/ui/main_window.cpp`

**New State Variables**:
```cpp
enum class ApplicationMode {
    FILE_BROWSER,
    LOG_VIEWER
};

ApplicationMode current_mode_ = ApplicationMode::LOG_VIEWER;
std::unique_ptr<FileBrowser> file_browser_;
```

**New Methods**:
```cpp
void EnterFileBrowserMode(const std::string& directory_path);
void EnterLogViewerMode(const std::string& file_path);
void OnFileSelected(const std::string& file_path);
```

### 4. Unreal Project Detection Utility

**Location**: `lib/common/unreal_project_utils.h` and `lib/common/unreal_project_utils.cpp`

**Functionality**:
```cpp
namespace ue_log {
namespace unreal_utils {

/**
 * Find the Saved/Logs directory relative to the current working directory.
 * @return Path to Saved/Logs if found, empty string otherwise.
 */
std::string FindSavedLogsDirectory();

/**
 * Check if a directory contains Unreal log files.
 * @param directory_path Path to check
 * @return True if directory contains *.log files
 */
bool ContainsLogFiles(const std::string& directory_path);

/**
 * Get all log files in a directory, sorted by modification time.
 * @param directory_path Directory to scan
 * @return Vector of file paths sorted by modification time (newest first)
 */
std::vector<std::string> GetLogFiles(const std::string& directory_path);

} // namespace unreal_utils
} // namespace ue_log
```

## Data Models

### FileInfo Structure

```cpp
struct FileInfo {
    std::string filename;           // Just the filename (e.g., "MyProject.log")
    std::string full_path;          // Full path to the file
    std::filesystem::file_time_type modified_time;  // Last modification time
    std::uintmax_t size;           // File size in bytes
    
    // Helper methods
    std::string GetFormattedSize() const;
    std::string GetFormattedModificationTime() const;
};
```

### Application State

The application will have two distinct modes:

1. **File Browser Mode**: Shows list of log files, handles navigation
2. **Log Viewer Mode**: Shows log content (existing functionality)

Mode transitions:
- Start in File Browser mode when directory is provided
- Transition to Log Viewer mode when file is selected
- Allow returning to File Browser mode (future enhancement)

## Error Handling

### Directory Access Errors
- **Invalid directory path**: Display error message and exit gracefully
- **Permission denied**: Show appropriate error message
- **No log files found**: Display informative message with suggestions

### File Selection Errors
- **File no longer exists**: Refresh file list and show error
- **File access denied**: Display error and remain in browser mode
- **File load failure**: Show error and return to file browser

### Unreal Project Detection Errors
- **No Saved/Logs found**: Fall back to current directory behavior
- **Multiple potential directories**: Use heuristics to select best match

## Testing Strategy

### Unit Tests

1. **FileBrowser Component Tests** (`tests/test_file_browser.cpp`)
   - File listing functionality
   - Navigation key handling
   - File selection logic
   - Directory scanning and sorting

2. **Unreal Utils Tests** (`tests/test_unreal_project_utils.cpp`)
   - Saved/Logs directory detection
   - Log file discovery
   - Path validation

3. **Command Line Tests** (`tests/test_cli_arguments.cpp`)
   - Directory vs file argument handling
   - Invalid path handling
   - Unreal project detection integration

### Integration Tests

1. **File Browser to Log Viewer Transition** (`tests/test_file_browser_integration.cpp`)
   - Mode switching
   - State preservation
   - Error handling during transitions

2. **End-to-End Workflow Tests** (`tests/test_folder_browsing_workflow.cpp`)
   - Complete user workflow from directory to log viewing
   - Vim navigation in file browser
   - File selection and loading

### Manual Testing Scenarios

1. **Basic Directory Browsing**
   - Launch with directory containing multiple log files
   - Verify files are sorted by modification date
   - Test vim navigation keys

2. **Unreal Project Integration**
   - Launch from Unreal project root
   - Verify automatic Saved/Logs detection
   - Test with projects that have/don't have Saved/Logs

3. **Edge Cases**
   - Empty directories
   - Directories with no log files
   - Very large numbers of log files
   - Files with special characters in names

## Implementation Phases

### Phase 1: Core File Browser Component
- Create FileBrowser class with basic file listing
- Implement vim-style navigation
- Add file selection functionality

### Phase 2: Command Line Integration
- Modify main.cpp to handle directory arguments
- Add directory vs file detection logic
- Integrate with MainWindow state management

### Phase 3: Unreal Project Detection
- Implement Saved/Logs directory detection
- Add automatic directory discovery when no arguments provided
- Handle edge cases and error conditions

### Phase 4: UI Polish and Testing
- Improve visual presentation of file browser
- Add comprehensive test coverage
- Performance optimization for large directories

## Security Considerations

- **Path Traversal**: Validate directory paths to prevent access outside intended directories
- **File System Permissions**: Handle permission errors gracefully
- **Resource Limits**: Limit number of files displayed to prevent memory issues
- **Input Validation**: Sanitize all file paths and directory inputs

## Performance Considerations

- **Directory Scanning**: Use efficient filesystem APIs for large directories
- **File Sorting**: Optimize sorting algorithm for large file lists
- **Memory Usage**: Limit file metadata caching for very large directories
- **UI Responsiveness**: Ensure navigation remains smooth with many files

## Future Enhancements

- **Recursive Directory Browsing**: Support for subdirectories
- **File Filtering**: Filter files by date, size, or name patterns
- **Multiple File Selection**: Select and compare multiple log files
- **Recent Files**: Remember recently accessed files and directories
- **Bookmarks**: Save frequently accessed directory paths