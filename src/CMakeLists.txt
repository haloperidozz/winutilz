if (BUILD_SHARED_LIBS)
    add_library(winutilz SHARED)
    target_compile_definitions(winutilz PRIVATE _WU_EXPORT)
else ()
    add_library(winutilz STATIC)
    target_compile_definitions(winutilz PUBLIC  _WU_STATIC)
endif ()

set_target_properties(winutilz PROPERTIES
    VERSION       ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    SOVERSION     ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/include/winutilz.h
    C_STANDARD    90
)

target_include_directories(winutilz
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(winutilz
    PRIVATE
        branding.c
        capture.c
        clipboard.c
        colors.c
        cursor.c
        image.c
        inputbox.c
        internal.c
        internet.c
        power.c
        process.c
        resource.c
        shell.c
        strconv.c
        undoc.c
        version.c
        wallpaper.c
        window.c
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h
    @ONLY
)

if (BUILD_SHARED_LIBS)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/winutilz.rc.in
        ${CMAKE_CURRENT_SOURCE_DIR}/winutilz.rc
        @ONLY
    )

    target_sources(winutilz
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/winutilz.rc
    )
endif()

target_link_libraries(winutilz
    PRIVATE
        ntdll
        wininet
        dwmapi
        shlwapi
        windowscodecs
)

if (MSVC)
    target_compile_options(winutilz PRIVATE /wd4996 /wd4005)
endif ()

if (MINGW)
    set_target_properties(winutilz PROPERTIES PREFIX "")
endif ()

if (WINUTILZ_INSTALL)
    include(GNUInstallDirs)

    install(TARGETS winutilz
        EXPORT WinUtilzTargets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT WinUtilzTargets
        FILE WinUtilzConfig.cmake
        EXPORT_LINK_INTERFACE_LIBRARIES
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/winutilz"
    )
endif ()
