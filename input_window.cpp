#include "input_window.h"
#include <algorithm>

using namespace ftxui;

InputWindow::InputWindow(int id, const std::string& title, std::string* content, const std::string& placeholder)
    : id_(id), title_(title), content_(content), placeholder_(placeholder) {
    input_ = Input(content_, placeholder_);
}

Element InputWindow::Render(bool is_selected, bool is_unfocused) const {
    std::string display_title = title_;
    if (!is_selected) {
        std::transform(display_title.begin(), display_title.end(), display_title.begin(), ::tolower);
    }

    auto title_text = "[" + std::to_string(id_) + "] " + display_title;

    auto input_element = is_unfocused ?
        text(content_->empty() ? placeholder_ : *content_) | color(Color::GrayDark) :
        input_->Render();

    return window(text(title_text), vbox({
        hbox({
            text(title_ + ": "),
            input_element | flex
        }),
        text("Content: " + *content_)
    }));
}
