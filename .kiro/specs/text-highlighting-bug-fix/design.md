# Text Highlighting Bug Fix Design

## Overview

This design addresses critical bugs in the text highlighting system where characters are being consumed or duplicated during the highlighting process. The root cause analysis reveals issues in string position calculations and length mismatches between case-sensitive and case-insensitive search operations.

## Root Cause Analysis

### Character Consumption Bug

**Problem:** In `RenderMessageWithSearchHighlight`, when performing case-insensitive search:
1. Match positions are found using lowercase transformed text (`search_text`)
2. But highlighted text is extracted using `search_query.length()` instead of `query.length()`
3. This causes length mismatches when the original and transformed queries differ

**Example:**
- Original text: "Property ArrayProperty FCk_Ability_OtherAbilitySettings"
- Search query: "Ability" (case-insensitive)
- Transformed query: "ability" 
- Match found at position X in lowercase text
- But extraction uses original query length, causing mismatch

### Filter Description Duplication Bug

**Problem:** Filter criteria may be getting processed multiple times or concatenated incorrectly during filter creation or display.

**Investigation needed:** Check filter creation pipeline for string duplication.

## Architecture

### Text Highlighting Pipeline

```
Input Text + Search Query
    ↓
Case Sensitivity Check
    ↓
Position Finding (using transformed text if needed)
    ↓
Text Extraction (using original text + correct lengths)
    ↓
Element Creation with Highlighting
    ↓
Final Rendered Output
```

### Key Components

1. **LogEntryRenderer::RenderMessageWithSearchHighlight**
   - Primary highlighting logic
   - Must handle case sensitivity correctly
   - Must preserve original text integrity

2. **Filter Creation Pipeline**
   - Filter name generation
   - Criteria storage and retrieval
   - Display formatting

## Implementation Strategy

### Phase 1: Fix Character Consumption

1. **Correct Length Calculation**
   - Use `query.length()` consistently for text extraction
   - Ensure position calculations match extraction lengths
   - Validate that `last_pos` tracking is accurate

2. **Improve Case Handling**
   - Separate position finding from text extraction
   - Always extract from original text using original positions
   - Ensure transformed text is only used for matching, not extraction

### Phase 2: Fix Filter Description Issues

1. **Audit Filter Creation**
   - Review all filter creation methods
   - Check for string concatenation issues
   - Validate filter name generation logic

2. **Standardize Filter Display**
   - Ensure consistent formatting across all filter types
   - Validate criteria storage and retrieval
   - Check for any string processing that might cause duplication

### Phase 3: Add Comprehensive Testing

1. **Unit Tests for Highlighting**
   - Test case-sensitive and case-insensitive scenarios
   - Test multiple matches in single text
   - Test edge cases (empty strings, special characters)

2. **Integration Tests for Filters**
   - Test filter creation from search promotion
   - Test filter display and description accuracy
   - Test filter highlighting functionality

## Error Handling

### Text Processing Safeguards

1. **Bounds Checking**
   - Validate all string positions before extraction
   - Ensure substring operations don't exceed string bounds
   - Handle empty or null strings gracefully

2. **Length Validation**
   - Verify match positions correspond to actual text
   - Ensure extraction lengths match expected query lengths
   - Validate that text segments don't overlap incorrectly

### Filter Creation Safeguards

1. **Input Validation**
   - Sanitize search terms before filter creation
   - Validate filter criteria before storage
   - Check for duplicate or malformed filter names

2. **Display Validation**
   - Ensure filter descriptions match actual criteria
   - Validate formatting consistency
   - Handle special characters in filter names

## Testing Strategy

### Highlighting Tests

```cpp
// Test case-insensitive highlighting
TEST_CASE("Case insensitive highlighting preserves original text") {
    std::string original = "Property ArrayProperty FCk_Ability_OtherAbilitySettings";
    std::string query = "Ability";
    // Verify highlighted result maintains original text integrity
}

// Test multiple matches
TEST_CASE("Multiple matches highlight correctly") {
    std::string original = "Ability test Ability again";
    std::string query = "Ability";
    // Verify both matches are highlighted without corruption
}
```

### Filter Tests

```cpp
// Test filter creation accuracy
TEST_CASE("Filter promotion creates correct description") {
    // Create search for "Ability"
    // Promote to filter
    // Verify filter description is "Message contains: Ability"
}
```

## Performance Considerations

### Optimization Opportunities

1. **Reduce String Transformations**
   - Cache transformed strings when possible
   - Minimize repeated case conversions
   - Use string views where appropriate

2. **Efficient Position Tracking**
   - Use iterators instead of repeated substring operations
   - Batch position calculations
   - Avoid redundant string operations

## Backward Compatibility

### Existing Functionality

- All existing search and filter functionality must continue to work
- Visual appearance should remain consistent (except for bug fixes)
- Keyboard shortcuts and user interactions unchanged
- Filter panel behavior preserved

### Migration Strategy

- Changes are primarily internal bug fixes
- No user-facing API changes required
- Existing filters should continue to work correctly
- No data migration needed