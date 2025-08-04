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

namespace ue_log
{
    /**
     * Main application window component.
     * Integrates all other UI components and manages the application state.
     */
    class MainWindow final : public Component
    {
        friend class MainWindowComponent;

    public:
        // Application mode management
        enum class EApplicationMode
        {
            FileBrowser,
            LogViewer
        };

        /**
         * Default constructor.
         */
        MainWindow();

        /**
         * Constructor.
         * @param config_manager Pointer to the configuration manager.
         */
        explicit
            MainWindow(
                ConfigManager* config_manager);

        /**
         * Destructor.
         */
        ~MainWindow() override;

        /**
         * Initialize the component.
         */
        auto
            Initialize()
                -> void override;

        /**
         * Render the component to an FTXUI Element.
         * @return FTXUI Element representing this component.
         */
        auto
            Render() const
                -> ftxui::Element override;

        /**
         * Create an FTXUI Component for this component.
         * @return FTXUI Component that can be used in the FTXUI framework.
         */
        auto
            CreateFTXUIComponent()
                -> ftxui::Component override;

        /**
         * Get the title of this component.
         * @return Title string.
         */
        auto
            GetTitle() const
                -> std::string override
        {
            return "Unreal Log Viewer";
        }

        /**
         * Load a log file.
         * @param file_path Path to the log file to load.
         * @return True if the file was loaded successfully, false otherwise.
         */
        auto
            LoadLogFile(
                const std::string& file_path)
                -> bool;

        /**
         * Reload the current log file.
         * @return True if the file was reloaded successfully, false otherwise.
         */
        auto
            ReloadLogFile()
                -> bool;

        /**
         * Get the path to the currently loaded log file.
         * @return Path to the current log file, or empty string if no file is loaded.
         */
        auto
            GetCurrentFilePath() const
                -> std::string
        {
            return _Current_File_Path_;
        }

        /**
         * Check if a file is currently loaded.
         * @return True if a file is loaded, false otherwise.
         */
        auto
            IsFileLoaded() const
                -> bool
        {
            return !_Current_File_Path_.empty();
        }

        /**
         * Start real-time monitoring of the current log file.
         * @return True if monitoring was started successfully, false otherwise.
         */
        auto
            StartRealTimeMonitoring()
                -> bool;

        /**
         * Stop real-time monitoring of the current log file.
         */
        auto
            StopRealTimeMonitoring()
                -> void;

        /**
         * Check if real-time monitoring is active.
         * @return True if monitoring is active, false otherwise.
         */
        auto
            IsRealTimeMonitoringActive() const
                -> bool;

        /**
         * Get the log parser.
         * @return Reference to the log parser.
         */
        auto
            GetLogParser()
                -> LogParser&
        {
            return *_Log_Parser_;
        }

        /**
         * Get the filter engine.
         * @return Reference to the filter engine.
         */
        auto
            GetFilterEngine()
                -> FilterEngine&
        {
            return *_Filter_Engine_;
        }

        /**
         * Get the file monitor.
         * @return Reference to the file monitor.
         */
        auto
            GetFileMonitor()
                -> FileMonitor&
        {
            return *_File_Monitor_;
        }

        /**
         * Get the configuration manager.
         * @return Reference to the configuration manager.
         */
        auto
            GetConfigManager()
                -> ConfigManager&
        {
            return *_Config_Manager_;
        }

        /**
         * Get the last error message.
         * @return Last error message.
         */
        auto
            GetLastError() const
                -> std::string
        {
            return _Last_Error_;
        }

        // Methods expected by tests (compatibility layer)
        auto
            GetDisplayedEntries() const
                -> const std::vector<LogEntry>&
        {
            return filtered_entries_;
        }

        auto
            GetSelectedEntryIndex() const
                -> int
        {
            return selected_entry_index_;
        }

        auto
            IsTailing() const
                -> bool
        {
            return is_tailing_;
        }

        auto
            ReloadCurrentFile()
                -> void
        {
            ReloadLogFile();
        }

        auto
            CloseCurrentFile()
                -> void;

        // Navigation methods for tests
        auto
            StartTailing()
                -> bool;
        auto
            StopTailing()
                -> void;

        // Tailing helper methods
        auto
            ShouldStopTailing(
                const ftxui::Event& event) const
                -> bool;
        auto
            AutoScrollToBottom()
                -> void;
        auto
            RefreshDisplay()
                -> void;
        auto
            SetTerminalSize(
                int width,
                int height)
                -> void;
        auto
            SetTailingPollInterval(
                int milliseconds)
                -> void;
        auto
            StartFileMonitoring()
                -> void;
        auto
            StopFileMonitoring()
                -> void;
        auto
            ApplyFiltersToNewEntries(
                const std::vector<LogEntry>& new_entries)
                -> void;
        auto
            ApplyContextToNewEntries(
                const std::vector<LogEntry>& new_entries)
                -> void;
        auto
            GoToTop()
                -> void;
        auto
            GoToBottom()
                -> void;
        auto
            GoToLine(
                int line_number)
                -> void;
        auto
            SelectNextEntry()
                -> void;
        auto
            SelectPreviousEntry()
                -> void;

        // FTXUI interface methods
        auto
            GetComponent()
                -> ftxui::Component;
        auto
            Render()
                -> ftxui::Element;
        auto
            OnEvent(
                ftxui::Event event)
                -> bool;

        // Component accessors for testing
        auto
            GetLogParser() const
                -> LogParser*
        {
            return _Log_Parser_.get();
        }

        auto
            GetFilterEngine() const
                -> FilterEngine*
        {
            return _Filter_Engine_.get();
        }

        auto
            GetFileMonitor() const
                -> FileMonitor*
        {
            return _File_Monitor_.get();
        }

        auto
            GetFilterPanel() const
                -> FilterPanel*
        {
            return _Filter_Panel_.get();
        }

        // Helper methods for testing
        auto
            HasLogParser() const
                -> bool
        {
            return _Log_Parser_ != nullptr;
        }

        auto
            HasFilterEngine() const
                -> bool
        {
            return _Filter_Engine_ != nullptr;
        }

        auto
            HasFileMonitor() const
                -> bool
        {
            return _File_Monitor_ != nullptr;
        }

        auto
            HasFilterPanel() const
                -> bool
        {
            return _Filter_Panel_ != nullptr;
        }

        /**
         * Set the last error message (for status bar display).
         * @param error The error message to display.
         */
        auto
            SetLastError(
                const std::string& error)
                -> void
        {
            _Last_Error_ = error;
        }

        /**
         * Set the exit callback function.
         * @param callback Function to call when the application should exit.
         */
        auto
            SetExitCallback(
                std::function<void()> callback)
                -> void
        {
            exit_callback_ = std::move(callback);
        }

        /**
         * Set the refresh callback function for triggering screen updates.
         * @param callback Function to call when the screen should be refreshed.
         */
        auto
            SetRefreshCallback(
                std::function<void()> callback)
                -> void
        {
            refresh_callback_ = std::move(callback);
        }

        /**
         * Exit the application.
         */
        auto
            Exit()
                -> void;

        /**
         * Run autotest mode - loads file, performs basic operations, and generates report.
         * @param log_file_path Path to the log file to test with
         * @param output_file_path Path to write the autotest report
         * @return True if autotest completed successfully, false otherwise
         */
        auto
            RunAutotest(
                const std::string& log_file_path,
                const std::string& output_file_path)
                -> bool;

        // Navigation methods
        auto
            ScrollUp(
                int count = 1)
                -> void;
        auto
            ScrollDown(
                int count = 1)
                -> void;
        auto
            PageUp()
                -> void;
        auto
            PageDown()
                -> void;
        auto
            HalfPageUp()
                -> void;
        auto
            HalfPageDown()
                -> void;
        auto
            ScrollToTop()
                -> void;
        auto
            ScrollToBottom()
                -> void;

        // Search functionality
        auto
            ShowSearch()
                -> void;
        auto
            HideSearch()
                -> void;
        auto
            PerformSearch(
                const std::string& query)
                -> void;
        auto
            PerformSearchHighlightOnly(
                const std::string& query)
                -> void;
        auto
            FindNext()
                -> void;
        auto
            FindPrevious()
                -> void;
        auto
            ClearSearch()
                -> void;
        auto
            PromoteSearchToFilter()
                -> void;
        auto
            ShowSearchPromotionDialog()
                -> void;
        auto
            HideSearchPromotionDialog()
                -> void;

        // Search input handling
        auto
            AppendToSearch(
                const std::string& text)
                -> void;
        auto
            ConfirmSearch()
                -> void;
        auto
            BackspaceSearch()
                -> void;

        // In-line search functionality (CTRL+L)
        auto
            ShowInlineSearch()
                -> void;
        auto
            HideInlineSearch()
                -> void;
        auto
            AppendToInlineSearch(
                const std::string& text)
                -> void;
        auto
            BackspaceInlineSearch()
                -> void;
        auto
            ConfirmInlineSearch()
                -> void;
        auto
            FindNextInlineMatch()
                -> void;
        auto
            FindPreviousInlineMatch()
                -> void;
        auto
            UpdateInlineSearchResults()
                -> void;

        auto
            IsInlineSearchActive() const
                -> bool
        {
            return show_inline_search_;
        }

        auto
            IsInlineSearchInputMode() const
                -> bool
        {
            return inline_search_input_mode_;
        }

        // Contextual filtering
        auto
            ShowContextualFilterDialog()
                -> void;
        auto
            HideContextualFilterDialog()
                -> void;
        auto
            CreateContextualFilter(
                FilterConditionType type)
                -> void;

        auto
            IsContextualFilterDialogActive() const
                -> bool
        {
            return show_contextual_filter_dialog_;
        }

        // Search promotion dialog
        auto
            IsSearchPromotionDialogActive() const
                -> bool
        {
            return show_search_promotion_;
        }

        auto
            CreateFilterFromSearch(
                FilterConditionType type)
                -> void;

        // Enhanced search and column filtering
        auto
            PromoteSearchToColumnFilter(
                int column_number)
                -> void;
        auto
            CreateDirectColumnFilter(
                int column_number)
                -> void;
        auto
            CreateDirectColumnExcludeFilter(
                int column_number)
                -> void;
        auto
            CreateFilterFromSearchAndColumn(
                FilterConditionType type,
                const std::string& search_term)
                -> void;

        // Direct column filter helper methods
        auto
            CreateLineNumberFilter(
                const LogEntry& entry)
                -> void;
        auto
            CreateTimestampAfterFilter(
                const LogEntry& entry)
                -> void;
        auto
            CreateFrameAfterFilter(
                const LogEntry& entry)
                -> void;
        auto
            CreateLoggerEqualsFilter(
                const LogEntry& entry)
                -> void;
        auto
            CreateLevelEqualsFilter(
                const LogEntry& entry)
                -> void;
        auto
            CreateMessageContainsFilter(
                const LogEntry& entry)
                -> void;

        // Search state checking
        auto
            IsSearchActive() const
                -> bool
        {
            return show_search_;
        }

        auto
            IsSearchInputMode() const
                -> bool
        {
            return search_input_mode_;
        }

        // Context lines functionality
        auto
            IncreaseContext()
                -> void;
        auto
            DecreaseContext()
                -> void;
        auto
            SetContextLines(
                int lines)
                -> void;
        auto
            ClearContext()
                -> void;

        // Jump functionality
        auto
            ShowJumpDialog()
                -> void;
        auto
            HideJumpDialog()
                -> void;
        auto
            JumpToLine(
                int line_number)
                -> void;
        auto
            JumpToTimestamp(
                const std::string& timestamp)
                -> void;
        auto
            JumpToPercentage(
                int percentage)
                -> void;
        auto
            ToggleJumpMode()
                -> void; // Switch between line/timestamp mode
        auto
            IsJumpDialogActive() const
                -> bool
        {
            return show_jump_dialog_;
        }

        auto
            AppendToJumpInput(
                const std::string& text)
                -> void;
        auto
            BackspaceJumpInput()
                -> void;
        auto
            ExecuteJump()
                -> void;

        // Quick filter functionality
        auto
            ShowQuickFilterDialog()
                -> void;
        auto
            HideQuickFilterDialog()
                -> void;
        auto
            ApplyQuickFilter(
                const std::string& filter_type)
                -> void;

        auto
            IsQuickFilterDialogActive() const
                -> bool
        {
            return show_quick_filter_dialog_;
        }

        // Pattern-based navigation
        auto
            JumpToNextError()
                -> void;
        auto
            JumpToPreviousError()
                -> void;
        auto
            JumpToNextWarning()
                -> void;
        auto
            JumpToPreviousWarning()
                -> void;

        // Filter panel functionality
        auto
            ToggleFilterPanel()
                -> void;

        auto
            GetFilterPanel()
                -> FilterPanel*
        {
            return _Filter_Panel_.get();
        }

        auto
            IsFilterPanelVisible() const
                -> bool
        {
            return show_filter_panel_;
        }

        // Word wrap and detail view functionality
        auto
            ToggleWordWrap()
                -> void;
        auto
            ToggleDetailView()
                -> void;

        auto
            IsWordWrapEnabled() const
                -> bool
        {
            return word_wrap_enabled_;
        }

        auto
            IsDetailViewVisible() const
                -> bool
        {
            return show_detail_view_;
        }

        // Detail view focus and navigation
        auto
            FocusDetailView()
                -> void;
        auto
            UnfocusDetailView()
                -> void;

        auto
            IsDetailViewFocused() const
                -> bool
        {
            return detail_view_focused_;
        }

        auto
            DetailViewScrollUp(
                int count = 1)
                -> void;
        auto
            DetailViewScrollDown(
                int count = 1)
                -> void;
        auto
            DetailViewPageUp()
                -> void;
        auto
            DetailViewPageDown()
                -> void;
        auto
            DetailViewHalfPageUp()
                -> void;
        auto
            DetailViewHalfPageDown()
                -> void;
        auto
            DetailViewScrollToTop()
                -> void;
        auto
            DetailViewScrollToBottom()
                -> void;

        // Vim-style navigation functionality
        auto
            HandleVimStyleNavigation(
                const std::string& input)
                -> bool;
        auto
            ExecuteVimNavigation(
                int count,
                char direction)
                -> void;
        auto
            ClearVimCommandBuffer()
                -> void;

        auto
            IsVimCommandMode() const
                -> bool
        {
            return vim_command_mode_;
        }

        auto
            BackspaceVimCommand()
                -> void;

        // Application mode management
        auto
            SetMode(
                EApplicationMode mode)
                -> void;

        auto
            GetMode() const
                -> EApplicationMode
        {
            return _Current_Mode_;
        }

        auto
            InitializeFileBrowser(
                const std::string& directory_path)
                -> void;
        auto
            TransitionToLogViewer(
                const std::string& file_path)
                -> void;
        auto
            OnFileSelected(
                const std::string& file_path)
                -> void;
        auto
            EnterFileBrowserMode(
                const std::string& directory_path)
                -> void;
        auto
            EnterLogViewerMode(
                const std::string& file_path)
                -> void;

        // Clipboard functionality
        auto
            CopyCurrentLineToClipboard()
                -> void;
        auto
            CopyVisualSelectionToClipboard()
                -> void;

        // Visual selection functionality
        auto
            EnterVisualSelectionMode()
                -> void;
        auto
            ExitVisualSelectionMode()
                -> void;
        auto
            IsVisualSelectionMode() const
                -> bool;
        auto
            ExtendVisualSelection(
                int new_end_index)
                -> void;
        auto
            GetVisualSelectionRange() const
                -> std::pair<int, int>;
        auto
            GetVisualSelectionSize() const
                -> int;

        // Filter highlighting functionality
        auto
            GetFilterHighlightTerm() const
                -> std::string;

    private:
        // FTXUI component
        ftxui::Component _Component_;

        // Backend components
        std::unique_ptr<LogParser> _Log_Parser_;
        std::unique_ptr<FilterEngine> _Filter_Engine_;
        std::unique_ptr<FileMonitor> _File_Monitor_;

        // UI components
        std::unique_ptr<FilterPanel> _Filter_Panel_;
        std::unique_ptr<VisualThemeManager> _Visual_Theme_Manager_;
        std::unique_ptr<LogEntryRenderer> _Log_Entry_Renderer_;
        std::unique_ptr<RelativeLineNumberSystem> _Relative_Line_System_;

        // Application mode management
        EApplicationMode _Current_Mode_ = EApplicationMode::LogViewer;
        std::unique_ptr<FileBrowser> _File_Browser_;
        std::string _Initial_Directory_;

        ConfigManager* _Config_Manager_ = nullptr;
        bool _Owns_Config_Manager_ = false;

        // Application state
        std::string _Current_File_Path_;
        std::string _Last_Error_;
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
        std::string vim_command_buffer_; // Buffer for accumulating vim commands like "5j"
        bool vim_command_mode_ = false; // True when accumulating a vim command

        // Tailing state
        bool is_tailing_ = false; // Whether tailing mode is active
        bool auto_scroll_enabled_ = false; // Whether auto-scroll is enabled during tailing
        std::chrono::steady_clock::time_point last_auto_scroll_time_; // For throttling auto-scroll

        // Visual selection state
        bool visual_selection_mode_ = false;
        int visual_selection_start_ = -1;  // Starting line index
        int visual_selection_end_ = -1;    // Current end line index
        int visual_selection_anchor_ = -1; // Fixed anchor point for selection

        // Callbacks
        std::function<void()> exit_callback_;
        std::function<void()> refresh_callback_;

        // Event handlers
        auto
            OnNewLogLines(
                const std::vector<std::string>& new_lines)
                -> void;
        auto
            OnFiltersChanged()
                -> void;
        auto
            OnFileMonitorError(
                const std::string& error)
                -> void;

        // Helper methods
        auto
            UpdateStatusBar()
                -> void;
        auto
            ApplyConfiguration()
                -> void;
        auto
            SaveConfiguration()
                -> void;
        auto
            GetVisibleHeight() const
                -> int;
        auto
            RenderLogTable() const
                -> ftxui::Element;
        auto
            RenderStatusBar() const
                -> ftxui::Element;
        auto
            RenderFilterPanel() const
                -> ftxui::Element;
        auto
            RenderHelpDialog() const
                -> ftxui::Element;

        // Rendering helpers
        auto
            RenderLogEntry(
                const LogEntry& entry,
                bool is_selected) const
                -> ftxui::Element;
        auto
            RenderTableHeader() const
                -> ftxui::Element;
        auto
            RenderDetailView() const
                -> ftxui::Element;
        auto
            RenderQuickFilterDialog() const
                -> ftxui::Element;
        auto
            RenderJumpDialog() const
                -> ftxui::Element;
        auto
            RenderCustomScrollbar() const
                -> ftxui::Element;
        auto
            GetColorForLogLevel(
                const std::string& level) const
                -> ftxui::Color;

        // Mode-specific rendering
        auto
            RenderFileBrowserMode() const
                -> ftxui::Element;
        auto
            RenderLogViewerMode() const
                -> ftxui::Element;

        // Navigation helpers
        auto
            SelectEntry(
                int index)
                -> void;
        auto
            EnsureSelectionVisible()
                -> void;

        // Sample data creation for testing
        auto
            CreateSampleLogEntries()
                -> void;
        auto
            CreateSampleFilters()
                -> void;

        // Helper methods
        auto
            HasUppercaseLetters(
                const std::string& text) const
                -> bool;
        auto
            CreateHighlightedMessageElement(
                const std::string& message) const
                -> ftxui::Element;

        // Enhanced status bar rendering
        auto
            RenderSearchStatusBar() const
                -> ftxui::Element;

        // Filter application helpers
        auto
            ApplyCurrentFilter()
                -> void;
        auto
            ApplyTraditionalFilters()
                -> void;
        auto
            BuildContextEntries(
                const std::vector<LogEntry>& matches)
                -> void;
    };
} // namespace ue_log
