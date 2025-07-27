#pragma once

#include "component.h"
#include <filesystem>
#include <vector>
#include <string>
#include <functional>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace ue_log {

/**
 * File browser component for navigating and selecting log files.
 * Provides vim-style navigation and file metadata display.
 */
class FileBrowser : public Component {
public:
    /**
     * Constructor.
     * @param directory_path Path to the directory to browse
     */
    explicit FileBrowser(const std::string& directory_path);
    
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
    std::string GetTitle() const override { return "File Browser"; }
    
    /**
     * Check if the component is focused.
     * @return True if the component is focused, false otherwise.
     */
    bool IsFocused() const override { return is_focused_; }
    
    /**
     * Set the focus state of the component.
     * @param focused True to focus the component, false to unfocus.
     */
    void SetFocus(bool focused) override { is_focused_ = focused; }
    
    // Navigation methods
    void SelectNext();
    void SelectPrevious();
    void PageDown();
    void PageUp();
    void HalfPageDown();
    void HalfPageUp();
    
    // File operations
    std::string GetSelectedFilePath() const;
    bool HasFiles() const;
    void RefreshFileList();
    
    // Event handling
    bool OnEvent(ftxui::Event event);
    
    // File selection callback
    using FileSelectionCallback = std::function<void(const std::string&)>;
    void SetFileSelectionCallback(FileSelectionCallback callback) {
        file_selection_callback_ = std::move(callback);
    }
    
    // Error handling callback
    using ErrorCallback = std::function<void(const std::string&)>;
    void SetErrorCallback(ErrorCallback callback) {
        error_callback_ = std::move(callback);
    }
    
    // Load selected file with error handling
    void LoadSelectedFile();

private:
    struct FileInfo {
        std::string filename;
        std::string full_path;
        std::filesystem::file_time_type modified_time;
        std::uintmax_t size;
        
        // Helper methods
        std::string GetFormattedSize() const;
        std::string GetFormattedModificationTime() const;
    };
    
    std::string directory_path_;
    std::vector<FileInfo> files_;
    int selected_index_ = 0;
    int scroll_offset_ = 0;
    bool is_focused_ = false;
    
    // FTXUI component
    ftxui::Component component_;
    
    // Callback for file selection
    FileSelectionCallback file_selection_callback_;
    
    // Callback for error handling
    ErrorCallback error_callback_;
    
    void ScanDirectory();
    void SortFilesByModificationTime();
    ftxui::Element RenderFileList() const;
    ftxui::Element RenderFileEntry(const FileInfo& file, bool selected) const;
    ftxui::Element RenderHeader() const;
    ftxui::Element RenderInstructions() const;
    
    // Helper methods for navigation
    int GetVisibleFileCount() const;
    void UpdateScrollOffset();
};

} // namespace ue_log