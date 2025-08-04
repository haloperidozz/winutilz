add_executable(NekoWallpaper WIN32 main.c)

set_target_properties(NekoWallpaper PROPERTIES C_STANDARD 90)

target_link_libraries(NekoWallpaper PRIVATE winutilz)

##########################################################################
#   cJSON (https://github.com/DaveGamble/cJSON)
##########################################################################

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cJSON/cJSON.c")
    message(FATAL_ERROR "cJSON not found. Did you forget to init submodules?")
endif()

target_include_directories(NekoWallpaper
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/cJSON
)

target_sources(NekoWallpaper
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/cJSON/cJSON.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cJSON/cJSON_Utils.c
)
