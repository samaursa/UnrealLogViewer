# Requirements Document

## Introduction

This document outlines the requirements for improving the search and filtering user experience in the Unreal Log Viewer. The improvements focus on providing quick keyboard shortcuts for column-based filtering, streamlined search-to-filter promotion, and intuitive column numbering for better user workflow.

## Requirements

### Requirement 1: Numbered Column Headers with Quick Filter Shortcuts

**User Story:** As a user viewing log entries, I want to see numbered column headers and use keyboard shortcuts to quickly create filters on specific columns, so that I can rapidly filter data without navigating through menus.

#### Acceptance Criteria

1. WHEN the log table header is displayed THEN each column SHALL show a number prefix in format "[^N] ColumnName"
2. WHEN the user presses CTRL+0 THEN the system SHALL create a line number filter dialog
3. WHEN the user presses CTRL+1 THEN the system SHALL create a timestamp filter dialog
4. WHEN the user presses CTRL+2 THEN the system SHALL create a frame number filter dialog
5. WHEN the user presses CTRL+3 THEN the system SHALL create a logger name filter dialog
6. WHEN the user presses CTRL+4 THEN the system SHALL create a log level filter dialog
7. WHEN a column filter dialog is shown THEN the user SHALL be able to enter filter criteria and press Enter to apply
8. WHEN a column filter dialog is shown THEN the user SHALL be able to press Escape to cancel

### Requirement 2: Enhanced Search-to-Filter Promotion

**User Story:** As a user who has performed a search, I want quick ways to convert my search into permanent filters, so that I can build complex filtering criteria efficiently.

#### Acceptance Criteria

1. WHEN the user has an active search THEN pressing '+' SHALL show a simplified filter promotion dialog
2. WHEN the filter promotion dialog is shown THEN pressing '0' SHALL create an "Any field contains" filter with the search term
3. WHEN the filter promotion dialog is shown THEN pressing '1' SHALL create a "Timestamp contains" filter with the search term
4. WHEN the filter promotion dialog is shown THEN pressing '2' SHALL create a "Frame equals" filter with the search term (if numeric)
5. WHEN the filter promotion dialog is shown THEN pressing '3' SHALL create a "Logger contains" filter with the search term
6. WHEN the filter promotion dialog is shown THEN pressing '4' SHALL create a "Log level equals" filter with the search term
7. WHEN the filter promotion dialog is shown THEN pressing '5' SHALL create a "Message contains" filter with the search term
8. WHEN a search-to-filter promotion is applied THEN the search SHALL remain active for further navigation

### Requirement 3: Direct Column Filtering Without Search

**User Story:** As a user wanting to filter specific columns, I want to directly create column filters without first performing a search, so that I can quickly narrow down log entries based on known criteria.

#### Acceptance Criteria

1. WHEN the user presses CTRL+0 without active search THEN the system SHALL prompt for line number criteria
2. WHEN the user presses CTRL+1 without active search THEN the system SHALL prompt for timestamp criteria
3. WHEN the user presses CTRL+2 without active search THEN the system SHALL prompt for frame number criteria
4. WHEN the user presses CTRL+3 without active search THEN the system SHALL prompt for logger name criteria
5. WHEN the user presses CTRL+4 without active search THEN the system SHALL prompt for log level criteria
6. WHEN a direct column filter prompt is shown THEN the user SHALL be able to enter filter criteria
7. WHEN a direct column filter is applied THEN it SHALL be added to the active filter list
8. WHEN multiple column filters are applied THEN they SHALL be combined with AND logic

### Requirement 4: Improved Filter Dialog User Experience

**User Story:** As a user creating filters, I want clear and consistent filter dialogs that show what type of filter I'm creating, so that I can understand and manage my filtering criteria effectively.

#### Acceptance Criteria

1. WHEN a column filter dialog is displayed THEN it SHALL show the column name and filter type clearly
2. WHEN a filter dialog is displayed THEN it SHALL show example input formats where applicable
3. WHEN a filter is successfully created THEN the system SHALL show confirmation in the status bar
4. WHEN a filter creation fails THEN the system SHALL show a clear error message
5. WHEN multiple filters are active THEN the status bar SHALL indicate the number of active filters
6. WHEN filters are applied THEN the log table SHALL update immediately to show filtered results