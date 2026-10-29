cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

if (NOT WIN32)
    message(FATAL_ERROR "This program has been specified to compile on Microsoft Windows OS only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Shared application metadata. CMake regenerates the C++/RC headers from this.
set(GENHOTKEYS_VERSION          "0.7.0")                                   # App version
set(GENHOTKEYS_VERSION_REVISION 0)                                         # Fourth component of the Windows file version.
set(GENHOTKEYS_VERSION_STAGE    "Beta")                                    # Leave empty for a stable release.
set(GENHOTKEYS_AUTHORS          "mah_boi, nikitvs")                        # Authors
set(GENHOTKEYS_COPYRIGHT        "Copyright (C) GenHotkeys Devs 2023-2026") # Copyright

project(
    GenHotkeys
    VERSION     "${GENHOTKEYS_VERSION}"
    DESCRIPTION "A tool for editing hotkeys for C&C: Generals and C&C: Generals - Zero Hour"
    LANGUAGES   CXX
)

# Shared metadata for C++ sources and Windows resource scripts.
set(GENHOTKEYS_DISPLAY_VERSION "${PROJECT_VERSION}")
if(NOT GENHOTKEYS_VERSION_STAGE STREQUAL "")
    string(PREPEND GENHOTKEYS_DISPLAY_VERSION "${GENHOTKEYS_VERSION_STAGE} ")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/Info.hpp"
    @ONLY ESCAPE_QUOTES
)

add_library(GenHotkeysInfo INTERFACE)
target_include_directories(GenHotkeysInfo INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/generated")

# message(STATUS "${PROJECT_NAME} ${GENHOTKEYS_DISPLAY_VERSION}; authors: ${GENHOTKEYS_AUTHORS}")

# Existed paths
set(PROJECT_LIBS_FOLDER                  "${CMAKE_CURRENT_SOURCE_DIR}/libs"                 CACHE STRING "Global project libraries path")
set(PROJECT_RESOURCE_FOLDER              "${CMAKE_CURRENT_SOURCE_DIR}/src/Resources"        CACHE STRING "Global project resources path")

# Generated paths
set(PROJECT_DESTINATION_FOLDER           "${CMAKE_BINARY_DIR}/exe"                          CACHE STRING "Global project destination path")
set(PROJECT_DESTINATION_RESOURCES_FOLDER "${PROJECT_DESTINATION_FOLDER}/Resources"          CACHE STRING "Global project destination resources path")
set(PROJECT_DESTINATION_BIN_FOLDER       "${PROJECT_DESTINATION_RESOURCES_FOLDER}/Binaries" CACHE STRING "Global project destination binary path")
set(PROJECT_DESTINATION_LOGS_FOLDER      "${PROJECT_DESTINATION_FOLDER}/Logs"               CACHE STRING "Global project destination logs path")
file(MAKE_DIRECTORY ${PROJECT_DESTINATION_FOLDER})
file(MAKE_DIRECTORY ${PROJECT_DESTINATION_RESOURCES_FOLDER})
file(MAKE_DIRECTORY ${PROJECT_DESTINATION_BIN_FOLDER})
file(MAKE_DIRECTORY ${PROJECT_DESTINATION_LOGS_FOLDER})

# TODO: temporary Qt paths
set(CMAKE_PREFIX_PATH "C:/Qt/5.15.2/mingw81_64" "C:/Qt/Tools/mingw_64")

set(CMAKE_BUILD_PARALLEL_LEVEL 4)

add_subdirectory(src)
