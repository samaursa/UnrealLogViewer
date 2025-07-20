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

# Install targets
install(TARGETS unreal_log_viewer DESTINATION bin)
install(TARGETS unreal_log_tests DESTINATION bin)
