#include "main_window.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <filesystem>

namespace ue_log {

using namespace ftxui;

class MainWindowComponent : public ComponentBase {
public:
    MainWindowComponent(MainWindow* parent) : parent_(parent) {}
    
    Element Render() override {
        return parent_->Render();
    }
    
    bool OnEvent(Event event) override {
        // Handle global keyboard shortcuts
        if (event == Event::Character('q') || event == Event::Escape) {
            parent_->Exit();
            return true;
        }
        
        if (event == Event::Character('h') || event == Event::F1) {
            // Toggle help - placeholder for now
            return true;
        }
        
        // Navigation keys
        if (event == Event::ArrowUp || event == Event::Character('k')) {
            parent_->ScrollUp();
            return true;
        }
        if (event == Event::ArrowDown || event == Event::Character('j')) {
            parent_->ScrollDown();
            return true;
        }
        if (event == Event::PageUp) {
            parent_->PageUp();
            return true;
        }
        if (event == Event::PageDown) {
            parent_->PageDown();
            return true;
        }
        if (event == Event::Home) {
            parent_->ScrollToTop();
            return true;
        }
        if (event == Event::End) {
            parent_->ScrollToBottom();
            return true;
        }
        
        return false;
    }
    
private:
    MainWindow* parent_;
};

MainWindow::MainWindow(ConfigManager* config_manager)
    : config_manager_(config_manager) {
    
    // Create owned config manager if none provided
    if (!config_manager_) {
        config_manager_ = new ConfigManager();
        owns_config_manager_ = true;
    }
    
    // Initialize backend components
    log_parser_ = std::make_unique<LogParser>();
    filter_engine_ = std::make_unique<FilterEngine>();
    file_monitor_ = std::make_unique<FileMonitor>();
}

MainWindow::~MainWindow() {
    // Clean up owned config manager
    if (owns_config_manager_) {
        delete config_manager_;
    }
}

void MainWindow::Initialize() {
    // Apply basic configuration
    ApplyConfiguration();
    
    // Create FTXUI component
    component_ = Make<MainWindowComponent>(this);
}

ftxui::Element MainWindow::Render() const {
    // Simple layout for now
    std::vector<Element> main_elements;
    
    // Add log table (takes most of the space)
    main_elements.push_back(RenderLogTable() | flex);
    
    // Add status bar at the bottom
    main_elements.push_back(RenderStatusBar());
    
    // Combine into vertical layout
    return vbox(main_elements);
}

ftxui::Component MainWindow::CreateFTXUIComponent() {
    if (!component_) {
        Initialize();
    }
    return component_;
}

bool MainWindow::LoadLogFile(const std::string& file_path) {
    if (file_path.empty()) {
        last_error_ = "File path is empty";
        return false;
    }
    
    // Store the file path
    current_file_path_ = file_path;
    
    // Create some sample log entries for testing the display
    log_entries_.clear();
    CreateSampleLogEntries();
    
    // Apply filters (for now just copy all entries)
    OnFiltersChanged();
    
    // Reset scroll position and selection
    scroll_offset_ = 0;
    selected_entry_index_ = 0;
    
    last_error_.clear();
    return true;
}

bool MainWindow::ReloadLogFile() {
    if (current_file_path_.empty()) {
        last_error_ = "No file currently loaded";
        return false;
    }
    
    return LoadLogFile(current_file_path_);
}

bool MainWindow::StartRealTimeMonitoring() {
    // Placeholder implementation
    return true;
}

void MainWindow::StopRealTimeMonitoring() {
    // Placeholder implementation
}

bool MainWindow::IsRealTimeMonitoringActive() const {
    // Placeholder implementation
    return false;
}

void MainWindow::Exit() {
    // Call exit callback if set
    if (exit_callback_) {
        exit_callback_();
    }
}

void MainWindow::ScrollUp(int count) {
    if (selected_entry_index_ > 0) {
        SelectEntry(selected_entry_index_ - count);
    }
}

void MainWindow::ScrollDown(int count) {
    if (selected_entry_index_ < static_cast<int>(filtered_entries_.size()) - 1) {
        SelectEntry(selected_entry_index_ + count);
    }
}

void MainWindow::PageUp() {
    int page_size = std::max(1, window_height_ - 3);
    ScrollUp(page_size);
}

void MainWindow::PageDown() {
    int page_size = std::max(1, window_height_ - 3);
    ScrollDown(page_size);
}

void MainWindow::ScrollToTop() {
    SelectEntry(0);
}

void MainWindow::ScrollToBottom() {
    if (!filtered_entries_.empty()) {
        SelectEntry(static_cast<int>(filtered_entries_.size()) - 1);
    }
}

void MainWindow::ApplyConfiguration() {
    // Basic configuration - use defaults for now
    show_line_numbers_ = true;
    auto_scroll_ = true;
}

void MainWindow::SaveConfiguration() {
    // Placeholder implementation
}

ftxui::Element MainWindow::RenderLogTable() const {
    std::vector<Element> rows;
    
    // Add header
    rows.push_back(RenderTableHeader());
    
    // If no entries, show message
    if (filtered_entries_.empty()) {
        if (log_entries_.empty()) {
            rows.push_back(text("No log file loaded. Use command line argument to load a file.") | center);
        } else {
            rows.push_back(text("No entries match the current filters.") | center);
        }
    } else {
        // Calculate visible range for virtual scrolling
        // Use a reasonable default height for the display area
        int visible_height = 15; // Show 15 entries at once
        int start_index = scroll_offset_;
        int end_index = std::min(start_index + visible_height, static_cast<int>(filtered_entries_.size()));
        
        // Render visible log entries
        for (int i = start_index; i < end_index; ++i) {
            bool is_selected = (i == selected_entry_index_);
            rows.push_back(RenderLogEntry(filtered_entries_[i], is_selected));
        }
        
        // Add scroll indicator if there are more entries
        if (filtered_entries_.size() > static_cast<size_t>(visible_height)) {
            std::string scroll_info = "Showing " + std::to_string(start_index + 1) + "-" + 
                                    std::to_string(end_index) + " of " + 
                                    std::to_string(filtered_entries_.size()) + " entries";
            rows.push_back(separator());
            rows.push_back(text(scroll_info) | center | dim);
        }
    }
    
    return window(text(GetTitle()), vbox(rows));
}

ftxui::Element MainWindow::RenderStatusBar() const {
    std::vector<Element> status_elements;
    
    // File info
    std::string file_info = current_file_path_.empty() ? "No file" : 
                           std::filesystem::path(current_file_path_).filename().string();
    status_elements.push_back(text(file_info) | size(WIDTH, EQUAL, 25));
    
    // Entry count
    std::string count_info = std::to_string(log_entries_.size()) + " entries";
    status_elements.push_back(text(count_info) | size(WIDTH, EQUAL, 15));
    
    // Monitoring status
    std::string monitor_info = IsRealTimeMonitoringActive() ? "LIVE" : "STATIC";
    status_elements.push_back(text(monitor_info) | size(WIDTH, EQUAL, 8));
    
    // Error message or help
    std::string message = last_error_.empty() ? "Press 'h' for help, 'q' to quit" : last_error_;
    status_elements.push_back(text(message) | flex);
    
    return hbox(status_elements) | inverted;
}

ftxui::Element MainWindow::RenderFilterPanel() const {
    std::vector<Element> filter_elements;
    
    filter_elements.push_back(text("Filters") | bold);
    filter_elements.push_back(separator());
    filter_elements.push_back(text("No filters active"));
    filter_elements.push_back(separator());
    filter_elements.push_back(text("F - Toggle panel"));
    filter_elements.push_back(text("A - Add filter"));
    
    return window(text("Filters"), vbox(filter_elements));
}

ftxui::Element MainWindow::RenderHelpDialog() const {
    std::vector<Element> help_lines = {
        text("Unreal Log Viewer - Help") | bold | center,
        separator(),
        text("Navigation:"),
        text("  ↑/k     - Move up"),
        text("  ↓/j     - Move down"),
        separator(),
        text("Other:"),
        text("  h/F1    - Show/hide this help"),
        text("  q/Esc   - Quit"),
        separator(),
        text("Press any key to close")
    };
    
    return window(text("Help"), vbox(help_lines)) | 
           size(WIDTH, EQUAL, 40) | size(HEIGHT, EQUAL, 15);
}

ftxui::Element MainWindow::RenderLogEntry(const LogEntry& entry, bool is_selected) const {
    // Helper function to pad string to fixed width
    auto padString = [](const std::string& str, size_t width) -> std::string {
        if (str.length() >= width) {
            return str.substr(0, width);
        }
        return str + std::string(width - str.length(), ' ');
    };
    
    // Build the row as a single formatted string
    std::string row_text;
    
    // Line number column (if enabled)
    if (show_line_numbers_) {
        std::string line_num = std::to_string(entry.Get_line_number());
        row_text += padString(line_num, 4) + " ";
    }
    
    // Timestamp column
    std::string timestamp_str = entry.Get_timestamp().has_value() ? 
                               entry.Get_timestamp().value() : "N/A";
    row_text += padString(timestamp_str, 25) + " ";
    
    // Frame column
    std::string frame_str = entry.Get_frame_number().has_value() ? 
                           std::to_string(entry.Get_frame_number().value()) : "N/A";
    row_text += padString(frame_str, 3) + " ";
    
    // Logger column
    std::string logger_str = entry.Get_logger_name();
    row_text += padString(logger_str, 18) + " ";
    
    // Log level column
    std::string level_str = entry.Get_log_level().has_value() ? 
                           entry.Get_log_level().value() : "N/A";
    row_text += padString(level_str, 8) + " ";
    
    // Message column
    row_text += entry.Get_message();
    
    // Create the element
    Element row = text(row_text);
    
    // Apply log level color to the entire row for now
    if (entry.Get_log_level().has_value()) {
        Color level_color = GetColorForLogLevel(entry.Get_log_level().value());
        // Only apply color if it's an error or warning for visibility
        if (entry.Get_log_level().value() == "Error") {
            row = row | color(Color::RedLight);
        } else if (entry.Get_log_level().value() == "Warning") {
            row = row | color(Color::YellowLight);
        }
    }
    
    // Apply selection highlighting
    if (is_selected) {
        row = row | inverted;
    }
    
    return row;
}

ftxui::Element MainWindow::RenderTableHeader() const {
    // Helper function to pad string to fixed width
    auto padString = [](const std::string& str, size_t width) -> std::string {
        if (str.length() >= width) {
            return str.substr(0, width);
        }
        return str + std::string(width - str.length(), ' ');
    };
    
    // Build the header as a single formatted string
    std::string header_text;
    
    if (show_line_numbers_) {
        header_text += padString("Line", 4) + " ";
    }
    
    header_text += padString("Timestamp", 25) + " ";
    header_text += padString("Frm", 3) + " ";
    header_text += padString("Logger", 18) + " ";
    header_text += padString("Level", 8) + " ";
    header_text += "Message";
    
    return text(header_text) | bold | inverted;
}

ftxui::Color MainWindow::GetColorForLogLevel(const std::string& level) const {
    if (level == "Error") {
        return Color::Red;
    } else if (level == "Warning") {
        return Color::Yellow;
    } else if (level == "Info") {
        return Color::Green;
    } else if (level == "Debug") {
        return Color::GrayLight;
    }
    return Color::White;
}

void MainWindow::SelectEntry(int index) {
    if (filtered_entries_.empty()) {
        selected_entry_index_ = -1;
        return;
    }
    
    // Clamp to valid range
    if (index < 0) {
        selected_entry_index_ = 0;
    } else if (index >= static_cast<int>(filtered_entries_.size())) {
        selected_entry_index_ = static_cast<int>(filtered_entries_.size()) - 1;
    } else {
        selected_entry_index_ = index;
    }
    
    EnsureSelectionVisible();
}

void MainWindow::EnsureSelectionVisible() {
    if (selected_entry_index_ < 0) {
        return;
    }
    
    // Use a fixed visible height that matches our rendering
    int visible_height = 15; // Same as in RenderLogTable
    
    // Only scroll if selection is outside visible area
    // If selection is above visible area, scroll up to show it
    if (selected_entry_index_ < scroll_offset_) {
        scroll_offset_ = selected_entry_index_;
    }
    // If selection is below visible area, scroll down to show it
    else if (selected_entry_index_ >= scroll_offset_ + visible_height) {
        scroll_offset_ = selected_entry_index_ - visible_height + 1;
    }
    
    // Ensure we don't scroll past the beginning or end
    scroll_offset_ = std::max(0, scroll_offset_);
    int max_offset = std::max(0, static_cast<int>(filtered_entries_.size()) - visible_height);
    scroll_offset_ = std::min(scroll_offset_, max_offset);
}

// Placeholder implementations for methods that don't exist in backend yet
void MainWindow::OnNewLogLines(const std::vector<std::string>& new_lines) {
    // Placeholder - will be implemented when backend supports it
}

void MainWindow::OnFiltersChanged() {
    // Placeholder - will be implemented when backend supports it
    filtered_entries_ = log_entries_;
}

void MainWindow::OnFileMonitorError(const std::string& error) {
    last_error_ = "File monitoring error: " + error;
}

void MainWindow::UpdateStatusBar() {
    // Status bar updates are handled in RenderStatusBar()
}

void MainWindow::CreateSampleLogEntries() {
    // Create sample log entries that represent typical Unreal Engine log output
    log_entries_.clear();
    
    // Sample entries with different log levels and formats
    log_entries_.emplace_back(
        LogEntryType::Structured,
        std::string("2024.09.30-14.22.24:342"),
        std::optional<int>(0),
        "LogInit",
        std::string("Display"),
        "Running engine for game: ProjectTitan",
        "[2024.09.30-14.22.24:342][  0]LogInit: Display: Running engine for game: ProjectTitan",
        1
    );
    
    log_entries_.emplace_back(
        LogEntryType::Structured,
        std::string("2024.09.30-14.22.24:343"),
        std::optional<int>(0),
        "LogWindows",
        std::string("Error"),
        "Failed to load 'aqProf.dll' (GetLastError=126)",
        "[2024.09.30-14.22.24:343][  0]LogWindows: Error: Failed to load 'aqProf.dll' (GetLastError=126)",
        2
    );
    
    log_entries_.emplace_back(
        LogEntryType::Structured,
        std::string("2024.09.30-14.22.24:344"),
        std::optional<int>(0),
        "LogCore",
        std::string("Warning"),
        "UTS: Unreal Trace Server launched successfully",
        "[2024.09.30-14.22.24:344][  0]LogCore: Warning: UTS: Unreal Trace Server launched successfully",
        3
    );
    
    log_entries_.emplace_back(
        LogEntryType::SemiStructured,
        std::string("2024.09.30-14.22.24:345"),
        std::optional<int>(0),
        "LogTrace",
        std::nullopt,
        "Initializing trace...",
        "[2024.09.30-14.22.24:345][  0]LogTrace: Initializing trace...",
        4
    );
    
    log_entries_.emplace_back(
        LogEntryType::Structured,
        std::string("2024.09.30-14.22.24:346"),
        std::optional<int>(0),
        "LogPluginManager",
        std::string("Display"),
        "Mounting Engine plugin Bridge",
        "[2024.09.30-14.22.24:346][  0]LogPluginManager: Display: Mounting Engine plugin Bridge",
        5
    );
    
    log_entries_.emplace_back(
        LogEntryType::Structured,
        std::string("2024.09.30-14.22.24:347"),
        std::optional<int>(0),
        "LogConfig",
        std::string("Info"),
        "Set CVar [[r.VSync:0]]",
        "[2024.09.30-14.22.24:347][  0]LogConfig: Info: Set CVar [[r.VSync:0]]",
        6
    );
    
    log_entries_.emplace_back(
        LogEntryType::Unstructured,
        std::nullopt,
        std::nullopt,
        "LogEOSSDK",
        std::string("Error"),
        "LogEOS: [Boot] EOSSDK Version 1.16.3-35276460 booting failed",
        "LogEOSSDK: Error: LogEOS: [Boot] EOSSDK Version 1.16.3-35276460 booting failed",
        7
    );
    
    log_entries_.emplace_back(
        LogEntryType::Structured,
        std::string("2024.09.30-14.22.24:348"),
        std::optional<int>(0),
        "LogOnline",
        std::string("Display"),
        "OSS: Created online subsystem instance for: EIK",
        "[2024.09.30-14.22.24:348][  0]LogOnline: Display: OSS: Created online subsystem instance for: EIK",
        8
    );
    
    log_entries_.emplace_back(
        LogEntryType::Structured,
        std::string("2024.09.30-14.22.24:349"),
        std::optional<int>(0),
        "LogInit",
        std::string("Debug"),
        "Build Configuration: DebugGame",
        "[2024.09.30-14.22.24:349][  0]LogInit: Debug: Build Configuration: DebugGame",
        9
    );
    
    log_entries_.emplace_back(
        LogEntryType::Structured,
        std::string("2024.09.30-14.22.24:350"),
        std::optional<int>(0),
        "LogRendering",
        std::string("Warning"),
        "GPU memory usage exceeds recommended limits",
        "[2024.09.30-14.22.24:350][  0]LogRendering: Warning: GPU memory usage exceeds recommended limits",
        10
    );
    
    // Add more entries to test scrolling
    for (int i = 11; i <= 50; ++i) {
        std::string timestamp = "2024.09.30-14.22.24:" + std::to_string(350 + i);
        std::string level = (i % 4 == 0) ? "Error" : (i % 3 == 0) ? "Warning" : (i % 2 == 0) ? "Info" : "Display";
        std::string logger = "LogTest" + std::to_string(i % 5);
        std::string message = "Sample log message " + std::to_string(i) + " for testing scrolling and display";
        std::string raw = "[" + timestamp + "][  0]" + logger + ": " + level + ": " + message;
        
        log_entries_.emplace_back(
            LogEntryType::Structured,
            timestamp,
            std::optional<int>(0),
            logger,
            level,
            message,
            raw,
            i
        );
    }
}

} // namespace ue_log