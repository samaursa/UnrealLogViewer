#pragma once

#include "../../macros.h"
#include "../../lib/filter_engine/filter_engine.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <memory>
#include <string>

namespace ue_log {
    
    class FilterPanel {
        CK_GENERATED_BODY(FilterPanel);
        
    private:
        FilterEngine* filter_engine;
        int selected_filter_index;
        bool is_editing_filter;
        std::string edit_buffer;
        
        // FTXUI Components
        ftxui::Component filter_tree;
        ftxui::Component filter_controls;
        
    public:
        // Constructor
        FilterPanel(FilterEngine* engine);
        
        // FTXUI integration
        ftxui::Component GetComponent();
        ftxui::Element Render();
        
        // Filter management (to be implemented in task 8)
        void AddNewFilter();
        void EditSelectedFilter();
        void DeleteSelectedFilter();
        void ToggleSelectedFilter();
        
    private:
        // Rendering helpers (to be implemented in task 8)
        ftxui::Element RenderFilterTree();
        ftxui::Element RenderFilterControls();
        void SetupFilterKeyBindings();
    };
    
} // namespace ue_log