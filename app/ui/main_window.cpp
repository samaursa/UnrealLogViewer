#include "main_window.h"
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/table.hpp>
#include <algorithm>
#include <sstream>

namespace ue_log {
    
    MainWindow::MainWindow() 
        : log_parser(std::make_unique<LogParser>()),
          filter_engine(std::make_unique<FilterEngine>()),
          file_monitor(std::make_unique<FileMonitor>()),
          config_manager(nullptr),
          selected_entry_index(0),
          scroll_offset(0),
          is_tailing_mode(false),
          show_filter_panel(false),
          is_file_loaded(false),
          terminal_width(80),
          terminal_height(24),
          visible_rows(20),
          needs_refresh(true),
          is_updating(false) {
    }
    
    MainWindow::MainWindow(ConfigManager* config_mgr)
        : MainWindow() {
        config_manager = config_mgr;
    }
    
    void MainWindow::Initialize() {
        InitializeComponents();
        SetupKeyBindings();
        ApplyConfiguration();
        
        // Set up file monitor callback
        file_monitor->SetCallback([this](const std::string& file_path, const std::vector<std::string>& new_lines) {
            OnNewLogLines(new_lines);
        });
        
        needs_refresh = true;
    }
    
    void MainWindow::Shutdown() {
        StopTailing();
        CloseCurrentFile();
    }
    
    bool MainWindow::LoadLogFile(const std::string& file_path) {
        ClearError();
        
        try {
            // Stop any current tailing
            StopTailing();
            
            // Load file with log parser
            auto load_result = log_parser->LoadFile(file_path);
            if (load_result.IsError()) {
                SetError("Failed to load log file: " + file_path + " - " + load_result.Get_error_message());
                return false;
            }
            
            // Parse entries
            auto entries = log_parser->ParseEntries();
            if (entries.empty()) {
                SetError("No valid log entries found in file: " + file_path);
                return false;
            }
            
            // Update state
            displayed_entries = std::move(entries);
            current_file_path = file_path;
            is_file_loaded = true;
            selected_entry_index = 0;
            scroll_offset = 0;
            
            // Apply current filters
            ApplyFilters();
            UpdateDisplayedEntries();
            
            needs_refresh = true;
            return true;
            
        } catch (const std::exception& e) {
            SetError("Exception while loading file: " + std::string(e.what()));
            return false;
        }
    }
    
    void MainWindow::ReloadCurrentFile() {
        if (!current_file_path.empty()) {
            LoadLogFile(current_file_path);
        }
    }
    
    void MainWindow::CloseCurrentFile() {
        StopTailing();
        displayed_entries.clear();
        filtered_entries.clear();
        current_file_path.clear();
        is_file_loaded = false;
        selected_entry_index = 0;
        scroll_offset = 0;
        ClearError();
        needs_refresh = true;
    }
    
    void MainWindow::StartTailing() {
        if (!is_file_loaded || current_file_path.empty()) {
            SetError("No file loaded for tailing");
            return;
        }
        
        auto start_result = file_monitor->StartMonitoring(current_file_path);
        if (start_result.IsSuccess()) {
            is_tailing_mode = true;
            needs_refresh = true;
        } else {
            SetError("Failed to start file monitoring: " + start_result.Get_error_message());
        }
    }
    
    void MainWindow::StopTailing() {
        if (is_tailing_mode) {
            file_monitor->StopMonitoring();
            is_tailing_mode = false;
            needs_refresh = true;
        }
    }
    
    bool MainWindow::IsTailing() const {
        return is_tailing_mode;
    }
    
    void MainWindow::RefreshDisplay() {
        needs_refresh = true;
    }
    
    void MainWindow::UpdateDisplayedEntries() {
        if (is_updating.exchange(true)) {
            return; // Already updating
        }
        
        // Apply filters to get the entries to display
        ApplyFilters();
        
        // Update visible rows based on terminal size
        UpdateVisibleRows();
        
        // Ensure selection is valid
        ClampSelection();
        ClampScrollOffset();
        
        needs_refresh = true;
        is_updating = false;
    }
    
    void MainWindow::ToggleFilterPanel() {
        show_filter_panel = !show_filter_panel;
        UpdateVisibleRows();
        needs_refresh = true;
    }
    
    void MainWindow::SetTerminalSize(int width, int height) {
        terminal_width = width;
        terminal_height = height;
        UpdateVisibleRows();
        needs_refresh = true;
    }
    
    void MainWindow::ScrollUp(int lines) {
        scroll_offset = std::max(0, scroll_offset - lines);
        needs_refresh = true;
    }
    
    void MainWindow::ScrollDown(int lines) {
        if (filtered_entries.size() <= static_cast<size_t>(visible_rows)) {
            // If we have fewer entries than visible rows, don't scroll
            return;
        }

        int max_scroll = static_cast<int>(filtered_entries.size()) - visible_rows;
        scroll_offset = std::min(max_scroll, scroll_offset + lines);
        needs_refresh = true;
    }
    
    void MainWindow::PageUp() {
        ScrollUp(visible_rows - 1);
    }
    
    void MainWindow::PageDown() {
        ScrollDown(visible_rows - 1);
    }
    
    void MainWindow::GoToTop() {
        scroll_offset = 0;
        selected_entry_index = 0;
        needs_refresh = true;
    }
    
    void MainWindow::GoToBottom() {
        if (!filtered_entries.empty()) {
            selected_entry_index = static_cast<int>(filtered_entries.size()) - 1;

            // Position scroll so the last entries are visible
            if (filtered_entries.size() <= static_cast<size_t>(visible_rows)) {
                scroll_offset = 0;
            } else {
                scroll_offset = static_cast<int>(filtered_entries.size()) - visible_rows;
            }
        }
        needs_refresh = true;
    }
    
    void MainWindow::GoToLine(int line_number) {
        if (line_number >= 0 && line_number < static_cast<int>(filtered_entries.size())) {
            selected_entry_index = line_number;
            EnsureSelectionVisible();
            needs_refresh = true;
        }
    }
    
    void MainWindow::SelectEntry(int index) {
        if (IsValidEntryIndex(index)) {
            selected_entry_index = index;
            EnsureSelectionVisible();
            needs_refresh = true;
        }
    }
    
    void MainWindow::SelectNextEntry() {
        if (selected_entry_index < static_cast<int>(filtered_entries.size()) - 1) {
            selected_entry_index++;
            EnsureSelectionVisible();
            needs_refresh = true;
        }
    }
    
    void MainWindow::SelectPreviousEntry() {
        if (selected_entry_index > 0) {
            selected_entry_index--;
            EnsureSelectionVisible();
            needs_refresh = true;
        }
    }
    
    int MainWindow::GetSelectedEntryIndex() const {
        return selected_entry_index;
    }
    
    ftxui::Component MainWindow::GetComponent() {
        if (!main_container) {
            InitializeComponents();
        }
        return main_container;
    }
    
    ftxui::Element MainWindow::Render() {
        if (!is_file_loaded) {
            return RenderEmptyState();
        }
        
        if (!last_error.empty()) {
            return RenderErrorState();
        }
        
        return RenderMainLayout();
    }
    
    bool MainWindow::OnEvent(ftxui::Event event) {
        // Handle keyboard events
        if (event == ftxui::Event::ArrowUp) {
            SelectPreviousEntry();
            return true;
        }
        if (event == ftxui::Event::ArrowDown) {
            SelectNextEntry();
            return true;
        }
        if (event == ftxui::Event::PageUp) {
            PageUp();
            return true;
        }
        if (event == ftxui::Event::PageDown) {
            PageDown();
            return true;
        }
        if (event == ftxui::Event::Home) {
            GoToTop();
            return true;
        }
        if (event == ftxui::Event::End) {
            GoToBottom();
            return true;
        }
        if (event == ftxui::Event::Tab) {
            ToggleFilterPanel();
            return true;
        }
        if (event == ftxui::Event::F5) {
            if (is_tailing_mode) {
                StopTailing();
            } else {
                StartTailing();
            }
            return true;
        }
        
        return false;
    }
    
    // State accessors
    const std::vector<LogEntry>& MainWindow::GetDisplayedEntries() const {
        return filtered_entries;
    }
    
    bool MainWindow::IsFileLoaded() const {
        return is_file_loaded;
    }
    
    const std::string& MainWindow::GetCurrentFilePath() const {
        return current_file_path;
    }
    
    const std::string& MainWindow::GetLastError() const {
        return last_error;
    }
    
    bool MainWindow::IsFilterPanelVisible() const {
        return show_filter_panel;
    }
    
    // Component accessors for testing
    LogParser* MainWindow::GetLogParser() const {
        return log_parser.get();
    }
    
    FilterEngine* MainWindow::GetFilterEngine() const {
        return filter_engine.get();
    }
    
    FileMonitor* MainWindow::GetFileMonitor() const {
        return file_monitor.get();
    }
    
    FilterPanel* MainWindow::GetFilterPanel() const {
        return filter_panel.get();
    }
    
    // Private implementation methods
    void MainWindow::InitializeComponents() {
        // Create filter panel
        filter_panel = std::make_unique<FilterPanel>(filter_engine.get());
        
        // Create FTXUI components
        main_container = ftxui::Container::Vertical({});
        log_table_component = ftxui::Container::Vertical({});
        status_bar_component = ftxui::Container::Horizontal({});
        
        if (filter_panel) {
            filter_panel_component = filter_panel->GetComponent();
        }
    }
    
    void MainWindow::SetupKeyBindings() {
        // Key bindings are handled in OnEvent method
        // This method can be extended to support configurable key bindings
        if (config_manager) {
            const auto& key_bindings = config_manager->GetConfig().Get_key_bindings();
            // TODO: Apply custom key bindings from configuration
        }
    }
    
    void MainWindow::ApplyConfiguration() {
        if (config_manager) {
            const auto& config = config_manager->GetConfig();
            
            // Apply color scheme, key bindings, etc.
            // This will be expanded when we implement visual highlighting
        }
    }
    
    void MainWindow::OnNewLogLines(const std::vector<std::string>& new_lines) {
        if (!is_file_loaded || !is_tailing_mode) {
            return;
        }
        
        try {
            // Parse new lines
            size_t line_number = displayed_entries.size();
            for (const auto& line : new_lines) {
                auto entry = log_parser->ParseSingleEntry(line, line_number++);
                displayed_entries.push_back(entry);
            }
            
            // Update display
            UpdateDisplayedEntries();
            
            // Auto-scroll to bottom if enabled
            if (config_manager && config_manager->GetConfig().Get_auto_scroll_to_bottom()) {
                GoToBottom();
            }
            
        } catch (const std::exception& e) {
            SetError("Error processing new log lines: " + std::string(e.what()));
        }
    }
    
    void MainWindow::OnConfigurationChanged() {
        ApplyConfiguration();
        needs_refresh = true;
    }
    
    void MainWindow::OnFilterChanged() {
        ApplyFilters();
        UpdateDisplayedEntries();
    }
    
    ftxui::Element MainWindow::RenderMainLayout() {
        auto log_table = RenderLogTable();
        auto status_bar = RenderStatusBar();
        
        if (show_filter_panel && filter_panel) {
            auto filter_panel_element = filter_panel->Render();
            return ftxui::vbox({
                ftxui::hbox({
                    log_table | ftxui::flex,
                    ftxui::separator(),
                    filter_panel_element | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 30)
                }) | ftxui::flex,
                ftxui::separator(),
                status_bar
            });
        } else {
            return ftxui::vbox({
                log_table | ftxui::flex,
                ftxui::separator(),
                status_bar
            });
        }
    }
    
    ftxui::Element MainWindow::RenderLogTable() {
        if (filtered_entries.empty()) {
            return ftxui::text("No log entries to display") | ftxui::center;
        }
        
        // Create table rows for visible entries
        std::vector<std::vector<std::string>> table_data;
        
        // Add header row
        table_data.push_back({"Time", "Frame", "Logger", "Level", "Message"});
        
        // Add visible log entries
        int start_idx = scroll_offset;
        int end_idx = std::min(start_idx + visible_rows, static_cast<int>(filtered_entries.size()));
        
        for (int i = start_idx; i < end_idx; ++i) {
            const auto& entry = filtered_entries[i];
            
            std::vector<std::string> row;
            row.push_back(entry.GetDisplayTimestamp());
            row.push_back(entry.HasFrameNumber() ? std::to_string(entry.Get_frame_number().value()) : "");
            row.push_back(entry.Get_logger_name());
            row.push_back(entry.GetDisplayLogLevel());
            row.push_back(entry.Get_message());
            
            table_data.push_back(row);
        }
        
        // Create table
        auto table = ftxui::Table(table_data);
        
        // Style the table
        table.SelectAll().Border(ftxui::LIGHT);
        table.SelectRow(0).Decorate(ftxui::bold);
        
        // Highlight selected row
        if (selected_entry_index >= start_idx && selected_entry_index < end_idx) {
            int table_row = selected_entry_index - start_idx + 1; // +1 for header
            table.SelectRow(table_row).Decorate(ftxui::inverted);
        }
        
        return table.Render();
    }
    
    ftxui::Element MainWindow::RenderStatusBar() {
        std::ostringstream status;
        
        if (is_file_loaded) {
            status << "File: " << current_file_path;
            status << " | Entries: " << filtered_entries.size();
            if (filtered_entries.size() != displayed_entries.size()) {
                status << " (filtered from " << displayed_entries.size() << ")";
            }
            status << " | Line: " << (selected_entry_index + 1);
            
            if (is_tailing_mode) {
                status << " | TAILING";
            }
            
            if (show_filter_panel) {
                status << " | FILTERS";
            }
        } else {
            status << "No file loaded";
        }
        
        return ftxui::text(status.str()) | ftxui::bgcolor(ftxui::Color::Blue);
    }
    
    ftxui::Element MainWindow::RenderEmptyState() {
        return ftxui::vbox({
            ftxui::text("Unreal Engine Log Viewer") | ftxui::bold | ftxui::center,
            ftxui::text("") | ftxui::center,
            ftxui::text("No log file loaded") | ftxui::center,
            ftxui::text("Press Ctrl+O to open a file") | ftxui::center,
            ftxui::text("") | ftxui::center,
            RenderStatusBar()
        }) | ftxui::center;
    }
    
    ftxui::Element MainWindow::RenderErrorState() {
        return ftxui::vbox({
            ftxui::text("Error") | ftxui::bold | ftxui::color(ftxui::Color::Red) | ftxui::center,
            ftxui::text("") | ftxui::center,
            ftxui::text(last_error) | ftxui::color(ftxui::Color::Red) | ftxui::center,
            ftxui::text("") | ftxui::center,
            RenderStatusBar()
        }) | ftxui::center;
    }
    
    void MainWindow::UpdateVisibleRows() {
        // Calculate visible rows based on terminal height and UI elements
        int ui_overhead = 3; // Header, separator, status bar
        if (show_filter_panel) {
            ui_overhead += 1; // Additional separator
        }
        
        visible_rows = std::max(1, terminal_height - ui_overhead);
    }
    
    void MainWindow::EnsureSelectionVisible() {
        if (selected_entry_index < scroll_offset) {
            scroll_offset = selected_entry_index;
        } else if (selected_entry_index >= scroll_offset + visible_rows) {
            scroll_offset = selected_entry_index - visible_rows + 1;
        }
        ClampScrollOffset();
    }
    
    void MainWindow::ApplyFilters() {
        if (!filter_engine || displayed_entries.empty()) {
            filtered_entries = displayed_entries;
            return;
        }
        
        // Apply filters to get the entries to display
        filtered_entries = filter_engine->ApplyFilters(displayed_entries);
    }
    
    std::string MainWindow::GetHighlightColor(const LogEntry& entry) {
        // This will be implemented in task 7.2 with visual highlighting
        // For now, return empty string (no highlighting)
        return "";
    }
    
    std::string MainWindow::FormatLogEntry(const LogEntry& entry, int max_width) {
        // This will be expanded in task 7.2 for better formatting
        std::ostringstream formatted;
        formatted << entry.GetDisplayTimestamp() << " ";
        if (entry.HasFrameNumber()) {
            formatted << "[" << entry.Get_frame_number().value() << "] ";
        }
        formatted << entry.Get_logger_name() << ": ";
        formatted << entry.GetDisplayLogLevel() << ": ";
        formatted << entry.Get_message();
        
        std::string result = formatted.str();
        if (static_cast<int>(result.length()) > max_width) {
            result = result.substr(0, max_width - 3) + "...";
        }
        
        return result;
    }
    
    void MainWindow::SetError(const std::string& error_message) {
        last_error = error_message;
        needs_refresh = true;
    }
    
    void MainWindow::ClearError() {
        last_error.clear();
    }
    
    bool MainWindow::IsValidEntryIndex(int index) const {
        return index >= 0 && index < static_cast<int>(filtered_entries.size());
    }
    
    void MainWindow::ClampScrollOffset() {
        if (filtered_entries.size() <= static_cast<size_t>(visible_rows)) {
            // If we have fewer entries than visible rows, always start from the beginning
            scroll_offset = 0;
        } else {
            // Ensure we don't scroll past the point where we can fill all visible rows
            int max_scroll = static_cast<int>(filtered_entries.size()) - visible_rows;
            scroll_offset = std::clamp(scroll_offset, 0, max_scroll);
        }
    }
    
    void MainWindow::ClampSelection() {
        if (!filtered_entries.empty()) {
            selected_entry_index = std::clamp(selected_entry_index, 0, static_cast<int>(filtered_entries.size()) - 1);
        } else {
            selected_entry_index = 0;
        }
    }
    
} // namespace ue_log