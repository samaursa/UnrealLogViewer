#pragma once

#include <vector>
#include <string>

namespace ue_log {

/**
 * System for managing vim-style relative line numbers and navigation.
 * Provides relative line number calculation and vim-style jump command parsing.
 */
class RelativeLineNumberSystem {
public:
    /**
     * Information about a line number display.
     */
    struct LineNumberInfo {
        int absolute_number;     // The actual line number in the full log
        int relative_number;     // The relative number to display (0 for current, +/- for others)
        bool is_current_line;    // True if this is the currently selected line
        
        LineNumberInfo(int abs_num, int rel_num, bool is_current)
            : absolute_number(abs_num), relative_number(rel_num), is_current_line(is_current) {}
    };
    
    /**
     * Calculate relative line numbers for visible entries.
     * @param current_selection The index of the currently selected entry (0-based)
     * @param visible_start The index of the first visible entry (0-based)
     * @param visible_count The number of visible entries
     * @return Vector of LineNumberInfo for each visible entry
     */
    std::vector<LineNumberInfo> CalculateRelativeNumbers(
        int current_selection, int visible_start, int visible_count) const;
    
    /**
     * Parse vim-style navigation input and extract jump information.
     * Supports formats like "5j", "10k", "j", "k"
     * @param input The input string to parse
     * @param jump_distance Output parameter for the number of lines to jump
     * @param direction Output parameter for the direction ('j' for down, 'k' for up)
     * @return True if the input was successfully parsed as a navigation command
     */
    bool HandleNavigationInput(const std::string& input, int& jump_distance, char& direction) const;
    
    /**
     * Format a relative line number for display.
     * @param relative_number The relative line number (0 for current, +/- for others)
     * @param is_current_line True if this is the currently selected line
     * @param width The desired width for the formatted string
     * @return Formatted string for display
     */
    std::string FormatRelativeNumber(int relative_number, bool is_current_line, int width = 4) const;
    
private:
    /**
     * Parse a jump command string into count and command character.
     * @param input The input string (e.g., "5j", "k", "10k")
     * @param count Output parameter for the numeric count (defaults to 1)
     * @param command Output parameter for the command character ('j' or 'k')
     * @return True if parsing was successful
     */
    bool ParseJumpCommand(const std::string& input, int& count, char& command) const;
};

} // namespace ue_log