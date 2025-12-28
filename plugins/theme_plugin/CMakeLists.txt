cmake_minimum_required(VERSION 3.20)
project(theme_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(theme_plugin SHARED
    theme_plugin.cpp
)

target_include_directories(theme_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendors/tinyvk/vendors/imgui/imgui
)

target_link_libraries(theme_plugin PRIVATE lunaris_core)

set_target_properties(theme_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/plugins
    PREFIX ""
)

if(APPLE)
    set_target_properties(theme_plugin PROPERTIES
        SUFFIX ".dylib"
        INSTALL_RPATH "@loader_path/.."
        BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(UNIX)
    set_target_properties(theme_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN/.."
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
