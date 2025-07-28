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
        // Enhanced empty state with better visual presentation
        std::vector<Element> content;
        content.push_back(RenderHeader());
        content.push_back(separator() | color(Color::Blue));
        content.push_back(text("") | size(HEIGHT, EQUAL, 1));  // Spacing
        
        if (!last_scan_error_.empty()) {
            // Show the specific error with better formatting
            auto error_icon = text("⚠️") | color(Color::Red);
            auto error_text = text("Error: " + last_scan_error_) | color(Color::Red) | bold;
            content.push_back(hbox({error_icon, text(" "), error_text}) | center);
            content.push_back(text("") | size(HEIGHT, EQUAL, 2));  // More spacing
            
            // Provide helpful suggestions with better visual hierarchy
            auto suggestions_title = text("💡 Suggestions:") | bold | color(Color::Yellow);
            content.push_back(suggestions_title | center);
            content.push_back(text("") | size(HEIGHT, EQUAL, 1));
            
            if (last_scan_error_.find("Permission denied") != std::string::npos) {
                content.push_back(text("• Check directory permissions") | center | dim);
                content.push_back(text("• Try running with appropriate privileges") | center | dim);
                content.push_back(text("• Verify you have read access to the directory") | center | dim);
            } else if (last_scan_error_.find("does not exist") != std::string::npos) {
                content.push_back(text("• Verify the directory path is correct") | center | dim);
                content.push_back(text("• Check if the directory was moved or deleted") | center | dim);
                content.push_back(text("• Try specifying a different directory") | center | dim);
            } else if (last_scan_error_.find("No .log files found") != std::string::npos) {
                content.push_back(text("• Check if log files have .log extension") | center | dim);
                content.push_back(text("• Verify this is the correct log directory") | center | dim);
                content.push_back(text("• Try looking in subdirectories") | center | dim);
            } else if (last_scan_error_.find("empty") != std::string::npos) {
                content.push_back(text("• Directory contains no files") | center | dim);
                content.push_back(text("• Try a different directory") | center | dim);
                content.push_back(text("• Check if files were moved elsewhere") | center | dim);
            }
        } else {
            // Generic message with better visual presentation
            auto empty_icon = text("📂") | color(Color::Yellow);
            auto empty_text = text("No log files found in directory") | color(Color::Yellow);
            content.push_back(hbox({empty_icon, text(" "), empty_text}) | center);
            content.push_back(text("") | size(HEIGHT, EQUAL, 1));
            content.push_back(text(directory_path_) | bold | color(Color::Blue) | center);
        }
        
        content.push_back(text("") | flex);
        content.push_back(separator() | color(Color::Blue));
        content.push_back(RenderInstructions());
        
        return vbox(content) | border;
    }
    
    // Enhanced layout for file list with better visual hierarchy
    return vbox({
        RenderHeader(),
        separator() | color(Color::Blue),
        RenderFileList() | flex,
        separator() | color(Color::Blue),
        RenderStatusLine(),
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
    int old_file_count = static_cast<int>(files_.size());
    
    ScanDirectory();
    SortFilesByModificationTime();
    
    // Ensure selected index is still valid
    if (selected_index_ >= static_cast<int>(files_.size())) {
        selected_index_ = std::max(0, static_cast<int>(files_.size()) - 1);
    }
    
    // Provide feedback about the refresh operation
    int new_file_count = static_cast<int>(files_.size());
    if (new_file_count != old_file_count && status_callback_) {
        if (new_file_count > old_file_count) {
            int added = new_file_count - old_file_count;
            status_callback_("Refresh: " + std::to_string(added) + " new file" + 
                           (added == 1 ? "" : "s") + " found");
        } else if (new_file_count < old_file_count) {
            int removed = old_file_count - new_file_count;
            status_callback_("Refresh: " + std::to_string(removed) + " file" + 
                           (removed == 1 ? "" : "s") + " no longer available");
        } else if (new_file_count > 0) {
            status_callback_("File list refreshed (" + std::to_string(new_file_count) + " files)");
        }
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
    last_scan_error_.clear();
    
    try {
        // Check if directory exists
        if (!std::filesystem::exists(directory_path_)) {
            last_scan_error_ = "Directory does not exist: " + directory_path_;
            if (error_callback_) {
                error_callback_(last_scan_error_);
            }
            return;
        }
        
        // Check if path is actually a directory
        if (!std::filesystem::is_directory(directory_path_)) {
            last_scan_error_ = "Path is not a directory: " + directory_path_;
            if (error_callback_) {
                error_callback_(last_scan_error_);
            }
            return;
        }
        
        // Try to iterate through directory
        std::error_code ec;
        auto dir_iter = std::filesystem::directory_iterator(directory_path_, ec);
        if (ec) {
            if (ec == std::errc::permission_denied) {
                last_scan_error_ = "Permission denied accessing directory: " + directory_path_;
            } else {
                last_scan_error_ = "Cannot access directory: " + directory_path_ + " (" + ec.message() + ")";
            }
            if (error_callback_) {
                error_callback_(last_scan_error_);
            }
            return;
        }
        
        // Scan for log files
        int total_files = 0;
        int log_files_found = 0;
        
        for (const auto& entry : dir_iter) {
            total_files++;
            
            std::error_code entry_ec;
            if (entry.is_regular_file(entry_ec) && !entry_ec) {
                if (entry.path().extension() == ".log") {
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.full_path = entry.path().string();
                    
                    // Get modification time with error handling
                    auto mod_time = entry.last_write_time(entry_ec);
                    if (entry_ec) {
                        // Use current time as fallback
                        file_info.modified_time = std::filesystem::file_time_type::clock::now();
                    } else {
                        file_info.modified_time = mod_time;
                    }
                    
                    // Get file size with error handling
                    file_info.size = std::filesystem::file_size(entry.path(), entry_ec);
                    if (entry_ec) {
                        file_info.size = 0;
                    }
                    
                    files_.push_back(file_info);
                    log_files_found++;
                }
            }
        }
        
        // Provide user feedback about scan results
        if (log_files_found == 0) {
            if (total_files == 0) {
                last_scan_error_ = "Directory is empty: " + directory_path_;
            } else {
                last_scan_error_ = "No .log files found in directory: " + directory_path_ + 
                                 " (found " + std::to_string(total_files) + " other files)";
            }
            if (error_callback_) {
                error_callback_(last_scan_error_);
            }
        } else {
            // Success - provide positive feedback
            std::string success_message = "Found " + std::to_string(log_files_found) + " log file";
            if (log_files_found != 1) success_message += "s";
            success_message += " in " + directory_path_;
            
            if (status_callback_) {
                status_callback_(success_message);
            }
        }
        
    } catch (const std::filesystem::filesystem_error& e) {
        last_scan_error_ = "Filesystem error accessing " + directory_path_ + ": " + e.what();
        if (error_callback_) {
            error_callback_(last_scan_error_);
        }
        files_.clear();
    } catch (const std::exception& e) {
        last_scan_error_ = "Unexpected error scanning directory " + directory_path_ + ": " + e.what();
        if (error_callback_) {
            error_callback_(last_scan_error_);
        }
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
    
    // Add column headers for better organization
    auto header = hbox({
        text("  "),  // Space for selection indicator
        text(" "),
        text("Filename") | bold | color(Color::Cyan) | flex,
        text("  "),
        text("Size") | bold | color(Color::Cyan) | size(WIDTH, EQUAL, 8) | align_right,
        text("  "),
        text("Modified") | bold | color(Color::Cyan) | size(WIDTH, EQUAL, 16),
        text("  ")
    });
    
    // Add separator line under headers
    auto separator_line = text(std::string(60, '-')) | color(Color::Blue) | dim;
    
    file_elements.push_back(header);
    file_elements.push_back(separator_line);
    file_elements.push_back(text("") | size(HEIGHT, EQUAL, 1));  // Spacing
    
    // Add file entries
    for (size_t i = 0; i < files_.size(); ++i) {
        bool is_selected = (static_cast<int>(i) == selected_index_);
        file_elements.push_back(RenderFileEntry(files_[i], is_selected));
    }
    
    // Add some spacing at the bottom if there are files
    if (!files_.empty()) {
        file_elements.push_back(text("") | size(HEIGHT, EQUAL, 1));
    }
    
    return vbox(file_elements);
}

ftxui::Element FileBrowser::RenderFileEntry(const FileInfo& file, bool selected) const {
    using namespace ftxui;
    
    // Create filename element with proper truncation
    auto filename_element = text(file.filename) | size(WIDTH, LESS_THAN, 40);
    
    // Create size element with right alignment
    auto size_element = text(file.GetFormattedSize()) | size(WIDTH, EQUAL, 8) | align_right;
    
    // Create time element with consistent formatting
    auto time_element = text(file.GetFormattedModificationTime()) | size(WIDTH, EQUAL, 16);
    
    if (selected) {
        // Enhanced selection highlighting with arrow indicator
        auto selection_indicator = text("►") | color(Color::Yellow) | bold;
        auto highlighted_entry = hbox({
            selection_indicator,
            text(" "),
            filename_element | flex,
            text("  "),
            size_element | color(Color::White),
            text("  "),
            time_element | color(Color::White),
            text("  ")
        });
        return highlighted_entry | bgcolor(Color::Blue) | color(Color::White) | bold;
    } else {
        // Non-selected entries with subtle styling
        auto normal_entry = hbox({
            text(" "),  // Space for selection indicator
            text(" "),
            filename_element | color(Color::Default) | flex,
            text("  "),
            size_element | color(Color::Blue) | dim,
            text("  "),
            time_element | color(Color::Green) | dim,
            text("  ")
        });
        return normal_entry;
    }
}

ftxui::Element FileBrowser::RenderHeader() const {
    using namespace ftxui;
    
    // Create a more visually appealing header with better formatting
    auto title = text("📁 File Browser") | bold | color(Color::Cyan);
    auto path = text(directory_path_) | color(Color::Yellow);
    auto file_count = text("(" + std::to_string(files_.size()) + " files)") | dim | color(Color::White);
    
    return vbox({
        hbox({
            title,
            text(" "),
            path | flex,
            text(" "),
            file_count
        }),
        text("") | size(HEIGHT, EQUAL, 1)  // Add some spacing
    });
}

ftxui::Element FileBrowser::RenderStatusLine() const {
    using namespace ftxui;
    
    if (files_.empty()) {
        return text("") | size(HEIGHT, EQUAL, 0);  // No status line when empty
    }
    
    // Show current selection info
    auto current_file = files_[selected_index_];
    auto selection_info = hbox({
        text("Selected: ") | dim,
        text(current_file.filename) | bold | color(Color::Yellow),
        text(" (") | dim,
        text(current_file.GetFormattedSize()) | color(Color::Blue),
        text(", ") | dim,
        text(current_file.GetFormattedModificationTime()) | color(Color::Green),
        text(")") | dim
    });
    
    // Show position in list
    auto position_info = hbox({
        text("[") | dim,
        text(std::to_string(selected_index_ + 1)) | bold,
        text("/") | dim,
        text(std::to_string(files_.size())) | bold,
        text("]") | dim
    });
    
    return hbox({
        selection_info | flex,
        text("  "),
        position_info
    });
}

ftxui::Element FileBrowser::RenderInstructions() const {
    using namespace ftxui;
    
    // Create a more comprehensive and visually appealing instructions section
    auto nav_title = text("Navigation:") | bold | color(Color::Cyan);
    
    // Group related instructions with consistent styling
    auto basic_nav = hbox({
        text("j/k") | bold | color(Color::Yellow),
        text(" up/down") | dim
    });
    
    auto page_nav = hbox({
        text("Ctrl+u/d") | bold | color(Color::Yellow),
        text(" half page") | dim
    });
    
    auto arrow_nav = hbox({
        text("↑/↓") | bold | color(Color::Yellow),
        text(" arrows") | dim
    });
    
    auto select_action = hbox({
        text("Enter") | bold | color(Color::Green),
        text(" select file") | dim
    });
    
    // Create a well-formatted instruction layout
    return hbox({
        nav_title,
        text("  "),
        basic_nav,
        text("  •  "),
        page_nav,
        text("  •  "),
        arrow_nav,
        text("  •  "),
        select_action
    }) | center;
}

// FileInfo helper methods implementation
std::string FileBrowser::FileInfo::GetFormattedSize() const {
    if (size == 0) {
        return "0 B";
    }
    
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size_double = static_cast<double>(size);
    
    while (size_double >= 1024.0 && unit_index < 4) {
        size_double /= 1024.0;
        unit_index++;
    }
    
    std::ostringstream oss;
    if (unit_index == 0) {
        // For bytes, show whole numbers
        oss << static_cast<int>(size_double) << " " << units[unit_index];
    } else if (size_double < 10.0) {
        // For small values, show one decimal place
        oss << std::fixed << std::setprecision(1) << size_double << " " << units[unit_index];
    } else {
        // For larger values, show whole numbers
        oss << std::fixed << std::setprecision(0) << size_double << " " << units[unit_index];
    }
    
    return oss.str();
}

std::string FileBrowser::FileInfo::GetFormattedModificationTime() const {
    try {
        // Convert file_time_type to system_clock time_point
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            modified_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        
        auto time_t = std::chrono::system_clock::to_time_t(sctp);
        auto now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        
        // Calculate time difference for more user-friendly display
        auto diff_seconds = std::difftime(now_time_t, time_t);
        
        std::ostringstream oss;
        
        if (diff_seconds < 60) {
            // Less than a minute ago
            oss << "just now";
        } else if (diff_seconds < 3600) {
            // Less than an hour ago
            int minutes = static_cast<int>(diff_seconds / 60);
            oss << minutes << "m ago";
        } else if (diff_seconds < 86400) {
            // Less than a day ago
            int hours = static_cast<int>(diff_seconds / 3600);
            oss << hours << "h ago";
        } else if (diff_seconds < 604800) {
            // Less than a week ago
            int days = static_cast<int>(diff_seconds / 86400);
            oss << days << "d ago";
        } else {
            // More than a week ago, show actual date
#ifdef _WIN32
            struct tm tm_buf;
            localtime_s(&tm_buf, &time_t);
            oss << std::put_time(&tm_buf, "%m/%d %H:%M");
#else
            oss << std::put_time(std::localtime(&time_t), "%m/%d %H:%M");
#endif
        }
        
        return oss.str();
    } catch (...) {
        // Fallback to basic format if time conversion fails
        return "unknown";
    }
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
        std::string error_msg;
        if (!last_scan_error_.empty()) {
            error_msg = "Cannot load file: " + last_scan_error_;
        } else {
            error_msg = "No files available to load";
        }
        if (error_callback_) {
            error_callback_(error_msg);
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
    
    // Provide feedback that we're attempting to load the file
    if (status_callback_) {
        status_callback_("Loading file: " + std::filesystem::path(selected_file).filename().string());
    }
    
    // Check if file still exists and is readable
    try {
        std::error_code ec;
        
        // Check if file exists
        if (!std::filesystem::exists(selected_file, ec)) {
            std::string error_msg = "Selected file no longer exists: " + selected_file;
            if (ec) {
                error_msg += " (" + ec.message() + ")";
            }
            if (error_callback_) {
                error_callback_(error_msg);
            }
            // Refresh the file list to remove non-existent files
            RefreshFileList();
            return;
        }
        
        // Check if it's a regular file
        if (!std::filesystem::is_regular_file(selected_file, ec)) {
            std::string error_msg = "Selected path is not a regular file: " + selected_file;
            if (ec) {
                error_msg += " (" + ec.message() + ")";
            }
            if (error_callback_) {
                error_callback_(error_msg);
            }
            return;
        }
        
        // Check file size to provide better feedback
        auto file_size = std::filesystem::file_size(selected_file, ec);
        if (!ec && file_size == 0) {
            if (error_callback_) {
                error_callback_("Warning: Selected file is empty: " + selected_file);
            }
            // Still allow loading empty files, but warn the user
        }
        
        // Try to open the file to check if it's readable
        std::ifstream test_file(selected_file);
        if (!test_file.is_open()) {
            std::string error_msg = "Cannot open file: " + selected_file;
            
            // Try to determine the specific reason for failure
            std::filesystem::perms perms = std::filesystem::status(selected_file, ec).permissions();
            if (!ec) {
                if ((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none &&
                    (perms & std::filesystem::perms::group_read) == std::filesystem::perms::none &&
                    (perms & std::filesystem::perms::others_read) == std::filesystem::perms::none) {
                    error_msg += " (no read permissions)";
                } else {
                    error_msg += " (permission denied or file in use)";
                }
            } else {
                error_msg += " (access denied)";
            }
            
            if (error_callback_) {
                error_callback_(error_msg);
            }
            return;
        }
        test_file.close();
        
        // File is valid, provide success feedback and call the selection callback
        if (status_callback_) {
            std::string size_str = "unknown size";
            if (!ec && file_size > 0) {
                // Format file size nicely
                if (file_size < 1024) {
                    size_str = std::to_string(file_size) + " bytes";
                } else if (file_size < 1024 * 1024) {
                    size_str = std::to_string(file_size / 1024) + " KB";
                } else {
                    size_str = std::to_string(file_size / (1024 * 1024)) + " MB";
                }
            }
            status_callback_("Successfully loaded: " + std::filesystem::path(selected_file).filename().string() + 
                           " (" + size_str + ")");
        }
        
        if (file_selection_callback_) {
            file_selection_callback_(selected_file);
        }
        
    } catch (const std::filesystem::filesystem_error& e) {
        std::string error_msg = "Filesystem error accessing " + selected_file + ": " + e.what();
        if (error_callback_) {
            error_callback_(error_msg);
        }
    } catch (const std::exception& e) {
        std::string error_msg = "Unexpected error loading file " + selected_file + ": " + e.what();
        if (error_callback_) {
            error_callback_(error_msg);
        }
    }
}

} // namespace ue_log