#include "file_monitor.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace ue_log {
    
    FileMonitor::FileMonitor() 
        : status(FileMonitorStatus::Stopped), 
          should_stop(false),
          poll_interval(std::chrono::milliseconds(100)),
          last_file_size(0),
          last_read_position(0),
          total_lines_processed(0),
          total_callbacks_triggered(0),
          average_processing_time(std::chrono::milliseconds(0)) {
    }
    
    FileMonitor::FileMonitor(FileChangeCallback callback) 
        : FileMonitor() {
        change_callback = callback;
    }
    
    FileMonitor::~FileMonitor() {
        if (IsMonitoring()) {
            StopMonitoring();
        }
    }
    
    Result FileMonitor::StartMonitoring(const std::string& file_path) {
        if (IsMonitoring()) {
            return Result::Error(ErrorCode::InvalidLogFormat, "FileMonitor is already running");
        }
        
        if (file_path.empty()) {
            return Result::Error(ErrorCode::FileNotFound, "File path cannot be empty");
        }
        
        if (!change_callback) {
            return Result::Error(ErrorCode::InvalidLogFormat, "Callback function must be set before starting monitoring");
        }
        
        // Set the file path first, then check if file exists
        monitored_file_path = file_path;
        Result file_check = CheckFileExists();
        if (file_check.IsError()) {
            monitored_file_path.clear(); // Clear on error
            return file_check;
        }
        should_stop = false;
        SetStatus(FileMonitorStatus::Starting);
        
        try {
            // Initialize file state and set read position to end of file
            UpdateFileState();
            
            // Set initial read position to end of file so we only read new content
            std::ifstream file(monitored_file_path);
            if (file.is_open()) {
                file.seekg(0, std::ios::end);
                last_read_position = file.tellg();
                file.close();
            }
            
            // Start monitoring thread
            monitor_thread = std::thread(&FileMonitor::MonitoringLoop, this);
            
            // Wait for thread to start
            if (!WaitForStatus(FileMonitorStatus::Running, std::chrono::milliseconds(1000))) {
                should_stop = true;
                if (monitor_thread.joinable()) {
                    monitor_thread.join();
                }
                SetStatus(FileMonitorStatus::Error);
                return Result::Error(ErrorCode::InvalidLogFormat, "Failed to start monitoring thread");
            }
            
            return Result::Success();
        } catch (const std::exception& e) {
            SetStatus(FileMonitorStatus::Error);
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Exception while starting file monitoring: " + std::string(e.what()));
        }
    }
    
    Result FileMonitor::StopMonitoring() {
        if (!IsMonitoring()) {
            return Result::Success(); // Already stopped
        }
        
        SetStatus(FileMonitorStatus::Stopping);
        should_stop = true;
        
        try {
            if (monitor_thread.joinable()) {
                monitor_thread.join();
            }
            
            SetStatus(FileMonitorStatus::Stopped);
            return Result::Success();
        } catch (const std::exception& e) {
            SetStatus(FileMonitorStatus::Error);
            return Result::Error(ErrorCode::InvalidLogFormat, 
                "Exception while stopping file monitoring: " + std::string(e.what()));
        }
    }
    
    bool FileMonitor::IsMonitoring() const {
        FileMonitorStatus current_status = status.load();
        return current_status == FileMonitorStatus::Running || 
               current_status == FileMonitorStatus::Starting;
    }
    
    void FileMonitor::SetCallback(FileChangeCallback callback) {
        change_callback = callback;
    }
    
    void FileMonitor::SetPollInterval(std::chrono::milliseconds interval) {
        if (interval.count() > 0) {
            poll_interval = interval;
        }
    }
    
    Result FileMonitor::CheckFileExists() const {
        if (monitored_file_path.empty()) {
            return Result::Error(ErrorCode::FileNotFound, "No file path specified");
        }
        
        std::filesystem::path file_path(monitored_file_path);
        if (!std::filesystem::exists(file_path)) {
            return Result::Error(ErrorCode::FileNotFound, 
                "File does not exist: " + monitored_file_path);
        }
        
        if (!std::filesystem::is_regular_file(file_path)) {
            return Result::Error(ErrorCode::FileNotFound, 
                "Path is not a regular file: " + monitored_file_path);
        }
        
        return Result::Success();
    }
    
    Result FileMonitor::GetFileInfo(std::uintmax_t& size, std::filesystem::file_time_type& last_write) const {
        try {
            std::filesystem::path file_path(monitored_file_path);
            
            if (!std::filesystem::exists(file_path)) {
                return Result::Error(ErrorCode::FileNotFound, "File does not exist");
            }
            
            size = std::filesystem::file_size(file_path);
            last_write = std::filesystem::last_write_time(file_path);
            
            return Result::Success();
        } catch (const std::filesystem::filesystem_error& e) {
            return Result::Error(ErrorCode::FileNotFound, 
                "Filesystem error: " + std::string(e.what()));
        }
    }
    
    void FileMonitor::ResetStatistics() {
        total_lines_processed = 0;
        total_callbacks_triggered = 0;
        average_processing_time = std::chrono::milliseconds(0);
    }
    
    std::string FileMonitor::GetStatisticsSummary() const {
        std::ostringstream oss;
        oss << "FileMonitor Statistics:\n";
        oss << "  Status: " << GetStatusString() << "\n";
        oss << "  Monitored File: " << monitored_file_path << "\n";
        oss << "  Poll Interval: " << poll_interval.count() << "ms\n";
        oss << "  Total Lines Processed: " << total_lines_processed.load() << "\n";
        oss << "  Total Callbacks Triggered: " << total_callbacks_triggered.load() << "\n";
        oss << "  Average Processing Time: " << average_processing_time.load().count() << "ms\n";
        
        if (!monitored_file_path.empty()) {
            std::uintmax_t file_size;
            std::filesystem::file_time_type last_write;
            if (GetFileInfo(file_size, last_write).IsSuccess()) {
                oss << "  Current File Size: " << file_size << " bytes\n";
                oss << "  Last Read Position: " << last_read_position << "\n";
            }
        }
        
        return oss.str();
    }
    
    std::string FileMonitor::GetStatusString() const {
        switch (status.load()) {
            case FileMonitorStatus::Stopped: return "Stopped";
            case FileMonitorStatus::Starting: return "Starting";
            case FileMonitorStatus::Running: return "Running";
            case FileMonitorStatus::Stopping: return "Stopping";
            case FileMonitorStatus::Error: return "Error";
            default: return "Unknown";
        }
    }
    
    bool FileMonitor::WaitForStatus(FileMonitorStatus target_status, std::chrono::milliseconds timeout) const {
        auto start_time = std::chrono::steady_clock::now();
        
        while (std::chrono::steady_clock::now() - start_time < timeout) {
            if (status.load() == target_status) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        return false;
    }
    
    // Private methods
    
    void FileMonitor::MonitoringLoop() {
        SetStatus(FileMonitorStatus::Running);
        
        while (ShouldContinueMonitoring()) {
            try {
                auto start_time = std::chrono::steady_clock::now();
                
                if (HasFileChanged()) {
                    ProcessNewContent();
                }
                
                auto end_time = std::chrono::steady_clock::now();
                auto processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                
                // Update average processing time (simple moving average)
                auto current_avg = average_processing_time.load();
                auto new_avg = std::chrono::milliseconds((current_avg.count() + processing_time.count()) / 2);
                average_processing_time = new_avg;
                
                std::this_thread::sleep_for(poll_interval);
            } catch (const std::exception& e) {
                // Log error but continue monitoring
                SetStatus(FileMonitorStatus::Error);
                std::this_thread::sleep_for(poll_interval);
            }
        }
        
        SetStatus(FileMonitorStatus::Stopped);
    }
    
    std::vector<std::string> FileMonitor::ReadNewLines() {
        std::vector<std::string> new_lines;
        
        try {
            std::ifstream file(monitored_file_path);
            if (!file.is_open()) {
                return new_lines;
            }
            
            // Seek to last read position
            file.seekg(last_read_position);
            
            std::string line;
            while (std::getline(file, line)) {
                new_lines.push_back(line);
                total_lines_processed++;
            }
            
            // Update last read position
            last_read_position = file.tellg();
            
        } catch (const std::exception& e) {
            // Return empty vector on error
        }
        
        return new_lines;
    }
    
    bool FileMonitor::HasFileChanged() const {
        try {
            std::uintmax_t current_size;
            std::filesystem::file_time_type current_write_time;
            
            if (GetFileInfo(current_size, current_write_time).IsError()) {
                return false;
            }
            
            return current_size != last_file_size || current_write_time != last_write_time;
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    void FileMonitor::UpdateFileState() {
        try {
            std::uintmax_t current_size;
            std::filesystem::file_time_type current_write_time;
            
            if (GetFileInfo(current_size, current_write_time).IsSuccess()) {
                last_file_size = current_size;
                last_write_time = current_write_time;
                
                // If file is smaller than before, it might have been rotated
                if (current_size < static_cast<std::uintmax_t>(last_read_position)) {
                    HandleFileRotation();
                }
            }
        } catch (const std::exception& e) {
            // Ignore errors in state update
        }
    }
    
    void FileMonitor::HandleFileRotation() {
        // File was rotated or truncated, reset read position
        last_read_position = 0;
    }
    
    void FileMonitor::ProcessNewContent() {
        std::vector<std::string> new_lines = ReadNewLines();
        
        if (!new_lines.empty() && change_callback) {
            change_callback(monitored_file_path, new_lines);
            total_callbacks_triggered++;
        }
        
        UpdateFileState();
    }
    
    void FileMonitor::SetStatus(FileMonitorStatus new_status) {
        status = new_status;
    }
    
    bool FileMonitor::ShouldContinueMonitoring() const {
        return !should_stop.load() && status.load() != FileMonitorStatus::Stopping;
    }
    
} // namespace ue_log