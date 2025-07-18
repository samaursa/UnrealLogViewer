#pragma once

#include "component.h"
#include "../log_parser/log_parser.h"
#include "../filter_engine/filter_engine.h"
#include "../file_monitor/file_monitor.h"
#include "../config/config_manager.h"
#include <memory>
#include <string>
#include <functional>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace ue_log {

/**
 * Main application window component.
 * Integrates all other UI components and manages the application state.
 */
class MainWindow : public Component {
public:
    /**
     * Constructor.
     * @param config_manager Pointer to the configuration manager.
     */
    explicit MainWindow(ConfigManager* config_manager = nullptr);
    
    /**
     * Destructor.
     */
    ~MainWindow() override;
    
    /**
     * Initialize the component.
     */
    void Initialize() override;
    
    /**
     * Render the component to an FTXUI Element.
     * @return FTXUI Element representing this component.
     */
    ftxui::Element Render() const override;
    
    /**
     * Create an FTXUI Component for this component.
     * @return FTXUI Component that can be used in the FTXUI framework.
     */
    ftxui::Component CreateFTXUIComponent() override;
    
    /**
     * Get the title of this component.
     * @return Title string.
     */
    std::string GetTitle() const override { return "Unreal Log Viewer"; }
    
    /**
     * Load a log file.
     * @param file_path Path to the log file to load.
     * @return True if the file was loaded successfully, false otherwise.
     */
    bool LoadLogFile(const std::string& file_path);
    
    /**
     * Reload the current log file.
     * @return True if the file was reloaded successfully, false otherwise.
     */
    bool ReloadLogFile();
    
    /**
     * Get the path to the currently loaded log file.
     * @return Path to the current log file, or empty string if no file is loaded.
     */
    std::string GetCurrentFilePath() const { return current_file_path_; }
    
    /**
     * Check if a file is currently loaded.
     * @return True if a file is loaded, false otherwise.
     */
    bool IsFileLoaded() const { return !current_file_path_.empty(); }
    
    /**
     * Start real-time monitoring of the current log file.
     * @return True if monitoring was started successfully, false otherwise.
     */
    bool StartRealTimeMonitoring();
    
    /**
     * Stop real-time monitoring of the current log file.
     */
    void StopRealTimeMonitoring();
    
    /**
     * Check if real-time monitoring is active.
     * @return True if monitoring is active, false otherwise.
     */
    bool IsRealTimeMonitoringActive() const;
    
    /**
     * Get the log parser.
     * @return Reference to the log parser.
     */
    LogParser& GetLogParser() { return *log_parser_; }
    
    /**
     * Get the filter engine.
     * @return Reference to the filter engine.
     */
    FilterEngine& GetFilterEngine() { return *filter_engine_; }
    
    /**
     * Get the file monitor.
     * @return Reference to the file monitor.
     */
    FileMonitor& GetFileMonitor() { return *file_monitor_; }
    
    /**
     * Get the configuration manager.
     * @return Reference to the configuration manager.
     */
    ConfigManager& GetConfigManager() { return *config_manager_; }
    
    /**
     * Get the last error message.
     * @return Last error message.
     */
    std::string GetLastError() const { return last_error_; }
    
    /**
     * Set the exit callback function.
     * @param callback Function to call when the application should exit.
     */
    void SetExitCallback(std::function<void()> callback) {
        exit_callback_ = std::move(callback);
    }
    
    /**
     * Exit the application.
     */
    void Exit();
    
    // Navigation methods
    void ScrollUp(int count = 1);
    void ScrollDown(int count = 1);
    void PageUp();
    void PageDown();
    void ScrollToTop();
    void ScrollToBottom();

private:
    // FTXUI component
    ftxui::Component component_;
    
    // Backend components
    std::unique_ptr<LogParser> log_parser_;
    std::unique_ptr<FilterEngine> filter_engine_;
    std::unique_ptr<FileMonitor> file_monitor_;
    
    ConfigManager* config_manager_ = nullptr;
    bool owns_config_manager_ = false;
    
    // Application state
    std::string current_file_path_;
    std::string last_error_;
    std::vector<LogEntry> log_entries_;
    std::vector<LogEntry> filtered_entries_;
    int selected_entry_index_ = -1;
    int scroll_offset_ = 0;
    bool show_line_numbers_ = true;
    bool auto_scroll_ = true;
    
    // UI state
    bool show_help_ = false;
    bool show_filter_panel_ = true;
    int window_width_ = 0;
    int window_height_ = 0;
    
    // Callbacks
    std::function<void()> exit_callback_;
    
    // Event handlers
    void OnNewLogLines(const std::vector<std::string>& new_lines);
    void OnFiltersChanged();
    void OnFileMonitorError(const std::string& error);
    
    // Helper methods
    void UpdateStatusBar();
    void ApplyConfiguration();
    void SaveConfiguration();
    ftxui::Element RenderLogTable() const;
    ftxui::Element RenderStatusBar() const;
    ftxui::Element RenderFilterPanel() const;
    ftxui::Element RenderHelpDialog() const;
    
    // Rendering helpers
    ftxui::Element RenderLogEntry(const LogEntry& entry, bool is_selected) const;
    ftxui::Element RenderTableHeader() const;
    ftxui::Color GetColorForLogLevel(const std::string& level) const;
    
    // Navigation helpers
    void SelectEntry(int index);
    void EnsureSelectionVisible();
    
    // Sample data creation for testing
    void CreateSampleLogEntries();
};

} // namespace ue_log