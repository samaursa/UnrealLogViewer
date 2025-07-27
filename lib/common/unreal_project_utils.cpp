#include "unreal_project_utils.h"
#include <filesystem>
#include <algorithm>
#include <system_error>

namespace ue_log {
namespace unreal_utils {

std::string FindSavedLogsDirectory() {
    return FindSavedLogsDirectory(std::filesystem::current_path().string());
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

} // namespace unreal_utils
} // namespace ue_log