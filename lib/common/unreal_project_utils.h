#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace ue_log {
namespace unreal_utils {

/**
 * Find the Saved/Logs directory relative to the current working directory.
 * @return Path to Saved/Logs if found, empty string otherwise.
 */
std::string FindSavedLogsDirectory();

/**
 * Find the Saved/Logs directory relative to a specific directory.
 * @param base_directory Directory to search from
 * @return Path to Saved/Logs if found, empty string otherwise.
 */
std::string FindSavedLogsDirectory(const std::string& base_directory);

/**
 * Check if a directory contains Unreal log files.
 * @param directory_path Path to check
 * @return True if directory contains *.log files
 */
bool ContainsLogFiles(const std::string& directory_path);

/**
 * Get all log files in a directory, sorted by modification time.
 * @param directory_path Directory to scan
 * @return Vector of file paths sorted by modification time (newest first)
 */
std::vector<std::string> GetLogFiles(const std::string& directory_path);

/**
 * Validate that a directory path exists and is accessible.
 * @param directory_path Path to validate
 * @return True if directory exists and is accessible
 */
bool ValidateDirectoryPath(const std::string& directory_path);

/**
 * Get file metadata for a specific file.
 * @param file_path Path to the file
 * @return Tuple containing (file_size, modification_time) or empty values if file doesn't exist
 */
std::pair<std::uintmax_t, std::filesystem::file_time_type> GetFileMetadata(const std::string& file_path);

} // namespace unreal_utils
} // namespace ue_log