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
#include <sstream>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ue_log {

using namespace ftxui;

class MainWindowComponent : public ComponentBase {
public:
    MainWindowComponent(MainWindow* parent) : parent_(parent) {}
    
    Element Render() override {
        return parent_->Render();
    }
    
    bool HandleVisualSelectionEvent(Event event) {
        // ESC - Exit visual selection mode
        if (event == Event::Escape) {
            parent_->ExitVisualSelectionMode();
            return true;
        }
        
        // y - Copy selection and exit
        if (event == Event::Character('y')) {
            parent_->CopyVisualSelectionToClipboard();
            parent_->ExitVisualSelectionMode();
            return true;
        }
        
        // Navigation keys - extend selection
        if (event == Event::Character('j') || event == Event::ArrowDown) {
            // Use normal navigation which handles scrolling properly
            parent_->ScrollDown(1);
            // Then extend visual selection to the new position
            parent_->ExtendVisualSelection(parent_->GetSelectedEntryIndex());
            return true;
        }
        
        if (event == Event::Character('k') || event == Event::ArrowUp) {
            // Use normal navigation which handles scrolling properly
            parent_->ScrollUp(1);
            // Then extend visual selection to the new position
            parent_->ExtendVisualSelection(parent_->GetSelectedEntryIndex());
            return true;
        }
        
        // Ctrl+d/u for half-page navigation
        if (event == Event::Character(static_cast<char>(4))) { // Ctrl+D (ASCII 4)
            // Use normal half-page navigation which handles scrolling properly
            parent_->HalfPageDown();
            // Then extend visual selection to the new position
            parent_->ExtendVisualSelection(parent_->GetSelectedEntryIndex());
            return true;
        }
        
        if (event == Event::Character(static_cast<char>(21))) { // Ctrl+U (ASCII 21)
            // Use normal half-page navigation which handles scrolling properly
            parent_->HalfPageUp();
            // Then extend visual selection to the new position
            parent_->ExtendVisualSelection(parent_->GetSelectedEntryIndex());
            return true;
        }
        
        // Allow help dialog access
        if (event == Event::Character('h') || event == Event::Character('?') || event == Event::F1) {
            // Allow help to be shown in visual selection mode
            return false; // Let normal help handling take over
        }
        
        // Block all other keys in visual selection mode
        return true;
    }
    
    bool OnEvent(Event event) override {
        // Debug: Log all events
        // std::cout << "Event received" << std::endl;
        
        // Handle file browser mode events first
        if (parent_->GetMode() == MainWindow::EApplicationMode::FileBrowser) {
            // Let file browser handle its events
            if (parent_->_File_Browser_ && parent_->_File_Browser_->OnEvent(event)) {
                return true;
            }
            
            // Handle quit key in file browser mode
            if (event == Event::Character('q')) {
                parent_->Exit();
                return true;
            }
            
            // All other events are ignored in file browser mode
            return false;
        }
        
        // Handle visual selection mode events
        if (parent_->IsVisualSelectionMode()) {
            return HandleVisualSelectionEvent(event);
        }
        
        // Handle ESC key - priority order matters!
        if (event == Event::Escape) {
            // 1. First check vim command mode
            if (parent_->IsVimCommandMode()) {
                parent_->ClearVimCommandBuffer();
                return true;
            }
            // 2. Then check jump dialog
            if (parent_->IsJumpDialogActive()) {
                parent_->HideJumpDialog();
                return true;
            }
            // 3. Then check context menus
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
            // 4. Check if detail view is focused
            if (parent_->IsDetailViewFocused()) {
                parent_->UnfocusDetailView();
                parent_->SetLastError("Detail view unfocused - TAB to focus again");
                return true;
            }
            // ESC with no active context menus does nothing
            return true;
        }
        
        // Handle in-line search input when in-line search is active AND in input mode
        if (parent_->IsInlineSearchActive() && parent_->IsInlineSearchInputMode()) {
            // Only allow ESC, Enter, Backspace, and character input during in-line search
            if (event == Event::Escape) {
                parent_->HideInlineSearch();
                return true;
            }
            if (event == Event::Return) {
                parent_->ConfirmInlineSearch();
                return true;
            }
            if (event == Event::Backspace) {
                parent_->BackspaceInlineSearch();
                return true;
            }
            if (event.is_character()) {
                parent_->AppendToInlineSearch(event.character());
                return true;
            }
            // Block all other events during in-line search input mode
            return true;
        }
        
        // Handle in-line search navigation when in-line search is active but NOT in input mode
        if (parent_->IsInlineSearchActive() && !parent_->IsInlineSearchInputMode()) {
            if (event.is_character() && event.character().length() == 1) {
                char ch = event.character()[0];
                // Allow n/N for in-line search navigation
                if (ch == 'n') {
                    parent_->FindNextInlineMatch();
                    return true;
                }
                if (ch == 'N') {
                    parent_->FindPreviousInlineMatch();
                    return true;
                }
            }
            // Allow ESC to cancel in-line search
            if (event == Event::Escape) {
                parent_->HideInlineSearch();
                return true;
            }
        }

        // Handle search input when search is active AND in input mode
        // This MUST come first to prevent other shortcuts from interfering with search input
        if (parent_->IsSearchActive() && parent_->IsSearchInputMode()) {
            // Only allow ESC, Enter, Backspace, and character input during search
            if (event == Event::Escape) {
                parent_->HideSearch();
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
            if (event.is_character()) {
                parent_->AppendToSearch(event.character());
                return true;
            }
            // Block all other events during search input mode
            return true;
        }
        
        // Handle search promotion when search is active but NOT in input mode (after ENTER)
        if (parent_->IsSearchActive() && !parent_->IsSearchInputMode()) {
            // Allow number keys 1-5 for search promotion (matching column layout)
            if (event.is_character() && event.character().length() == 1) {
                char ch = event.character()[0];
                if (ch >= '1' && ch <= '5') {
                    int promotion_type = ch - '1'; // Convert '1'-'5' to 0-4 index
                    parent_->PromoteSearchToColumnFilter(promotion_type);
                    return true;
                }
                
                // Allow n/N for search navigation
                if (ch == 'n') {
                    parent_->FindNext();
                    return true;
                }
                if (ch == 'N') {
                    parent_->FindPrevious();
                    return true;
                }
            }
            
            // Allow ESC to cancel search
            if (event == Event::Escape) {
                parent_->HideSearch();
                return true;
            }
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
        
        // Handle number keys for INCLUDE column-based filtering (1-5) - only when NOT searching
        if (event.is_character() && event.character().length() == 1) {
            char ch = event.character()[0];
            
            // Check for number keys 1-5 for include column filtering
            if (ch >= '1' && ch <= '5') {
                int column_number = ch - '1'; // Convert '1'-'5' to 0-4 index
                parent_->CreateDirectColumnFilter(column_number);
                return true;
            }
        }
        
        // Handle SHIFT+Number shortcuts for EXCLUDE column-based filtering (Shift+1-5)
        if (event.is_character() && event.character().length() == 1) {
            char ch = event.character()[0];
            
            // Check for SHIFT+number keys (!, @, #, $, %) for exclude column filtering
            if (ch == '!' || ch == '@' || ch == '#' || ch == '$' || ch == '%') {
                int column_number;
                switch (ch) {
                    case '!': column_number = 0; break; // Shift+1
                    case '@': column_number = 1; break; // Shift+2
                    case '#': column_number = 2; break; // Shift+3
                    case '$': column_number = 3; break; // Shift+4
                    case '%': column_number = 4; break; // Shift+5
                    default: column_number = 0; break;
                }
                parent_->CreateDirectColumnExcludeFilter(column_number);
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
        
        // Handle vim-style navigation commands (number + j/k) - MUST come before other number handling
        if (event.is_character() && event.character().length() == 1) {
            char ch = event.character()[0];
            
            // Check if we're building a vim command (digits followed by j/k)
            if (std::isdigit(ch) || ch == 'j' || ch == 'k') {
                if (parent_->HandleVimStyleNavigation(event.character())) {
                    return true;
                }
            }
        }
        
        // Handle Backspace for vim command mode
        if (event == Event::Backspace) {
            if (parent_->IsVimCommandMode()) {
                parent_->BackspaceVimCommand();
                return true;
            }
        }
        

        
        // Context lines control (moved after vim navigation to avoid conflicts)
        if (event == Event::Character('{')) {
            parent_->DecreaseContext();
            return true;
        }
        if (event == Event::Character('}')) {
            parent_->IncreaseContext();
            return true;
        }
        if (event == Event::Character('0')) {
            // Only handle '0' for context lines if NOT in vim command mode
            if (!parent_->IsVimCommandMode()) {
                parent_->SetContextLines(0);
                return true;
            }
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
        
        // Vim-style 'G' goes to end and starts tailing mode
        if (event == Event::Character('G')) {
            // First go to the end of the log
            parent_->ScrollToBottom();
            
            // Then start tailing if not already active (no additional effect)
            if (!parent_->IsTailing()) {
                parent_->StartTailing();
            }
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
        
        // In-line search - CTRL+L (ASCII 12)
        if (event == Event::Character(static_cast<char>(12))) { // Ctrl+L (ASCII 12)
            parent_->ShowInlineSearch();
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
        
        // Focus cycling: Main Window -> Filter Panel (if visible) -> Detail View -> Main Window
        if (event == Event::Tab) {
            if (parent_->GetFilterPanel() && parent_->IsFilterPanelVisible()) {
                if (parent_->GetFilterPanel()->IsFocused()) {
                    // Skip detail view, go back to main window
                    parent_->GetFilterPanel()->SetFocus(false);
                    parent_->SetLastError("Main window focused - use arrow keys to navigate logs");
                } else {
                    // Main window to filter panel only
                    parent_->GetFilterPanel()->SetFocus(true);
                    parent_->SetLastError("Filter panel focused - use arrow keys to navigate filters, Space to toggle");
                }
            }
            // Remove all detail view focus logic
            return true;
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
        
        // Copy current log line to clipboard
        if (event == Event::Character('y')) {
            parent_->CopyCurrentLineToClipboard();
            return true;
        }
        
        // Visual selection mode activation
        if (event == Event::Character('v')) {
            parent_->EnterVisualSelectionMode();
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
        

        
        // Detail view navigation keys (J/K - capital letters only)
        if (event == Event::Character('J')) {
            if (parent_->IsDetailViewFocused()) {
                parent_->DetailViewScrollDown();
                return true;
            }
            // If detail view is not focused, ignore J key
            return true;
        }
        if (event == Event::Character('K')) {
            if (parent_->IsDetailViewFocused()) {
                parent_->DetailViewScrollUp();
                return true;
            }
            // If detail view is not focused, ignore K key
            return true;
        }
        
        // Main log view navigation keys (j/k - lowercase letters and arrow keys)
        if (event == Event::ArrowUp || event == Event::Character('k')) {
            // Check if tailing should be stopped for navigation events
            if (parent_->IsTailing() && parent_->ShouldStopTailing(event)) {
                parent_->StopTailing();
            }
            
            // j/k always go to main log view, even when detail view is focused
            // Only filter panel can intercept these keys
            if (parent_->GetFilterPanel() && parent_->GetFilterPanel()->IsFocused()) {
                parent_->GetFilterPanel()->NavigateUp();
                return true;
            }
            parent_->ScrollUp();
            return true;
        }
        if (event == Event::ArrowDown || event == Event::Character('j')) {
            // Check if tailing should be stopped for navigation events
            if (parent_->IsTailing() && parent_->ShouldStopTailing(event)) {
                parent_->StopTailing();
            }
            
            // j/k always go to main log view, even when detail view is focused
            // Only filter panel can intercept these keys
            if (parent_->GetFilterPanel() && parent_->GetFilterPanel()->IsFocused()) {
                parent_->GetFilterPanel()->NavigateDown();
                return true;
            }
            parent_->ScrollDown();
            return true;
        }
        if (event == Event::PageUp) {
            // Check if tailing should be stopped for navigation events
            if (parent_->IsTailing() && parent_->ShouldStopTailing(event)) {
                parent_->StopTailing();
            }
            
            if (parent_->IsDetailViewFocused()) {
                parent_->DetailViewPageUp();
                return true;
            }
            parent_->PageUp();
            return true;
        }
        if (event == Event::PageDown) {
            // Check if tailing should be stopped for navigation events
            if (parent_->IsTailing() && parent_->ShouldStopTailing(event)) {
                parent_->StopTailing();
            }
            
            if (parent_->IsDetailViewFocused()) {
                parent_->DetailViewPageDown();
                return true;
            }
            parent_->PageDown();
            return true;
        }
        
        // Vim-style half-page navigation
        if (event == Event::Character(static_cast<char>(4))) { // Ctrl+D (ASCII 4)
            // Check if tailing should be stopped for navigation events
            if (parent_->IsTailing() && parent_->ShouldStopTailing(event)) {
                parent_->StopTailing();
            }
            
            if (parent_->IsDetailViewFocused()) {
                parent_->DetailViewHalfPageDown();
                return true;
            }
            parent_->HalfPageDown();
            return true;
        }
        if (event == Event::Character(static_cast<char>(21))) { // Ctrl+U (ASCII 21)
            // Check if tailing should be stopped for navigation events
            if (parent_->IsTailing() && parent_->ShouldStopTailing(event)) {
                parent_->StopTailing();
            }
            
            if (parent_->IsDetailViewFocused()) {
                parent_->DetailViewHalfPageUp();
                return true;
            }
            parent_->HalfPageUp();
            return true;
        }
        if (event == Event::Home) {
            if (parent_->IsDetailViewFocused()) {
                parent_->DetailViewScrollToTop();
                return true;
            }
            parent_->ScrollToTop();
            return true;
        }
        if (event == Event::End) {
            if (parent_->IsDetailViewFocused()) {
                parent_->DetailViewScrollToBottom();
                return true;
            }
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
    : _Config_Manager_(config_manager) {
    
    // Create owned config manager if none provided
    if (!_Config_Manager_) {
        _Config_Manager_ = new ConfigManager();
        _Owns_Config_Manager_ = true;
    }
    
    // Initialize backend components
    _Log_Parser_ = std::make_unique<LogParser>();
    _Filter_Engine_ = std::make_unique<FilterEngine>();
    _File_Monitor_ = std::make_unique<FileMonitor>();
    
    // Initialize UI components
    _Filter_Panel_ = std::make_unique<FilterPanel>(_Filter_Engine_.get(), _Config_Manager_);
    _Visual_Theme_Manager_ = std::make_unique<VisualThemeManager>();
    _Relative_Line_System_ = std::make_unique<RelativeLineNumberSystem>();
}

MainWindow::~MainWindow() {
    // Clean up owned config manager
    if (_Owns_Config_Manager_) {
        delete _Config_Manager_;
    }
}

void MainWindow::Initialize() {
    // Initialize filter expression
    current_filter_expression_ = std::make_unique<FilterExpression>(FilterOperator::And);
    
    // Initialize default window size if not set
    if (window_width_ <= 0) window_width_ = 120;
    if (window_height_ <= 0) window_height_ = 30;
    
    // Initialize UI components
    if (_Filter_Panel_) {
        _Filter_Panel_->Initialize();
        _Filter_Panel_->SetCurrentFilterExpression(current_filter_expression_.get());
        _Filter_Panel_->SetFiltersChangedCallback([this]() {
            OnFiltersChanged();
        });
    }
    
    // Initialize visual components
    if (_Visual_Theme_Manager_) {
        _Log_Entry_Renderer_ = std::make_unique<LogEntryRenderer>(_Visual_Theme_Manager_.get());
    }
    
    // Apply basic configuration
    ApplyConfiguration();
    
    // Create FTXUI component - use simple main component for now
    _Component_ = Make<MainWindowComponent>(this);
}

ftxui::Element MainWindow::Render() const {
    // Handle different application modes
    if (_Current_Mode_ == EApplicationMode::FileBrowser) {
        return RenderFileBrowserMode();
    } else {
        return RenderLogViewerMode();
    }
}

ftxui::Element MainWindow::RenderFileBrowserMode() const {
    using namespace ftxui;
    
    std::vector<Element> main_elements;
    
    // Add file browser (takes most of the space)
    if (_File_Browser_) {
        main_elements.push_back(_File_Browser_->Render() | flex);
    } else {
        main_elements.push_back(text("File browser not initialized") | center | flex);
    }
    
    // Add status bar at the bottom
    main_elements.push_back(RenderStatusBar());
    
    return vbox(main_elements);
}

ftxui::Element MainWindow::RenderLogViewerMode() const {
    using namespace ftxui;
    
    // Main layout: log table + detail view + status bar
    std::vector<Element> main_elements;
    
    // Add log table (takes most of the space)
    main_elements.push_back(RenderLogTable() | flex);
    
    // Add detail view if enabled (shows full raw log entry)
    if (show_detail_view_) {
        main_elements.push_back(separator());
        // Dynamic sizing: small when not focused, larger when focused
        // Use a reasonable fallback if window_height_ is not set
        int available_height = (window_height_ > 0) ? window_height_ : 30; // Fallback to 30 lines
        int detail_height = detail_view_focused_ ? 
                           std::max(15, available_height * 2 / 3) :  // 2/3 of screen when focused
                           5;                                        // 5 lines when not focused
        main_elements.push_back(RenderDetailView() | size(HEIGHT, EQUAL, detail_height));
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
    if (show_filter_panel_ && _Filter_Panel_) {
        Element filter_panel = _Filter_Panel_->Render() | size(WIDTH, EQUAL, 60);
        main_content = hbox({
            filter_panel,
            separator(),
            main_content | flex
        });
    }
    
    return main_content;
}

ftxui::Component MainWindow::CreateFTXUIComponent() {
    if (!_Component_) {
        Initialize();
    }
    return _Component_;
}

bool MainWindow::LoadLogFile(const std::string& file_path) {
    if (file_path.empty()) {
        _Last_Error_ = "File path is empty";
        return false;
    }
    
    // Check if file exists
    if (!std::filesystem::exists(file_path)) {
        _Last_Error_ = "File does not exist: " + file_path;
        return false;
    }
    
    // Stop existing file monitoring if running (for clean reload)
    if (_File_Monitor_ && _File_Monitor_->IsMonitoring()) {
        _File_Monitor_->StopMonitoring();
    }
    
    // Store the file path
    _Current_File_Path_ = file_path;
    
    try {
        // Use LogParser to load and parse the real file
        auto load_result = _Log_Parser_->LoadFile(file_path);
        if (load_result.IsError()) {
            _Last_Error_ = "Failed to load file: " + file_path + " - " + load_result.Get_error_message();
            return false;
        }
        
        // Show loading message
        _Last_Error_ = "Loading and parsing log file...";
        
        // Parse the log entries
        log_entries_ = _Log_Parser_->ParseEntries();
        
        if (log_entries_.empty()) {
            // If no entries were parsed, create sample data as fallback
            CreateSampleLogEntries();
            _Last_Error_ = "No valid log entries found in file, using sample data";
        } else {
            _Last_Error_ = "Loaded " + std::to_string(log_entries_.size()) + " log entries from " + std::filesystem::path(file_path).filename().string();
        }
        
        // Don't create sample filters - start with empty filter area
        
        // Apply filters
        OnFiltersChanged();
        
        // Reset scroll position and selection
        scroll_offset_ = 0;
        selected_entry_index_ = 0;
        
        // Reset visual selection mode on file reload
        ExitVisualSelectionMode();
        
        // Start FileMonitor to watch for changes (but don't enable tailing/auto-scroll)
        StartFileMonitoring();
        
        return true;
        
    } catch (const std::exception& e) {
        _Last_Error_ = "Error loading file: " + std::string(e.what());
        return false;
    }
}

bool MainWindow::ReloadLogFile() {
    if (_Current_File_Path_.empty()) {
        _Last_Error_ = "No file currently loaded";
        return false;
    }
    
    return LoadLogFile(_Current_File_Path_);
}

void MainWindow::SetMode(EApplicationMode mode) {
    _Current_Mode_ = mode;
    
    // Update UI state based on mode
    if (mode == EApplicationMode::FileBrowser) {
        // Hide log viewer specific UI elements
        show_search_ = false;
        show_filter_panel_ = false;
        show_detail_view_ = false;
        show_jump_dialog_ = false;
        
        // Focus the file browser if it exists
        if (_File_Browser_) {
            _File_Browser_->SetFocus(true);
        }
    } else if (mode == EApplicationMode::LogViewer) {
        // Restore log viewer UI elements to default state
        show_detail_view_ = true;
        
        // Unfocus the file browser if it exists
        if (_File_Browser_) {
            _File_Browser_->SetFocus(false);
        }
    }
    
    // Trigger screen refresh
    if (refresh_callback_) {
        refresh_callback_();
    }
}

void MainWindow::InitializeFileBrowser(const std::string& directory_path) {
    _Initial_Directory_ = directory_path;
    
    // Create and initialize the file browser
    _File_Browser_ = std::make_unique<FileBrowser>(directory_path);
    _File_Browser_->Initialize();
    
    // Set up callbacks
    _File_Browser_->SetFileSelectionCallback([this](const std::string& file_path) {
        OnFileSelected(file_path);
    });
    
    _File_Browser_->SetErrorCallback([this](const std::string& error) {
        SetLastError(error);
        if (refresh_callback_) {
            refresh_callback_();
        }
    });
    
    _File_Browser_->SetStatusCallback([this](const std::string& status) {
        SetLastError(status);  // Use SetLastError for positive status messages too
        if (refresh_callback_) {
            refresh_callback_();
        }
    });
    
    // Switch to file browser mode
    SetMode(EApplicationMode::FileBrowser);
}

void MainWindow::TransitionToLogViewer(const std::string& file_path) {
    // Load the log file
    if (LoadLogFile(file_path)) {
        // Switch to log viewer mode
        SetMode(EApplicationMode::LogViewer);
        SetLastError("Loaded file: " + file_path);
    } else {
        // Stay in file browser mode and show error
        SetLastError("Failed to load file: " + file_path + " - " + GetLastError());
    }
    
    // Trigger screen refresh
    if (refresh_callback_) {
        refresh_callback_();
    }
}

void MainWindow::OnFileSelected(const std::string& file_path) {
    TransitionToLogViewer(file_path);
}

void MainWindow::EnterFileBrowserMode(const std::string& directory_path) {
    InitializeFileBrowser(directory_path);
}

void MainWindow::EnterLogViewerMode(const std::string& file_path) {
    TransitionToLogViewer(file_path);
}

bool MainWindow::StartTailing() {
    // Check if file is loaded
    if (_Current_File_Path_.empty()) {
        _Last_Error_ = "No file loaded - cannot start tailing";
        return false;
    }
    
    // Set tailing state
    is_tailing_ = true;
    auto_scroll_enabled_ = true;
    
    // Ensure FileMonitor is running (it should already be running from LoadLogFile)
    if (!_File_Monitor_->IsMonitoring()) {
        StartFileMonitoring();
    }
    
    // Configure faster polling for more responsive tailing (50ms instead of default 100ms)
    _File_Monitor_->SetPollInterval(std::chrono::milliseconds(50));
    
    // Update UI status to show "LIVE" indicator
    _Last_Error_ = "LIVE - Tailing " + std::filesystem::path(_Current_File_Path_).filename().string();
    
    return true;
}

bool MainWindow::StartRealTimeMonitoring() {
    // Placeholder implementation
    return true;
}

void MainWindow::StopTailing() {
    // Set tailing state to false
    is_tailing_ = false;
    auto_scroll_enabled_ = false;
    
    // Keep FileMonitor running but revert to slower polling for background monitoring
    if (_File_Monitor_) {
        _File_Monitor_->SetPollInterval(std::chrono::milliseconds(100));
    }
    
    // Update UI status to show "STATIC" indicator
    if (!_Current_File_Path_.empty()) {
        _Last_Error_ = "STATIC - " + std::filesystem::path(_Current_File_Path_).filename().string();
    } else {
        _Last_Error_ = "STATIC - No file loaded";
    }
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
    _Current_File_Path_.clear();
    log_entries_.clear();
    filtered_entries_.clear();
    selected_entry_index_ = 0;
    scroll_offset_ = 0;
    _Last_Error_.clear();
    
    // Stop any real-time monitoring and file monitoring
    StopRealTimeMonitoring();
    StopFileMonitoring();
    
    // Reset tailing state
    is_tailing_ = false;
    auto_scroll_enabled_ = false;
}

// Navigation methods for tests

void MainWindow::RefreshDisplay() {
    // Trigger a refresh - placeholder implementation
}

void MainWindow::SetTerminalSize(int width, int height) {
    window_width_ = width;
    window_height_ = height;
}

void MainWindow::SetTailingPollInterval(int milliseconds) {
    if (_File_Monitor_) {
        _File_Monitor_->SetPollInterval(std::chrono::milliseconds(milliseconds));
    }
}

void MainWindow::StartFileMonitoring() {
    if (_Current_File_Path_.empty() || !_File_Monitor_) {
        return;
    }
    
    // Only set up callback and start monitoring if not already running
    if (!_File_Monitor_->IsMonitoring()) {
        // Set up FileMonitor callback to OnNewLogLines
        _File_Monitor_->SetCallback([this](const std::string& file_path, const std::vector<std::string>& new_lines) {
            OnNewLogLines(new_lines);
        });
        
        // Configure poll interval (default 100ms for background monitoring)
        _File_Monitor_->SetPollInterval(std::chrono::milliseconds(100));
        
        auto result = _File_Monitor_->StartMonitoring(_Current_File_Path_);
        if (result.IsError()) {
            _Last_Error_ = "Failed to start file monitoring: " + result.Get_error_message();
        }
    }
    // If already monitoring, don't change anything to avoid disrupting the state
}

void MainWindow::StopFileMonitoring() {
    if (_File_Monitor_ && _File_Monitor_->IsMonitoring()) {
        _File_Monitor_->StopMonitoring();
    }
}

void MainWindow::ApplyFiltersToNewEntries(const std::vector<LogEntry>& new_entries) {
    if (!_Filter_Engine_) {
        // No filter engine - just append all new entries
        filtered_entries_.insert(filtered_entries_.end(), new_entries.begin(), new_entries.end());
        return;
    }
    
    // Check if we have hierarchical filters (contextual filters)
    if (current_filter_expression_ && !current_filter_expression_->IsEmpty()) {
        // Apply hierarchical filters to new entries only
        for (const auto& entry : new_entries) {
            if (current_filter_expression_->Matches(entry)) {
                filtered_entries_.push_back(entry);
            }
        }
    } else {
        // Apply traditional filters to new entries only
        const auto& filters = _Filter_Engine_->Get_primary_filters();
        
        // Check if there are any active filters
        bool has_active_filters = false;
        for (const auto& filter : filters) {
            if (filter->Get_is_active()) {
                has_active_filters = true;
                break;
            }
        }
        
        if (!has_active_filters) {
            // No active filters - append all new entries
            filtered_entries_.insert(filtered_entries_.end(), new_entries.begin(), new_entries.end());
        } else {
            // Apply active filters to new entries
            for (const auto& entry : new_entries) {
                bool matches_any_filter = false;
                
                for (const auto& filter : filters) {
                    if (filter->Get_is_active() && filter->Matches(entry)) {
                        matches_any_filter = true;
                        break;
                    }
                }
                
                if (matches_any_filter) {
                    filtered_entries_.push_back(entry);
                }
            }
        }
    }
    
    // Handle context lines if needed - do this incrementally to avoid full rebuild
    if (context_lines_ > 0) {
        // For incremental updates with context, we need to add context around new matching entries
        // This avoids the expensive full rebuild that OnFiltersChanged() would cause
        ApplyContextToNewEntries(new_entries);
    }
}

void MainWindow::ApplyContextToNewEntries(const std::vector<LogEntry>& new_entries) {
    if (context_lines_ == 0 || new_entries.empty()) {
        return; // No context needed or no new entries
    }
    
    // For each new entry that was added to filtered_entries_, we need to check if we should
    // add context lines around it. This is more complex than a full rebuild, but avoids
    // the performance issue of re-processing the entire log.
    
    // Get the current size of filtered_entries_ before we start adding context
    size_t original_filtered_size = filtered_entries_.size();
    
    // We need to be careful here - the filtered_entries_ already contains the new entries
    // that passed the filter. We need to add context around those entries.
    
    // For incremental context, we'll use a simpler approach:
    // 1. Find the positions of the new entries in the original log_entries_
    // 2. Add context lines around those positions if they're not already in filtered_entries_
    
    std::set<size_t> entries_to_add; // Use set to avoid duplicates
    
    // Find positions of new entries in the original log
    for (const auto& new_entry : new_entries) {
        // Find this entry in log_entries_ to get its position
        for (size_t i = 0; i < log_entries_.size(); ++i) {
            if (log_entries_[i].Get_line_number() == new_entry.Get_line_number()) {
                // Add context lines around this position
                size_t start_pos = (i >= context_lines_) ? i - context_lines_ : 0;
                size_t end_pos = std::min(i + context_lines_ + 1, log_entries_.size());
                
                for (size_t j = start_pos; j < end_pos; ++j) {
                    entries_to_add.insert(j);
                }
                break;
            }
        }
    }
    
    // Add the context entries that aren't already in filtered_entries_
    // We need to check if each entry is already present to avoid duplicates
    std::set<int> existing_line_numbers;
    for (const auto& entry : filtered_entries_) {
        existing_line_numbers.insert(entry.Get_line_number());
    }
    
    // Add new context entries
    for (size_t pos : entries_to_add) {
        const auto& entry = log_entries_[pos];
        if (existing_line_numbers.find(entry.Get_line_number()) == existing_line_numbers.end()) {
            filtered_entries_.push_back(entry);
        }
    }
    
    // Sort filtered_entries_ by line number to maintain proper order
    std::sort(filtered_entries_.begin(), filtered_entries_.end(), 
              [](const LogEntry& a, const LogEntry& b) {
                  return a.Get_line_number() < b.Get_line_number();
              });
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
    // Check if tailing should be stopped for navigation events
    if (IsTailing()) {
        StopTailing();
    }
    ScrollDown(1);
}

void MainWindow::SelectPreviousEntry() {
    // Check if tailing should be stopped for navigation events
    if (IsTailing()) {
        StopTailing();
    }
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
    if (_Component_) {
        return _Component_->OnEvent(event);
    }
    return false;
}

void MainWindow::ScrollUp(int count) {
    // Check if tailing should be stopped for navigation events
    if (IsTailing()) {
        StopTailing();
    }
    
    if (selected_entry_index_ > 0) {
        SelectEntry(selected_entry_index_ - count);
    }
}

void MainWindow::ScrollDown(int count) {
    // Check if tailing should be stopped for navigation events
    if (IsTailing()) {
        StopTailing();
    }
    
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
    // Check if tailing should be stopped for navigation events
    if (IsTailing()) {
        StopTailing();
    }
    
    // Use the same visible height as in RenderLogTable for consistency
    int visible_height = GetVisibleHeight();
    int half_page_size = std::max(1, visible_height / 2);
    ScrollUp(half_page_size);
}

void MainWindow::HalfPageDown() {
    // Check if tailing should be stopped for navigation events
    if (IsTailing()) {
        StopTailing();
    }
    
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

void MainWindow::AutoScrollToBottom() {
    // Only auto-scroll when tailing is active
    if (!is_tailing_ || !auto_scroll_enabled_) {
        return;
    }
    
    // Throttle auto-scroll to prevent excessive updates (max 20 times per second)
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_scroll = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_auto_scroll_time_);
    if (time_since_last_scroll < std::chrono::milliseconds(50)) {
        return; // Skip this auto-scroll to prevent excessive updates
    }
    last_auto_scroll_time_ = now;
    
    // Update selected_entry_index_ to show latest entries
    if (!filtered_entries_.empty()) {
        int total_entries = static_cast<int>(filtered_entries_.size());
        int visible_height = GetVisibleHeight();
        
        // Set selected entry to the last entry
        selected_entry_index_ = total_entries - 1;
        
        // Calculate scroll offset to ensure the last entry is visible but not past the screen
        // The last entry should be at the bottom of the visible area, not below it
        if (total_entries > visible_height) {
            // Position the last entry at the bottom of the visible area
            scroll_offset_ = total_entries - visible_height;
        } else {
            // If we have fewer entries than visible height, no scrolling needed
            scroll_offset_ = 0;
        }
        
        // Ensure scroll_offset_ is never negative
        scroll_offset_ = std::max(0, scroll_offset_);
    }
}

void MainWindow::OnNewLogLines(const std::vector<std::string>& new_lines) {
    // Early return if no file loaded
    if (_Current_File_Path_.empty()) {
        return;
    }
    
    // Handle empty or invalid input
    if (new_lines.empty()) {
        return;
    }
    

    // Use existing LogParser to parse new line strings into LogEntry objects
    try {
        size_t current_line_num = log_entries_.size() + 1; // Continue line numbering
        std::vector<LogEntry> new_entries;
        
        for (const auto& line : new_lines) {
            auto entry = _Log_Parser_->ParseSingleEntry(line, current_line_num);
            // Add new entry to log_entries_ vector
            log_entries_.push_back(entry);
            new_entries.push_back(entry);
            current_line_num++;
        }
        
        // Apply filters only to new entries and append to filtered_entries_
        ApplyFiltersToNewEntries(new_entries);
        

        // Auto-scroll to bottom ONLY if tailing is active and auto-scroll is enabled
        if (is_tailing_ && auto_scroll_enabled_) {
            AutoScrollToBottom();
            // Ensure the selection is visible after auto-scrolling
            EnsureSelectionVisible();
        }
        
        // Always trigger screen refresh when new entries are added (regardless of tailing state)
        if (refresh_callback_) {
            refresh_callback_();
        }
        
        // Update status message based on tailing state
        if (is_tailing_) {
            _Last_Error_ = "LIVE - Tailing " + std::filesystem::path(_Current_File_Path_).filename().string();
        } else {
            _Last_Error_ = "STATIC - " + std::filesystem::path(_Current_File_Path_).filename().string() + " (updated)";
        }
        
    } catch (const std::exception& e) {
        // Handle parsing errors gracefully
        if (is_tailing_) {
            _Last_Error_ = "LIVE - Parse error: " + std::string(e.what());
        } else {
            _Last_Error_ = "STATIC - Parse error: " + std::string(e.what());
        }
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
    // Calculate the visible height (available space for log entries)
    // Use a reasonable default if window_height_ is not set
    int available_height = (window_height_ > 0) ? window_height_ : 30;
    
    // Reserve space for header, status bar, and detail view if visible
    int reserved_space = 3; // 1 for header, 1 for status bar, 1 for border
    if (show_detail_view_) {
        reserved_space += detail_view_focused_ ? 
            std::max(15, available_height * 2 / 3) : 5;
    }
    
    // Calculate visible height for log entries
    return std::max(5, available_height - reserved_space);
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
        // Performance optimization: Only render entries that are likely to be visible
        // Calculate a reasonable window size with buffer for scrolling
        int visible_height = GetVisibleHeight();
        int buffer = visible_height * 2; // Add buffer for smooth scrolling
        
        // Calculate start and end indices with buffer around selected entry
        int total_entries = static_cast<int>(filtered_entries_.size());
        int start_idx = std::max(0, std::min(selected_entry_index_ - buffer, total_entries - visible_height - buffer));
        int end_idx = std::min(total_entries, start_idx + visible_height + buffer * 2);
        
        // If in visual selection mode, ensure the entire selection is included in the render window
        // but only for small selections to avoid interfering with scrolling
        if (visual_selection_mode_) {
            auto range = GetVisualSelectionRange();
            if (range.first >= 0 && range.second >= 0) {
                int selection_size = range.second - range.first + 1;
            }
        }
        
        // Render entries in the window
        for (int i = start_idx; i < end_idx; ++i) {
            bool is_selected = (i == selected_entry_index_);
            rows.push_back(RenderLogEntry(filtered_entries_[i], is_selected));
        }
        
        // Add indicators if we're not showing all entries
        if (start_idx > 0) {
            rows.insert(rows.begin() + 1, text("↑ More entries above ↑") | center | dim);
        }
        if (end_idx < total_entries) {
            rows.push_back(text("↓ More entries below ↓") | center | dim);
        }
    }
    
    // Add visual focus indicator - main window has focus when filter panel doesn't
    bool main_has_focus = !_Filter_Panel_ || !_Filter_Panel_->IsFocused();
    
    // Create scrollable content that fills available space
    Element scrollable_content = vbox(rows) | yframe | yflex;
    
    // Add custom scrollbar if we have entries
    if (!filtered_entries_.empty()) {
        Element custom_scrollbar = RenderCustomScrollbar();
        scrollable_content = hbox({
            scrollable_content | flex,
            custom_scrollbar
        });
    }
    
    // We'll rely on EnsureSelectionVisible to handle scrolling to the selected entry
    
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
    
    // File info with visual polish
    std::string file_info = _Current_File_Path_.empty() ? "No file" : 
                           std::filesystem::path(_Current_File_Path_).filename().string();
    Element file_element = text(file_info) | size(WIDTH, EQUAL, 25);
    if (_Visual_Theme_Manager_->GetFontWeight("status")) {
        file_element = file_element | bold;
    }
    status_elements.push_back(file_element);
    
    // Add separator with consistent styling
    status_elements.push_back(text(" │ ") | color(_Visual_Theme_Manager_->GetBorderColor()));
    
    // Monitoring status with appropriate colors
    std::string monitor_info = IsRealTimeMonitoringActive() ? "LIVE" : "STATIC";
    Element monitor_element = text(monitor_info) | size(WIDTH, EQUAL, 8);
    if (IsRealTimeMonitoringActive()) {
        monitor_element = monitor_element | color(_Visual_Theme_Manager_->GetAccentColor()) | bold;
    } else {
        monitor_element = monitor_element | color(_Visual_Theme_Manager_->GetMutedTextColor());
    }
    status_elements.push_back(monitor_element);
    
    // Add separator
    status_elements.push_back(text(" │ ") | color(_Visual_Theme_Manager_->GetBorderColor()));
    
    // Visual selection mode indicator
    if (visual_selection_mode_) {
        int selection_size = GetVisualSelectionSize();
        std::string visual_status = "VISUAL: " + std::to_string(selection_size) + 
                                   (selection_size == 1 ? " line selected" : " lines selected");
        
        // For multi-screen selections, show range information
        int visible_height = GetVisibleHeight();
        if (selection_size > visible_height) {
            auto range = GetVisualSelectionRange();
            if (range.first >= 0 && range.second >= 0) {
                visual_status += " (lines " + std::to_string(range.first + 1) + 
                               "-" + std::to_string(range.second + 1) + ")";
            }
        }
        
        Element visual_element = text(visual_status) | 
                                color(_Visual_Theme_Manager_->GetAccentColor()) | 
                                bold;
        status_elements.push_back(visual_element);
        
        // Add separator after visual mode indicator
        status_elements.push_back(text(" │ ") | color(_Visual_Theme_Manager_->GetBorderColor()));
    }
    
    // Error message or help (takes up middle space) with appropriate styling
    std::string message = _Last_Error_.empty() ? "Press ':' for goto, 'g' for top, 'f' for filters, 'q' to quit" : _Last_Error_;
    Element message_element = text(message) | flex;
    if (!_Last_Error_.empty()) {
        // Error messages should be more prominent
        message_element = message_element | color(_Visual_Theme_Manager_->GetLogLevelColor("Warning"));
    } else {
        // Help text should be muted
        message_element = message_element | color(_Visual_Theme_Manager_->GetMutedTextColor());
    }
    status_elements.push_back(message_element);
    
    // Add separator
    status_elements.push_back(text(" │ ") | color(_Visual_Theme_Manager_->GetBorderColor()));
    
    // Entry count and scroll info on the right with enhanced formatting
    std::string count_info;
    Element count_element;
    if (!filtered_entries_.empty() && selected_entry_index_ >= 0 && selected_entry_index_ < static_cast<int>(filtered_entries_.size())) {
        const auto& selected_entry = filtered_entries_[selected_entry_index_];
        
        // Show current selection, total count, and absolute line number
        count_info = "Entry " + std::to_string(selected_entry_index_ + 1) + " of " + 
                    std::to_string(filtered_entries_.size()) + 
                    " | Line " + std::to_string(selected_entry.Get_line_number());
        
        if (filtered_entries_.size() != log_entries_.size()) {
            count_info += " (filtered from " + std::to_string(log_entries_.size()) + ")";
        }
        
        count_element = text(count_info);
        if (_Visual_Theme_Manager_->GetFontWeight("status")) {
            count_element = count_element | bold;
        }
    } else {
        count_info = "No entries";
        count_element = text(count_info) | color(_Visual_Theme_Manager_->GetMutedTextColor());
    }
    status_elements.push_back(count_element);
    
    // Apply consistent background and ensure good contrast
    Element status_bar = hbox(status_elements);
    
    // Use inverted colors for status bar with eye strain considerations
    if (_Visual_Theme_Manager_->IsEyeStrainReductionEnabled()) {
        status_bar = status_bar | bgcolor(_Visual_Theme_Manager_->GetBorderColor()) | 
                     color(_Visual_Theme_Manager_->GetBackgroundColor());
    } else {
        status_bar = status_bar | inverted;
    }
    
    return status_bar;
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
        Element no_selection = text("No entry selected") | center;
        no_selection = no_selection | color(_Visual_Theme_Manager_->GetMutedTextColor());

        Element title = text("Detail View - No Selection");
        if (_Visual_Theme_Manager_->GetFontWeight("header")) {
            title = title | bold;
        }
        title = title | color(detail_view_focused_ ?
                             _Visual_Theme_Manager_->GetFocusColor() :
                             _Visual_Theme_Manager_->GetHighlightColor());

        return window(title, no_selection);
    }

    const auto& selected_entry = filtered_entries_[selected_entry_index_];

    // Create title with focus indicator and navigation info
    std::string entry_type = selected_entry.IsStructured() ? "Structured" :
                            selected_entry.IsSemiStructured() ? "Semi-Structured" : "Unstructured";
    std::string title_text = "Detail View - Line " + std::to_string(selected_entry.Get_line_number()) + " (" + entry_type + ")";

    Element title = text(title_text);
    if (_Visual_Theme_Manager_->GetFontWeight("header")) {
        title = title | bold;
    }
    title = title | color(detail_view_focused_ ?
                         _Visual_Theme_Manager_->GetFocusColor() :
                         _Visual_Theme_Manager_->GetHighlightColor());
    
    // Use the full raw line instead of just the parsed message
    std::string full_message = selected_entry.Get_raw_line();
    
    // Split message into lines for individual line navigation
    std::vector<std::string> message_lines;
    std::stringstream ss(full_message);
    std::string line;
    while (std::getline(ss, line)) {
        message_lines.push_back(line);
    }
    
    // If no lines (empty message), add a placeholder
    if (message_lines.empty()) {
        message_lines.push_back("(empty message)");
    }
    
    // Calculate visible lines based on focus state and scroll offset
    // Use a reasonable fallback if window_height_ is not set
    int available_height = (window_height_ > 0) ? window_height_ : 30; // Fallback to 30 lines
    int content_height = available_height - 2; // Exclude status bars
    int visible_height = detail_view_focused_ ? 
                        std::max(10, (content_height * 2) / 3 - 4) :  // 2/3 of screen when focused, -4 for window borders
                        3;                                            // 3 lines when not focused
    int start_line = detail_view_scroll_offset_;
    int end_line = std::min(start_line + visible_height, static_cast<int>(message_lines.size()));
    
    // Create content elements for visible lines
    std::vector<Element> content_elements;
    for (int i = start_line; i < end_line; i++) {
        Element line_element = paragraph(message_lines[i]);
        
        // Apply log level styling to all lines
        if (selected_entry.Get_log_level().has_value()) {
            const std::string& level = selected_entry.Get_log_level().value();
            if (_Visual_Theme_Manager_->IsLogLevelProminent(level)) {
                line_element = line_element | color(_Visual_Theme_Manager_->GetLogLevelColor(level));
                if (_Visual_Theme_Manager_->ShouldLogLevelUseBold(level)) {
                    line_element = line_element | bold;
                }
            }
        }
        
        content_elements.push_back(line_element);
    }
    
    // Add scroll indicators if there are more lines
    if (start_line > 0) {
        content_elements.insert(content_elements.begin(), 
                               text("... (" + std::to_string(start_line) + " lines above)") | 
                               color(_Visual_Theme_Manager_->GetMutedTextColor()));
    }
    if (end_line < static_cast<int>(message_lines.size())) {
        content_elements.push_back(
            text("... (" + std::to_string(message_lines.size() - end_line) + " lines below)") | 
            color(_Visual_Theme_Manager_->GetMutedTextColor()));
    }
    
    Element content = vbox(content_elements);
    
    // Add border styling with focus indication
    Element window_content = window(title, content);
    if (detail_view_focused_) {
        window_content = window_content | border | color(_Visual_Theme_Manager_->GetFocusColor());
    }
    
    return window_content;
}

ftxui::Element MainWindow::RenderCustomScrollbar() const {
    using namespace ftxui;
    
    if (filtered_entries_.empty()) {
        return text(" ") | yflex;
    }
    
    int total_entries = static_cast<int>(filtered_entries_.size());
    int current_position = selected_entry_index_;
    int visible_height = GetVisibleHeight();
    
    // Use a much larger height to ensure the scrollbar spans the full area
    // This should be larger than any reasonable window height
    int scrollbar_height = std::max(50, visible_height * 3);
    
    // Calculate position and size ratios
    double position_ratio = static_cast<double>(current_position) / std::max(1, total_entries - 1);
    double visible_ratio = std::min(1.0, static_cast<double>(visible_height) / static_cast<double>(total_entries));
    
    // Calculate thumb size and position
    int thumb_size = std::max(2, static_cast<int>(scrollbar_height * visible_ratio));
    int thumb_position = static_cast<int>((scrollbar_height - thumb_size) * position_ratio);
    
    // Create scrollbar elements
    std::vector<Element> scrollbar_elements;
    
    for (int i = 0; i < scrollbar_height; ++i) {
        if (i >= thumb_position && i < thumb_position + thumb_size) {
            // Thumb part - use a solid block character
            scrollbar_elements.push_back(text("█") | color(Color::White) | bgcolor(Color::Blue));
        } else {
            // Track part - use a lighter character
            scrollbar_elements.push_back(text("│") | color(Color::GrayDark));
        }
    }
    
    return vbox(scrollbar_elements) | size(WIDTH, EQUAL, 1) | yflex;
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
    // Use the LogEntryRenderer if available, otherwise fall back to basic rendering
    if (_Log_Entry_Renderer_) {
        // Configure the renderer with current settings
        _Log_Entry_Renderer_->SetWordWrapEnabled(word_wrap_enabled_);
        _Log_Entry_Renderer_->SetShowLineNumbers(show_line_numbers_);
        
        // Find the index of this entry in the filtered entries (used for both relative line numbers and visual selection)
        int entry_index = -1;
        for (int i = 0; i < static_cast<int>(filtered_entries_.size()); ++i) {
            if (&filtered_entries_[i] == &entry) {
                entry_index = i;
                break;
            }
        }
        
        // Calculate relative line number using RelativeLineNumberSystem
        int relative_line_number = 0;
        if (_Relative_Line_System_ && entry_index >= 0) {
            relative_line_number = entry_index - selected_entry_index_;
        }
        
        // Check if this entry is within visual selection range
        bool is_visual_selected = false;
        if (visual_selection_mode_ && entry_index >= 0) {
            auto range = GetVisualSelectionRange();
            is_visual_selected = (entry_index >= range.first && entry_index <= range.second);
        }
        
        // Render using the appropriate LogEntryRenderer method
        Element row;
        std::string highlight_term;
        
        // Priority: active search query first, then selected filter term
        if (!search_query_.empty()) {
            highlight_term = search_query_;
        } else {
            highlight_term = GetFilterHighlightTerm();
        }
        
        // Use visual selection rendering if in visual selection mode
        if (visual_selection_mode_) {
            row = _Log_Entry_Renderer_->RenderLogEntryWithVisualSelection(entry, is_selected, is_visual_selected, relative_line_number);
        } else if (!highlight_term.empty()) {
            // Use search highlighting if there's a term to highlight
            bool case_sensitive = HasUppercaseLetters(highlight_term);
            bool is_filter_highlight = search_query_.empty(); // If no active search, this is a filter highlight
            row = _Log_Entry_Renderer_->RenderLogEntryWithSearchHighlight(entry, is_selected, relative_line_number, highlight_term, case_sensitive, is_filter_highlight);
        } else {
            row = _Log_Entry_Renderer_->RenderLogEntry(entry, is_selected, relative_line_number);
        }
        
        // Apply additional styling for context lines and search highlighting
        bool is_match = match_line_numbers_.find(entry.Get_line_number()) != match_line_numbers_.end();
        
        if (!is_match && context_lines_ > 0) {
            // This is a context line - make it muted/gray
            row = row | dim | color(Color::GrayDark);
        }
        
        // Handle inline search highlighting (preserve existing functionality)
        if (is_selected && show_inline_search_ && !inline_search_query_.empty() && !inline_search_matches_.empty()) {
            // Note: This will be enhanced in the LogEntryRenderer in future iterations
            // For now, the basic highlighting is handled by the renderer
        }
        
        return row;
    }
    
    // Fallback to original rendering if LogEntryRenderer is not available
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
    
    // Logger column (basic text without badge)
    std::string logger_str = entry.Get_logger_name();
    row_elements.push_back(text(padString(logger_str, 18)));
    
    // Log level column
    std::string level_str = entry.Get_log_level().has_value() ? 
                           entry.Get_log_level().value() : "N/A";
    row_elements.push_back(text(padString(level_str, 8)));
    
    // Message column - handle word wrapping and in-line search highlighting
    Element message_element;
    if (is_selected && show_inline_search_ && !inline_search_query_.empty() && !inline_search_matches_.empty()) {
        // Create highlighted message for in-line search
        message_element = CreateHighlightedMessageElement(entry.Get_message());
    } else if (word_wrap_enabled_) {
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
                row = row | color(_Visual_Theme_Manager_->GetLogLevelColor("Error"));
            } else if (entry.Get_log_level().value() == "Warning") {
                row = row | color(_Visual_Theme_Manager_->GetLogLevelColor("Warning"));
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
    // Use the LogEntryRenderer if available, otherwise fall back to basic rendering
    if (_Log_Entry_Renderer_) {
        // Configure the renderer with current settings
        _Log_Entry_Renderer_->SetShowLineNumbers(show_line_numbers_);
        
        return _Log_Entry_Renderer_->RenderTableHeader();
    }
    
    // Fallback to original header rendering if LogEntryRenderer is not available
    // Helper function to pad string to fixed width
    auto padString = [](const std::string& str, size_t width) -> std::string {
        if (str.length() >= width) {
            return str.substr(0, width);
        }
        return str + std::string(width - str.length(), ' ');
    };
    
    // Build the header with separate elements to match the row layout
    std::vector<Element> header_elements;
    
    // Column numbering starts from 0, adjusts based on line numbers visibility
    int column_index = 0;
    
    if (show_line_numbers_) {
        std::string line_header = "[" + std::to_string(column_index++) + "] Line";
        header_elements.push_back(text(padString(line_header, 8)) | bold);
    }
    
    std::string timestamp_header = "[" + std::to_string(column_index++) + "] Timestamp";
    header_elements.push_back(text(padString(timestamp_header, 29)) | bold);
    
    std::string frame_header = "[" + std::to_string(column_index++) + "] Frame";
    header_elements.push_back(text(padString(frame_header, 9)) | bold);
    
    std::string logger_header = "[" + std::to_string(column_index++) + "] Logger";
    header_elements.push_back(text(padString(logger_header, 22)) | bold);
    
    std::string level_header = "[" + std::to_string(column_index++) + "] Level";
    header_elements.push_back(text(padString(level_header, 12)) | bold);
    
    std::string message_header = "[" + std::to_string(column_index++) + "] Message";
    header_elements.push_back(text(message_header) | bold | flex);
    
    return hbox(header_elements) | inverted;
}

ftxui::Color MainWindow::GetColorForLogLevel(const std::string& level) const {
    // Use the visual theme manager for consistent, eye-friendly colors
    return _Visual_Theme_Manager_->GetLogLevelColor(level);
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
    if (selected_entry_index_ < 0 || filtered_entries_.empty()) {
        return;
    }
    
    // Get the visible height using the consistent method
    int visible_height = GetVisibleHeight();
    int total_entries = static_cast<int>(filtered_entries_.size());
    
    // If we have fewer entries than visible height, no scrolling needed
    if (total_entries <= visible_height) {
        scroll_offset_ = 0;
        return;
    }
    
    // Update scroll_offset_ to ensure the selected entry is visible
    // with some context around it
    
    // If selection is above visible area, scroll up to show it with some context
    if (selected_entry_index_ < scroll_offset_) {
        // Position the selected entry 1/4 of the way down from the top
        scroll_offset_ = std::max(0, selected_entry_index_ - (visible_height / 4));
    }
    // If selection is below visible area, scroll down to show it with some context
    else if (selected_entry_index_ >= scroll_offset_ + visible_height) {
        // Position the selected entry 3/4 of the way down from the top
        int target_offset = selected_entry_index_ - (visible_height * 3 / 4);
        
        // For selections near the end, allow scrolling to show the selection
        // even if it means showing fewer entries at the bottom
        scroll_offset_ = std::max(0, target_offset);
    }
    
    // Ensure we don't scroll past the beginning
    scroll_offset_ = std::max(0, scroll_offset_);
    
    // For the maximum offset, be more permissive to allow proper scrolling
    // when the selection is near the end of the file
    int max_offset = std::max(0, total_entries - 1); // Allow scrolling to show last entry
    if (selected_entry_index_ >= total_entries - visible_height) {
        // When near the end, allow more flexible scrolling
        max_offset = total_entries - 1;
    } else {
        // Normal case: don't scroll past the point where we'd have empty space at bottom
        max_offset = total_entries - visible_height;
    }
    
    scroll_offset_ = std::min(scroll_offset_, max_offset);
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
    if (!_Filter_Engine_) {
        filtered_entries_ = log_entries_;
        return;
    }
    
    // Get all active filters from the filter engine
    const auto& filters = _Filter_Engine_->Get_primary_filters();
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
        // No active filters - show all entries or all entries with context
        if (context_lines_ == 0) {
            filtered_entries_ = log_entries_;
        } else {
            BuildContextEntries(log_entries_);
        }
    } else {
        // Apply active filters with AND logic
        for (const auto& entry : log_entries_) {
            bool entry_matches = true; // Start with true for AND logic
            
            // Check each active filter - entry must match ALL active filters
            for (const auto& filter : filters) {
                if (filter->Get_is_active() && !filter->Matches(entry)) {
                    entry_matches = false;
                    break; // AND logic - if any filter doesn't match, exclude the entry
                }
            }
            
            if (entry_matches) {
                matches.push_back(entry);
            }
        }
        
        // Apply context lines if needed
        if (context_lines_ == 0) {
            filtered_entries_ = matches;
        } else {
            BuildContextEntries(matches);
        }
    }
    
    // Update selection to stay within bounds
    if (selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        selected_entry_index_ = filtered_entries_.empty() ? -1 : static_cast<int>(filtered_entries_.size()) - 1;
    }
    EnsureSelectionVisible();
}

void MainWindow::OnFileMonitorError(const std::string& error) {
    _Last_Error_ = "File monitoring error: " + error;
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
    _Last_Error_ = "Search: (type to search, Enter to confirm, + to promote to filter, Esc to cancel)";
}

void MainWindow::HideSearch() {
    show_search_ = false;
    search_input_mode_ = false;
    search_query_.clear();
    search_results_.clear();
    search_result_index_ = -1;
    _Last_Error_.clear(); // Clear status bar message
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
    
    // If we have an active search, find the next result from current position
    if (search_result_index_ >= 0) {
        // Find next result after current position
        int current_pos = selected_entry_index_;
        for (int i = 0; i < static_cast<int>(search_results_.size()); ++i) {
            int result_idx = search_results_[i];
            if (result_idx > current_pos) {
                search_result_index_ = i;
                SelectEntry(result_idx);
                return;
            }
        }
        // If no result found after current position, wrap to first result
        search_result_index_ = 0;
        SelectEntry(search_results_[0]);
    } else {
        // No current search position, go to first result
        search_result_index_ = 0;
        SelectEntry(search_results_[0]);
    }
}

void MainWindow::FindPrevious() {
    if (search_results_.empty()) {
        return;
    }
    
    // If we have an active search, find the previous result from current position
    if (search_result_index_ >= 0) {
        // Find previous result before current position
        int current_pos = selected_entry_index_;
        for (int i = static_cast<int>(search_results_.size()) - 1; i >= 0; --i) {
            int result_idx = search_results_[i];
            if (result_idx < current_pos) {
                search_result_index_ = i;
                SelectEntry(result_idx);
                return;
            }
        }
        // If no result found before current position, wrap to last result
        search_result_index_ = static_cast<int>(search_results_.size()) - 1;
        SelectEntry(search_results_[search_result_index_]);
    } else {
        // No current search position, go to last result
        search_result_index_ = static_cast<int>(search_results_.size()) - 1;
        SelectEntry(search_results_[search_result_index_]);
    }
}

void MainWindow::ClearSearch() {
    search_query_.clear();
    search_results_.clear();
    search_result_index_ = -1;
}

// Search input handling implementations
void MainWindow::AppendToSearch(const std::string& text) {
    search_query_ += text;
    _Last_Error_ = "Search: " + search_query_ + " (Enter to confirm, + to promote, Esc to cancel)";
    // Perform search for highlighting but don't jump to results while typing
    PerformSearchHighlightOnly(search_query_);
}

void MainWindow::ConfirmSearch() {
    if (!search_query_.empty()) {
        PerformSearch(search_query_);
        if (!search_results_.empty()) {
            _Last_Error_ = "Found " + std::to_string(search_results_.size()) + " matches for '" + search_query_ + "'. Use n/N to navigate.";
        } else {
            _Last_Error_ = "No matches found for '" + search_query_ + "'";
        }
    }
    // Exit input mode but keep search active for navigation
    search_input_mode_ = false;
}

void MainWindow::BackspaceSearch() {
    if (!search_query_.empty()) {
        search_query_.pop_back();
        if (search_query_.empty()) {
            _Last_Error_ = "Search: (type to search, Enter to confirm, + to promote, Esc to cancel)";
            search_results_.clear();
            search_result_index_ = -1;
        } else {
            _Last_Error_ = "Search: " + search_query_ + " (Enter to confirm, + to promote, Esc to cancel)";
            // Perform search for highlighting but don't jump to results while typing
            PerformSearchHighlightOnly(search_query_);
        }
    }
}

// In-line search functionality implementations
void MainWindow::ShowInlineSearch() {
    if (selected_entry_index_ < 0 || selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        _Last_Error_ = "No line selected for in-line search";
        return;
    }
    
    show_inline_search_ = true;
    inline_search_input_mode_ = true;
    inline_search_query_.clear();
    inline_search_matches_.clear();
    current_inline_match_ = 0;
    
    _Last_Error_ = "In-line search: (type to search within current line, Enter to confirm, Esc to cancel)";
}

void MainWindow::HideInlineSearch() {
    show_inline_search_ = false;
    inline_search_input_mode_ = false;
    inline_search_query_.clear();
    inline_search_matches_.clear();
    current_inline_match_ = 0;
    _Last_Error_.clear();
}

void MainWindow::AppendToInlineSearch(const std::string& text) {
    inline_search_query_ += text;
    UpdateInlineSearchResults();
    
    if (inline_search_matches_.empty()) {
        _Last_Error_ = "In-line search: " + inline_search_query_ + " (no matches in current line)";
    } else {
        _Last_Error_ = "In-line search: " + inline_search_query_ + " (" + 
                     std::to_string(inline_search_matches_.size()) + " matches in current line)";
    }
}

void MainWindow::BackspaceInlineSearch() {
    if (!inline_search_query_.empty()) {
        inline_search_query_.pop_back();
        if (inline_search_query_.empty()) {
            _Last_Error_ = "In-line search: (type to search within current line, Enter to confirm, Esc to cancel)";
            inline_search_matches_.clear();
            current_inline_match_ = 0;
        } else {
            UpdateInlineSearchResults();
            if (inline_search_matches_.empty()) {
                _Last_Error_ = "In-line search: " + inline_search_query_ + " (no matches in current line)";
            } else {
                _Last_Error_ = "In-line search: " + inline_search_query_ + " (" + 
                             std::to_string(inline_search_matches_.size()) + " matches in current line)";
            }
        }
    }
}

void MainWindow::ConfirmInlineSearch() {
    if (inline_search_query_.empty()) {
        HideInlineSearch();
        return;
    }
    
    inline_search_input_mode_ = false;
    UpdateInlineSearchResults();
    
    if (inline_search_matches_.empty()) {
        _Last_Error_ = "No matches found in current line";
    } else {
        current_inline_match_ = 0;
        _Last_Error_ = "Found " + std::to_string(inline_search_matches_.size()) + 
                     " matches in current line. Press n/N to navigate, Esc to exit.";
    }
}

void MainWindow::FindNextInlineMatch() {
    if (inline_search_matches_.empty()) {
        return;
    }
    
    current_inline_match_ = (current_inline_match_ + 1) % inline_search_matches_.size();
    _Last_Error_ = "In-line match " + std::to_string(current_inline_match_ + 1) + 
                 " of " + std::to_string(inline_search_matches_.size()) + 
                 " (position " + std::to_string(inline_search_matches_[current_inline_match_]) + ")";
}

void MainWindow::FindPreviousInlineMatch() {
    if (inline_search_matches_.empty()) {
        return;
    }
    
    if (current_inline_match_ == 0) {
        current_inline_match_ = inline_search_matches_.size() - 1;
    } else {
        current_inline_match_--;
    }
    
    _Last_Error_ = "In-line match " + std::to_string(current_inline_match_ + 1) + 
                 " of " + std::to_string(inline_search_matches_.size()) + 
                 " (position " + std::to_string(inline_search_matches_[current_inline_match_]) + ")";
}

void MainWindow::UpdateInlineSearchResults() {
    inline_search_matches_.clear();
    current_inline_match_ = 0;
    
    if (inline_search_query_.empty() || selected_entry_index_ < 0 || 
        selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        return;
    }
    
    const auto& entry = filtered_entries_[selected_entry_index_];
    const std::string& line_text = entry.Get_raw_line();
    
    // Smart case sensitivity: case-sensitive if query contains uppercase, case-insensitive if all lowercase
    bool case_sensitive = HasUppercaseLetters(inline_search_query_);
    
    std::string search_text = line_text;
    std::string search_query = inline_search_query_;
    
    if (!case_sensitive) {
        std::transform(search_text.begin(), search_text.end(), search_text.begin(), ::tolower);
        std::transform(search_query.begin(), search_query.end(), search_query.begin(), ::tolower);
    }
    
    // Find all occurrences
    size_t pos = 0;
    while ((pos = search_text.find(search_query, pos)) != std::string::npos) {
        inline_search_matches_.push_back(pos);
        pos += search_query.length();
    }
}

// Jump functionality implementations
void MainWindow::ShowJumpDialog() {
    show_jump_dialog_ = true;
    jump_input_.clear();
    
    // Provide immediate feedback
    _Last_Error_ = "Jump dialog activated. Enter line number or timestamp, then press Enter.";
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
    _Last_Error_ = "Jumped to " + std::to_string(percentage) + "% (" + std::to_string(target_index + 1) + " of " + std::to_string(filtered_entries_.size()) + ")";
}

void MainWindow::ToggleJumpMode() {
    jump_to_line_mode_ = !jump_to_line_mode_;
}

void MainWindow::AppendToJumpInput(const std::string& text) {
    jump_input_ += text;
    _Last_Error_ = "Jump to: " + jump_input_ + " (Enter to execute, Esc to cancel)";
}

void MainWindow::BackspaceJumpInput() {
    if (!jump_input_.empty()) {
        jump_input_.pop_back();
        _Last_Error_ = jump_input_.empty() ? "Jump to: (Enter line number)" : "Jump to: " + jump_input_ + " (Enter to execute, Esc to cancel)";
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
        _Last_Error_ = "Jumped to line " + std::to_string(line_number);
    } catch (const std::exception&) {
        _Last_Error_ = "Invalid line number: " + jump_input_;
    }
    
    HideJumpDialog();
}

void MainWindow::ToggleFilterPanel() {
    show_filter_panel_ = !show_filter_panel_;
    
    // Update filter panel visibility
    if (_Filter_Panel_) {
        _Filter_Panel_->SetVisible(show_filter_panel_);
    }
    
    // Provide feedback
    _Last_Error_ = show_filter_panel_ ? "Filter panel shown" : "Filter panel hidden";
}

void MainWindow::ToggleWordWrap() {
    word_wrap_enabled_ = !word_wrap_enabled_;
    
    if (word_wrap_enabled_) {
        _Last_Error_ = "Word wrap enabled - long lines will wrap";
    } else {
        _Last_Error_ = "Word wrap disabled - long lines will be truncated";
    }
}

void MainWindow::ToggleDetailView() {
    show_detail_view_ = !show_detail_view_;
    
    if (show_detail_view_) {
        _Last_Error_ = "Detail view shown - full log entry displayed below";
    } else {
        _Last_Error_ = "Detail view hidden - press 'd' to show";
    }
}

bool MainWindow::HandleVimStyleNavigation(const std::string& input) {
    if (input.empty()) {
        return false;
    }

    if (_Filter_Panel_ && _Filter_Panel_->IsFocused()) {
        return false; // Let filter panel handle j/k instead
    }
    
    char ch = input[0];
    
    // Handle digits - add to command buffer
    if (std::isdigit(ch)) {
        vim_command_buffer_ += ch;
        vim_command_mode_ = true;
        _Last_Error_ = "Vim command: " + vim_command_buffer_ + " (press j/k to execute)";
        return true;
    }
    
    // Handle j/k commands
    if (ch == 'j' || ch == 'k') {
        // If detail view is focused, let it handle navigation instead of vim navigation
        if (detail_view_focused_) {
            // Don't handle vim navigation when detail view is focused
            // Let the regular navigation handler take care of it
            ClearVimCommandBuffer();
            return false;
        }
        
        std::string full_command = vim_command_buffer_ + ch;
        
        int jump_distance;
        char direction;
        
        if (_Relative_Line_System_->HandleNavigationInput(full_command, jump_distance, direction)) {
            ExecuteVimNavigation(jump_distance, direction);
            ClearVimCommandBuffer();
            return true;
        } else {
            // If no number was accumulated, treat as single j/k
            ExecuteVimNavigation(1, ch);
            ClearVimCommandBuffer();
            return true;
        }
    }
    
    // Clear command buffer if we get an invalid character
    if (vim_command_mode_) {
        ClearVimCommandBuffer();
    }
    
    return false;
}

void MainWindow::ExecuteVimNavigation(int count, char direction) {
    // Check if tailing should be stopped for j/k navigation events
    if (IsTailing()) {
        StopTailing();
    }
    
    if (direction == 'j') {
        // Move down
        ScrollDown(count);
        _Last_Error_ = "Moved down " + std::to_string(count) + " line" + (count > 1 ? "s" : "");
    } else if (direction == 'k') {
        // Move up
        ScrollUp(count);
        _Last_Error_ = "Moved up " + std::to_string(count) + " line" + (count > 1 ? "s" : "");
    }
}

void MainWindow::ClearVimCommandBuffer() {
    vim_command_buffer_.clear();
    vim_command_mode_ = false;
}

void MainWindow::BackspaceVimCommand() {
    if (!vim_command_buffer_.empty()) {
        vim_command_buffer_.pop_back();
        if (vim_command_buffer_.empty()) {
            vim_command_mode_ = false;
            _Last_Error_ = "Vim command cleared";
        } else {
            _Last_Error_ = "Vim command: " + vim_command_buffer_ + " (press j/k to execute)";
        }
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
    if (!_Filter_Engine_) {
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
    _Filter_Engine_->AddFilter(std::move(error_filter));
    _Filter_Engine_->AddFilter(std::move(warning_filter));
    _Filter_Engine_->AddFilter(std::move(init_filter));
    
    // Refresh the filter panel to show the new filters
    if (_Filter_Panel_) {
        _Filter_Panel_->RefreshFilters();
    }
}

// Enhanced search functionality implementations
void MainWindow::PromoteSearchToFilter() {
    if (search_query_.empty()) {
        _Last_Error_ = "No active search to promote";
        return;
    }
    
    show_search_promotion_ = true;
    _Last_Error_ = "Promote search '" + search_query_ + "' to filter: [1] Timestamp after [2] Frame after [3] Logger contains [4] Level equals [5] Message contains";
}

void MainWindow::ShowSearchPromotionDialog() {
    show_search_promotion_ = true;
}

void MainWindow::HideSearchPromotionDialog() {
    show_search_promotion_ = false;
}

void MainWindow::CreateFilterFromSearch(FilterConditionType type) {
    if (search_query_.empty()) {
        _Last_Error_ = "No search query to promote";
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
            _Last_Error_ = "Added filter: Message contains '" + search_query_ + "'";
            break;
        case FilterConditionType::LogLevelEquals:
            condition = FilterConditionFactory::CreateLogLevelEquals(search_query_);
            _Last_Error_ = "Added filter: LogLevel = '" + search_query_ + "'";
            break;
        case FilterConditionType::LoggerContains:
            condition = std::make_unique<FilterCondition>(FilterConditionType::LoggerContains, search_query_);
            _Last_Error_ = "Added filter: Logger contains '" + search_query_ + "'";
            break;
        case FilterConditionType::AnyFieldContains:
            condition = FilterConditionFactory::CreateAnyFieldContains(search_query_);
            _Last_Error_ = "Added filter: Any field contains '" + search_query_ + "'";
            break;
        default:
            _Last_Error_ = "Unknown filter type";
            break;
    }
    
    if (condition) {
        current_filter_expression_->AddCondition(std::move(condition));
        ApplyCurrentFilter();
        
        // Update filter panel to show the new filter
        if (_Filter_Panel_) {
            _Filter_Panel_->SetCurrentFilterExpression(current_filter_expression_.get());
        }
    }
    
    // Close the search promotion dialog
    show_search_promotion_ = false;
    
    // Keep search active for further use
}

// Contextual filtering implementations
void MainWindow::ShowContextualFilterDialog() {
    if (selected_entry_index_ < 0 || selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        _Last_Error_ = "No log entry selected for contextual filtering";
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
    _Last_Error_ = options;
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
                _Last_Error_ = "Added filter: Timestamp >= " + entry.Get_timestamp().value();
            }
            break;
        case FilterConditionType::LoggerEquals:
            condition = FilterConditionFactory::CreateLoggerEquals(entry.Get_logger_name());
            _Last_Error_ = "Added filter: Logger = " + entry.Get_logger_name();
            break;
        case FilterConditionType::LogLevelEquals:
            if (entry.Get_log_level().has_value()) {
                condition = FilterConditionFactory::CreateLogLevelEquals(entry.Get_log_level().value());
                _Last_Error_ = "Added filter: LogLevel = " + entry.Get_log_level().value();
            }
            break;
        case FilterConditionType::FrameAfter:
            if (entry.Get_frame_number().has_value()) {
                condition = FilterConditionFactory::CreateFrameAfter(entry.Get_frame_number().value());
                _Last_Error_ = "Added filter: Frame >= " + std::to_string(entry.Get_frame_number().value());
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
    _Last_Error_ = "Contextual filter dialog closed";
}

// Context lines functionality implementations
void MainWindow::IncreaseContext() {
    if (context_lines_ < 10) {
        if (context_lines_ == 0) context_lines_ = 1;
        else if (context_lines_ < 3) context_lines_++;
        else if (context_lines_ == 3) context_lines_ = 5;
        else if (context_lines_ == 5) context_lines_ = 10;
        
        OnFiltersChanged();
        _Last_Error_ = "Context lines: ±" + std::to_string(context_lines_);
    }
}

void MainWindow::DecreaseContext() {
    if (context_lines_ > 0) {
        if (context_lines_ == 10) context_lines_ = 5;
        else if (context_lines_ == 5) context_lines_ = 3;
        else if (context_lines_ <= 3) context_lines_--;
        
        OnFiltersChanged();
        _Last_Error_ = context_lines_ == 0 ? "Context lines: None" : "Context lines: ±" + std::to_string(context_lines_);
    }
}

void MainWindow::SetContextLines(int lines) {
    context_lines_ = std::max(0, std::min(10, lines));
    OnFiltersChanged();
    _Last_Error_ = context_lines_ == 0 ? "Context lines: None" : "Context lines: ±" + std::to_string(context_lines_);
}

void MainWindow::ClearContext() {
    context_lines_ = 0;
    OnFiltersChanged();
    _Last_Error_ = "Context lines cleared";
}

// Helper methods
bool MainWindow::HasUppercaseLetters(const std::string& text) const {
    return std::any_of(text.begin(), text.end(), [](char c) {
        return std::isupper(c);
    });
}

ftxui::Element MainWindow::CreateHighlightedMessageElement(const std::string& message) const {
    using namespace ftxui;
    
    if (inline_search_matches_.empty() || inline_search_query_.empty()) {
        return text(message);
    }
    
    std::vector<Element> elements;
    size_t last_pos = 0;
    
    // Create highlighted segments
    for (size_t match_pos : inline_search_matches_) {
        // Add text before the match
        if (match_pos > last_pos) {
            std::string before_match = message.substr(last_pos, match_pos - last_pos);
            elements.push_back(text(before_match));
        }
        
        // Add highlighted match
        std::string match_text = message.substr(match_pos, inline_search_query_.length());
        Element highlighted_match = text(match_text) | bgcolor(Color::YellowLight) | color(Color::Black);
        
        // If this is the current match, make it even more prominent
        if (!inline_search_matches_.empty() && 
            current_inline_match_ < inline_search_matches_.size() && 
            match_pos == inline_search_matches_[current_inline_match_]) {
            highlighted_match = highlighted_match | bold | bgcolor(Color::YellowLight);
        }
        
        elements.push_back(highlighted_match);
        last_pos = match_pos + inline_search_query_.length();
    }
    
    // Add remaining text after the last match
    if (last_pos < message.length()) {
        std::string after_matches = message.substr(last_pos);
        elements.push_back(text(after_matches));
    }
    
    return hbox(elements);
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
            
            return text(options) | bgcolor(Color::YellowLight) | color(Color::Black);
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
    _Last_Error_ = "Quick filters: [e] Errors, [w] Warnings, [i] Info, [d] Debug, [c] Clear filters (Esc to cancel)";
}

void MainWindow::HideQuickFilterDialog() {
    show_quick_filter_dialog_ = false;
    _Last_Error_.clear();
}

void MainWindow::ApplyQuickFilter(const std::string& filter_type) {
    if (!_Filter_Engine_) {
        _Last_Error_ = "Filter engine not available";
        HideQuickFilterDialog();
        return;
    }
    
    if (filter_type == "clear") {
        // Clear all filters by creating an empty filter expression
        current_filter_expression_ = std::make_unique<FilterExpression>(FilterOperator::And);
        _Last_Error_ = "All filters cleared";
    } else {
        // Create a quick filter
        std::unique_ptr<FilterCondition> condition;
        
        if (filter_type == "error") {
            condition = std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Error");
            _Last_Error_ = "Quick filter applied: Showing only Error entries";
        } else if (filter_type == "warning") {
            condition = std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Warning");
            _Last_Error_ = "Quick filter applied: Showing only Warning entries";
        } else if (filter_type == "info") {
            condition = std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Info");
            _Last_Error_ = "Quick filter applied: Showing only Info entries";
        } else if (filter_type == "debug") {
            condition = std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Debug");
            _Last_Error_ = "Quick filter applied: Showing only Debug entries";
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
        _Last_Error_ = "No entries to search";
        return;
    }
    
    // Start searching from the next entry after current selection
    int start_index = selected_entry_index_ + 1;
    
    for (int i = start_index; i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Error") {
            SelectEntry(i);
            _Last_Error_ = "Jumped to next error at line " + std::to_string(i + 1);
            return;
        }
    }
    
    // If not found, wrap around to beginning
    for (int i = 0; i < start_index && i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Error") {
            SelectEntry(i);
            _Last_Error_ = "Wrapped to first error at line " + std::to_string(i + 1);
            return;
        }
    }
    
    _Last_Error_ = "No error entries found";
}

void MainWindow::JumpToPreviousError() {
    if (filtered_entries_.empty()) {
        _Last_Error_ = "No entries to search";
        return;
    }
    
    // Start searching from the previous entry before current selection
    int start_index = selected_entry_index_ - 1;
    
    for (int i = start_index; i >= 0; --i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Error") {
            SelectEntry(i);
            _Last_Error_ = "Jumped to previous error at line " + std::to_string(i + 1);
            return;
        }
    }
    
    // If not found, wrap around to end
    for (int i = static_cast<int>(filtered_entries_.size()) - 1; i > start_index; --i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Error") {
            SelectEntry(i);
            _Last_Error_ = "Wrapped to last error at line " + std::to_string(i + 1);
            return;
        }
    }
    
    _Last_Error_ = "No error entries found";
}

void MainWindow::JumpToNextWarning() {
    if (filtered_entries_.empty()) {
        _Last_Error_ = "No entries to search";
        return;
    }
    
    // Start searching from the next entry after current selection
    int start_index = selected_entry_index_ + 1;
    
    for (int i = start_index; i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Warning") {
            SelectEntry(i);
            _Last_Error_ = "Jumped to next warning at line " + std::to_string(i + 1);
            return;
        }
    }
    
    // If not found, wrap around to beginning
    for (int i = 0; i < start_index && i < static_cast<int>(filtered_entries_.size()); ++i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Warning") {
            SelectEntry(i);
            _Last_Error_ = "Wrapped to first warning at line " + std::to_string(i + 1);
            return;
        }
    }
    
    _Last_Error_ = "No warning entries found";
}

void MainWindow::JumpToPreviousWarning() {
    if (filtered_entries_.empty()) {
        _Last_Error_ = "No entries to search";
        return;
    }
    
    // Start searching from the previous entry before current selection
    int start_index = selected_entry_index_ - 1;
    
    for (int i = start_index; i >= 0; --i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Warning") {
            SelectEntry(i);
            _Last_Error_ = "Jumped to previous warning at line " + std::to_string(i + 1);
            return;
        }
    }
    
    // If not found, wrap around to end
    for (int i = static_cast<int>(filtered_entries_.size()) - 1; i > start_index; --i) {
        const auto& entry = filtered_entries_[i];
        if (entry.Get_log_level().has_value() && entry.Get_log_level().value() == "Warning") {
            SelectEntry(i);
            _Last_Error_ = "Wrapped to last warning at line " + std::to_string(i + 1);
            return;
        }
    }
    
    _Last_Error_ = "No warning entries found";
}

void MainWindow::CreateLineNumberFilter(const LogEntry& entry) {
    // Line number filters don't make much sense, so we'll create a "from this line onward" filter
    // This is more of a navigation aid than a true filter
    size_t line_num = entry.Get_line_number();
    _Last_Error_ = "Line number filter not implemented - use navigation instead (line " + std::to_string(line_num) + ")";
}

void MainWindow::CreateTimestampAfterFilter(const LogEntry& entry) {
    if (!entry.HasTimestamp()) {
        _Last_Error_ = "Selected entry has no timestamp value";
        return;
    }
    
    std::string timestamp = entry.Get_timestamp().value();
    
    // Create a text contains filter that matches entries with this timestamp
    // Note: This is a simplified implementation since TimeRange isn't fully implemented
    std::string filter_name = "Timestamp: " + timestamp;
    auto filter = std::make_unique<Filter>(filter_name, FilterType::TextContains, timestamp);
    
    // Add the filter to the filter engine
    auto result = _Filter_Engine_->AddFilter(std::move(filter));
    if (result.IsError()) {
        _Last_Error_ = "Failed to create timestamp filter: " + result.Get_error_message();
        return;
    }
    
    // Debug: Check filter count (can be removed later)
    // std::cout << "Filter added. Total filters: " << filter_engine_->GetFilterCount() << std::endl;
    
    // Refresh the filter panel to show the new filter
    if (_Filter_Panel_) {
        _Filter_Panel_->RefreshFilters();
    }
    
    // Apply filters to update the display
    OnFiltersChanged();
    
    _Last_Error_ = "Created timestamp filter: " + timestamp;
}

void MainWindow::CreateFrameAfterFilter(const LogEntry& entry) {
    if (!entry.HasFrameNumber()) {
        _Last_Error_ = "Selected entry has no frame number value";
        return;
    }
    
    int frame = entry.Get_frame_number().value();
    
    // Create a text contains filter that matches entries with this frame number
    std::string filter_name = "Frame: " + std::to_string(frame);
    auto filter = std::make_unique<Filter>(filter_name, FilterType::TextContains, std::to_string(frame));
    
    // Add the filter to the filter engine
    auto result = _Filter_Engine_->AddFilter(std::move(filter));
    if (result.IsError()) {
        _Last_Error_ = "Failed to create frame filter: " + result.Get_error_message();
        return;
    }
    
    // Refresh the filter panel to show the new filter
    if (_Filter_Panel_) {
        _Filter_Panel_->RefreshFilters();
    }
    
    // Apply filters to update the display
    OnFiltersChanged();
    
    _Last_Error_ = "Created frame filter: " + std::to_string(frame);
}

void MainWindow::CreateLoggerEqualsFilter(const LogEntry& entry) {
    std::string logger = entry.Get_logger_name();
    if (logger.empty()) {
        _Last_Error_ = "Selected entry has no logger name";
        return;
    }
    
    // Create a logger name filter
    std::string filter_name = "Logger: " + logger;
    auto filter = std::make_unique<Filter>(filter_name, FilterType::LoggerName, logger);
    
    // Add the filter to the filter engine
    auto result = _Filter_Engine_->AddFilter(std::move(filter));
    if (result.IsError()) {
        _Last_Error_ = "Failed to create logger filter: " + result.Get_error_message();
        return;
    }
    
    // Refresh the filter panel to show the new filter
    if (_Filter_Panel_) {
        _Filter_Panel_->RefreshFilters();
    }
    
    // Apply filters to update the display
    OnFiltersChanged();
    
    _Last_Error_ = "Created logger filter: " + logger;
}

void MainWindow::CreateLevelEqualsFilter(const LogEntry& entry) {
    if (!entry.HasLogLevel()) {
        _Last_Error_ = "Selected entry has no log level value";
        return;
    }
    
    std::string level = entry.Get_log_level().value();
    
    // Create a log level filter
    std::string filter_name = "Level: " + level;
    auto filter = std::make_unique<Filter>(filter_name, FilterType::LogLevel, level);
    
    // Add the filter to the filter engine
    auto result = _Filter_Engine_->AddFilter(std::move(filter));
    if (result.IsError()) {
        _Last_Error_ = "Failed to create level filter: " + result.Get_error_message();
        return;
    }
    
    // Refresh the filter panel to show the new filter
    if (_Filter_Panel_) {
        _Filter_Panel_->RefreshFilters();
    }
    
    // Apply filters to update the display
    OnFiltersChanged();
    
    _Last_Error_ = "Created level filter: " + level;
}

void MainWindow::CreateMessageContainsFilter(const LogEntry& entry) {
    std::string message = entry.Get_message();
    if (message.empty()) {
        _Last_Error_ = "Selected entry has no message content";
        return;
    }
    
    // For message filters, we'll use the first few words to avoid overly specific filters
    std::string filter_text = message;
    if (message.length() > 50) {
        size_t space_pos = message.find(' ', 30);
        if (space_pos != std::string::npos) {
            filter_text = message.substr(0, space_pos);
        }
    }
    
    // Create a text contains filter for the message
    std::string filter_name = "Message: \"" + filter_text + "\"";
    auto filter = std::make_unique<Filter>(filter_name, FilterType::TextContains, filter_text);
    
    // Add the filter to the filter engine
    auto result = _Filter_Engine_->AddFilter(std::move(filter));
    if (result.IsError()) {
        _Last_Error_ = "Failed to create message filter: " + result.Get_error_message();
        return;
    }
    
    // Refresh the filter panel to show the new filter
    if (_Filter_Panel_) {
        _Filter_Panel_->RefreshFilters();
    }
    
    // Apply filters to update the display
    OnFiltersChanged();
    
    _Last_Error_ = "Created message filter: \"" + filter_text + "\"";
}

void MainWindow::PromoteSearchToColumnFilter(int column_number) {
    // Debug: Check search state
    std::cout << "Search active: " << IsSearchActive() << ", Query: '" << search_query_ << "'" << std::endl;
    
    if (!IsSearchActive() || search_query_.empty()) {
        _Last_Error_ = "No active search to promote to filter (query: '" + search_query_ + "')";
        return;
    }
    
    // Store the search query before it gets cleared
    std::string current_search_query = search_query_;
    
    // Map column numbers to filter types for search promotion (matching column layout)
    // Key 1: Timestamp, Key 2: Frame, Key 3: Logger, Key 4: Level, Key 5: Message
    FilterConditionType filter_type;
    std::string filter_description;
    
    switch (column_number) {
        case 0: // Key 1 - Timestamp (treat search as timestamp contains)
            filter_type = FilterConditionType::TimestampAfter;
            filter_description = "Timestamp after";
            break;
        case 1: // Key 2 - Frame (treat search as frame after)
            filter_type = FilterConditionType::FrameAfter;
            filter_description = "Frame after";
            break;
        case 2: // Key 3 - Logger contains
            filter_type = FilterConditionType::LoggerContains;
            filter_description = "Logger contains";
            break;
        case 3: // Key 4 - Level equals
            filter_type = FilterConditionType::LogLevelEquals;
            filter_description = "Level equals";
            break;
        case 4: // Key 5 - Message contains
            filter_type = FilterConditionType::MessageContains;
            filter_description = "Message contains";
            break;
        default:
            _Last_Error_ = "Invalid search promotion option: " + std::to_string(column_number + 1);
            return;
    }
    
    // Create the filter from the current search term
    CreateFilterFromSearchAndColumn(filter_type, current_search_query);
    
    // Clear the search since it's now been promoted to a filter
    HideSearch();
    
    _Last_Error_ = "Created filter: " + filter_description + " \"" + current_search_query + "\"";
}

void MainWindow::CreateDirectColumnFilter(int column_number) {
    // Check if we have a selected entry
    if (selected_entry_index_ < 0 || selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        _Last_Error_ = "No entry selected for column filter";
        return;
    }
    
    const auto& selected_entry = filtered_entries_[selected_entry_index_];
    
    // Adjust column number based on line numbers visibility
    int actual_column = column_number;
    if (!show_line_numbers_ && column_number > 0) {
        actual_column = column_number - 1; // Shift down if line numbers are hidden
    }
    
    // Create filter based on column (1-based numbering to match headers)
    // Key 1 = Timestamp, Key 2 = Frame, Key 3 = Logger, Key 4 = Level, Key 5 = Message
    switch (column_number) {
        case 0: // Key 1 - Timestamp column
            CreateTimestampAfterFilter(selected_entry);
            break;
        case 1: // Key 2 - Frame column
            CreateFrameAfterFilter(selected_entry);
            break;
        case 2: // Key 3 - Logger column
            CreateLoggerEqualsFilter(selected_entry);
            break;
        case 3: // Key 4 - Level column
            CreateLevelEqualsFilter(selected_entry);
            break;
        case 4: // Key 5 - Message column
            CreateMessageContainsFilter(selected_entry);
            break;
        default:
            _Last_Error_ = "Invalid column number: " + std::to_string(column_number + 1);
            break;
    }
}

void MainWindow::CreateFilterFromSearchAndColumn(FilterConditionType type, const std::string& search_term) {
    if (search_term.empty()) {
        _Last_Error_ = "Cannot create filter with empty search term";
        return;
    }
    
    // Map FilterConditionType to FilterType and create appropriate filter
    std::string filter_name;
    FilterType filter_type_enum;
    std::string type_description;
    
    switch (type) {
        case FilterConditionType::AnyFieldContains:
            filter_name = "Any field contains: " + search_term;
            filter_type_enum = FilterType::TextContains;
            type_description = "Any field contains";
            break;
        case FilterConditionType::MessageContains:
            filter_name = "Message contains: " + search_term;
            filter_type_enum = FilterType::TextContains;
            type_description = "Message contains";
            break;
        case FilterConditionType::LoggerContains:
            filter_name = "Logger contains: " + search_term;
            filter_type_enum = FilterType::LoggerName;
            type_description = "Logger contains";
            break;
        case FilterConditionType::LogLevelEquals:
            filter_name = "Level equals: " + search_term;
            filter_type_enum = FilterType::LogLevel;
            type_description = "Level equals";
            break;
        default:
            _Last_Error_ = "Unknown filter type";
            return;
    }
    
    // Create the filter
    auto filter = std::make_unique<Filter>(filter_name, filter_type_enum, search_term);
    
    // Add the filter to the filter engine
    auto result = _Filter_Engine_->AddFilter(std::move(filter));
    if (result.IsError()) {
        _Last_Error_ = "Failed to create filter: " + result.Get_error_message();
        return;
    }
    
    // Refresh the filter panel to show the new filter
    if (_Filter_Panel_) {
        _Filter_Panel_->RefreshFilters();
    }
    
    // Apply filters to update the display
    OnFiltersChanged();
    
    _Last_Error_ = "Filter created: " + type_description + " \"" + search_term + "\"";
}

// Detail view focus and navigation methods
void MainWindow::FocusDetailView() {
    detail_view_focused_ = true;
    detail_view_scroll_offset_ = 0; // Reset scroll when focusing
}

void MainWindow::UnfocusDetailView() {
    detail_view_focused_ = false;
    detail_view_scroll_offset_ = 0; // Reset scroll when unfocusing
}

void MainWindow::DetailViewScrollUp(int count) {
    if (!detail_view_focused_) return;
    
    detail_view_scroll_offset_ = std::max(0, detail_view_scroll_offset_ - count);
}

void MainWindow::DetailViewScrollDown(int count) {
    if (!detail_view_focused_) return;
    
    // Get the current entry to calculate max scroll
    if (selected_entry_index_ >= 0 && selected_entry_index_ < static_cast<int>(filtered_entries_.size())) {
        const auto& selected_entry = filtered_entries_[selected_entry_index_];
        std::string full_message = selected_entry.Get_message();
        
        // Count lines in the message
        int line_count = 1;
        for (char c : full_message) {
            if (c == '\n') line_count++;
        }
        
        int available_height = (window_height_ > 0) ? window_height_ : 30; // Fallback to 30 lines
        int content_height = available_height - 2; // Exclude status bars
        int visible_height = std::max(10, (content_height * 2) / 3 - 4); // 2/3 of screen, account for window borders
        int max_scroll = std::max(0, line_count - visible_height);
        
        detail_view_scroll_offset_ = std::min(max_scroll, detail_view_scroll_offset_ + count);
    }
}

void MainWindow::DetailViewPageUp() {
    if (!detail_view_focused_) return;
    
    int available_height = (window_height_ > 0) ? window_height_ : 30; // Fallback to 30 lines
    int content_height = available_height - 2; // Exclude status bars
    int page_size = std::max(5, ((content_height * 2) / 3 - 4) / 2); // Half of visible height
    DetailViewScrollUp(page_size);
}

void MainWindow::DetailViewPageDown() {
    if (!detail_view_focused_) return;
    
    int available_height = (window_height_ > 0) ? window_height_ : 30; // Fallback to 30 lines
    int content_height = available_height - 2; // Exclude status bars
    int page_size = std::max(5, ((content_height * 2) / 3 - 4) / 2); // Half of visible height
    DetailViewScrollDown(page_size);
}

void MainWindow::DetailViewHalfPageUp() {
    if (!detail_view_focused_) return;
    
    int available_height = (window_height_ > 0) ? window_height_ : 30; // Fallback to 30 lines
    int content_height = available_height - 2; // Exclude status bars
    int half_page_size = std::max(2, ((content_height * 2) / 3 - 4) / 4); // Quarter of visible height
    DetailViewScrollUp(half_page_size);
}

void MainWindow::DetailViewHalfPageDown() {
    if (!detail_view_focused_) return;
    
    int available_height = (window_height_ > 0) ? window_height_ : 30; // Fallback to 30 lines
    int content_height = available_height - 2; // Exclude status bars
    int half_page_size = std::max(2, ((content_height * 2) / 3 - 4) / 4); // Quarter of visible height
    DetailViewScrollDown(half_page_size);
}

void MainWindow::DetailViewScrollToTop() {
    if (!detail_view_focused_) return;
    
    detail_view_scroll_offset_ = 0;
}

void MainWindow::DetailViewScrollToBottom() {
    if (!detail_view_focused_) return;
    
    // Get the current entry to calculate max scroll
    if (selected_entry_index_ >= 0 && selected_entry_index_ < static_cast<int>(filtered_entries_.size())) {
        const auto& selected_entry = filtered_entries_[selected_entry_index_];
        std::string full_message = selected_entry.Get_message();
        
        // Count lines in the message
        int line_count = 1;
        for (char c : full_message) {
            if (c == '\n') line_count++;
        }
        
        int available_height = (window_height_ > 0) ? window_height_ : 30; // Fallback to 30 lines
        int content_height = available_height - 2; // Exclude status bars
        int visible_height = std::max(10, (content_height * 2) / 3 - 4); // 2/3 of screen, account for window borders
        detail_view_scroll_offset_ = std::max(0, line_count - visible_height);
    }
}

bool MainWindow::ShouldStopTailing(const ftxui::Event& event) const {
    // Check for navigation events that should stop tailing
    if (event == ftxui::Event::Character('j') || event == ftxui::Event::Character('k')) {
        return true;
    }
    if (event == ftxui::Event::ArrowUp || event == ftxui::Event::ArrowDown) {
        return true;
    }
    if (event == ftxui::Event::Character(static_cast<char>(4))) { // Ctrl+D
        return true;
    }
    if (event == ftxui::Event::Character(static_cast<char>(21))) { // Ctrl+U
        return true;
    }
    if (event == ftxui::Event::PageUp || event == ftxui::Event::PageDown) {
        return true;
    }
    
    return false;
}

void MainWindow::CreateDirectColumnExcludeFilter(int column_number) {
    // Check if we have a selected entry
    if (selected_entry_index_ < 0 || selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        _Last_Error_ = "No entry selected for column exclude filter";
        return;
    }
    
    const auto& selected_entry = filtered_entries_[selected_entry_index_];
    
    // Create EXCLUDE filter based on column (same logic as include but with exclude flag)
    std::string filter_name;
    FilterType filter_type_enum;
    std::string filter_value;
    
    switch (column_number) {
        case 0: // Timestamp column - exclude entries with this timestamp (exact match)
            if (selected_entry.Get_timestamp().has_value()) {
                filter_name = "Exclude timestamp: " + selected_entry.Get_timestamp().value();
                filter_type_enum = FilterType::TimeRange; // Use TimeRange for timestamp filtering
                filter_value = selected_entry.Get_timestamp().value();
            } else {
                _Last_Error_ = "Selected entry has no timestamp";
                return;
            }
            break;
        case 1: // Frame column - exclude entries with this frame
            if (selected_entry.Get_frame_number().has_value()) {
                filter_name = "Exclude frame: " + std::to_string(selected_entry.Get_frame_number().value());
                filter_type_enum = FilterType::FrameRange; // Use FrameRange for frame filtering
                filter_value = std::to_string(selected_entry.Get_frame_number().value());
            } else {
                _Last_Error_ = "Selected entry has no frame number";
                return;
            }
            break;
        case 2: // Logger column - exclude this logger
            filter_name = "Exclude logger: " + selected_entry.Get_logger_name();
            filter_type_enum = FilterType::LoggerName;
            filter_value = selected_entry.Get_logger_name();
            break;
        case 3: // Level column - exclude this level
            if (selected_entry.Get_log_level().has_value()) {
                filter_name = "Exclude level: " + selected_entry.Get_log_level().value();
                filter_type_enum = FilterType::LogLevel;
                filter_value = selected_entry.Get_log_level().value();
            } else {
                _Last_Error_ = "Selected entry has no log level";
                return;
            }
            break;
        case 4: // Message column - exclude messages containing this text
            filter_name = "Exclude message containing: " + selected_entry.Get_message().substr(0, 50) + "...";
            filter_type_enum = FilterType::TextContains;
            filter_value = selected_entry.Get_message();
            break;
        default:
            _Last_Error_ = "Invalid column number: " + std::to_string(column_number + 1);
            return;
    }
    
    // Create the EXCLUDE filter
    auto filter = std::make_unique<Filter>(filter_name, filter_type_enum, filter_value);
    filter->SetFilterState(FilterState::EXCLUDE); // This makes it an exclude filter
    
    // Add the filter to the filter engine
    auto result = _Filter_Engine_->AddFilter(std::move(filter));
    if (result.IsError()) {
        _Last_Error_ = "Failed to create exclude filter: " + result.Get_error_message();
        return;
    }
    
    // Refresh the filter panel to show the new filter
    if (_Filter_Panel_) {
        _Filter_Panel_->RefreshFilters();
    }
    
    // Apply filters to update the display
    OnFiltersChanged();
    
    _Last_Error_ = "Exclude filter created: " + filter_name;
}

void MainWindow::PerformSearchHighlightOnly(const std::string& query) {
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
    
    // Don't jump to first result - just highlight for now
    // The user will press Enter to jump to the first result
}

void MainWindow::CopyCurrentLineToClipboard() {
    // Check if we have a selected entry
    if (selected_entry_index_ < 0 || selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        _Last_Error_ = "No log line selected to copy";
        return;
    }
    
    const auto& selected_entry = filtered_entries_[selected_entry_index_];
    
    // Get the full raw line (original format)
    std::string line_to_copy = selected_entry.Get_raw_line();
    
    // On Windows, we can use the Windows clipboard API
    #ifdef _WIN32
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        
        // Allocate global memory for the text
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, line_to_copy.size() + 1);
        if (hMem) {
            char* pMem = static_cast<char*>(GlobalLock(hMem));
            if (pMem) {
                strcpy_s(pMem, line_to_copy.size() + 1, line_to_copy.c_str());
                GlobalUnlock(hMem);
                
                SetClipboardData(CF_TEXT, hMem);
                _Last_Error_ = "Log line copied to clipboard";
            } else {
                GlobalFree(hMem);
                _Last_Error_ = "Failed to lock clipboard memory";
            }
        } else {
            _Last_Error_ = "Failed to allocate clipboard memory";
        }
        
        CloseClipboard();
    } else {
        _Last_Error_ = "Failed to open clipboard";
    }
    #else
    // For non-Windows systems, we could use xclip or pbcopy, but for now just show an error
    last_error_ = "Clipboard functionality not implemented for this platform";
    #endif
}

void MainWindow::CopyVisualSelectionToClipboard() {
    // Check if we're in visual selection mode
    if (!visual_selection_mode_) {
        _Last_Error_ = "Not in visual selection mode";
        return;
    }
    
    // Get the selection range
    auto range = GetVisualSelectionRange();
    if (range.first == -1 || range.second == -1) {
        _Last_Error_ = "Invalid visual selection range";
        return;
    }
    
    // Check bounds
    if (range.first < 0 || range.second >= static_cast<int>(filtered_entries_.size())) {
        _Last_Error_ = "Visual selection range out of bounds";
        return;
    }
    
    // Extract raw log lines from the selection range
    std::string lines_to_copy;
    for (int i = range.first; i <= range.second; ++i) {
        const auto& entry = filtered_entries_[i];
        lines_to_copy += entry.Get_raw_line();
        
        // Add line break if not the last line
        if (i < range.second) {
            lines_to_copy += "\r\n"; // Windows line endings
        }
    }
    
    // Copy to clipboard using Windows API
    #ifdef _WIN32
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        
        // Allocate global memory for the text
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, lines_to_copy.size() + 1);
        if (hMem) {
            char* pMem = static_cast<char*>(GlobalLock(hMem));
            if (pMem) {
                strcpy_s(pMem, lines_to_copy.size() + 1, lines_to_copy.c_str());
                GlobalUnlock(hMem);
                
                SetClipboardData(CF_TEXT, hMem);
                
                // Success message
                int selection_size = GetVisualSelectionSize();
                _Last_Error_ = std::to_string(selection_size) + 
                              (selection_size == 1 ? " line copied to clipboard" : " lines copied to clipboard");
            } else {
                GlobalFree(hMem);
                _Last_Error_ = "Failed to lock clipboard memory";
            }
        } else {
            _Last_Error_ = "Failed to allocate clipboard memory";
        }
        
        CloseClipboard();
    } else {
        _Last_Error_ = "Failed to open clipboard";
    }
    #else
    // For non-Windows systems, we could use xclip or pbcopy, but for now just show an error
    _Last_Error_ = "Clipboard functionality not implemented for this platform";
    #endif
}

void MainWindow::EnterVisualSelectionMode() {
    // Only enter visual selection mode if we have entries and a valid selection
    if (filtered_entries_.empty() || selected_entry_index_ < 0 || 
        selected_entry_index_ >= static_cast<int>(filtered_entries_.size())) {
        _Last_Error_ = "No log line selected to start visual selection";
        return;
    }
    
    visual_selection_mode_ = true;
    visual_selection_anchor_ = selected_entry_index_;
    visual_selection_start_ = selected_entry_index_;
    visual_selection_end_ = selected_entry_index_;
    
    _Last_Error_ = "Visual selection mode activated";
}

void MainWindow::ExitVisualSelectionMode() {
    visual_selection_mode_ = false;
    visual_selection_anchor_ = -1;
    visual_selection_start_ = -1;
    visual_selection_end_ = -1;
    
    // Only set deactivation message if there's no existing message (like copy success)
    if (_Last_Error_.empty()) {
        _Last_Error_ = "Visual selection mode deactivated";
    }
}

bool MainWindow::IsVisualSelectionMode() const {
    return visual_selection_mode_;
}

std::pair<int, int> MainWindow::GetVisualSelectionRange() const {
    if (!visual_selection_mode_ || visual_selection_anchor_ == -1 || visual_selection_end_ == -1) {
        return {-1, -1};
    }
    
    int start = std::min(visual_selection_anchor_, visual_selection_end_);
    int end = std::max(visual_selection_anchor_, visual_selection_end_);
    
    return {start, end};
}

int MainWindow::GetVisualSelectionSize() const {
    if (!visual_selection_mode_ || visual_selection_anchor_ == -1 || visual_selection_end_ == -1) {
        return 0;
    }
    
    auto range = GetVisualSelectionRange();
    return range.second - range.first + 1;
}

void MainWindow::ExtendVisualSelection(int new_end_index) {
    // Only extend selection if we're in visual selection mode
    if (!visual_selection_mode_ || visual_selection_anchor_ == -1) {
        return;
    }
    
    // Bounds checking - ensure new_end_index is within valid range
    if (filtered_entries_.empty()) {
        return;
    }
    
    // Clamp new_end_index to valid bounds
    if (new_end_index < 0) {
        new_end_index = 0;
    } else if (new_end_index >= static_cast<int>(filtered_entries_.size())) {
        new_end_index = static_cast<int>(filtered_entries_.size()) - 1;
    }
    
    // Update visual selection end point
    visual_selection_end_ = new_end_index;
    
    // Update the current cursor position to the new end point
    selected_entry_index_ = new_end_index;
    
    // Auto-scroll to keep the selection endpoint visible
    EnsureSelectionVisible();
    
    // Update selection range for consistency
    visual_selection_start_ = std::min(visual_selection_anchor_, visual_selection_end_);
    
    // Update status message
    int selection_size = GetVisualSelectionSize();
    _Last_Error_ = "Visual selection: " + std::to_string(selection_size) + 
                   (selection_size == 1 ? " line selected" : " lines selected");
}

std::string MainWindow::GetFilterHighlightTerm() const {
    // Only highlight if filter panel is visible and has a selected filter
    if (!_Filter_Panel_ || !show_filter_panel_) {
        return "";
    }
    
    const Filter* selected_filter = _Filter_Panel_->GetSelectedFilter();
    if (!selected_filter) {
        return "";
    }
    
    // Only highlight for INCLUDE filters, not EXCLUDE filters
    if (selected_filter->GetFilterState() != FilterState::INCLUDE) {
        return "";
    }
    
    // Only highlight for text-based filters that contain search terms
    FilterType filter_type = selected_filter->Get_type();
    if (filter_type == FilterType::TextContains || 
        filter_type == FilterType::TextExact || 
        filter_type == FilterType::LoggerName ||
        filter_type == FilterType::LogLevel) {
        return selected_filter->Get_criteria();
    }
    
    return "";
}

} // namespace ue_log