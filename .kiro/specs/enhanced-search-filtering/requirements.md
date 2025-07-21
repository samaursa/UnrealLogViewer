# Requirements Document

## Introduction

This document outlines the requirements for enhancing the search and filtering functionality in the Unreal Log Viewer. The improvements focus on providing quick keyboard shortcuts for column-based filtering, streamlined search-to-filter promotion, and intuitive column numbering for better user experience.

## Essential Development Workflow Requirements

**CRITICAL:** These requirements must be followed in every development conversation to ensure proper workflow:

### Requirement 0: Development and Testing Protocol

**User Story:** As a developer working on this GUI application, I need to follow proper development protocols to avoid wasting time and ensure quality.

#### Acceptance Criteria

1. WHEN starting a new conversation THEN the developer SHALL ask the user what specific task or issue they were working on previously
2. WHEN implementing new features THEN the developer SHALL NOT automatically start the GUI application to test it
3. WHEN testing is needed THEN the developer SHALL understand that only the human user can test the GUI application
4. WHEN adding test coverage THEN the developer MAY add automated tests to the test suite
5. WHEN a feature is implemented THEN the developer SHALL wait for the user to test the application and report results
6. WHEN the user reports issues THEN the developer SHALL focus on fixing those specific issues before moving to new features
7. WHEN building the application THEN the developer SHALL only build to verify compilation, not to run tests

## Requirements

### Requirement 1: Numbered Column Headers with Quick Filter Shortcuts

**User Story:** As a user viewing log entries, I want to quickly create filters based on specific columns using keyboard shortcuts, so that I can rapidly narrow down log entries without navigating through menus.

#### Acceptance Criteria

1. WHEN the log table header is displayed THEN the system SHALL show column numbers in brackets before each column name
2. WHEN column headers are displayed THEN they SHALL follow the format "[^N] ColumnName" where N is the column number
3. WHEN the user presses Ctrl+0 THEN the system SHALL create a filter for the Line column (if line numbers are enabled)
4. WHEN the user presses Ctrl+1 THEN the system SHALL create a filter for the Timestamp column
5. WHEN the user presses Ctrl+2 THEN the system SHALL create a filter for the Frame column
6. WHEN the user presses Ctrl+3 THEN the system SHALL create a filter for the Logger column
7. WHEN the user presses Ctrl+4 THEN the system SHALL create a filter for the Level column
8. WHEN the user presses Ctrl+5 THEN the system SHALL create a filter for the Message column
9. WHEN a quick filter shortcut is pressed THEN the system SHALL open a filter creation dialog pre-populated with the selected column type

### Requirement 2: Enhanced Search-to-Filter Promotion

**User Story:** As a user who has performed a search, I want to easily convert my search into different types of filters, so that I can refine my log viewing without re-typing search terms.

#### Acceptance Criteria

1. WHEN the user has an active search THEN the system SHALL provide numbered options for filter promotion
2. WHEN the search promotion dialog is displayed THEN option 0 SHALL be "Any field contains"
3. WHEN the search promotion dialog is displayed THEN option 1 SHALL be "Message contains"
4. WHEN the search promotion dialog is displayed THEN option 2 SHALL be "Logger contains"
5. WHEN the search promotion dialog is displayed THEN option 3 SHALL be "Level equals"
6. WHEN the user presses the '+' key during search THEN the system SHALL show the search promotion dialog with numbered options
7. WHEN the user presses Ctrl+0 during active search THEN the system SHALL immediately create an "Any field contains" filter
8. WHEN the user presses Ctrl+1 during active search THEN the system SHALL immediately create a "Message contains" filter
9. WHEN the user presses Ctrl+2 during active search THEN the system SHALL immediately create a "Logger contains" filter
10. WHEN the user presses Ctrl+3 during active search THEN the system SHALL immediately create a "Level equals" filter

### Requirement 3: Direct Column Filtering Without Search

**User Story:** As a user examining log entries, I want to quickly create filters for specific columns based on the currently selected entry, so that I can filter by values I can see without having to type them manually.

#### Acceptance Criteria

1. WHEN the user presses Ctrl+1 without active search AND an entry is selected THEN the system SHALL create a "Timestamp after" filter using the selected entry's timestamp
2. WHEN the user presses Ctrl+2 without active search AND an entry is selected THEN the system SHALL create a "Frame after" filter using the selected entry's frame number
3. WHEN the user presses Ctrl+3 without active search AND an entry is selected THEN the system SHALL create a "Logger equals" filter using the selected entry's logger name
4. WHEN the user presses Ctrl+4 without active search AND an entry is selected THEN the system SHALL create a "Level equals" filter using the selected entry's log level
5. WHEN no entry is selected OR the selected entry lacks the required field THEN the system SHALL show an appropriate error message
6. WHEN a direct column filter is created THEN the system SHALL immediately apply the filter and update the display

### Requirement 4: Improved User Interface Feedback

**User Story:** As a user creating filters through keyboard shortcuts, I want clear visual feedback about what filters are being created, so that I understand what filtering actions are being performed.

#### Acceptance Criteria

1. WHEN a filter is created via keyboard shortcut THEN the system SHALL display a status message indicating the filter type and value
2. WHEN a filter creation fails THEN the system SHALL display a clear error message explaining why
3. WHEN the search promotion dialog is shown THEN it SHALL clearly indicate which options correspond to which keyboard shortcuts
4. WHEN column headers are displayed THEN the column numbers SHALL be visually distinct from the column names