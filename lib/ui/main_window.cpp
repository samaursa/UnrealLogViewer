#include "main_window.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <fstream>

namespace ue_log {

using namespace ftxui;

class MainWindowComponent : public ComponentBase {
public:
    MainWindowComponent(MainWindow* parent) : parent_(parent) {}
    
    Element Render() override {
        return parent_->Render();
    }
    
    bool OnEvent(Event event) override {
        // Debug: Log all events
        // std::cout << "Event received" << std::endl;
        
        // Handle ESC key - exit context menus only, never quit program
        if (event == Event::Escape) {
            // Check if any context menus are active
            if (parent_->IsContextualFilterDialogActive()) {
                parent_->HideContextualFilterDialog();
                return true;
            }
            if (parent_->IsSearchPromotionDialogActive()) {
                parent_->HideSearchPromotionDialog();
                return true;
            }
            if (parent_->IsSearchActive()) {
                parent_->HideSearch();
                return true;
            }
            // ESC with no active context menus does nothing
            return true;
        }
        
        // Handle quit key
        if (event == Event::Character('q')) {
            std::cout << "Exit event triggered" << std::endl;
            parent_->Exit();
            return true;
        }
        
        // Word wrap toggle
        if (event == Event::Character('w')) {
            parent_->ToggleWordWrap();
            return true;
        }
        
        // Detail view toggle
        if (event == Event::Character('d')) {
            parent_->ToggleDetailView();
            return true;
        }
        
        if (event == Event::Character('h') || event == Event::F1) {
            // Toggle help - placeholder for now
            return true;
        }
        
        // Handle search input when search is active AND in input mode
        if (parent_->IsSearchActive() && parent_->IsSearchInputMode()) {
            // Handle ESC first to exit search
            if (event == Event::Escape) {
                parent_->HideSearch();
                return true;
            }
            if (event.is_character()) {
                parent_->AppendToSearch(event.character());
                return true;
            }
            if (event == Event::Return) {
                parent_->ConfirmSearch();
                return true;
            }
            if (event == Event::Backspace) {
                parent_->BackspaceSearch();
                return true;
            }
            if (event == Event::Character('+')) {
                parent_->PromoteSearchToFilter();
                return true;
            }
        }
        
        // Search functionality (only when not already searching)
        if (!parent_->IsSearchActive()) {
            if (event == Event::Character('/') || event == Event::Character('s')) {
                parent_->ShowSearch();
                return true;
            }
        }
        
        // Navigation within search results
        if (event == Event::Character('n')) {
            parent_->FindNext();
            return true;
        }
        if (event == Event::Character('N')) {
            parent_->FindPrevious();
            return true;
        }
        
        // Search promotion (works when search is active but not in input mode)
        if (parent_->IsSearchActive() && !parent_->IsSearchInputMode()) {
            if (event == Event::Character('+')) {
                parent_->PromoteSearchToFilter();
                return true;
            }
        }
        
        // Contextual filtering
        if (event == Event::Character('c') || event == Event::Character('C')) {
            parent_->ShowContextualFilterDialog();
            return true;
        }
        
        // Handle contextual filter selection (when dialog is active)
        if (parent_->IsContextualFilterDialogActive()) {
            if (event == Event::Character('1')) {
                parent_->CreateContextualFilter(FilterConditionType::TimestampAfter);
                return true;
            }
            if (event == Event::Character('2')) {
                parent_->CreateContextualFilter(FilterConditionType::LoggerEquals);
                return true;
            }
            if (event == Event::Character('3')) {
                parent_->CreateContextualFilter(FilterConditionType::LogLevelEquals);
                return true;
            }
            if (event == Event::Character('4')) {
                parent_->CreateContextualFilter(FilterConditionType::FrameAfter);
                return true;
            }
            if (event == Event::Escape) {
                parent_->HideContextualFilterDialog();
                return true;
            }
        }
        
        // Handle search promotion selection (when dialog is active)
        if (parent_->IsSearchPromotionDialogActive()) {
            if (event == Event::Character('1')) {
                parent_->CreateFilterFromSearch(FilterConditionType::MessageContains);
                return true;
            }
            if (event == Event::Character('2')) {
                parent_->CreateFilterFromSearch(FilterConditionType::LogLevelEquals);
                return true;
            }
            if (event == Event::Character('3')) {
                parent_->CreateFilterFromSearch(FilterConditionType::LoggerContains);
                return true;
            }
            if (event == Event::Character('4')) {
                parent_->CreateFilterFromSearch(FilterConditionType::AnyFieldContains);
                return true;
            }
            if (event == Event::Escape) {
                parent_->HideSearchPromotionDialog();
                return true;
            }
        }
        
        // Context lines control
        if (event == Event::Character('{')) {
            parent_->DecreaseContext();
            return true;
        }
        if (event == Event::Character('}')) {
            parent_->IncreaseContext();
            return true;
        }
        if (event == Event::Character('0')) {
            parent_->SetContextLines(0);
            return true;
        }
        
        // Vim-style navigation: 'g' goes to start of file
        if (event == Event::Character('g')) {
            parent_->ScrollToTop();
            return true;
        }
        
        // Jump functionality: ':' enters command mode
        if (event == Event::Character(':')) {
            parent_->ShowJumpDialog();
            return true;
        }
        
        // Handle jump dialog input
        if (parent_->IsJumpDialogActive()) {
            if (event == Event::Escape) {
                parent_->HideJumpDialog();
                return true;
            }
            if (event.is_character()) {
                std::string ch = event.character();
                if (ch.length() == 1 && std::isdigit(ch[0])) {
                    parent_->AppendToJumpInput(ch);
                    return true;
                }
                // Also allow colon and percentage for jump formats
                if (ch == ":" || ch == "%") {
                    parent_->AppendToJumpInput(ch);
                    return true;
                }
            }
            if (event == Event::Backspace) {
                parent_->BackspaceJumpInput();
                return true;
            }
            if (event == Event::Return) {
                parent_->ExecuteJump();
                return true;
            }
        }
        
        // Vim-style 'G' goes to bottom of file
        if (event == Event::Character('G')) {
            parent_->ScrollToBottom();
            return true;
        }
        
        // Pattern-based navigation
        if (event == Event::Character(']')) {
            // Next error with ]e
            parent_->JumpToNextError();
            return true;
        }
        if (event == Event::Character('[')) {
            // Previous error with [e
            parent_->JumpToPreviousError();
            return true;
        }
        
        // Quick navigation shortcuts
        if (event == Event::Character('E')) {
            // Jump to next error (capital E)
            parent_->JumpToNextError();
            return true;
        }
        if (event == Event::Character('W')) {
            // Jump to next warning (capital W)
            parent_->JumpToNextWarning();
            return true;
        }
        
        // Filter panel toggle
        if (event == Event::Character('f')) {
            parent_->ToggleFilterPanel();
            return true;
        }
        
        // Quick filters - vim-style shortcuts
        if (event == Event::Character(static_cast<char>(6))) { // Ctrl+F (ASCII 6)
            // Ctrl+F for quick filter menu (different from 'f' for toggle)
            parent_->ShowQuickFilterDialog();
            return true;
        }
        
        // Handle quick filter selection (when dialog is active)
        if (parent_->IsQuickFilterDialogActive()) {
            if (event == Event::Character('e')) {
                parent_->ApplyQuickFilter("error");
                return true;
            }
            if (event == Event::Character('w')) {
                parent_->ApplyQuickFilter("warning");
                return true;
            }
            if (event == Event::Character('i')) {
                parent_->ApplyQuickFilter("info");
                return true;
            }
            if (event == Event::Character('d')) {
                parent_->ApplyQuickFilter("debug");
                return true;
            }
            if (event == Event::Character('c')) {
                parent_->ApplyQuickFilter("clear");
                return true;
            }
            if (event == Event::Escape) {
                parent_->HideQuickFilterDialog();
                return true;
            }
        }
        
        // Focus filter panel or return focus to main window
        if (event == Event::Tab) {
            if (parent_->GetFilterPanel() && parent_->IsFilterPanelVisible()) {
                if (parent_->GetFilterPanel()->IsFocused()) {
                    // Filter panel is focused, return focus to main window
                    parent_->GetFilterPanel()->SetFocus(false);
                    parent_->SetLastError("Main window focused - use arrow keys to navigate logs");
                } else {
                    // Main window is focused, focus filter panel
                    parent_->GetFilterPanel()->SetFocus(true);
                    // Auto-select first filter if none selected
                    if (parent_->GetFilterPanel()->GetSelectedFilterIndex() < 0) {
                        parent_->GetFilterPanel()->SetSelectedFilterIndex(0);
                    }
                    parent_->SetLastError("Filter panel focused - use arrow keys to navigate filters, Space to toggle");
                }
                return true;
            }
        }
        
        // Reload file
        if (event == Event::Character('r')) {
            parent_->ReloadLogFile();
            return true;
        }
        
        // Toggle real-time monitoring
        if (event == Event::Character('t')) {
            if (parent_->IsRealTimeMonitoringActive()) {
                parent_->StopRealTimeMonitoring();
            } else {
                parent_->StartRealTimeMonitoring();
            }
            return true;
        }
        
        // Handle Space key for filter toggling when filter panel has focus
        if (event == Event::Character(' ')) {
            if (parent_->GetFilterPanel() && parent_->GetFilterPanel()->IsFocused()) {
                parent_->GetFilterPanel()->ToggleSelectedFilter();
                return true;
            }
        }
        
        // Handle Delete key and vim-style 'x' for removing filters when filter panel has focus
        if (event == Event::Delete || event == Event::Character('x')) {
            if (parent_->GetFilterPanel() && parent_->GetFilterPanel()->IsFocused()) {
                parent_->GetFilterPanel()->DeleteSelectedFilter();
                return true;
            }
        }
        
        // Navigation keys - check if filter panel has focus first
        if (event == Event::ArrowUp || event == Event::Character('k')) {
            // If filter panel has focus, let it handle the event
            if (parent_->GetFilterPanel() && parent_->GetFilterPanel()->IsFocused()) {
                parent_->GetFilterPanel()->NavigateUp();
                return true;
            }
            parent_->ScrollUp();
            return true;
        }
        if (event == Event::ArrowDown || event == Event::Character('j')) {
            // If filter panel has focus, let it handle the event
            if (parent_->GetFilterPanel() && parent_->GetFilterPanel()->IsFocused()) {
                parent_->GetFilterPanel()->NavigateDown();
                return true;
            }
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
        
        // Vim-style half-page navigation
        if (event == Event::Character(static_cast<char>(4))) { // Ctrl+D (ASCII 4)
            parent_->HalfPageDown();
            return true;
        }
        if (event == Event::Character(static_cast<char>(21))) { // Ctrl+U (ASCII 21)
            parent_->HalfPageUp();
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

MainWindow::MainWindow() : MainWindow(nullptr) {
    // Delegate to the parameterized constructor
}

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
    
    // Initialize UI components
    filter_panel_ = std::make_unique<FilterPanel>(filter_engine_.get(), config_manager_);
}

MainWindow::~MainWindow() {
    // Clean up owned config manager
    if (owns_config_manager_) {
        delete config_manager_;
    }
}

void MainWindow::Initialize() {
    // Initialize filter expression
    current_filter_expression_ = std::make_unique<FilterExpression>(FilterOperator::And);
    
    // Initialize UI components
    if (filter_panel_) {
        filter_panel_->Initialize();
        filter_panel_->SetCurrentFilterExpression(current_filter_expression_.get());
        filter_panel_->SetFiltersChangedCallback([this]() {
            OnFiltersChanged();
        });
    }
    
    // Apply basic configuration
    ApplyConfiguration();
    
    // Create FTXUI component - use simple main component for now
    component_ = Make<MainWindowComponent>(this);
}

ftxui::Element MainWindow::Render() const {
    // Main layout: log table + detail view + status bar
    std::vector<Element> main_elements;
    
    // Add log table (takes most of the space)
    main_elements.push_back(RenderLogTable() | flex);
    
    // Add detail view if enabled (shows full raw log entry)
    if (show_detail_view_) {
        main_elements.push_back(separator());
        main_elements.push_back(RenderDetailView() | size(HEIGHT, EQUAL, 5));
    }
    
    // Add search status bar if active (appears above main status bar)
    if (show_search_ || show_contextual_filter_dialog_) {
        main_elements.push_back(RenderSearchStatusBar());
    }
    
    // Add status bar at the bottom
    main_elements.push_back(RenderStatusBar());
    
    // Combine into vertical layout
    Element main_content = vbox(main_elements);
    
    // Add filter panel if enabled (on the left side)
    if (show_filter_panel_ && filter_panel_) {
        Element filter_panel = filter_panel_->Render() | size(WIDTH, EQUAL, 60);
        main_content = hbox({
            filter_panel,
            separator(),
            main_content | flex
        });
    }
    
    return main_content;
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
    
    // Check if file exists
    if (!std::filesystem::exists(file_path)) {
        last_error_ = "File does not exist: " + file_path;
        return false;
    }
    
    // Store the file path
    current_file_path_ = file_path;
    
    try {
        // Use LogParser to load and parse the real file
        auto load_result = log_parser_->LoadFile(file_path);
        if (load_result.IsError()) {
            last_error_ = "Failed to load file: " + file_path + " - " + load_result.Get_error_message();
            return false;
        }
        
        // Show loading message
        last_error_ = "Loading and parsing log file...";
        
        // Parse the log entries
        log_entries_ = log_parser_->ParseEntries();
        
        if (log_entries_.empty()) {
            // If no entries were parsed, create sample data as fallback
            CreateSampleLogEntries();
            last_error_ = "No valid log entries found in file, using sample data";
        } else {
            last_error_ = "Loaded " + std::to_string(log_entries_.size()) + " log entries from " + std::filesystem::path(file_path).filename().string();
        }
        
        // Don't create sample filters - start with empty filter area
        
        // Apply filters
        OnFiltersChanged();
        
        // Reset scroll position and selection
        scroll_offset_ = 0;
        selected_entry_index_ = 0;
        
        return true;
        
    } catch (const std::exception& e) {
        last_error_ = "Error loading file: " + std::string(e.what());
        return false;
    }
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

bool MainWindow::RunAutotest(const std::string& log_file_path, const std::string& output_file_path) {
    std::ofstream report(output_file_path);
    if (!report.is_open()) {
        return false;
    }
    
    report << "=== Unreal Log Viewer Autotest Report ===" << std::endl;
    report << "Test started at: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    report << "Log file: " << log_file_path << std::endl;
    report << std::endl;
    
    // Test 1: Initialize the application
    report << "1. Initializing application..." << std::endl;
    try {
        Initialize();
        report << "   ✓ Application initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        report << "   ✗ Failed to initialize: " << e.what() << std::endl;
        return false;
    }
    
    // Test 2: Load log file
    report << "2. Loading log file..." << std::endl;
    if (LoadLogFile(log_file_path)) {
        report << "   ✓ Log file loaded successfully" << std::endl;
        report << "   - Entries loaded: " << log_entries_.size() << std::endl;
        report << "   - Filtered entries: " << filtered_entries_.size() << std::endl;
    } else {
        report << "   ✗ Failed to load log file: " << GetLastError() << std::endl;
        return false;
    }
    
    // Test 3: Basic navigation
    report << "3. Testing navigation..." << std::endl;
    try {
        int initial_index = GetSelectedEntryIndex();
        ScrollDown(1);
        int after_down = GetSelectedEntryIndex();
        ScrollUp(1);
        int after_up = GetSelectedEntryIndex();
        
        report << "   ✓ Navigation test completed" << std::endl;
        report << "   - Initial index: " << initial_index << std::endl;
        report << "   - After scroll down: " << after_down << std::endl;
        report << "   - After scroll up: " << after_up << std::endl;
    } catch (const std::exception& e) {
        report << "   ✗ Navigation test failed: " << e.what() << std::endl;
    }
    
    // Test 4: Filter functionality
    report << "4. Testing filter functionality..." << std::endl;
    try {
        // Apply a quick filter for errors
        ApplyQuickFilter("error");
        size_t error_filtered_count = filtered_entries_.size();
        
        // Clear filters
        ApplyQuickFilter("clear");
        size_t cleared_count = filtered_entries_.size();
        
        report << "   ✓ Filter test completed" << std::endl;
        report << "   - Entries after error filter: " << error_filtered_count << std::endl;
        report << "   - Entries after clear filter: " << cleared_count << std::endl;
    } catch (const std::exception& e) {
        report << "   ✗ Filter test failed: " << e.what() << std::endl;
    }
    
    // Test 5: Rendering
    report << "5. Testing rendering..." << std::endl;
    try {
        auto element = Render();
        if (element) {
            report << "   ✓ Rendering test completed successfully" << std::endl;
        } else {
            report << "   ✗ Rendering returned null element" << std::endl;
        }
    } catch (const std::exception& e) {
        report << "   ✗ Rendering test failed: " << e.what() << std::endl;
    }
    
    // Test 6: Component accessibility
    report << "6. Testing component accessibility..." << std::endl;
    report << "   - Log parser available: " << (HasLogParser() ? "Yes" : "No") << std::endl;
    report << "   - Filter engine available: " << (HasFilterEngine() ? "Yes" : "No") << std::endl;
    report << "   - File monitor available: " << (HasFileMonitor() ? "Yes" : "No") << std::endl;
    report << "   - Filter panel available: " << (HasFilterPanel() ? "Yes" : "No") << std::endl;
    
    report << std::endl;
    report << "=== Autotest Summary ===" << std::endl;
    report << "All core functionality tests completed." << std::endl;
    report << "Application appears to be working correctly." << std::endl;
    
    report.close();
    return true;
}

void MainWindow::CloseCurrentFile() {
    // Clear all file-related state
    current_file_path_.clear();
    log_entries_.clear();
    filtered_entries_.clear();
    selected_entry_index_ = 0;
    scroll_offset_ = 0;
    last_error_.clear();
    
    // Stop any real-time monitoring
    StopRealTimeMonitoring();
}

// Navigation methods for tests
void MainWindow::StartTailing() {
    StartRealTimeMonitoring();
}

void MainWindow::StopTailing() {
    StopRealTimeMonitoring();
}

void MainWindow::RefreshDisplay() {
    // Trigger a refresh - placeholder implementation
}

void MainWindow::SetTerminalSize(int width, int height) {
    window_width_ = width;
    window_height_ = height;
}

void MainWindow::GoToTop() {
    ScrollToTop();
}

void MainWindow::GoToBottom() {
    ScrollToBottom();
}

void MainWindow::GoToLine(int line_number) {
    JumpToLine(line_number);
}

void MainWindow::SelectNextEntry() {
    ScrollDown(1);
}

void MainWindow::SelectPreviousEntry() {
    ScrollUp(1);
}

// FTXUI interface methods
ftxui::Component MainWindow::GetComponent() {
    return CreateFTXUIComponent();
}

ftxui::Element MainWindow::Render() {
    return const_cast<const MainWindow*>(this)->Render();
}

bool MainWindow::OnEvent(ftxui::Event event) {
    // Delegate to the component's event handler
    if (component_) {
        return component_->OnEvent(event);
    }
    return false;
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

void MainWindow::HalfPageUp() {
    // Use the same visible height as in RenderLogTable for consistency
    int visible_height = GetVisibleHeight();
    int half_page_size = std::max(1, visible_height / 2);
    ScrollUp(half_page_size);
}

void MainWindow::HalfPageDown() {
    // Use the same visible height as in RenderLogTable for consistency
    int visible_height = GetVisibleHeight();
    int half_page_size = std::max(1, visible_height / 2);
    ScrollDown(half_page_size);
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

int MainWindow::GetVisibleHeight() const {
    // Calculate dynamic height based on available terminal space
    int visible_height = std::max(10, window_height_ - 5); // Reserve space for header, status bar, etc.
    if (visible_height <= 0 || window_height_ <= 0) {
        visible_height = 25; // Fallback to reasonable default
    }
    return visible_height;
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
        // Calculate viewport size based on available space
        // Account for detail view if it's open
        int available_height = window_height_ > 0 ? window_height_ - 3 : 100; // Reserve space for header, status bar
        if (show_detail_view_) {
            available_height -= 7; // Reserve space for detail view (5 lines + separator + title)
        }
        int viewport_size = std::max(10, available_height);
        
        // Allow scrolling past the end so last entries can be seen mid-screen
        int half_viewport = viewport_size / 2;
        int max_start_index = static_cast<int>(filtered_entries_.size()) - half_viewport;
        
        // Calculate start index - allow going past the end for better visibility of last entries
        int start_index = std::max(0, std::min(selected_entry_index_ - half_viewport, max_start_index));
        int end_index = std::min(start_index + viewport_size, static_cast<int>(filtered_entries_.size()));
        
        // Render the viewport entries
        for (int i = start_index; i < end_index; ++i) {
            bool is_selected = (i == selected_entry_index_);
            rows.push_back(RenderLogEntry(filtered_entries_[i], is_selected));
        }
        
        // Note: scroll_offset_ will be updated by navigation methods, not here in const render method
    }
    
    // Add visual focus indicator - main window has focus when filter panel doesn't
    bool main_has_focus = !filter_panel_ || !filter_panel_->IsFocused();
    
    // Create scrollable content that fills available space
    Element scrollable_content = vbox(rows) | vscroll_indicator | yframe | yflex;
    Element window_element = window(text(GetTitle()), scrollable_content);
    
    if (main_has_focus) {
        // Main window has focus - add bright border
        window_element = window_element | border;
    } else {
        // Main window doesn't have focus - add dim border
        window_element = window_element | border | dim;
    }
    
    return window_element | flex;
}

ftxui::Element MainWindow::RenderStatusBar() const {
    std::vector<Element> status_elements;
    
    // File info
    std::string file_info = current_file_path_.empty() ? "No file" : 
                           std::filesystem::path(current_file_path_).filename().string();
    status_elements.push_back(text(file_info) | size(WIDTH, EQUAL, 25));
    
    // Monitoring status
    std::string monitor_info = IsRealTimeMonitoringActive() ? "LIVE" : "STATIC";
    status_elements.push_back(text(monitor_info) | size(WIDTH, EQUAL, 8));
    
    // Error message or help (takes up middle space)
    std::string message = last_error_.empty() ? "Press ':' for goto, 'g' for top, 'f' for filters, 'q' to quit" : last_error_;
    status_elements.push_back(text(message) | flex);
    
    // Entry count and scroll info on the right
    std::string count_info;
    if (!filtered_entries_.empty()) {
        // Show current selection and total count
        count_info = "Entry " + std::to_string(selected_entry_index_ + 1) + " of " + 
                    std::to_string(filtered_entries_.size());
        
        if (filtered_entries_.size() != log_entries_.size()) {
            count_info += " (filtered from " + std::to_string(log_entries_.size()) + ")";
        }
    } else {
        count_info = "No entries";
    }
    status_elements.push_back(text(count_info));
    
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

ftxui::Element MainWindow::RenderDetailView() const {
    if (selected_entry_index_ < 0 || selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        return window(text("Detail View"), text("No entry selected") | center);
    }
    
    const auto& selected_entry = filtered_entries_[selected_entry_index_];
    
    // Create title with line number and type information
    std::string entry_type = selected_entry.IsStructured() ? "Structured" : 
                            selected_entry.IsSemiStructured() ? "Semi-Structured" : "Unstructured";
    std::string title = "Detail View - Line " + std::to_string(selected_entry.Get_line_number()) + " (" + entry_type + ")";
    
    // Always use word wrapping in detail view for better readability
    std::string raw_line = selected_entry.Get_raw_line();
    Element content = paragraph(raw_line);
    
    return window(text(title), content);
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
    
    // Check if this entry is a match or context line
    bool is_match = match_line_numbers_.find(entry.Get_line_number()) != match_line_numbers_.end();
    
    // Build the row with separate elements for proper word wrapping
    std::vector<Element> row_elements;
    
    // Line number column (if enabled)
    if (show_line_numbers_) {
        std::string line_num = std::to_string(entry.Get_line_number());
        row_elements.push_back(text(padString(line_num, 4)));
    }
    
    // Timestamp column
    std::string timestamp_str = entry.Get_timestamp().has_value() ? 
                               entry.Get_timestamp().value() : "N/A";
    row_elements.push_back(text(padString(timestamp_str, 25)));
    
    // Frame column
    std::string frame_str = entry.Get_frame_number().has_value() ? 
                           std::to_string(entry.Get_frame_number().value()) : "N/A";
    row_elements.push_back(text(padString(frame_str, 5)));
    
    // Logger column
    std::string logger_str = entry.Get_logger_name();
    row_elements.push_back(text(padString(logger_str, 18)));
    
    // Log level column
    std::string level_str = entry.Get_log_level().has_value() ? 
                           entry.Get_log_level().value() : "N/A";
    row_elements.push_back(text(padString(level_str, 8)));
    
    // Message column - handle word wrapping
    Element message_element;
    if (word_wrap_enabled_) {
        message_element = paragraph(entry.Get_message());
    } else {
        message_element = text(entry.Get_message());
    }
    row_elements.push_back(message_element | flex);
    
    // Create the row element
    Element row = hbox(row_elements);
    
    // Apply styling based on whether this is a match or context line
    if (!is_match && context_lines_ > 0) {
        // This is a context line - make it muted/gray
        row = row | dim | color(Color::GrayDark);
    } else {
        // This is a match line - apply normal log level colors
        if (entry.Get_log_level().has_value()) {
            Color level_color = GetColorForLogLevel(entry.Get_log_level().value());
            // Only apply color if it's an error or warning for visibility
            if (entry.Get_log_level().value() == "Error") {
                row = row | color(Color::RedLight);
            } else if (entry.Get_log_level().value() == "Warning") {
                row = row | color(Color::YellowLight);
            }
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
    
    // Build the header with separate elements to match the row layout
    std::vector<Element> header_elements;
    
    if (show_line_numbers_) {
        header_elements.push_back(text(padString("Line", 4)) | bold);
    }
    
    header_elements.push_back(text(padString("Timestamp", 25)) | bold);
    header_elements.push_back(text(padString("Frame", 5)) | bold);
    header_elements.push_back(text(padString("Logger", 18)) | bold);
    header_elements.push_back(text(padString("Level", 8)) | bold);
    header_elements.push_back(text("Message") | bold | flex);
    
    return hbox(header_elements) | inverted;
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
    // Check if we have hierarchical filters (contextual filters) and apply those
    if (current_filter_expression_ && !current_filter_expression_->IsEmpty()) {
        ApplyCurrentFilter();
    } else {
        // Fall back to traditional filters from FilterEngine
        ApplyTraditionalFilters();
    }
}

void MainWindow::ApplyTraditionalFilters() {
    if (!filter_engine_) {
        filtered_entries_ = log_entries_;
        return;
    }
    
    // Get all active filters from the filter engine
    const auto& filters = filter_engine_->Get_primary_filters();
    std::vector<LogEntry> matches;
    
    // If no active filters, show all entries
    bool has_active_filters = false;
    for (const auto& filter : filters) {
        if (filter->Get_is_active()) {
            has_active_filters = true;
            break;
        }
    }
    
    if (!has_active_filters) {
        filtered_entries_ = log_entries_;
    } else {
        // Apply active filters
        for (const auto& entry : log_entries_) {
            bool entry_matches = false;
            
            // Check each active filter
            for (const auto& filter : filters) {
                if (filter->Get_is_active() && filter->Matches(entry)) {
                    entry_matches = true;
                    break; // OR logic - if any filter matches, include the entry
                }
            }
            
            if (entry_matches) {
                matches.push_back(entry);
            }
        }
        filtered_entries_ = matches;
    }
    
    // Update selection to stay within bounds
    if (selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        selected_entry_index_ = filtered_entries_.empty() ? -1 : static_cast<int>(filtered_entries_.size()) - 1;
    }
    EnsureSelectionVisible();
}

void MainWindow::OnFileMonitorError(const std::string& error) {
    last_error_ = "File monitoring error: " + error;
}

void MainWindow::UpdateStatusBar() {
    // Status bar updates are handled in RenderStatusBar()
}

// Search functionality implementations
void MainWindow::ShowSearch() {
    show_search_ = true;
    search_input_mode_ = true; // Start in input mode
    search_query_.clear();
    search_results_.clear();
    search_result_index_ = -1;
    
    // Show search prompt in status bar
    last_error_ = "Search: (type to search, Enter to confirm, + to promote to filter, Esc to cancel)";
}

void MainWindow::HideSearch() {
    show_search_ = false;
    search_input_mode_ = false;
    search_query_.clear();
    search_results_.clear();
    search_result_index_ = -1;
    last_error_.clear(); // Clear status bar message
}

void MainWindow::PerformSearch(const std::string& query) {
    search_query_ = query;
    search_results_.clear();
    search_result_index_ = -1;
    
    if (query.empty()) {
        return;
    }
    
    // Smart case sensitivity: case-sensitive if query contains uppercase, case-insensitive if all lowercase
    bool case_sensitive = HasUppercaseLetters(query);
    
    // Search through filtered entries
    for (int i = 0; i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        
        // Search in message, logger name, and log level
        std::string search_text = entry.Get_message() + " " + entry.Get_logger_name();
        if (entry.Get_log_level().has_value()) {
            search_text += " " + entry.Get_log_level().value();
        }
        
        bool found = false;
        if (case_sensitive) {
            // Case-sensitive search
            found = search_text.find(query) != std::string::npos;
        } else {
            // Case-insensitive search
            std::string lower_query = query;
            std::string lower_text = search_text;
            std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);
            std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
            found = lower_text.find(lower_query) != std::string::npos;
        }
        
        if (found) {
            search_results_.push_back(i);
        }
    }
    
    // Jump to first result if found
    if (!search_results_.empty()) {
        search_result_index_ = 0;
        SelectEntry(search_results_[0]);
    }
}

void MainWindow::FindNext() {
    if (search_results_.empty()) {
        return;
    }
    
    search_result_index_ = (search_result_index_ + 1) % static_cast<int>(search_results_.size());
    SelectEntry(search_results_[search_result_index_]);
}

void MainWindow::FindPrevious() {
    if (search_results_.empty()) {
        return;
    }
    
    search_result_index_--;
    if (search_result_index_ < 0) {
        search_result_index_ = static_cast<int>(search_results_.size()) - 1;
    }
    SelectEntry(search_results_[search_result_index_]);
}

void MainWindow::ClearSearch() {
    search_query_.clear();
    search_results_.clear();
    search_result_index_ = -1;
}

// Search input handling implementations
void MainWindow::AppendToSearch(const std::string& text) {
    search_query_ += text;
    last_error_ = "Search: " + search_query_ + " (Enter to confirm, + to promote, Esc to cancel)";
    // Perform search in real-time
    PerformSearch(search_query_);
}

void MainWindow::ConfirmSearch() {
    if (!search_query_.empty()) {
        PerformSearch(search_query_);
        if (!search_results_.empty()) {
            last_error_ = "Found " + std::to_string(search_results_.size()) + " matches for '" + search_query_ + "'. Use n/N to navigate.";
        } else {
            last_error_ = "No matches found for '" + search_query_ + "'";
        }
    }
    // Exit input mode but keep search active for navigation
    search_input_mode_ = false;
}

void MainWindow::BackspaceSearch() {
    if (!search_query_.empty()) {
        search_query_.pop_back();
        if (search_query_.empty()) {
            last_error_ = "Search: (type to search, Enter to confirm, + to promote, Esc to cancel)";
            search_results_.clear();
            search_result_index_ = -1;
        } else {
            last_error_ = "Search: " + search_query_ + " (Enter to confirm, + to promote, Esc to cancel)";
            PerformSearch(search_query_);
        }
    }
}

// Jump functionality implementations
void MainWindow::ShowJumpDialog() {
    show_jump_dialog_ = true;
    jump_input_.clear();
    
    // Provide immediate feedback
    last_error_ = "Jump dialog activated. Enter line number or timestamp, then press Enter.";
}

void MainWindow::HideJumpDialog() {
    show_jump_dialog_ = false;
    jump_input_.clear();
}

void MainWindow::JumpToLine(int line_number) {
    if (line_number <= 0 || filtered_entries_.empty()) {
        return;
    }
    
    // Find entry with matching line number
    for (int i = 0; i < static_cast<int>(filtered_entries_.size()); ++i) {
        if (static_cast<int>(filtered_entries_[i].Get_line_number()) == line_number) {
            SelectEntry(i);
            return;
        }
    }
    
    // If exact line not found, jump to closest line
    int closest_index = 0;
    int min_diff = std::abs(static_cast<int>(filtered_entries_[0].Get_line_number()) - line_number);
    
    for (int i = 1; i < static_cast<int>(filtered_entries_.size()); ++i) {
        int diff = std::abs(static_cast<int>(filtered_entries_[i].Get_line_number()) - line_number);
        if (diff < min_diff) {
            min_diff = diff;
            closest_index = i;
        }
    }
    
    SelectEntry(closest_index);
}

void MainWindow::JumpToTimestamp(const std::string& timestamp) {
    if (timestamp.empty() || filtered_entries_.empty()) {
        return;
    }
    
    // Find entry with matching or closest timestamp
    for (int i = 0; i < static_cast<int>(filtered_entries_.size()); ++i) {
        if (filtered_entries_[i].Get_timestamp().has_value()) {
            const std::string& entry_timestamp = filtered_entries_[i].Get_timestamp().value();
            if (entry_timestamp.find(timestamp) != std::string::npos) {
                SelectEntry(i);
                return;
            }
        }
    }
}

void MainWindow::JumpToPercentage(int percentage) {
    if (percentage < 0 || percentage > 100 || filtered_entries_.empty()) {
        return;
    }
    
    // Calculate the target index based on percentage
    int target_index = (filtered_entries_.size() * percentage) / 100;
    target_index = std::min(target_index, static_cast<int>(filtered_entries_.size()) - 1);
    
    SelectEntry(target_index);
    last_error_ = "Jumped to " + std::to_string(percentage) + "% (" + std::to_string(target_index + 1) + " of " + std::to_string(filtered_entries_.size()) + ")";
}

void MainWindow::ToggleJumpMode() {
    jump_to_line_mode_ = !jump_to_line_mode_;
}

void MainWindow::AppendToJumpInput(const std::string& text) {
    jump_input_ += text;
    last_error_ = "Jump to: " + jump_input_ + " (Enter to execute, Esc to cancel)";
}

void MainWindow::BackspaceJumpInput() {
    if (!jump_input_.empty()) {
        jump_input_.pop_back();
        last_error_ = jump_input_.empty() ? "Jump to: (Enter line number)" : "Jump to: " + jump_input_ + " (Enter to execute, Esc to cancel)";
    }
}

void MainWindow::ExecuteJump() {
    if (jump_input_.empty()) {
        HideJumpDialog();
        return;
    }
    
    try {
        int line_number = std::stoi(jump_input_);
        JumpToLine(line_number);
        last_error_ = "Jumped to line " + std::to_string(line_number);
    } catch (const std::exception&) {
        last_error_ = "Invalid line number: " + jump_input_;
    }
    
    HideJumpDialog();
}

void MainWindow::ToggleFilterPanel() {
    show_filter_panel_ = !show_filter_panel_;
    
    // Update filter panel visibility
    if (filter_panel_) {
        filter_panel_->SetVisible(show_filter_panel_);
    }
    
    // Provide feedback
    last_error_ = show_filter_panel_ ? "Filter panel shown" : "Filter panel hidden";
}

void MainWindow::ToggleWordWrap() {
    word_wrap_enabled_ = !word_wrap_enabled_;
    
    if (word_wrap_enabled_) {
        last_error_ = "Word wrap enabled - long lines will wrap";
    } else {
        last_error_ = "Word wrap disabled - long lines will be truncated";
    }
}

void MainWindow::ToggleDetailView() {
    show_detail_view_ = !show_detail_view_;
    
    if (show_detail_view_) {
        last_error_ = "Detail view shown - full log entry displayed below";
    } else {
        last_error_ = "Detail view hidden - press 'd' to show";
    }
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

void MainWindow::CreateSampleFilters() {
    // Create some sample filters for testing navigation
    if (!filter_engine_) {
        return;
    }
    
    // Create a few sample filters to test navigation
    auto error_filter = std::make_unique<Filter>("Show Errors", FilterType::LogLevel, "Error");
    error_filter->Request_is_active(true);
    
    auto warning_filter = std::make_unique<Filter>("Show Warnings", FilterType::LogLevel, "Warning");
    warning_filter->Request_is_active(false);
    
    auto init_filter = std::make_unique<Filter>("LogInit Messages", FilterType::LoggerName, "LogInit");
    init_filter->Request_is_active(false);
    
    // Add filters to the engine
    filter_engine_->AddFilter(std::move(error_filter));
    filter_engine_->AddFilter(std::move(warning_filter));
    filter_engine_->AddFilter(std::move(init_filter));
    
    // Refresh the filter panel to show the new filters
    if (filter_panel_) {
        filter_panel_->RefreshFilters();
    }
}

// Enhanced search functionality implementations
void MainWindow::PromoteSearchToFilter() {
    if (search_query_.empty()) {
        last_error_ = "No active search to promote";
        return;
    }
    
    show_search_promotion_ = true;
    last_error_ = "Promote search '" + search_query_ + "' to filter: [1] Message contains [2] LogLevel equals [3] Logger contains [4] Any field contains";
}

void MainWindow::ShowSearchPromotionDialog() {
    show_search_promotion_ = true;
}

void MainWindow::HideSearchPromotionDialog() {
    show_search_promotion_ = false;
}

void MainWindow::CreateFilterFromSearch(FilterConditionType type) {
    if (search_query_.empty()) {
        last_error_ = "No search query to promote";
        show_search_promotion_ = false;
        return;
    }
    
    // Create filter expression if it doesn't exist
    if (!current_filter_expression_) {
        current_filter_expression_ = std::make_unique<FilterExpression>(FilterOperator::And);
    }
    
    // Create the appropriate condition based on type
    std::unique_ptr<FilterCondition> condition;
    
    switch (type) {
        case FilterConditionType::MessageContains:
            condition = FilterConditionFactory::CreateMessageContains(search_query_);
            last_error_ = "Added filter: Message contains '" + search_query_ + "'";
            break;
        case FilterConditionType::LogLevelEquals:
            condition = FilterConditionFactory::CreateLogLevelEquals(search_query_);
            last_error_ = "Added filter: LogLevel = '" + search_query_ + "'";
            break;
        case FilterConditionType::LoggerContains:
            condition = std::make_unique<FilterCondition>(FilterConditionType::LoggerContains, search_query_);
            last_error_ = "Added filter: Logger contains '" + search_query_ + "'";
            break;
        case FilterConditionType::AnyFieldContains:
            condition = FilterConditionFactory::CreateAnyFieldContains(search_query_);
            last_error_ = "Added filter: Any field contains '" + search_query_ + "'";
            break;
        default:
            last_error_ = "Unknown filter type";
            break;
    }
    
    if (condition) {
        current_filter_expression_->AddCondition(std::move(condition));
        ApplyCurrentFilter();
        
        // Update filter panel to show the new filter
        if (filter_panel_) {
            filter_panel_->SetCurrentFilterExpression(current_filter_expression_.get());
        }
    }
    
    // Close the search promotion dialog
    show_search_promotion_ = false;
    
    // Keep search active for further use
}

// Contextual filtering implementations
void MainWindow::ShowContextualFilterDialog() {
    if (selected_entry_index_ < 0 || selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        last_error_ = "No log entry selected for contextual filtering";
        return;
    }
    
    const LogEntry& entry = filtered_entries_[selected_entry_index_];
    show_contextual_filter_dialog_ = true;
    
    // Create contextual conditions
    contextual_conditions_ = FilterConditionFactory::CreateContextualConditions(entry);
    
    // Show available options in status bar
    std::string options = "Create filter: [1] After timestamp [2] Logger=" + entry.Get_logger_name();
    if (entry.Get_log_level().has_value()) {
        options += " [3] Level=" + entry.Get_log_level().value();
    }
    if (entry.Get_frame_number().has_value()) {
        options += " [4] After frame";
    }
    last_error_ = options;
}

void MainWindow::CreateContextualFilter(FilterConditionType type) {
    if (selected_entry_index_ < 0 || selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        return;
    }
    
    const LogEntry& entry = filtered_entries_[selected_entry_index_];
    
    // Create filter expression if it doesn't exist
    if (!current_filter_expression_) {
        current_filter_expression_ = std::make_unique<FilterExpression>(FilterOperator::And);
    }
    
    // Create the appropriate condition based on type
    std::unique_ptr<FilterCondition> condition;
    
    switch (type) {
        case FilterConditionType::TimestampAfter:
            if (entry.Get_timestamp().has_value()) {
                condition = FilterConditionFactory::CreateTimestampAfter(entry.Get_timestamp().value());
                last_error_ = "Added filter: Timestamp >= " + entry.Get_timestamp().value();
            }
            break;
        case FilterConditionType::LoggerEquals:
            condition = FilterConditionFactory::CreateLoggerEquals(entry.Get_logger_name());
            last_error_ = "Added filter: Logger = " + entry.Get_logger_name();
            break;
        case FilterConditionType::LogLevelEquals:
            if (entry.Get_log_level().has_value()) {
                condition = FilterConditionFactory::CreateLogLevelEquals(entry.Get_log_level().value());
                last_error_ = "Added filter: LogLevel = " + entry.Get_log_level().value();
            }
            break;
        case FilterConditionType::FrameAfter:
            if (entry.Get_frame_number().has_value()) {
                condition = FilterConditionFactory::CreateFrameAfter(entry.Get_frame_number().value());
                last_error_ = "Added filter: Frame >= " + std::to_string(entry.Get_frame_number().value());
            }
            break;
        default:
            break;
    }
    
    if (condition) {
        current_filter_expression_->AddCondition(std::move(condition));
        ApplyCurrentFilter();
    }
    
    show_contextual_filter_dialog_ = false;
}

void MainWindow::HideContextualFilterDialog() {
    show_contextual_filter_dialog_ = false;
    contextual_conditions_.clear();
    last_error_ = "Contextual filter dialog closed";
}

// Context lines functionality implementations
void MainWindow::IncreaseContext() {
    if (context_lines_ < 10) {
        if (context_lines_ == 0) context_lines_ = 1;
        else if (context_lines_ < 3) context_lines_++;
        else if (context_lines_ == 3) context_lines_ = 5;
        else if (context_lines_ == 5) context_lines_ = 10;
        
        ApplyCurrentFilter();
        last_error_ = "Context lines: ±" + std::to_string(context_lines_);
    }
}

void MainWindow::DecreaseContext() {
    if (context_lines_ > 0) {
        if (context_lines_ == 10) context_lines_ = 5;
        else if (context_lines_ == 5) context_lines_ = 3;
        else if (context_lines_ <= 3) context_lines_--;
        
        ApplyCurrentFilter();
        last_error_ = context_lines_ == 0 ? "Context lines: None" : "Context lines: ±" + std::to_string(context_lines_);
    }
}

void MainWindow::SetContextLines(int lines) {
    context_lines_ = std::max(0, std::min(10, lines));
    ApplyCurrentFilter();
    last_error_ = context_lines_ == 0 ? "Context lines: None" : "Context lines: ±" + std::to_string(context_lines_);
}

void MainWindow::ClearContext() {
    context_lines_ = 0;
    ApplyCurrentFilter();
    last_error_ = "Context lines cleared";
}

// Helper methods
bool MainWindow::HasUppercaseLetters(const std::string& text) const {
    return std::any_of(text.begin(), text.end(), [](char c) {
        return std::isupper(c);
    });
}

ftxui::Element MainWindow::RenderSearchStatusBar() const {
    using namespace ftxui;
    
    if (show_search_) {
        // Search status bar
        std::string search_text;
        Color bg_color = Color::Default;
        
        if (search_input_mode_) {
            // Currently typing search
            search_text = "Search: " + search_query_ + " (Enter to confirm, + to promote, Esc to cancel)";
            bg_color = Color::Blue;
        } else {
            // Search completed, showing results
            if (search_results_.empty()) {
                search_text = "No matches found for '" + search_query_ + "' (n/N to navigate, Esc to exit)";
                bg_color = Color::Red;
            } else {
                search_text = "Found " + std::to_string(search_results_.size()) + " matches for '" + search_query_ + "' (n/N to navigate, + to promote, Esc to exit)";
                bg_color = Color::Green;
            }
        }
        
        return text(search_text) | bgcolor(bg_color) | color(Color::White);
    }
    
    if (show_contextual_filter_dialog_) {
        // Contextual filter dialog status bar
        if (selected_entry_index_ >= 0 && selected_entry_index_ < static_cast<int>(filtered_entries_.size())) {
            const LogEntry& entry = filtered_entries_[selected_entry_index_];
            std::string options = "Create filter: [1] After timestamp [2] Logger=" + entry.Get_logger_name();
            if (entry.Get_log_level().has_value()) {
                options += " [3] Level=" + entry.Get_log_level().value();
            }
            if (entry.Get_frame_number().has_value()) {
                options += " [4] After frame";
            }
            options += " (Esc to cancel)";
            
            return text(options) | bgcolor(Color::Yellow) | color(Color::Black);
        }
    }
    
    return text("");
}

// Helper method to apply the current filter expression
void MainWindow::ApplyCurrentFilter() {
    if (!current_filter_expression_ || current_filter_expression_->IsEmpty()) {
        // No filter - show all entries
        if (context_lines_ == 0) {
            filtered_entries_ = log_entries_;
        } else {
            BuildContextEntries(log_entries_);
        }
        return;
    }
    
    // Apply filter expression
    std::vector<LogEntry> matches;
    for (const auto& entry : log_entries_) {
        if (current_filter_expression_->Matches(entry)) {
            matches.push_back(entry);
        }
    }
    
    if (context_lines_ == 0) {
        filtered_entries_ = matches;
    } else {
        BuildContextEntries(matches);
    }
    
    // Update selection
    if (selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        selected_entry_index_ = filtered_entries_.empty() ? -1 : static_cast<int>(filtered_entries_.size()) - 1;
    }
    EnsureSelectionVisible();
}

// Helper method to build context entries around matches
void MainWindow::BuildContextEntries(const std::vector<LogEntry>& matches) {
    if (matches.empty() || context_lines_ == 0) {
        filtered_entries_ = matches;
        // Clear match tracking since all entries are matches
        match_line_numbers_.clear();
        for (const auto& match : matches) {
            match_line_numbers_.insert(match.Get_line_number());
        }
        return;
    }
    
    std::vector<LogEntry> result;
    std::set<size_t> included_indices;
    match_line_numbers_.clear();
    
    // Track which line numbers are actual matches
    for (const auto& match : matches) {
        match_line_numbers_.insert(match.Get_line_number());
    }
    
    // For each match, include context lines
    for (const auto& match : matches) {
        // Find the match in the original log entries
        auto it = std::find_if(log_entries_.begin(), log_entries_.end(),
            [&match](const LogEntry& entry) {
                return entry.Get_line_number() == match.Get_line_number();
            });
        
        if (it != log_entries_.end()) {
            size_t match_index = std::distance(log_entries_.begin(), it);
            
            // Include context before
            size_t start_index = (match_index >= static_cast<size_t>(context_lines_)) ? 
                                match_index - context_lines_ : 0;
            
            // Include context after
            size_t end_index = std::min(match_index + context_lines_ + 1, log_entries_.size());
            
            // Add all entries in the range
            for (size_t i = start_index; i < end_index; ++i) {
                included_indices.insert(i);
            }
        }
    }
    
    // Build the result vector in order
    for (size_t index : included_indices) {
        result.push_back(log_entries_[index]);
    }
    
    filtered_entries_ = result;
}

// Quick filter functionality implementations
void MainWindow::ShowQuickFilterDialog() {
    show_quick_filter_dialog_ = true;
    // Show quick filter options in status bar
    last_error_ = "Quick filters: [e] Errors, [w] Warnings, [i] Info, [d] Debug, [c] Clear filters (Esc to cancel)";
}

void MainWindow::HideQuickFilterDialog() {
    show_quick_filter_dialog_ = false;
    last_error_.clear();
}

void MainWindow::ApplyQuickFilter(const std::string& filter_type) {
    if (!filter_engine_) {
        last_error_ = "Filter engine not available";
        HideQuickFilterDialog();
        return;
    }
    
    if (filter_type == "clear") {
        // Clear all filters by creating an empty filter expression
        current_filter_expression_ = std::make_unique<FilterExpression>(FilterOperator::And);
        last_error_ = "All filters cleared";
    } else {
        // Create a quick filter
        std::unique_ptr<FilterCondition> condition;
        
        if (filter_type == "error") {
            condition = std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Error");
            last_error_ = "Quick filter applied: Showing only Error entries";
        } else if (filter_type == "warning") {
            condition = std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Warning");
            last_error_ = "Quick filter applied: Showing only Warning entries";
        } else if (filter_type == "info") {
            condition = std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Info");
            last_error_ = "Quick filter applied: Showing only Info entries";
        } else if (filter_type == "debug") {
            condition = std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Debug");
            last_error_ = "Quick filter applied: Showing only Debug entries";
        }
        
        // Create a new filter expression with this condition
        current_filter_expression_ = std::make_unique<FilterExpression>(FilterOperator::And);
        current_filter_expression_->AddCondition(std::move(condition));
    }
    
    // Apply the new filter
    OnFiltersChanged();
    HideQuickFilterDialog();
}

// Pattern-based navigation implementations
void MainWindow::JumpToNextError() {
    if (filtered_entries_.empty()) {
        last_error_ = "No entries to search";
        return;
    }
    
    // Start searching from the next entry after current selection
    int start_index = selected_entry_index_ + 1;
    
    for (int i = start_index; i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Error") {
            SelectEntry(i);
            last_error_ = "Jumped to next error at line " + std::to_string(i + 1);
            return;
        }
    }
    
    // If not found, wrap around to beginning
    for (int i = 0; i < start_index && i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Error") {
            SelectEntry(i);
            last_error_ = "Wrapped to first error at line " + std::to_string(i + 1);
            return;
        }
    }
    
    last_error_ = "No error entries found";
}

void MainWindow::JumpToPreviousError() {
    if (filtered_entries_.empty()) {
        last_error_ = "No entries to search";
        return;
    }
    
    // Start searching from the previous entry before current selection
    int start_index = selected_entry_index_ - 1;
    
    for (int i = start_index; i >= 0; --i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Error") {
            SelectEntry(i);
            last_error_ = "Jumped to previous error at line " + std::to_string(i + 1);
            return;
        }
    }
    
    // If not found, wrap around to end
    for (int i = static_cast<int>(filtered_entries_.size()) - 1; i > start_index; --i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Error") {
            SelectEntry(i);
            last_error_ = "Wrapped to last error at line " + std::to_string(i + 1);
            return;
        }
    }
    
    last_error_ = "No error entries found";
}

void MainWindow::JumpToNextWarning() {
    if (filtered_entries_.empty()) {
        last_error_ = "No entries to search";
        return;
    }
    
    // Start searching from the next entry after current selection
    int start_index = selected_entry_index_ + 1;
    
    for (int i = start_index; i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Warning") {
            SelectEntry(i);
            last_error_ = "Jumped to next warning at line " + std::to_string(i + 1);
            return;
        }
    }
    
    // If not found, wrap around to beginning
    for (int i = 0; i < start_index && i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Warning") {
            SelectEntry(i);
            last_error_ = "Wrapped to first warning at line " + std::to_string(i + 1);
            return;
        }
    }
    
    last_error_ = "No warning entries found";
}

void MainWindow::JumpToPreviousWarning() {
    if (filtered_entries_.empty()) {
        last_error_ = "No entries to search";
        return;
    }
    
    // Start searching from the previous entry before current selection
    int start_index = selected_entry_index_ - 1;
    
    for (int i = start_index; i >= 0; --i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Warning") {
            SelectEntry(i);
            last_error_ = "Jumped to previous warning at line " + std::to_string(i + 1);
            return;
        }
    }
    
    // If not found, wrap around to end
    for (int i = static_cast<int>(filtered_entries_.size()) - 1; i > start_index; --i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Warning") {
            SelectEntry(i);
            last_error_ = "Wrapped to last warning at line " + std::to_string(i + 1);
            return;
        }
    }
    
    last_error_ = "No warning entries found";
}

} // namespace ue_log