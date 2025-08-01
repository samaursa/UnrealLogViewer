#pragma once

#include "component.h"
#include "filter_panel.h"
#include "visual_theme_manager.h"
#include "log_entry_renderer.h"
#include "relative_line_number_system.h"
#include "file_browser.h"
#include "../log_parser/log_parser.h"
#include "../filter_engine/filter_engine.h"
#include "../filter_engine/filter_expression.h"
#include "../file_monitor/file_monitor.h"
#include "../config/config_manager.h"
#include <memory>
#include <string>
#include <functional>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <set>

namespace ue_log {

/**
 * Main application window component.
 * Integrates all other UI components and manages the application state.
 */
class MainWindow : public Component {
    friend class MainWindowComponent;
    
public:
    // Application mode management
    enum class ApplicationMode {
        FILE_BROWSER,
        LOG_VIEWER
    };
    
    /**
     * Default constructor.
     */
    MainWindow();
    
    /**
     * Constructor.
     * @param config_manager Pointer to the configuration manager.
     */
    explicit MainWindow(ConfigManager* config_manager);
    
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
    
    // Methods expected by tests (compatibility layer)
    const std::vector<LogEntry>& GetDisplayedEntries() const { return filtered_entries_; }
    int GetSelectedEntryIndex() const { return selected_entry_index_; }
    bool IsTailing() const { return is_tailing_; }
    void ReloadCurrentFile() { ReloadLogFile(); }
    void CloseCurrentFile();
    
    // Navigation methods for tests
    bool StartTailing();
    void StopTailing();
    
    // Tailing helper methods
    bool ShouldStopTailing(const ftxui::Event& event) const;
    void AutoScrollToBottom();
    void RefreshDisplay();
    void SetTerminalSize(int width, int height);
    void SetTailingPollInterval(int milliseconds);
    void StartFileMonitoring();
    void StopFileMonitoring();
    void ApplyFiltersToNewEntries(const std::vector<LogEntry>& new_entries);
    void ApplyContextToNewEntries(const std::vector<LogEntry>& new_entries);
    void GoToTop();
    void GoToBottom();
    void GoToLine(int line_number);
    void SelectNextEntry();
    void SelectPreviousEntry();
    
    // FTXUI interface methods
    ftxui::Component GetComponent();
    ftxui::Element Render();
    bool OnEvent(ftxui::Event event);
    
    // Component accessors for testing
    LogParser* GetLogParser() const { return log_parser_.get(); }
    FilterEngine* GetFilterEngine() const { return filter_engine_.get(); }
    FileMonitor* GetFileMonitor() const { return file_monitor_.get(); }
    FilterPanel* GetFilterPanel() const { return filter_panel_.get(); }
    
    // Helper methods for testing
    bool HasLogParser() const { return log_parser_ != nullptr; }
    bool HasFilterEngine() const { return filter_engine_ != nullptr; }
    bool HasFileMonitor() const { return file_monitor_ != nullptr; }
    bool HasFilterPanel() const { return filter_panel_ != nullptr; }
    
    /**
     * Set the last error message (for status bar display).
     * @param error The error message to display.
     */
    void SetLastError(const std::string& error) { last_error_ = error; }
    
    /**
     * Set the exit callback function.
     * @param callback Function to call when the application should exit.
     */
    void SetExitCallback(std::function<void()> callback) {
        exit_callback_ = std::move(callback);
    }
    
    /**
     * Set the refresh callback function for triggering screen updates.
     * @param callback Function to call when the screen should be refreshed.
     */
    void SetRefreshCallback(std::function<void()> callback) {
        refresh_callback_ = std::move(callback);
    }
    
    /**
     * Exit the application.
     */
    void Exit();
    
    /**
     * Run autotest mode - loads file, performs basic operations, and generates report.
     * @param log_file_path Path to the log file to test with
     * @param output_file_path Path to write the autotest report
     * @return True if autotest completed successfully, false otherwise
     */
    bool RunAutotest(const std::string& log_file_path, const std::string& output_file_path);
    
    // Navigation methods
    void ScrollUp(int count = 1);
    void ScrollDown(int count = 1);
    void PageUp();
    void PageDown();
    void HalfPageUp();
    void HalfPageDown();
    void ScrollToTop();
    void ScrollToBottom();
    
    // Search functionality
    void ShowSearch();
    void HideSearch();
    void PerformSearch(const std::string& query);
    void FindNext();
    void FindPrevious();
    void ClearSearch();
    void PromoteSearchToFilter();
    void ShowSearchPromotionDialog();
    void HideSearchPromotionDialog();
    
    // Search input handling
    void AppendToSearch(const std::string& text);
    void ConfirmSearch();
    void BackspaceSearch();
    
    // In-line search functionality (CTRL+L)
    void ShowInlineSearch();
    void HideInlineSearch();
    void AppendToInlineSearch(const std::string& text);
    void BackspaceInlineSearch();
    void ConfirmInlineSearch();
    void FindNextInlineMatch();
    void FindPreviousInlineMatch();
    void UpdateInlineSearchResults();
    bool IsInlineSearchActive() const { return show_inline_search_; }
    bool IsInlineSearchInputMode() const { return inline_search_input_mode_; }
    
    // Contextual filtering
    void ShowContextualFilterDialog();
    void HideContextualFilterDialog();
    void CreateContextualFilter(FilterConditionType type);
    bool IsContextualFilterDialogActive() const { return show_contextual_filter_dialog_; }
    
    // Search promotion dialog
    bool IsSearchPromotionDialogActive() const { return show_search_promotion_; }
    void CreateFilterFromSearch(FilterConditionType type);
    
    // Enhanced search and column filtering
    void PromoteSearchToColumnFilter(int column_number);
    void CreateDirectColumnFilter(int column_number);
    void CreateFilterFromSearchAndColumn(FilterConditionType type, const std::string& search_term);
    
    // Direct column filter helper methods
    void CreateLineNumberFilter(const LogEntry& entry);
    void CreateTimestampAfterFilter(const LogEntry& entry);
    void CreateFrameAfterFilter(const LogEntry& entry);
    void CreateLoggerEqualsFilter(const LogEntry& entry);
    void CreateLevelEqualsFilter(const LogEntry& entry);
    void CreateMessageContainsFilter(const LogEntry& entry);
    
    // Search state checking
    bool IsSearchActive() const { return show_search_; }
    bool IsSearchInputMode() const { return search_input_mode_; }
    
    // Context lines functionality
    void IncreaseContext();
    void DecreaseContext();
    void SetContextLines(int lines);
    void ClearContext();
    
    // Jump functionality
    void ShowJumpDialog();
    void HideJumpDialog();
    void JumpToLine(int line_number);
    void JumpToTimestamp(const std::string& timestamp);
    void JumpToPercentage(int percentage);
    void ToggleJumpMode(); // Switch between line/timestamp mode
    bool IsJumpDialogActive() const { return show_jump_dialog_; }
    void AppendToJumpInput(const std::string& text);
    void BackspaceJumpInput();
    void ExecuteJump();
    
    // Quick filter functionality
    void ShowQuickFilterDialog();
    void HideQuickFilterDialog();
    void ApplyQuickFilter(const std::string& filter_type);
    bool IsQuickFilterDialogActive() const { return show_quick_filter_dialog_; }
    
    // Pattern-based navigation
    void JumpToNextError();
    void JumpToPreviousError();
    void JumpToNextWarning();
    void JumpToPreviousWarning();
    
    // Filter panel functionality
    void ToggleFilterPanel();
    FilterPanel* GetFilterPanel() { return filter_panel_.get(); }
    bool IsFilterPanelVisible() const { return show_filter_panel_; }
    
    // Word wrap and detail view functionality
    void ToggleWordWrap();
    void ToggleDetailView();
    bool IsWordWrapEnabled() const { return word_wrap_enabled_; }
    bool IsDetailViewVisible() const { return show_detail_view_; }
    
    // Detail view focus and navigation
    void FocusDetailView();
    void UnfocusDetailView();
    bool IsDetailViewFocused() const { return detail_view_focused_; }
    void DetailViewScrollUp(int count = 1);
    void DetailViewScrollDown(int count = 1);
    void DetailViewPageUp();
    void DetailViewPageDown();
    void DetailViewHalfPageUp();
    void DetailViewHalfPageDown();
    void DetailViewScrollToTop();
    void DetailViewScrollToBottom();
    
    // Vim-style navigation functionality
    bool HandleVimStyleNavigation(const std::string& input);
    void ExecuteVimNavigation(int count, char direction);
    void ClearVimCommandBuffer();
    bool IsVimCommandMode() const { return vim_command_mode_; }
    void BackspaceVimCommand();
    
    // Application mode management
    void SetMode(ApplicationMode mode);
    ApplicationMode GetMode() const { return current_mode_; }
    void InitializeFileBrowser(const std::string& directory_path);
    void TransitionToLogViewer(const std::string& file_path);
    void OnFileSelected(const std::string& file_path);
    void EnterFileBrowserMode(const std::string& directory_path);
    void EnterLogViewerMode(const std::string& file_path);

private:
    // FTXUI component
    ftxui::Component component_;
    
    // Backend components
    std::unique_ptr<LogParser> log_parser_;
    std::unique_ptr<FilterEngine> filter_engine_;
    std::unique_ptr<FileMonitor> file_monitor_;
    
    // UI components
    std::unique_ptr<FilterPanel> filter_panel_;
    std::unique_ptr<VisualThemeManager> visual_theme_manager_;
    std::unique_ptr<LogEntryRenderer> log_entry_renderer_;
    std::unique_ptr<RelativeLineNumberSystem> relative_line_system_;
    
    // Application mode management
    ApplicationMode current_mode_ = ApplicationMode::LOG_VIEWER;
    std::unique_ptr<FileBrowser> file_browser_;
    std::string initial_directory_;
    
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
    bool show_filter_panel_ = false; // Start hidden by default
    bool show_search_ = false;
    bool show_jump_dialog_ = false;
    bool word_wrap_enabled_ = false; // Word wrap toggle
    bool show_detail_view_ = true; // Detail view window toggle
    bool detail_view_focused_ = false; // Whether detail view has focus
    int detail_view_scroll_offset_ = 0; // Scroll position within detail view
    int window_width_ = 0;
    int window_height_ = 0;
    
    // Search state
    std::string search_query_;
    int search_result_index_ = -1;
    std::vector<int> search_results_;
    bool show_search_promotion_ = false;
    bool search_input_mode_ = false; // true when typing search, false when navigating results
    
    // In-line search state (CTRL+L)
    bool show_inline_search_ = false;
    bool inline_search_input_mode_ = false;
    std::string inline_search_query_;
    std::vector<size_t> inline_search_matches_; // Character positions within the current line
    size_t current_inline_match_ = 0;
    
    // Context lines state
    int context_lines_ = 0; // Number of context lines to show around matches
    std::vector<LogEntry> context_entries_; // Filtered entries with context
    std::set<size_t> match_line_numbers_; // Line numbers that are actual matches (not context)
    
    // Contextual filter state
    bool show_contextual_filter_dialog_ = false;
    std::vector<std::unique_ptr<FilterCondition>> contextual_conditions_;
    
    // Hierarchical filter state
    std::unique_ptr<FilterExpression> current_filter_expression_;
    
    // Jump dialog state
    std::string jump_input_;
    bool jump_to_line_mode_ = true; // true for line, false for timestamp
    
    // Quick filter dialog state
    bool show_quick_filter_dialog_ = false;
    
    // Vim-style navigation state
    std::string vim_command_buffer_;  // Buffer for accumulating vim commands like "5j"
    bool vim_command_mode_ = false;   // True when accumulating a vim command
    
    // Tailing state
    bool is_tailing_ = false;         // Whether tailing mode is active
    bool auto_scroll_enabled_ = false; // Whether auto-scroll is enabled during tailing
    std::chrono::steady_clock::time_point last_auto_scroll_time_; // For throttling auto-scroll
    
    // Callbacks
    std::function<void()> exit_callback_;
    std::function<void()> refresh_callback_;
    
    // Event handlers
    void OnNewLogLines(const std::vector<std::string>& new_lines);
    void OnFiltersChanged();
    void OnFileMonitorError(const std::string& error);
    
    // Helper methods
    void UpdateStatusBar();
    void ApplyConfiguration();
    void SaveConfiguration();
    int GetVisibleHeight() const;
    ftxui::Element RenderLogTable() const;
    ftxui::Element RenderStatusBar() const;
    ftxui::Element RenderFilterPanel() const;
    ftxui::Element RenderHelpDialog() const;
    

    
    // Rendering helpers
    ftxui::Element RenderLogEntry(const LogEntry& entry, bool is_selected) const;
    ftxui::Element RenderTableHeader() const;
    ftxui::Element RenderDetailView() const;
    ftxui::Element RenderQuickFilterDialog() const;
    ftxui::Element RenderJumpDialog() const;
    ftxui::Color GetColorForLogLevel(const std::string& level) const;
    
    // Mode-specific rendering
    ftxui::Element RenderFileBrowserMode() const;
    ftxui::Element RenderLogViewerMode() const;
    
    // Navigation helpers
    void SelectEntry(int index);
    void EnsureSelectionVisible();
    
    // Sample data creation for testing
    void CreateSampleLogEntries();
    void CreateSampleFilters();
    
    // Helper methods
    bool HasUppercaseLetters(const std::string& text) const;
    ftxui::Element CreateHighlightedMessageElement(const std::string& message) const;
    
    // Enhanced status bar rendering
    ftxui::Element RenderSearchStatusBar() const;
    
    // Filter application helpers
    void ApplyCurrentFilter();
    void ApplyTraditionalFilters();
    void BuildContextEntries(const std::vector<LogEntry>& matches);
};

} // namespace ue_log