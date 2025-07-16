#include "macros.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp> // For color
#include <string>
#include <vector>

using namespace ftxui;

namespace ue_log
{
    class Input
    {
        CK_GENERATED_BODY(Input);

    public:
        explicit Input(const char* InDefaultText = "Type here...")
          : m_input(ftxui::Input(&m_data, InDefaultText))
        {
        }

    private:
        std::string m_data;
        ftxui::Component m_input;

    public:
        CK_PROPERTY_GET(m_data)
        CK_PROPERTY_GET(m_input)
    };
}

auto
    main() -> int
{
    auto screen = ScreenInteractive::TerminalOutput();

    std::string input;
    std::vector<std::string> list_entries;
    int list_selected = 0;

    // Components
    auto input_box = Input(&input, "Type here...");
    auto listbox = Menu(&list_entries, &list_selected);

    auto button = Button("Add", [&] {
        if (!input.empty()) {
            list_entries.push_back(input);
            input.clear();
        }
    });

    auto layout = Container::Vertical({
        input_box,
        button,
        listbox
    });

    auto renderer = Renderer(layout, [&] {
        return window(
            text("FTXUI Sub-Window Example") | bold | color(Color::Cyan),
            vbox({
                hbox({
                    text("Input: ") | color(Color::Yellow),
                    input_box->Render() | color(Color::White)
                }),
                separator(),
                button->Render() | color(Color::Green),
                separator(),
                text("Listbox:") | color(Color::LightSkyBlue1),
                listbox->Render() | frame | size(HEIGHT, LESS_THAN, 10) | color(Color::GrayLight)
            })
        );
    });

    screen.Loop(renderer);
    return 0;
}
