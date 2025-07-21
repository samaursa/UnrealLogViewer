#include "relative_line_number_system.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace ue_log {

std::vector<RelativeLineNumberSystem::LineNumberInfo> 
RelativeLineNumberSystem::CalculateRelativeNumbers(
    int current_selection, int visible_start, int visible_count) const {
    
    std::vector<LineNumberInfo> result;
    result.reserve(visible_count);
    
    // Calculate relative numbers for each visible entry
    for (int i = 0; i < visible_count; ++i) {
        int absolute_index = visible_start + i;
        int relative_number = absolute_index - current_selection;
        bool is_current = (absolute_index == current_selection);
        
        result.emplace_back(absolute_index, relative_number, is_current);
    }
    
    return result;
}

bool RelativeLineNumberSystem::HandleNavigationInput(
    const std::string& input, int& jump_distance, char& direction) const {
    
    if (input.empty()) {
        return false;
    }
    
    return ParseJumpCommand(input, jump_distance, direction);
}

std::string RelativeLineNumberSystem::FormatRelativeNumber(
    int relative_number, bool is_current_line, int width) const {
    
    std::ostringstream oss;
    
    if (is_current_line) {
        // Current line shows as "0" right-aligned
        oss << std::setw(width) << "0";
    } else if (relative_number > 0) {
        // Lines below current selection show positive numbers (no + sign needed for vim users)
        oss << std::setw(width) << relative_number;
    } else {
        // Lines above current selection show absolute value (no - sign needed for vim users)
        oss << std::setw(width) << std::abs(relative_number);
    }
    
    return oss.str();
}

bool RelativeLineNumberSystem::ParseJumpCommand(
    const std::string& input, int& count, char& command) const {
    
    if (input.empty()) {
        return false;
    }
    
    // Initialize defaults
    count = 1;
    command = '\0';
    
    // Check if the last character is a valid command
    char last_char = input.back();
    if (last_char != 'j' && last_char != 'k') {
        return false;
    }
    
    command = last_char;
    
    // If input is just the command character, count is 1
    if (input.length() == 1) {
        count = 1;
        return true;
    }
    
    // Extract the numeric part (everything except the last character)
    std::string numeric_part = input.substr(0, input.length() - 1);
    
    // Validate that the numeric part contains only digits
    for (char c : numeric_part) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    
    // Parse the count
    try {
        count = std::stoi(numeric_part);
        if (count <= 0) {
            return false; // Count must be positive
        }
    } catch (const std::exception&) {
        return false;
    }
    
    return true;
}

} // namespace ue_log