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

/**
 * Validate that a directory path exists and is accessible, with detailed error information.
 * @param directory_path Path to validate
 * @return Pair of (success, error_message). If success is true, error_message contains status info.
 */
std::pair<bool, std::string> ValidateDirectoryPathWithError(const std::string& directory_path);

/**
 * Get all log files in a directory with detailed error information.
 * @param directory_path Directory to scan
 * @return Pair of (file_paths, status_message). file_paths is empty on error.
 */
std::pair<std::vector<std::string>, std::string> GetLogFilesWithError(const std::string& directory_path);

/**
 * Find the Saved/Logs directory with detailed error information.
 * @return Pair of (directory_path, status_message). directory_path is empty if not found.
 */
std::pair<std::string, std::string> FindSavedLogsDirectoryWithError();

/**
 * Find the Saved/Logs directory relative to a specific directory with detailed error information.
 * @param base_directory Directory to search from
 * @return Pair of (directory_path, status_message). directory_path is empty if not found.
 */
std::pair<std::string, std::string> FindSavedLogsDirectoryWithError(const std::string& base_directory);

} // namespace unreal_utils
} // namespace ue_log