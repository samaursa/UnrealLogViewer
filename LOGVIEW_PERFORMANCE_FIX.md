# Log View Performance Fix

## Problem
The log view was constrained to a limited number of rows in height and had performance issues when trying to render all log entries at once. This caused:
1. Poor performance with large log files
2. Inability to scroll properly
3. Inefficient use of screen space

## Solution
Implemented a windowed rendering approach with buffer zones that only renders entries that are likely to be visible:

1. **Smart Windowing with Buffer**:
   - Render entries around the selected entry with a buffer zone for smooth scrolling
   - Add visual indicators when there are more entries above or below
   - Maintain proper scrolling functionality

2. **Dynamic Viewport Calculation**:
   - Added consistent calculation of visible height based on available screen space
   - Created a reusable `GetVisibleHeight()` method for consistent calculations across the application
   - Properly accounts for detail view size (5 lines when unfocused, 2/3 of screen when focused)

3. **Improved Scrolling**:
   - Updated `EnsureSelectionVisible()` to position the selected entry with context
   - Added logic to position the selected entry 1/4 or 3/4 of the way down for better context
   - Ensured all navigation methods work with the new approach

## Benefits
1. **Improved Performance**: Only rendering visible entries plus a buffer significantly reduces CPU and memory usage
2. **Proper Scrolling**: Scrolling now works correctly with large log files
3. **Full Screen Utilization**: The log view now properly uses all available screen space
4. **Better Context**: The selected entry is shown with context above and below
5. **Visual Indicators**: Users can see when there are more entries above or below

## Files Modified
- `lib/ui/main_window.cpp`
  - Updated `RenderLogTable()` to use smart windowing with buffer
  - Updated `GetVisibleHeight()` to calculate viewport size consistently
  - Updated `EnsureSelectionVisible()` to position entries with context