cmake_minimum_required(VERSION 3.20)
project(lunaris LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

set(LUNARIS_CORE_SOURCES
    src/core/job_system.cpp
    src/core/theme.cpp
    src/core/command.cpp
    src/core/command_registry.cpp
    src/core/settings.cpp
    src/plugin/plugin_manager.cpp
    src/plugin/plugin_loader.cpp
)

add_library(lunaris_core SHARED ${LUNARIS_CORE_SOURCES})

target_include_directories(lunaris_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/tinyvk/vendors/imgui/imgui
)

target_link_libraries(lunaris_core PUBLIC tinyvk)

set(LUNARIS_SOURCES
    src/main.cpp
    src/core/application.cpp
    src/ui/components.cpp
    src/editor/editor_layer.cpp
    src/editor/workspace.cpp
    src/editor/status_bar.cpp
    src/editor/menu_bar.cpp
    src/editor/sidebar.cpp
    src/editor/tab_bar.cpp
    src/editor/bottom_panel.cpp
    src/editor/command_palette.cpp
    src/editor/file_tree.cpp
)

add_executable(lunaris ${LUNARIS_SOURCES})

target_include_directories(lunaris PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/tinyvk/vendors/imgui/imgui
)

target_link_libraries(lunaris PRIVATE lunaris_core tinyvk)

if(APPLE)
    find_library(VULKAN_LIBRARY NAMES vulkan vulkan.1 HINTS ENV VULKAN_SDK PATH_SUFFIXES lib)
    get_filename_component(VULKAN_LIB_DIR "${VULKAN_LIBRARY}" DIRECTORY)
    
    set_target_properties(lunaris PROPERTIES
        MACOSX_BUNDLE FALSE
        INSTALL_RPATH "@executable_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    set_target_properties(lunaris_core PROPERTIES
        INSTALL_RPATH "@executable_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    add_custom_command(TARGET lunaris POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_LIB_DIR}/libvulkan.1.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/bin/libvulkan.1.dylib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_LIB_DIR}/libMoltenVK.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/bin/libMoltenVK.dylib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_LIB_DIR}/libshaderc_shared.1.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/bin/libshaderc_shared.1.dylib"
        COMMAND install_name_tool -change "@rpath/libvulkan.1.dylib" "@executable_path/libvulkan.1.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/bin/lunaris"
        COMMAND install_name_tool -change "@rpath/libshaderc_shared.1.dylib" "@executable_path/libshaderc_shared.1.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/bin/lunaris"
        COMMAND install_name_tool -change "@rpath/libvulkan.1.dylib" "@executable_path/libvulkan.1.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/bin/liblunaris_core.dylib"
        COMMAND install_name_tool -change "@rpath/libshaderc_shared.1.dylib" "@executable_path/libshaderc_shared.1.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/bin/liblunaris_core.dylib"
        COMMENT "Bundling Vulkan libraries for standalone distribution"
    )
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/plugins)

add_subdirectory(plugins/theme_plugin)
