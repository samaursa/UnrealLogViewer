# Requirements Document

## Introduction

This document outlines two minor but important user experience improvements for the Unreal Log Viewer: enhanced Unreal project detection when launching with folder arguments, and three-state filter functionality for more flexible log filtering.

## Requirements

### Requirement 1: Enhanced Unreal Project Detection

**User Story:** As an Unreal Engine developer, I want the log viewer to automatically detect and use the Saved/Logs directory when I launch it with any folder argument that might be an Unreal project root, so that I don't have to navigate to the logs directory manually.

#### Acceptance Criteria

1. WHEN the user launches the log viewer with a folder argument THEN the system SHALL check if that folder contains a `Saved/Logs` subdirectory
2. WHEN a `Saved/Logs` subdirectory is found in the provided folder THEN the system SHALL automatically use `<provided_folder>/Saved/Logs` as the target directory for log browsing
3. WHEN no `Saved/Logs` subdirectory is found THEN the system SHALL use the provided folder directly as originally specified
4. WHEN the `Saved/Logs` directory exists but is empty or contains no log files THEN the system SHALL display an appropriate message indicating no log files were found
5. WHEN the `Saved/Logs` directory is found and contains log files THEN the system SHALL proceed with normal file browsing behavior

### Requirement 2: Three-State Filter System

**User Story:** As a user analyzing log files, I want filters to support three states (include, exclude, disabled) so that I can both include specific content and exclude unwanted content for more precise log filtering.

#### Acceptance Criteria

1. WHEN a filter is created THEN it SHALL default to the "include" state (ON - green with checkmark)
2. WHEN the user clicks on an active "include" filter THEN it SHALL change to the "exclude" state (INVERTED - red with negative sign)
3. WHEN the user clicks on an active "exclude" filter THEN it SHALL change to the "disabled" state (OFF - greyed out)
4. WHEN the user clicks on a disabled filter THEN it SHALL change to the "include" state (ON - green with checkmark)
5. WHEN a filter is in "include" state THEN the system SHALL show only log entries that match the filter criteria
6. WHEN a filter is in "exclude" state THEN the system SHALL hide all log entries that match the filter criteria
7. WHEN a filter is in "disabled" state THEN the system SHALL ignore the filter criteria completely
8. WHEN multiple filters are active THEN the system SHALL apply all "include" filters as AND conditions and all "exclude" filters as NOT conditions
9. WHEN displaying filters THEN the system SHALL use distinct visual indicators: green checkmark for include, red negative sign for exclude, greyed out appearance for disabled
10. WHEN the user hovers over a filter THEN the system SHALL show a tooltip indicating the current state and what clicking will do next