# Text Highlighting Bug Fix Implementation Plan

## Task Overview

This implementation plan addresses critical bugs in text highlighting where characters are consumed/corrupted and filter descriptions show duplicated text. The tasks are organized to fix the most critical issues first, then add comprehensive testing.

## Implementation Tasks

- [x] 1. Fix character consumption in text highlighting





  - Correct the length calculation bug in `RenderMessageWithSearchHighlight`
  - Ensure case-insensitive search uses correct string lengths for extraction
  - Validate that text before and after highlights remains intact
  - Test with the specific example: "Property ArrayProperty FCk_Ability_OtherAbilitySettings" + "Ability"
  - _Requirements: 1.1, 1.2, 1.3, 1.4_



- [ ] 2. Investigate and fix filter description duplication
  - Audit the filter creation pipeline for string duplication issues
  - Check `CreateFilterFromSearchAndColumn` method for concatenation problems
  - Verify filter name generation in `PromoteSearchToColumnFilter`
  - Ensure filter criteria storage and retrieval is accurate
  - _Requirements: 2.1, 2.2, 2.3, 2.4_

- [ ] 3. Improve case-insensitive search logic
  - Separate position finding from text extraction in highlighting
  - Always extract highlighted text from original string using correct positions
  - Ensure transformed text is only used for matching, not for extraction
  - Add bounds checking for all string operations
  - _Requirements: 1.2, 4.1, 4.2, 4.3_

- [ ] 4. Add comprehensive highlighting tests
  - Create unit tests for case-sensitive and case-insensitive highlighting
  - Test multiple matches in single text line
  - Test edge cases: empty strings, special characters, Unicode
  - Add regression tests for the specific reported bugs
  - _Requirements: 4.4, 4.5_

- [ ] 5. Add filter creation and display tests
  - Test search promotion to filter accuracy
  - Verify filter descriptions match actual criteria
  - Test filter highlighting functionality end-to-end
  - Add tests for all filter types (TextContains, LoggerName, LogLevel)
  - _Requirements: 2.1, 2.2, 2.3, 3.1, 3.2_

- [ ] 6. Validate highlighting consistency across features
  - Ensure search highlighting and filter highlighting work consistently
  - Test priority system (search vs filter highlighting)
  - Verify visual indicators work correctly for both types
  - Test switching between different highlighting modes
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5_

- [ ] 7. Add error handling and safeguards
  - Add bounds checking for all string operations
  - Handle empty or null strings gracefully
  - Add input validation for filter creation
  - Ensure robust handling of malformed search queries
  - _Requirements: 4.5_

- [ ] 8. Performance optimization and cleanup
  - Reduce redundant string transformations
  - Cache transformed strings where appropriate
  - Use string views for read-only operations
  - Profile highlighting performance with large log files
  - _Requirements: Performance considerations from design_

## Testing Strategy

### Unit Tests
- `test_highlighting_character_preservation.cpp` - Test character integrity
- `test_case_insensitive_highlighting.cpp` - Test case handling
- `test_filter_description_accuracy.cpp` - Test filter naming
- `test_multiple_match_highlighting.cpp` - Test multiple highlights

### Integration Tests  
- `test_search_to_filter_promotion.cpp` - End-to-end search promotion
- `test_highlighting_priority_system.cpp` - Test search vs filter priority
- `test_filter_panel_integration.cpp` - Test filter panel display

### Regression Tests
- Specific test for "Property ArrayProperty FCk_Ability_OtherAbilitySettings" + "Ability"
- Test for "Message contains: AbAbility" duplication bug
- Edge cases that previously caused crashes or corruption

## Success Criteria

### Character Preservation
- Text "Property ArrayProperty FCk_Ability_OtherAbilitySettings" with search "Ability" displays correctly
- No characters are missing or corrupted in highlighted text
- Case-insensitive search preserves original text case

### Filter Accuracy  
- Search promotion creates filters with correct descriptions
- Filter criteria exactly matches search terms
- No duplication in filter names or descriptions

### System Stability
- All existing functionality continues to work
- No performance regression in highlighting
- Robust error handling for edge cases

## Risk Mitigation

### Backward Compatibility
- All changes are internal bug fixes
- Existing user workflows remain unchanged
- Filter data remains compatible

### Testing Coverage
- Comprehensive test suite prevents regression
- Edge cases are explicitly tested
- Performance impact is measured and validated

### Rollback Plan
- Changes are isolated to specific methods
- Original logic can be restored if needed
- Tests validate that fixes don't break other functionality