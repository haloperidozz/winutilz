cmake_minimum_required(VERSION 3.10)
project(winutilz VERSION 1.1.0 LANGUAGES C)

option(BUILD_SHARED_LIBS       "Build winutilz as a shared library" OFF)
option(WINUTILZ_INSTALL        "Generate installation target"       ON)
option(WINUTILZ_BUILD_EXAMPLES "Build winutilz examples"            OFF)

add_subdirectory(src)

if (WINUTILZ_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
