cmake_minimum_required(VERSION 3.20)
project(lunaris LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_subdirectory(vendors/tinyvk)

set(LUNARIS_SOURCES
    src/main.cpp
    src/core/application.cpp
    src/core/job_system.cpp
    src/core/theme.cpp
    src/ui/components.cpp
    src/editor/editor_layer.cpp
    src/editor/workspace.cpp
    src/editor/status_bar.cpp
    src/editor/activity_bar.cpp
    src/plugin/plugin_manager.cpp
)

add_executable(lunaris ${LUNARIS_SOURCES})

target_include_directories(lunaris PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/tinyvk/vendors/imgui/imgui
)

target_link_libraries(lunaris PRIVATE tinyvk)

if(APPLE)
    set_target_properties(lunaris PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()
