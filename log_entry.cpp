#include "log_entry.h"

LogEntry UnrealLogParser::ParseLine(const std::string& line) {
    LogEntry entry;
    entry.raw_line = line;

    if (TryParseStandardFormat(line, entry) ||
        TryParseAlternateFormat(line, entry) ||
        TryParseSimpleFormat(line, entry)) {
        return entry;
    }

    ExtractCategoryFromMessage(line, entry);
    return entry;
}

bool UnrealLogParser::TryParseStandardFormat(const std::string& line, LogEntry& entry) {
    // Pattern: [2024.06.30-12:34:56:789][123]LogTemp: Display: Message here
    std::regex pattern(R"(\[([^\]]+)\]\[\s*(\d*)\]([^:]+):\s*([^:]*?):\s*(.*))", std::regex_constants::optimize);
    std::smatch match;

    if (std::regex_match(line, match, pattern) && match.size() >= 6) {
        entry.timestamp = match[1].str();
        entry.frame = match[2].str();
        entry.category = match[3].str();
        entry.level = match[4].str();
        entry.message = match[5].str();
        return true;
    }
    return false;
}

bool UnrealLogParser::TryParseAlternateFormat(const std::string& line, LogEntry& entry) {
    // Pattern: [2024.06.30-12:34:56:789]LogTemp: Display: Message
    std::regex pattern(R"(\[([^\]]+)\]([^:]+):\s*([^:]*?):\s*(.*))", std::regex_constants::optimize);
    std::smatch match;

    if (std::regex_match(line, match, pattern) && match.size() >= 5) {
        entry.timestamp = match[1].str();
        entry.category = match[2].str();
        entry.level = match[3].str();
        entry.message = match[4].str();
        return true;
    }
    return false;
}

bool UnrealLogParser::TryParseSimpleFormat(const std::string& line, LogEntry& entry) {
    // Pattern: LogTemp: Message (no level)
    std::regex pattern(R"(^([a-zA-Z][a-zA-Z0-9_]*)\s*:\s*(.*))", std::regex_constants::optimize);
    std::smatch match;

    if (std::regex_match(line, match, pattern) && match.size() >= 3) {
        entry.category = match[1].str();
        entry.level = ""; // No level for this format
        entry.message = match[2].str();
        return true;
    }
    return false;
}

void UnrealLogParser::ExtractCategoryFromMessage(const std::string& line, LogEntry& entry) {
    // Try to find common log patterns at start of line
    std::regex category_pattern(R"(^([A-Za-z][A-Za-z0-9_]*)\s+(.*))", std::regex_constants::optimize);
    std::smatch match;

    if (std::regex_match(line, match, category_pattern) && match.size() >= 3) {
        std::string potential_category = match[1].str();

        // Check if it looks like a log category
        if (potential_category.find("Log") == 0 || potential_category.length() > 4) {
            entry.category = potential_category;
            entry.message = match[2].str();
            return;
        }
    }

    // Final fallback
    entry.message = line;
    entry.category = "Unknown";
}