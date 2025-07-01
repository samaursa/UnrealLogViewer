#pragma once
#include <string>
#include <regex>

struct LogEntry {
    std::string timestamp;
    std::string frame;
    std::string category;
    std::string level;
    std::string message;
    std::string raw_line;
};

class UnrealLogParser {
public:
    LogEntry ParseLine(const std::string& line);

private:
    bool TryParseStandardFormat(const std::string& line, LogEntry& entry);
    bool TryParseAlternateFormat(const std::string& line, LogEntry& entry);
    bool TryParseSimpleFormat(const std::string& line, LogEntry& entry);
    void ExtractCategoryFromMessage(const std::string& line, LogEntry& entry);
};