# Implementation Plan

- [x] 1. Update column headers to show numbered shortcuts


  - Modify RenderTableHeader() method to include column numbers in format "[^N] ColumnName"
  - Handle dynamic column numbering based on whether line numbers are enabled
  - Ensure proper alignment and visual distinction of column numbers
  - _Requirements: 1.1, 1.2_

- [x] 2. Implement Ctrl+N keyboard event handling


  - Add detection for Ctrl+Number key combinations in MainWindowComponent::OnEvent()
  - Map ASCII control characters (1-6) to column numbers
  - Route events to appropriate handler based on search state (active vs inactive)
  - _Requirements: 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9_

- [x] 3. Create direct column filter functionality for non-search mode


  - Implement CreateDirectColumnFilter() method to handle Ctrl+N when no search is active
  - Add CreateTimestampAfterFilter() method to create timestamp-based filters
  - Add CreateFrameAfterFilter() method to create frame-based filters
  - Add CreateLoggerEqualsFilter() method to create logger-based filters
  - Add CreateLevelEqualsFilter() method to create level-based filters
  - Validate selected entry exists and has required field values
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6_

- [x] 4. Enhance search-to-filter promotion system



  - Implement PromoteSearchToColumnFilter() method for Ctrl+N during active search
  - Update existing search promotion dialog to show numbered options
  - Add immediate filter creation shortcuts (Ctrl+0,1,2,3 during search)
  - Map search promotion options: 0=Any field, 1=Message, 2=Logger, 3=Level
  - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 2.10_

- [ ] 5. Update search promotion dialog rendering
  - Modify RenderSearchPromotionDialog() to show numbered options clearly
  - Add visual indicators for keyboard shortcuts in the dialog
  - Ensure dialog shows current search term and available filter options
  - _Requirements: 2.1, 2.2, 2.3, 2.4, 4.3_

- [ ] 6. Implement enhanced status messaging system
  - Add status messages for successful filter creation via shortcuts
  - Add error messages for invalid filter creation attempts
  - Update status bar to show filter creation feedback
  - Ensure messages are clear and informative about the action performed
  - _Requirements: 4.1, 4.2, 3.5_

- [ ] 7. Add input validation and error handling
  - Validate selected entry exists before creating direct column filters
  - Check that selected entry has required field values (timestamp, frame, etc.)
  - Handle cases where fields are missing or invalid
  - Provide helpful error messages for each failure case
  - _Requirements: 3.5, 4.2_

- [ ] 8. Update help system with new keyboard shortcuts
  - Add documentation for Ctrl+0-5 shortcuts in help dialog
  - Explain different behavior based on search state
  - Document column numbering system
  - _Requirements: 4.4_

- [ ] 9. Test and validate all keyboard shortcuts
  - Test Ctrl+N shortcuts work correctly in both search and non-search modes
  - Verify column numbering updates correctly when line numbers are toggled
  - Test error handling for invalid operations
  - Validate filter creation and application works properly
  - _Requirements: All requirements validation_