#pragma once

#include "../../macros.h"
#include "../common/result.h"
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <filesystem>
#include <chrono>
#include <vector>

namespace ue_log {
    
    // Callback function type for file change notifications
    using FileChangeCallback = std::function<void(const std::string& file_path, const std::vector<std::string>& new_lines)>;
    
    enum class FileMonitorStatus {
        Stopped,
        Starting,
        Running,
        Stopping,
        Error
    };
    
    class FileMonitor {
        CK_GENERATED_BODY(FileMonitor);
        
    private:
        std::string monitored_file_path;
        FileChangeCallback change_callback;
        std::atomic<FileMonitorStatus> status;
        std::atomic<bool> should_stop;
        std::thread monitor_thread;
        std::chrono::milliseconds poll_interval;
        
        // File state tracking
        std::filesystem::file_time_type last_write_time;
        std::uintmax_t last_file_size;
        std::streampos last_read_position;
        
        // Statistics
        std::atomic<size_t> total_lines_processed;
        std::atomic<size_t> total_callbacks_triggered;
        std::atomic<std::chrono::milliseconds> average_processing_time;
        
    public:
        // Default constructor
        FileMonitor();
        
        // Constructor with callback
        FileMonitor(FileChangeCallback callback);
        
        // Destructor
        ~FileMonitor();
        
        // Properties using macros
        CK_PROPERTY_GET(monitored_file_path);
        CK_PROPERTY_GET(status);
        CK_PROPERTY_GET(poll_interval);
        CK_PROPERTY_GET(total_lines_processed);
        CK_PROPERTY_GET(total_callbacks_triggered);
        CK_PROPERTY_GET(average_processing_time);
        
        // File monitoring control
        Result StartMonitoring(const std::string& file_path);
        Result StopMonitoring();
        bool IsMonitoring() const;
        
        // Configuration
        void SetCallback(FileChangeCallback callback);
        void SetPollInterval(std::chrono::milliseconds interval);
        
        // File operations
        Result CheckFileExists() const;
        Result GetFileInfo(std::uintmax_t& size, std::filesystem::file_time_type& last_write) const;
        
        // Statistics
        void ResetStatistics();
        std::string GetStatisticsSummary() const;
        
        // Utility methods
        std::string GetStatusString() const;
        bool WaitForStatus(FileMonitorStatus target_status, std::chrono::milliseconds timeout) const;
        
    private:
        // Internal monitoring methods
        void MonitoringLoop();
        std::vector<std::string> ReadNewLines();
        bool HasFileChanged() const;
        void UpdateFileState();
        void HandleFileRotation();
        void ProcessNewContent();
        
        // Thread safety helpers
        void SetStatus(FileMonitorStatus new_status);
        bool ShouldContinueMonitoring() const;
    };
    
} // namespace ue_log