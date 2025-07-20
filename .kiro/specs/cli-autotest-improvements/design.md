# CLI and Autotest Improvements Design

## Overview

The current autotest implementation has a critical flaw: it attempts to test UI functionality without running the FTXUI event loop, making the tests ineffective. This design addresses this by creating a proper separation between UI and business logic, implementing headless testing capabilities, and improving the overall CLI experience.

## Architecture

### Current Problem Analysis

The existing `RunAutotest` method calls UI methods like `ScrollDown()`, `ScrollUp()`, and `ApplyQuickFilter()` without initializing the FTXUI event loop. These methods depend on UI state and rendering context that doesn't exist in headless mode, causing the tests to either fail silently or produce meaningless results.

### Proposed Solution Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application Layer                         │
├─────────────────────────────────────────────────────────────────┤
│  CLI Parser (CLI11)  │  Interactive Mode  │  Autotest Mode     │
├─────────────────────────────────────────────────────────────────┤
│                     MainWindow (UI Layer)                       │
├─────────────────────────────────────────────────────────────────┤
│                  Core Business Logic Layer                      │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐   │
│  │   LogProcessor  │ │ FilterManager   │ │ NavigationState │   │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│                    Backend Components                           │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐   │
│  │   LogParser     │ │  FilterEngine   │ │  FileMonitor    │   │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### 1. Core Business Logic Abstraction

Create new classes that encapsulate the business logic without UI dependencies:

#### LogProcessor
```cpp
class LogProcessor {
public:
    bool LoadFile(const std::string& file_path);
    const std::vector<LogEntry>& GetEntries() const;
    const std::vector<LogEntry>& GetFilteredEntries() const;
    void ApplyFilters(const std::vector<std::unique_ptr<FilterCondition>>& filters);
    void ClearFilters();
    
    // Navigation without UI
    void SetCurrentIndex(int index);
    int GetCurrentIndex() const;
    bool CanNavigateUp() const;
    bool CanNavigateDown() const;
    void NavigateUp(int count = 1);
    void NavigateDown(int count = 1);
    
    // Search functionality
    std::vector<int> Search(const std::string& query) const;
    void SetSearchResults(const std::vector<int>& results);
    int GetCurrentSearchIndex() const;
    void NavigateToNextSearchResult();
    void NavigateToPreviousSearchResult();
};
```

#### FilterManager
```cpp
class FilterManager {
public:
    void AddQuickFilter(const std::string& filter_type);
    void ClearAllFilters();
    std::vector<LogEntry> ApplyFilters(const std::vector<LogEntry>& entries) const;
    size_t GetActiveFilterCount() const;
    std::vector<std::string> GetActiveFilterDescriptions() const;
};
```

#### NavigationState
```cpp
class NavigationState {
public:
    void SetTotalEntries(size_t count);
    void SetCurrentIndex(int index);
    int GetCurrentIndex() const;
    int GetVisibleStartIndex(int viewport_height) const;
    int GetVisibleEndIndex(int viewport_height) const;
    bool IsValidIndex(int index) const;
    
    // Percentage-based navigation
    void JumpToPercentage(int percentage);
    int GetCurrentPercentage() const;
};
```

### 2. Headless Testing Framework

#### AutotestRunner
```cpp
class AutotestRunner {
public:
    AutotestRunner(const std::string& log_file_path, const std::string& output_file_path);
    bool RunAllTests();
    
private:
    bool TestFileLoading();
    bool TestLogParsing();
    bool TestFilterFunctionality();
    bool TestNavigationLogic();
    bool TestSearchFunctionality();
    bool TestErrorHandling();
    bool TestPerformance();
    
    void WriteTestResult(const std::string& test_name, bool passed, const std::string& details = "");
    void WriteTestHeader(const std::string& test_name);
    void WriteTestSummary();
    
    std::unique_ptr<LogProcessor> log_processor_;
    std::unique_ptr<FilterManager> filter_manager_;
    std::unique_ptr<NavigationState> navigation_state_;
    std::ofstream report_file_;
    int tests_passed_;
    int tests_failed_;
};
```

### 3. Enhanced MainWindow Architecture

Refactor MainWindow to use the new business logic components:

```cpp
class MainWindow : public Component {
private:
    // Business logic components (can be used headlessly)
    std::unique_ptr<LogProcessor> log_processor_;
    std::unique_ptr<FilterManager> filter_manager_;
    std::unique_ptr<NavigationState> navigation_state_;
    
    // UI-specific components
    std::unique_ptr<FilterPanel> filter_panel_;
    ftxui::Component component_;
    
    // Mode flags
    bool headless_mode_ = false;
    
public:
    // Constructor for headless mode
    MainWindow(bool headless = false);
    
    // Headless-compatible methods
    bool LoadLogFileHeadless(const std::string& file_path);
    void ApplyQuickFilterHeadless(const std::string& filter_type);
    void NavigateHeadless(int direction);
    
    // UI-dependent methods (only work in interactive mode)
    void ScrollUp(int count = 1) override;
    void ScrollDown(int count = 1) override;
    ftxui::Element Render() const override;
    
    // Accessors for testing
    LogProcessor* GetLogProcessor() const { return log_processor_.get(); }
    FilterManager* GetFilterManager() const { return filter_manager_.get(); }
    NavigationState* GetNavigationState() const { return navigation_state_.get(); }
};
```

## Data Models

### Test Result Structure
```cpp
struct TestResult {
    std::string test_name;
    bool passed;
    std::string details;
    std::chrono::milliseconds duration;
    std::string error_message;
};

struct AutotestReport {
    std::string log_file_path;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::vector<TestResult> test_results;
    int total_tests;
    int passed_tests;
    int failed_tests;
    bool overall_success;
};
```

### CLI Configuration
```cpp
struct CLIConfig {
    std::string log_file_path;
    bool autotest_mode = false;
    std::string autotest_output = "autotest_report.txt";
    bool verbose = false;
    bool version_requested = false;
    bool help_requested = false;
};
```

## Error Handling

### Autotest Error Categories
1. **File Access Errors**: Log file doesn't exist, can't write report file
2. **Parsing Errors**: Invalid log format, corrupted file
3. **Logic Errors**: Navigation out of bounds, invalid filter operations
4. **Performance Errors**: Operations taking too long, memory issues
5. **Component Errors**: Backend components failing to initialize

### Error Reporting Strategy
- Each test captures exceptions and converts them to test failures
- Detailed error messages include stack traces and context
- Failed tests don't stop the entire autotest run
- Final report includes error summary and recommendations

## Testing Strategy

### Autotest Test Categories

#### 1. File Operations Tests
- Load valid log files of different sizes
- Handle missing files gracefully
- Test with corrupted/invalid log files
- Verify file monitoring setup (without actual monitoring)

#### 2. Parsing Tests
- Parse different Unreal Engine log formats
- Handle malformed log entries
- Verify timestamp parsing
- Test log level detection

#### 3. Filter Tests
- Apply each quick filter type (error, warning, info, debug)
- Test filter combinations
- Verify filter clearing
- Test custom filter expressions

#### 4. Navigation Tests
- Navigate through entries sequentially
- Test boundary conditions (first/last entry)
- Verify percentage-based jumping
- Test line number jumping

#### 5. Search Tests
- Search for text in log messages
- Test case sensitivity
- Verify search result navigation
- Test regex search patterns

#### 6. Performance Tests
- Load large log files (if available)
- Measure parsing time
- Test memory usage with large datasets
- Verify filter performance

### Test Data Strategy
- Use existing test log files from the project
- Generate synthetic log data for edge cases
- Test with empty files and single-line files
- Include files with various log levels and formats

## Implementation Plan

### Phase 1: Core Business Logic Extraction
1. Create LogProcessor class with headless functionality
2. Create FilterManager for filter operations
3. Create NavigationState for position tracking
4. Refactor MainWindow to use new components

### Phase 2: Headless Testing Framework
1. Implement AutotestRunner class
2. Create comprehensive test suite
3. Implement detailed reporting system
4. Add performance benchmarking

### Phase 3: CLI Enhancement
1. Improve CLI11 integration
2. Add verbose mode and additional options
3. Enhance error messages and help text
4. Add configuration file support

### Phase 4: Integration and Testing
1. Test autotest with various log files
2. Verify interactive mode still works
3. Test CI/CD integration scenarios
4. Performance optimization and cleanup

## Security Considerations

- Validate all file paths to prevent directory traversal
- Limit memory usage when loading large files
- Sanitize log content in reports to avoid information leakage
- Ensure temporary files are cleaned up properly

## Performance Considerations

- Stream large log files instead of loading entirely into memory
- Implement lazy loading for filtered results
- Use efficient data structures for search indexing
- Provide progress feedback for long-running operations

## Compatibility

- Maintain backward compatibility with existing command line usage
- Ensure all existing keyboard shortcuts work in interactive mode
- Support both Windows and Unix-style file paths
- Handle different line ending formats (CRLF, LF)