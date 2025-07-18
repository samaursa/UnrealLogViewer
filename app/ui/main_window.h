#pragma once

#include "../../macros.h"
#include "../../lib/log_parser/log_parser.h"
#include "../../lib/filter_engine/filter_engine.h"
#include "../../lib/file_monitor/file_monitor.h"
#include "../../lib/config/config_manager.h"
#include "filter_panel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <memory>
#include <vector>
#include <string>
#include <atomic>

namespace ue_log {
    
    class MainWindow {
        CK_GENERATED_BODY(MainWindow);
        
    private:
        // Core components
        std::unique_ptr<LogParser> log_parser;
        std::unique_ptr<FilterEngine> filter_engine;
        std::unique_ptr<FileMonitor> file_monitor;
        std::unique_ptr<FilterPanel> filter_panel;
        ConfigManager* config_manager;
        
        // Display state
        std::vector<LogEntry> displayed_entries;
        std::vector<LogEntry> filtered_entries;
        int selected_entry_index;
        int scroll_offset;
        bool is_tailing_mode;
        bool show_filter_panel;
        bool is_file_loaded;
        std::string current_file_path;
        std::string last_error;
        
        // UI state
        int terminal_width;
        int terminal_height;
        int visible_rows;
        bool needs_refresh;
        std::atomic<bool> is_updating;
        
        // FTXUI Components
        ftxui::Component main_container;
        ftxui::Component log_table_component;
        ftxui::Component status_bar_component;
        ftxui::Component filter_panel_component;
        
    public:
        // Constructors
        MainWindow();
        MainWindow(ConfigManager* config_mgr);
        
        // Initialization and lifecycle
        void Initialize();
        void Shutdown();
        
        // File operations
        bool LoadLogFile(const std::string& file_path);
        void ReloadCurrentFile();
        void CloseCurrentFile();
        
        // Real-time monitoring
        void StartTailing();
        void StopTailing();
        bool IsTailing() const;
        
        // Display management
        void RefreshDisplay();
        void UpdateDisplayedEntries();
        void ToggleFilterPanel();
        void SetTerminalSize(int width, int height);
        
        // Navigation
        void ScrollUp(int lines = 1);
        void ScrollDown(int lines = 1);
        void PageUp();
        void PageDown();
        void GoToTop();
        void GoToBottom();
        void GoToLine(int line_number);
        
        // Selection
        void SelectEntry(int index);
        void SelectNextEntry();
        void SelectPreviousEntry();
        int GetSelectedEntryIndex() const;
        
        // FTXUI integration
        ftxui::Component GetComponent();
        ftxui::Element Render();
        bool OnEvent(ftxui::Event event);
        
        // State accessors
        const std::vector<LogEntry>& GetDisplayedEntries() const;
        bool IsFileLoaded() const;
        const std::string& GetCurrentFilePath() const;
        const std::string& GetLastError() const;
        bool IsFilterPanelVisible() const;
        
        // Component accessors for testing
        LogParser* GetLogParser() const;
        FilterEngine* GetFilterEngine() const;
        FileMonitor* GetFileMonitor() const;
        FilterPanel* GetFilterPanel() const;
        
    private:
        // Initialization helpers
        void InitializeComponents();
        void SetupKeyBindings();
        void ApplyConfiguration();
        
        // Event handlers
        void OnNewLogLines(const std::vector<std::string>& new_lines);
        void OnConfigurationChanged();
        void OnFilterChanged();
        
        // Rendering helpers
        ftxui::Element RenderMainLayout();
        ftxui::Element RenderLogTable();
        ftxui::Element RenderStatusBar();
        ftxui::Element RenderEmptyState();
        ftxui::Element RenderErrorState();
        
        // Display helpers
        void UpdateVisibleRows();
        void EnsureSelectionVisible();
        void ApplyFilters();
        std::string GetHighlightColor(const LogEntry& entry);
        std::string FormatLogEntry(const LogEntry& entry, int max_width);
        
        // Utility methods
        void SetError(const std::string& error_message);
        void ClearError();
        bool IsValidEntryIndex(int index) const;
        void ClampScrollOffset();
        void ClampSelection();
    };
    
} // namespace ue_log