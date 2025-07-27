# Requirements Document

## Introduction

This feature enhances the Unreal log viewer to support folder browsing, allowing users to view and select from multiple log files within a directory. The feature is specifically designed to streamline the workflow for Unreal Engine developers by providing intelligent folder detection and vim-style navigation for log file selection.

## Requirements

### Requirement 1

**User Story:** As an Unreal Engine developer, I want to pass a folder path as a command line argument, so that I can browse all log files in that directory without having to specify individual files.

#### Acceptance Criteria

1. WHEN the user provides a folder path as a command line argument THEN the system SHALL accept and validate the folder path
2. WHEN the folder path is valid THEN the system SHALL scan the directory for `*.log` files
3. WHEN the folder path is invalid or inaccessible THEN the system SHALL display an appropriate error message
4. WHEN no log files are found in the specified folder THEN the system SHALL display a message indicating no log files were found

### Requirement 2

**User Story:** As a user browsing log files, I want to see all log files sorted by modification date, so that I can quickly identify the most recent logs.

#### Acceptance Criteria

1. WHEN log files are found in the specified folder THEN the system SHALL display them in a list format
2. WHEN displaying the file list THEN the system SHALL sort files by modification date with the most recent first
3. WHEN displaying each file THEN the system SHALL show the filename and modification timestamp
4. WHEN the file list is displayed THEN the system SHALL highlight the first (most recent) file by default

### Requirement 3

**User Story:** As a user navigating the file list, I want to use vim-style keyboard shortcuts, so that I can efficiently browse through the available log files.

#### Acceptance Criteria

1. WHEN the user presses 'j' THEN the system SHALL move the selection down one file
2. WHEN the user presses 'k' THEN the system SHALL move the selection up one file
3. WHEN the user presses 'Ctrl+d' THEN the system SHALL move the selection down by half a screen
4. WHEN the user presses 'Ctrl+u' THEN the system SHALL move the selection up by half a screen
5. WHEN the user reaches the end of the list and presses 'j' THEN the selection SHALL remain on the last item
6. WHEN the user reaches the beginning of the list and presses 'k' THEN the selection SHALL remain on the first item

### Requirement 4

**User Story:** As a user who has selected a log file, I want to press Enter to load it, so that I can quickly open the file I want to view.

#### Acceptance Criteria

1. WHEN the user presses Enter on a selected file THEN the system SHALL load that log file into the main viewer
2. WHEN a file is successfully loaded THEN the system SHALL transition from file browser mode to log viewing mode
3. WHEN a file fails to load THEN the system SHALL display an error message and remain in file browser mode

### Requirement 5

**User Story:** As an Unreal Engine developer, I want the log viewer to automatically detect the Saved/Logs directory when launched from a project root, so that I can quickly access project logs without specifying the full path.

#### Acceptance Criteria

1. WHEN the user launches the log viewer without arguments from any directory THEN the system SHALL check for a `Saved/Logs` subdirectory
2. WHEN a `Saved/Logs` directory is found THEN the system SHALL automatically use that path for log file browsing
3. WHEN no `Saved/Logs` directory is found THEN the system SHALL display an appropriate message or fall back to current directory behavior
4. WHEN the `Saved/Logs` directory exists but contains no log files THEN the system SHALL display a message indicating no log files were found

### Requirement 6

**User Story:** As a user of the file browser, I want clear visual feedback about the current selection and available actions, so that I understand how to interact with the interface.

#### Acceptance Criteria

1. WHEN the file browser is displayed THEN the system SHALL clearly highlight the currently selected file
2. WHEN the file browser is active THEN the system SHALL display usage instructions for navigation keys
3. WHEN displaying the file list THEN the system SHALL show file sizes and modification dates in a readable format
4. WHEN the user navigates the list THEN the system SHALL provide smooth visual feedback for selection changes