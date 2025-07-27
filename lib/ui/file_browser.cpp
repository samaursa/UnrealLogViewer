#include "file_browser.h"
#include <filesystem>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <fstream>
#include <ftxui/component/event.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

namespace ue_log {

FileBrowser::FileBrowser(const std::string& directory_path) 
    : directory_path_(directory_path) {
}

void FileBrowser::Initialize() {
    ScanDirectory();
    SortFilesByModificationTime();
    
    // Create FTXUI component with event handling
    component_ = ftxui::CatchEvent(
        ftxui::Renderer([this] { return Render(); }),
        [this](ftxui::Event event) { return OnEvent(event); }
    );
}

ftxui::Element FileBrowser::Render() const {
    using namespace ftxui;
    
    if (files_.empty()) {
        return vbox({
            RenderHeader(),
            separator(),
            text("No log files found in directory: " + directory_path_) | center,
            text("") | flex,
            RenderInstructions()
        }) | border;
    }
    
    return vbox({
        RenderHeader(),
        separator(),
        RenderFileList() | flex,
        separator(),
        RenderInstructions()
    }) | border;
}

ftxui::Component FileBrowser::CreateFTXUIComponent() {
    return component_;
}

void FileBrowser::SelectNext() {
    if (!files_.empty() && selected_index_ < static_cast<int>(files_.size()) - 1) {
        selected_index_++;
        UpdateScrollOffset();
    }
}

void FileBrowser::SelectPrevious() {
    if (selected_index_ > 0) {
        selected_index_--;
        UpdateScrollOffset();
    }
}

void FileBrowser::PageDown() {
    if (files_.empty()) return;
    
    const int page_size = GetVisibleFileCount();
    selected_index_ = std::min(selected_index_ + page_size, static_cast<int>(files_.size()) - 1);
    UpdateScrollOffset();
}

void FileBrowser::PageUp() {
    if (files_.empty()) return;
    
    const int page_size = GetVisibleFileCount();
    selected_index_ = std::max(selected_index_ - page_size, 0);
    UpdateScrollOffset();
}

void FileBrowser::HalfPageDown() {
    if (files_.empty()) return;
    
    const int half_page_size = std::max(1, GetVisibleFileCount() / 2);
    selected_index_ = std::min(selected_index_ + half_page_size, static_cast<int>(files_.size()) - 1);
    UpdateScrollOffset();
}

void FileBrowser::HalfPageUp() {
    if (files_.empty()) return;
    
    const int half_page_size = std::max(1, GetVisibleFileCount() / 2);
    selected_index_ = std::max(selected_index_ - half_page_size, 0);
    UpdateScrollOffset();
}

std::string FileBrowser::GetSelectedFilePath() const {
    if (files_.empty() || selected_index_ < 0 || selected_index_ >= static_cast<int>(files_.size())) {
        return "";
    }
    return files_[selected_index_].full_path;
}

bool FileBrowser::HasFiles() const {
    return !files_.empty();
}

void FileBrowser::RefreshFileList() {
    ScanDirectory();
    SortFilesByModificationTime();
    
    // Ensure selected index is still valid
    if (selected_index_ >= static_cast<int>(files_.size())) {
        selected_index_ = std::max(0, static_cast<int>(files_.size()) - 1);
    }
}

bool FileBrowser::OnEvent(ftxui::Event event) {
    if (!is_focused_) {
        return false;
    }
    
    // Handle vim-style navigation keys
    if (event == ftxui::Event::Character('j')) {
        SelectNext();
        return true;
    }
    if (event == ftxui::Event::Character('k')) {
        SelectPrevious();
        return true;
    }
    
    // Handle Ctrl+D and Ctrl+U for half-page scrolling
    if (event == ftxui::Event::Character(static_cast<char>(4))) { // Ctrl+D (ASCII 4)
        HalfPageDown();
        return true;
    }
    if (event == ftxui::Event::Character(static_cast<char>(21))) { // Ctrl+U (ASCII 21)
        HalfPageUp();
        return true;
    }
    
    // Handle standard page navigation
    if (event == ftxui::Event::PageDown) {
        PageDown();
        return true;
    }
    if (event == ftxui::Event::PageUp) {
        PageUp();
        return true;
    }
    
    // Handle arrow keys as well for accessibility
    if (event == ftxui::Event::ArrowDown) {
        SelectNext();
        return true;
    }
    if (event == ftxui::Event::ArrowUp) {
        SelectPrevious();
        return true;
    }
    
    // Handle Enter key for file selection
    if (event == ftxui::Event::Return) {
        LoadSelectedFile();
        return true;
    }
    
    return false;
}

void FileBrowser::ScanDirectory() {
    files_.clear();
    
    try {
        if (!std::filesystem::exists(directory_path_) || !std::filesystem::is_directory(directory_path_)) {
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(directory_path_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                FileInfo file_info;
                file_info.filename = entry.path().filename().string();
                file_info.full_path = entry.path().string();
                file_info.modified_time = entry.last_write_time();
                
                std::error_code ec;
                file_info.size = std::filesystem::file_size(entry.path(), ec);
                if (ec) {
                    file_info.size = 0;
                }
                
                files_.push_back(file_info);
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Handle filesystem errors gracefully
        files_.clear();
    }
}

void FileBrowser::SortFilesByModificationTime() {
    std::sort(files_.begin(), files_.end(), 
        [](const FileInfo& a, const FileInfo& b) {
            return a.modified_time > b.modified_time; // Most recent first
        });
}

ftxui::Element FileBrowser::RenderFileList() const {
    using namespace ftxui;
    
    std::vector<Element> file_elements;
    
    for (size_t i = 0; i < files_.size(); ++i) {
        bool is_selected = (static_cast<int>(i) == selected_index_);
        file_elements.push_back(RenderFileEntry(files_[i], is_selected));
    }
    
    return vbox(file_elements);
}

ftxui::Element FileBrowser::RenderFileEntry(const FileInfo& file, bool selected) const {
    using namespace ftxui;
    
    auto filename_element = text(file.filename);
    auto size_element = text(file.GetFormattedSize());
    auto time_element = text(file.GetFormattedModificationTime());
    
    auto entry = hbox({
        filename_element | size(WIDTH, GREATER_THAN, 30),
        separator(),
        size_element | size(WIDTH, EQUAL, 10),
        separator(),
        time_element | size(WIDTH, EQUAL, 20)
    });
    
    if (selected) {
        return entry | bgcolor(Color::Blue) | color(Color::White);
    } else {
        return entry;
    }
}

ftxui::Element FileBrowser::RenderHeader() const {
    using namespace ftxui;
    
    return hbox({
        text("File Browser: " + directory_path_) | bold,
        text(" (" + std::to_string(files_.size()) + " files)") | dim
    });
}

ftxui::Element FileBrowser::RenderInstructions() const {
    using namespace ftxui;
    
    return hbox({
        text("Navigation: ") | dim,
        text("j/k") | bold,
        text(" (up/down), ") | dim,
        text("Ctrl+u/d") | bold,
        text(" (half page), ") | dim,
        text("Enter") | bold,
        text(" (select)") | dim
    });
}

// FileInfo helper methods implementation
std::string FileBrowser::FileInfo::GetFormattedSize() const {
    if (size == 0) {
        return "0 B";
    }
    
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit_index = 0;
    double size_double = static_cast<double>(size);
    
    while (size_double >= 1024.0 && unit_index < 3) {
        size_double /= 1024.0;
        unit_index++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size_double << " " << units[unit_index];
    return oss.str();
}

std::string FileBrowser::FileInfo::GetFormattedModificationTime() const {
    // Convert file_time_type to system_clock time_point
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        modified_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    
    auto time_t = std::chrono::system_clock::to_time_t(sctp);
    
    std::ostringstream oss;
#ifdef _WIN32
    struct tm tm_buf;
    localtime_s(&tm_buf, &time_t);
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M");
#else
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M");
#endif
    return oss.str();
}

int FileBrowser::GetVisibleFileCount() const {
    // Estimate visible file count based on typical terminal height
    // This is a reasonable approximation for half-page scrolling
    return 10; // Can be made configurable later
}

void FileBrowser::UpdateScrollOffset() {
    // This method can be used for future scrolling improvements
    // For now, FTXUI handles scrolling automatically
}

void FileBrowser::LoadSelectedFile() {
    if (!HasFiles()) {
        if (error_callback_) {
            error_callback_("No files available to load");
        }
        return;
    }
    
    std::string selected_file = GetSelectedFilePath();
    if (selected_file.empty()) {
        if (error_callback_) {
            error_callback_("No file selected");
        }
        return;
    }
    
    // Check if file still exists and is readable
    try {
        if (!std::filesystem::exists(selected_file)) {
            if (error_callback_) {
                error_callback_("Selected file no longer exists: " + selected_file);
            }
            // Refresh the file list to remove non-existent files
            RefreshFileList();
            return;
        }
        
        if (!std::filesystem::is_regular_file(selected_file)) {
            if (error_callback_) {
                error_callback_("Selected path is not a regular file: " + selected_file);
            }
            return;
        }
        
        // Try to open the file to check if it's readable
        std::ifstream test_file(selected_file);
        if (!test_file.is_open()) {
            if (error_callback_) {
                error_callback_("Cannot open file (permission denied?): " + selected_file);
            }
            return;
        }
        test_file.close();
        
        // File is valid, call the selection callback
        if (file_selection_callback_) {
            file_selection_callback_(selected_file);
        }
        
    } catch (const std::filesystem::filesystem_error& e) {
        if (error_callback_) {
            error_callback_("Filesystem error: " + std::string(e.what()));
        }
    } catch (const std::exception& e) {
        if (error_callback_) {
            error_callback_("Error loading file: " + std::string(e.what()));
        }
    }
}

} // namespace ue_log