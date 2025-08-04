cmake_minimum_required(VERSION 3.16)
project(UnrealLogViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable threading support
find_package(Threads REQUIRED)

# Find or fetch dependencies
include(FetchContent)

# FTXUI for terminal UI
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0
)

# MIO for memory-mapped file I/O
FetchContent_Declare(mio
  GIT_REPOSITORY https://github.com/mandreyel/mio.git
  GIT_TAG master
)

# Catch2 for unit testing
FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)

# CLI11 for command line parsing
FetchContent_Declare(CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.3.2
)

# Make dependencies available
FetchContent_MakeAvailable(ftxui mio Catch2 CLI11)

# Create the core logging library
add_library(unreal_log_lib
  macros.h
  lib/common/result.h
  lib/common/result.cpp
  lib/common/unreal_project_utils.h
  lib/common/unreal_project_utils.cpp
  lib/core/log_processor.h
  lib/core/log_processor.cpp
  lib/core/filter_manager.h
  lib/core/filter_manager.cpp
  lib/core/autotest_runner.h
  lib/core/autotest_runner.cpp
  lib/log_parser/log_entry.h
  lib/log_parser/log_entry.cpp
  lib/log_parser/log_parser.h
  lib/log_parser/log_parser.cpp
  lib/filter_engine/filter.h
  lib/filter_engine/filter.cpp
  lib/filter_engine/filter_engine.h
  lib/filter_engine/filter_engine.cpp
  lib/filter_engine/filter_expression.h
  lib/filter_engine/filter_expression.cpp
  lib/file_monitor/file_monitor.h
  lib/file_monitor/file_monitor.cpp
  lib/config/config_manager.h
  lib/config/config_manager.cpp
  lib/ui/component.h
  lib/ui/main_window.h
  lib/ui/main_window.cpp
  lib/ui/filter_panel.h
  lib/ui/filter_panel.cpp
  lib/ui/visual_theme_manager.h
  lib/ui/visual_theme_manager.cpp
  lib/ui/log_entry_renderer.h
  lib/ui/log_entry_renderer.cpp
  lib/ui/relative_line_number_system.h
  lib/ui/relative_line_number_system.cpp
  lib/ui/file_browser.h
  lib/ui/file_browser.cpp
)

# Set library properties
target_include_directories(unreal_log_lib PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Link library dependencies
target_link_libraries(unreal_log_lib
  PUBLIC mio::mio
  PUBLIC Threads::Threads
  PUBLIC ftxui::screen
  PUBLIC ftxui::dom
  PUBLIC ftxui::component
)

# Create the main application executable
add_executable(unreal_log_viewer
  app/main.cpp
)

# Link main application dependencies
target_link_libraries(unreal_log_viewer
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE CLI11::CLI11
)

# Create the test application executable
add_executable(unreal_log_tests
  tests/test_main.cpp
  tests/test_log_parser.cpp
  tests/test_filter_engine.cpp
  tests/test_file_monitor.cpp
  tests/test_integration.cpp
  tests/test_simple_integration.cpp
  tests/test_result.cpp
  tests/test_config_manager.cpp
  tests/test_main_window.cpp
  tests/test_visual_theme_manager.cpp
  tests/test_log_entry_renderer.cpp
  tests/test_relative_line_number_system.cpp
  tests/test_column_spacing.cpp
  tests/test_visual_hierarchy_comprehensive.cpp
  tests/test_visual_polish_integration.cpp
  tests/test_multiline_parsing.cpp
  tests/test_detail_view_focus.cpp
  tests/test_unreal_project_utils.cpp
  tests/test_file_browser.cpp
  tests/test_visual_selection_extension.cpp
  tests/test_visual_selection_rendering.cpp
  tests/test_visual_selection_clipboard.cpp
  tests/test_visual_selection_status_bar.cpp
  tests/test_y_key_copy_functionality.cpp
  tests/test_visual_selection_comprehensive.cpp
  tests/test_visual_selection_error_handling.cpp
  tests/test_visual_selection_performance.cpp
  tests/test_visual_selection_manual_scenarios.cpp
  tests/test_visual_selection_comprehensive_validation.cpp
)

# Link test application dependencies
target_link_libraries(unreal_log_tests
  PRIVATE unreal_log_lib
  PRIVATE Catch2::Catch2WithMain
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create debug test executable
add_executable(test_exclude_debug
  test_exclude_debug.cpp
)

target_link_libraries(test_exclude_debug
  PRIVATE unreal_log_lib
)

# Compiler-specific options for all targets
if(MSVC)
    target_compile_options(unreal_log_lib PRIVATE /W4)
    target_compile_options(unreal_log_viewer PRIVATE /W4)
    target_compile_options(unreal_log_tests PRIVATE /W4)
else()
    target_compile_options(unreal_log_lib PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(unreal_log_viewer PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(unreal_log_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Enable testing
enable_testing()
add_test(NAME unreal_log_unit_tests COMMAND unreal_log_tests)

# Create a simple badge test executable
add_executable(test_simple_badge
  test_simple_badge.cpp
)

# Link simple badge test dependencies
target_link_libraries(test_simple_badge
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a column spacing demo executable
add_executable(test_column_spacing_demo
  test_column_spacing_demo.cpp
)

# Link column spacing demo dependencies
target_link_libraries(test_column_spacing_demo
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create an Unreal log levels test executable
add_executable(test_unreal_log_levels
  test_unreal_log_levels.cpp
)

# Link Unreal log levels test dependencies
target_link_libraries(test_unreal_log_levels
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a visual hierarchy test executable
add_executable(test_visual_hierarchy
  test_visual_hierarchy.cpp
)

# Link visual hierarchy test dependencies
target_link_libraries(test_visual_hierarchy
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a three-state filters test executable
add_executable(test_three_state_filters
  test_three_state_filters.cpp
)

# Link three-state filters test dependencies
target_link_libraries(test_three_state_filters
  PRIVATE unreal_log_lib
)

# Create a UI three-state filters test executable
add_executable(test_ui_three_state
  test_ui_three_state.cpp
)

# Link UI three-state filters test dependencies
target_link_libraries(test_ui_three_state
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a basic tailing test executable
add_executable(test_tailing_basic
  test_tailing_basic.cpp
)

# Link tailing test dependencies
target_link_libraries(test_tailing_basic
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a G key tailing test executable
add_executable(test_g_key_tailing
  test_g_key_tailing.cpp
)

# Link G key tailing test dependencies
target_link_libraries(test_g_key_tailing
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a tailing navigation test executable
add_executable(test_tailing_navigation
  test_tailing_navigation.cpp
)

# Link tailing navigation test dependencies
target_link_libraries(test_tailing_navigation
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a context incremental test executable
add_executable(test_context_incremental
  test_context_incremental.cpp
)

# Link context incremental test dependencies
target_link_libraries(test_context_incremental
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a G key debug test executable
add_executable(test_g_key_debug
  test_g_key_debug.cpp
)

# Link G key debug test dependencies
target_link_libraries(test_g_key_debug
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a visual selection keyboard test executable
add_executable(test_visual_selection_keyboard
  test_visual_selection_keyboard.cpp
)

# Link visual selection keyboard test dependencies
target_link_libraries(test_visual_selection_keyboard
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a visual selection activation test executable
add_executable(test_visual_selection_activation
  test_visual_selection_activation.cpp
)

# Link visual selection activation test dependencies
target_link_libraries(test_visual_selection_activation
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a visual selection integration test executable
add_executable(test_visual_selection_integration
  test_visual_selection_integration.cpp
)

# Link visual selection integration test dependencies
target_link_libraries(test_visual_selection_integration
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a visual selection extension demo executable
add_executable(test_visual_selection_extension_demo
  test_visual_selection_extension_demo.cpp
)

# Link visual selection extension demo dependencies
target_link_libraries(test_visual_selection_extension_demo
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a visual selection feature compatibility test executable
add_executable(test_visual_selection_feature_compatibility
  tests/test_visual_selection_feature_compatibility.cpp
)

# Link visual selection feature compatibility test dependencies
target_link_libraries(test_visual_selection_feature_compatibility
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a visual selection filter interaction test executable
add_executable(test_visual_selection_filter_interaction
  tests/test_visual_selection_filter_interaction.cpp
)

# Link visual selection filter interaction test dependencies
target_link_libraries(test_visual_selection_filter_interaction
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)



# Create a G key interactive test executable
add_executable(test_g_key_interactive
  test_g_key_interactive.cpp
)

# Link G key interactive test dependencies
target_link_libraries(test_g_key_interactive
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a G key visual test executable
add_executable(test_g_key_visual
  test_g_key_visual.cpp
)

# Link G key visual test dependencies
target_link_libraries(test_g_key_visual
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Install targets
install(TARGETS unreal_log_viewer DESTINATION bin)
install(TARGETS unreal_log_tests DESTINATION bin)
# Create a scroll fix test executable
add_executable(test_scroll_fix
  test_scroll_fix.cpp
)

# Link scroll fix test dependencies
target_link_libraries(test_scroll_fix
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a simple scroll test executable
add_executable(test_simple_scroll
  test_simple_scroll.cpp
)

# Link simple scroll test dependencies
target_link_libraries(test_simple_scroll
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a poll interval test executable
add_executable(test_poll_interval
  test_poll_interval.cpp
)

# Link poll interval test dependencies
target_link_libraries(test_poll_interval
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a debug file monitor test executable
add_executable(debug_file_monitor
  debug_file_monitor.cpp
)

# Link debug file monitor test dependencies
target_link_libraries(debug_file_monitor
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a real tailing test executable
add_executable(test_real_tailing
  test_real_tailing.cpp
)

# Link real tailing test dependencies
target_link_libraries(test_real_tailing
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a file browser demo executable
add_executable(test_file_browser_demo
  test_file_browser_demo.cpp
)

# Link file browser demo dependencies
target_link_libraries(test_file_browser_demo
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)
# Create a new features test executable
add_executable(test_new_features
  test_new_features.cpp
)

# Link new features test dependencies
target_link_libraries(test_new_features
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)



# Create a keyboard shortcuts test executable
add_executable(test_keyboard_shortcuts
  test_keyboard_shortcuts.cpp
)

# Link keyboard shortcuts test dependencies
target_link_libraries(test_keyboard_shortcuts
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a filter highlighting test executable
add_executable(test_filter_highlighting
  test_filter_highlighting.cpp
)

# Link filter highlighting test dependencies
target_link_libraries(test_filter_highlighting
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a highlighting bug test executable
add_executable(test_highlighting_bug
  test_highlighting_bug.cpp
)

# Link highlighting bug test dependencies
target_link_libraries(test_highlighting_bug
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a search promotion bug test executable
add_executable(test_search_promotion_bug
  test_search_promotion_bug.cpp
)

# Link search promotion bug test dependencies
target_link_libraries(test_search_promotion_bug
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a filter duplication debug test executable
add_executable(test_filter_duplication_debug
  test_filter_duplication_debug.cpp
)

# Link filter duplication debug test dependencies
target_link_libraries(test_filter_duplication_debug
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a character consumption fix test executable
add_executable(test_character_consumption_fix
  test_character_consumption_fix.cpp
)

# Link character consumption fix test dependencies
target_link_libraries(test_character_consumption_fix
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a search scrollbar fix test executable
add_executable(test_search_scrollbar_fix
  test_search_scrollbar_fix.cpp
)

# Link search scrollbar fix test dependencies
target_link_libraries(test_search_scrollbar_fix
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a comprehensive search scrollbar test executable
add_executable(test_search_scrollbar_comprehensive
  test_search_scrollbar_comprehensive.cpp
)

# Link comprehensive search scrollbar test dependencies
target_link_libraries(test_search_scrollbar_comprehensive
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a scrollbar visual test executable
add_executable(test_scrollbar_visual
  test_scrollbar_visual.cpp
)

# Link scrollbar visual test dependencies
target_link_libraries(test_scrollbar_visual
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a scrollbar debug test executable
add_executable(test_scrollbar_debug
  test_scrollbar_debug.cpp
)

# Link scrollbar debug test dependencies
target_link_libraries(test_scrollbar_debug
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a tall scrollbar test executable
add_executable(test_scrollbar_tall
  test_scrollbar_tall.cpp
)

# Link tall scrollbar test dependencies
target_link_libraries(test_scrollbar_tall
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a clipboard demo executable
add_executable(test_clipboard_demo
  test_clipboard_demo.cpp
)

# Link clipboard demo dependencies
target_link_libraries(test_clipboard_demo
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a status bar demo executable
add_executable(test_status_bar_demo
  test_status_bar_demo.cpp
)

# Link status bar demo dependencies
target_link_libraries(test_status_bar_demo
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a reload behavior test executable
add_executable(test_reload_behavior
  test_reload_behavior.cpp
)

# Link reload behavior test dependencies
target_link_libraries(test_reload_behavior
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Create a keyboard debug test executable
add_executable(test_keyboard_debug
  test_keyboard_debug.cpp
)

# Link keyboard debug test dependencies
target_link_libraries(test_keyboard_debug
  PRIVATE unreal_log_lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

