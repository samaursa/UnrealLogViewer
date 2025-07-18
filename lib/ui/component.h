#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <string>

namespace ue_log {

/**
 * Base class for all UI components in the application.
 * Provides common functionality and interface for UI components.
 */
class Component {
public:
    Component() = default;
    virtual ~Component() = default;
    
    /**
     * Initialize the component.
     * Should be called before using the component.
     */
    virtual void Initialize() = 0;
    
    /**
     * Render the component to an FTXUI Element.
     * @return FTXUI Element representing this component.
     */
    virtual ftxui::Element Render() const = 0;
    
    /**
     * Create an FTXUI Component for this component.
     * @return FTXUI Component that can be used in the FTXUI framework.
     */
    virtual ftxui::Component CreateFTXUIComponent() = 0;
    
    /**
     * Get the title of this component.
     * @return Title string.
     */
    virtual std::string GetTitle() const { return ""; }
    
    /**
     * Check if the component is focused.
     * @return True if the component is focused, false otherwise.
     */
    virtual bool IsFocused() const { return false; }
    
    /**
     * Set the focus state of the component.
     * @param focused True to focus the component, false to unfocus.
     */
    virtual void SetFocus(bool focused) {}
};

} // namespace ue_log