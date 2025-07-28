#include "unreal_project_utils.h"
#include <filesystem>
#include <algorithm>
#include <system_error>

namespace ue_log {
namespace unreal_utils {

std::string FindSavedLogsDirectory() {
    try {
        return FindSavedLogsDirectory(std::filesystem::current_path().string());
    } catch (const std::filesystem::filesystem_error&) {
        return "";
    }
}

std::string FindSavedLogsDirectory(const std::string& base_directory) {
    try {
        std::filesystem::path base_path(base_directory);
        
        // Check if the base directory itself is a Saved/Logs directory
        if (base_path.filename() == "Logs" && base_path.parent_path().filename() == "Saved") {
            if (std::filesystem::exists(base_path) && std::filesystem::is_directory(base_path)) {
                return base_path.string();
            }
        }
        
        // Look for Saved/Logs subdirectory
        std::filesystem::path saved_logs_path = base_path / "Saved" / "Logs";
        if (std::filesystem::exists(saved_logs_path) && std::filesystem::is_directory(saved_logs_path)) {
            return saved_logs_path.string();
        }
        
        // Look for just Logs subdirectory
        std::filesystem::path logs_path = base_path / "Logs";
        if (std::filesystem::exists(logs_path) && std::filesystem::is_directory(logs_path)) {
            return logs_path.string();
        }
        
        return "";
    } catch (const std::filesystem::filesystem_error&) {
        return "";
    }
}

bool ContainsLogFiles(const std::string& directory_path) {
    try {
        if (!ValidateDirectoryPath(directory_path)) {
            return false;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                return true;
            }
        }
        return false;
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

std::vector<std::string> GetLogFiles(const std::string& directory_path) {
    std::vector<std::pair<std::string, std::filesystem::file_time_type>> files_with_time;
    
    try {
        if (!ValidateDirectoryPath(directory_path)) {
            return {};
        }
        
        // Collect all .log files with their modification times
        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                try {
                    auto mod_time = entry.last_write_time();
                    files_with_time.emplace_back(entry.path().string(), mod_time);
                } catch (const std::filesystem::filesystem_error&) {
                    // Skip files we can't get modification time for
                    continue;
                }
            }
        }
        
        // Sort by modification time (newest first)
        std::sort(files_with_time.begin(), files_with_time.end(),
                  [](const auto& a, const auto& b) {
                      return a.second > b.second;
                  });
        
        // Extract just the file paths
        std::vector<std::string> result;
        result.reserve(files_with_time.size());
        for (const auto& file_info : files_with_time) {
            result.push_back(file_info.first);
        }
        
        return result;
    } catch (const std::filesystem::filesystem_error&) {
        return {};
    }
}

bool ValidateDirectoryPath(const std::string& directory_path) {
    try {
        std::filesystem::path path(directory_path);
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

std::pair<std::uintmax_t, std::filesystem::file_time_type> GetFileMetadata(const std::string& file_path) {
    try {
        std::filesystem::path path(file_path);
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
            return {0, std::filesystem::file_time_type{}};
        }
        
        auto size = std::filesystem::file_size(path);
        auto mod_time = std::filesystem::last_write_time(path);
        
        return {size, mod_time};
    } catch (const std::filesystem::filesystem_error&) {
        return {0, std::filesystem::file_time_type{}};
    }
}

std::pair<bool, std::string> ValidateDirectoryPathWithError(const std::string& directory_path) {
    try {
        std::filesystem::path path(directory_path);
        
        // Check if path exists
        std::error_code ec;
        if (!std::filesystem::exists(path, ec)) {
            if (ec) {
                return {false, "Cannot access path: " + ec.message()};
            }
            return {false, "Directory does not exist: " + directory_path};
        }
        
        // Check if it's a directory
        if (!std::filesystem::is_directory(path, ec)) {
            if (ec) {
                return {false, "Cannot determine if path is directory: " + ec.message()};
            }
            return {false, "Path is not a directory: " + directory_path};
        }
        
        // Try to iterate to check permissions
        auto dir_iter = std::filesystem::directory_iterator(path, ec);
        if (ec) {
            if (ec == std::errc::permission_denied) {
                return {false, "Permission denied accessing directory: " + directory_path};
            }
            return {false, "Cannot access directory: " + ec.message()};
        }
        
        return {true, "Directory is valid and accessible"};
        
    } catch (const std::filesystem::filesystem_error& e) {
        return {false, "Filesystem error: " + std::string(e.what())};
    } catch (const std::exception& e) {
        return {false, "Unexpected error: " + std::string(e.what())};
    }
}

std::pair<std::vector<std::string>, std::string> GetLogFilesWithError(const std::string& directory_path) {
    std::vector<std::pair<std::string, std::filesystem::file_time_type>> files_with_time;
    
    try {
        // First validate the directory
        auto [is_valid, error_msg] = ValidateDirectoryPathWithError(directory_path);
        if (!is_valid) {
            return {{}, error_msg};
        }
        
        // Count total files for better feedback
        int total_files = 0;
        int log_files = 0;
        
        // Collect all .log files with their modification times
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(directory_path, ec)) {
            if (ec) {
                return {{}, "Error reading directory contents: " + ec.message()};
            }
            
            total_files++;
            
            std::error_code entry_ec;
            if (entry.is_regular_file(entry_ec) && !entry_ec) {
                if (entry.path().extension() == ".log") {
                    try {
                        auto mod_time = entry.last_write_time();
                        files_with_time.emplace_back(entry.path().string(), mod_time);
                        log_files++;
                    } catch (const std::filesystem::filesystem_error&) {
                        // Skip files we can't get modification time for, but continue
                        continue;
                    }
                }
            }
        }
        
        // Provide detailed feedback about what was found
        std::string status_msg;
        if (log_files == 0) {
            if (total_files == 0) {
                status_msg = "Directory is empty";
            } else {
                status_msg = "No .log files found (found " + std::to_string(total_files) + " other files)";
            }
        } else {
            status_msg = "Found " + std::to_string(log_files) + " log file" + 
                        (log_files == 1 ? "" : "s");
            if (total_files > log_files) {
                status_msg += " (out of " + std::to_string(total_files) + " total files)";
            }
        }
        
        // Sort by modification time (newest first)
        std::sort(files_with_time.begin(), files_with_time.end(),
                  [](const auto& a, const auto& b) {
                      return a.second > b.second;
                  });
        
        // Extract just the file paths
        std::vector<std::string> result;
        result.reserve(files_with_time.size());
        for (const auto& file_info : files_with_time) {
            result.push_back(file_info.first);
        }
        
        return {result, status_msg};
        
    } catch (const std::filesystem::filesystem_error& e) {
        return {{}, "Filesystem error: " + std::string(e.what())};
    } catch (const std::exception& e) {
        return {{}, "Unexpected error: " + std::string(e.what())};
    }
}

std::pair<std::string, std::string> FindSavedLogsDirectoryWithError() {
    return FindSavedLogsDirectoryWithError(std::filesystem::current_path().string());
}

std::pair<std::string, std::string> FindSavedLogsDirectoryWithError(const std::string& base_directory) {
    try {
        std::filesystem::path base_path(base_directory);
        
        // Check if the base directory itself is accessible
        std::error_code ec;
        if (!std::filesystem::exists(base_path, ec)) {
            if (ec) {
                return {"", "Cannot access base directory: " + ec.message()};
            }
            return {"", "Base directory does not exist: " + base_directory};
        }
        
        if (!std::filesystem::is_directory(base_path, ec)) {
            if (ec) {
                return {"", "Cannot determine if base path is directory: " + ec.message()};
            }
            return {"", "Base path is not a directory: " + base_directory};
        }
        
        // Check if the base directory itself is a Saved/Logs directory
        if (base_path.filename() == "Logs" && base_path.parent_path().filename() == "Saved") {
            if (std::filesystem::exists(base_path, ec) && !ec && 
                std::filesystem::is_directory(base_path, ec) && !ec) {
                return {base_path.string(), "Using current directory (Saved/Logs)"};
            }
        }
        
        // Look for Saved/Logs subdirectory
        std::filesystem::path saved_logs_path = base_path / "Saved" / "Logs";
        if (std::filesystem::exists(saved_logs_path, ec) && !ec && 
            std::filesystem::is_directory(saved_logs_path, ec) && !ec) {
            return {saved_logs_path.string(), "Found Saved/Logs subdirectory"};
        }
        
        // Look for just Logs subdirectory
        std::filesystem::path logs_path = base_path / "Logs";
        if (std::filesystem::exists(logs_path, ec) && !ec && 
            std::filesystem::is_directory(logs_path, ec) && !ec) {
            return {logs_path.string(), "Found Logs subdirectory"};
        }
        
        return {"", "No Saved/Logs or Logs directory found in " + base_directory};
        
    } catch (const std::filesystem::filesystem_error& e) {
        return {"", "Filesystem error: " + std::string(e.what())};
    } catch (const std::exception& e) {
        return {"", "Unexpected error: " + std::string(e.what())};
    }
}

} // namespace unreal_utils
} // namespace ue_log