cmake_minimum_required(VERSION 3.16)
project(UnrealLogViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or fetch FTXUI
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Create the executable with all source files
add_executable(unreal_log_viewer
  main.cpp
  log_entry.cpp
  filter_manager.cpp
  log_viewer.cpp
  ui_renderer.cpp
  input_window.cpp
  window_switcher.cpp
  input_manager.cpp
  log_window.cpp

  # Header files for IDE support
  log_entry.h
  filter_manager.h
  log_viewer.h
  ui_renderer.h
  input_window.h
  window_switcher.h
  input_manager.h
  log_window.h
)

# Link FTXUI libraries
target_link_libraries(unreal_log_viewer
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# Enable threading support
find_package(Threads REQUIRED)
target_link_libraries(unreal_log_viewer PRIVATE Threads::Threads)

# Compiler-specific options
if(MSVC)
    target_compile_options(unreal_log_viewer PRIVATE /W4)
else()
    target_compile_options(unreal_log_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install target
install(TARGETS unreal_log_viewer DESTINATION bin)
