# Text Highlighting Bug Fix Requirements

## Introduction

This spec addresses critical bugs in the text highlighting functionality that cause character corruption and incorrect filter descriptions. The highlighting system currently has issues where characters are consumed or duplicated during the highlighting process, leading to broken text display and confusing filter names.

## Requirements

### Requirement 1: Fix Character Consumption in Text Highlighting

**User Story:** As a user searching for text in log entries, I want the highlighted text to display correctly without missing or corrupted characters, so that I can read the log entries properly.

#### Acceptance Criteria

1. WHEN I search for "Ability" in text "Property ArrayProperty FCk_Ability_OtherAbilitySettings" THEN the result SHALL display "Property ArrayProperty FCk_**Ability**_OtherAbilitySettings" with correct highlighting
2. WHEN I perform case-insensitive search THEN the original text SHALL be preserved exactly with only the matched portion highlighted
3. WHEN multiple matches exist in a single line THEN all matches SHALL be highlighted correctly without character loss
4. WHEN highlighting is applied THEN the text before and after highlights SHALL remain intact
5. WHEN word wrapping is enabled THEN highlighted text SHALL wrap correctly without character corruption

### Requirement 2: Fix Filter Description Duplication

**User Story:** As a user creating filters from search terms, I want the filter descriptions to show the correct search term without duplication, so that I can understand what the filter is doing.

#### Acceptance Criteria

1. WHEN I promote a search for "Ability" to a filter THEN the filter description SHALL read "Message contains: Ability" not "Message contains: AbAbility"
2. WHEN I create any text-based filter THEN the filter criteria SHALL match exactly what was searched for
3. WHEN I view filter details THEN the filter name and criteria SHALL be consistent and accurate
4. WHEN multiple filters are created THEN each SHALL have the correct, non-duplicated criteria
5. WHEN filters are displayed in the filter panel THEN the descriptions SHALL be clear and accurate

### Requirement 3: Ensure Highlighting Consistency Across Features

**User Story:** As a user using both search highlighting and filter highlighting, I want both features to work consistently without text corruption, so that I can rely on the visual feedback.

#### Acceptance Criteria

1. WHEN I use search highlighting THEN text SHALL be displayed correctly without corruption
2. WHEN I use filter highlighting THEN text SHALL be displayed correctly without corruption  
3. WHEN I switch between search and filter highlighting THEN both SHALL work consistently
4. WHEN I have both active search and selected filter THEN the higher priority highlighting SHALL work correctly
5. WHEN highlighting is disabled THEN original text SHALL be displayed without any artifacts

### Requirement 4: Validate Text Processing Logic

**User Story:** As a developer maintaining the highlighting system, I want the text processing logic to be robust and well-tested, so that highlighting bugs don't reoccur.

#### Acceptance Criteria

1. WHEN case-insensitive matching is performed THEN the original text case SHALL be preserved in display
2. WHEN search positions are calculated THEN they SHALL correspond exactly to the original text positions
3. WHEN text segments are extracted THEN the lengths SHALL match the search query length exactly
4. WHEN multiple highlighting passes occur THEN text integrity SHALL be maintained
5. WHEN edge cases occur (empty strings, special characters) THEN the system SHALL handle them gracefully