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
)

# Link test application dependencies
target_link_libraries(unreal_log_tests
  PRIVATE unreal_log_lib
  PRIVATE Catch2::Catch2WithMain
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
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
