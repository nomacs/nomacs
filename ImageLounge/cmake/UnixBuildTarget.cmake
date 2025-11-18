if(ENABLE_PLUGINS)
    add_definitions(-DWITH_PLUGINS)
endif()

set(OUR_SO_VERSION "")
set(OUR_VERSION "")

# create the targets
set(BINARY_NAME ${PROJECT_NAME})
set(DLL_CORE_NAME ${PROJECT_NAME}Core)

#binary
link_directories(${LIBRAW_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS} ${EXIV2_LIBRARY_DIRS} ${CMAKE_BINARY_DIR})
add_executable(
    ${BINARY_NAME}
    WIN32
    MACOSX_BUNDLE
    ${NOMACS_EXE_SOURCES}
    ${NOMACS_EXE_HEADERS}
    ${NOMACS_QM}
    ${NOMACS_TRANSLATIONS}
    ${NOMACS_RC}
)
target_link_libraries(${BINARY_NAME} ${DLL_CORE_NAME})

set_target_properties(${BINARY_NAME} PROPERTIES COMPILE_FLAGS "-DDK_DLL_IMPORT -DNOMINMAX")
set_target_properties(${BINARY_NAME} PROPERTIES IMPORTED_IMPLIB "")
set_target_properties(${BINARY_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

# add core
add_library(
    ${DLL_CORE_NAME}
    SHARED
    ${GUI_SOURCES}
    ${GUI_HEADERS}
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${NOMACS_RCC}
    ${NOMACS_RC}
    ${QUAZIP_SOURCES}
    ${LIBQPSD_SOURCES}
    ${LIBQPSD_HEADERS}
)
target_link_libraries(
    ${DLL_CORE_NAME}
    ${EXIV2_LIBRARIES}
    ${LIBRAW_LIBRARIES}
    ${OpenCV_LIBS}
    ${TIFF_LIBRARIES}
    ${QUAZIP_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${LIBQPSD_LIBRARY}
)

set_property(
    TARGET ${DLL_CORE_NAME}
    PROPERTY VERSION ${NOMACS_VERSION_MAJOR}.${NOMACS_VERSION_MINOR}.${NOMACS_VERSION_PATCH}
)
set_property(TARGET ${DLL_CORE_NAME} PROPERTY SOVERSION ${NOMACS_VERSION_MAJOR})

add_dependencies(${BINARY_NAME} ${DLL_CORE_NAME} ${QUAZIP_DEPENDENCY} ${LIBQPSD_LIBRARY})

target_link_libraries(${BINARY_NAME} Qt::Widgets Qt::Gui)
target_link_libraries(
    ${DLL_CORE_NAME}
    Qt::Widgets
    Qt::Gui
    Qt::Network
    Qt::PrintSupport
    Qt::Concurrent
    Qt::Svg
)

# core flags
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/libs)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/libs)

set_target_properties(${DLL_CORE_NAME} PROPERTIES COMPILE_FLAGS "-DDK_CORE_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_CORE_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_CORE_NAME}d)
set_target_properties(${DLL_CORE_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_CORE_NAME})

# installation
#  binary
install(TARGETS ${BINARY_NAME} ${DLL_CORE_NAME} DESTINATION bin LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
#  translations
install(FILES ${NOMACS_QM} DESTINATION "share/nomacs/Image Lounge/translations")
#  manpage
if(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
    install(FILES xgd-data/nomacs.1 DESTINATION man/man1)
else()
    install(FILES xgd-data/nomacs.1 DESTINATION share/man/man1)
endif()

# themes
install(FILES ${NOMACS_THEMES} DESTINATION "share/nomacs/Image Lounge/themes")

# "make dist" target
string(TOLOWER ${PROJECT_NAME} CPACK_PACKAGE_NAME)
set(CPACK_PACKAGE_VERSION "${NOMACS_VERSION}")
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_IGNORE_FILES
    "/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;refresh-copyright-and-license.pl;build;release;"
)
set(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
include(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# AppImage using linuxdeploy which seems to be the most active/supported method
# This will include kimageformats-plugins if installed on the host
# a known-working linuxdeploy may be installed via scripts/build-linuxdeploy.sh
find_program(LINUXDEPLOY_BINARY linuxdeploy)
if(NOT LINUXDEPLOY_BINARY)
    message(NOTICE "linuxdeploy is not available or working, required for AppImage")
    message(NOTICE "you may install a compatible version with scripts/build-linuxdeploy.sh")
else()
    if(Qt6_VERSION VERSION_LESS "6.10.0")
        set(PLATFORM_PLUGINS "libqwayland-egl.so\\;libqwayland-generic.so")
    else()
        set(PLATFORM_PLUGINS "libqwayland.so")
    endif()

    # we may have multiple targets writing into ./AppDir/usr/lib, this target serializes them properly
    add_custom_target(
        _appdir_create
        COMMAND ${CMAKE_COMMAND} -E remove_directory ./AppDir
        COMMAND ${CMAKE_COMMAND} -E make_directory ./AppDir/usr/lib
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Creating empty AppDir"
    )

    add_custom_target(
        _appdir_install_nomacs
        COMMAND cmake --install . --prefix ./AppDir/usr/
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Installing nomacs to AppDir"
    )
    add_dependencies(_appdir_install_nomacs _appdir_create)

    # libheif uses a plugin system that is not discoverable by linuxdeploy,
    # this breaks kimg_heif and thus heif/heic files will not load...
    #
    # we can find the heif plugins by inspecting the heif header files, this
    # is the only method to get the correct location as it was determined
    # when libheif was compiled
    #
    # we also want to check if kimg_heif.so plugin installed, if not libheif won't be used
    # and we would be adding a lot of bloat to the AppImage
    #
    set(QJPEG_TARGET Qt${QT_VERSION_MAJOR}::QJpegPlugin)
    if(TARGET ${QJPEG_TARGET})
        get_target_property(QJPEG_PATH ${QJPEG_TARGET} LOCATION_${CMAKE_BUILD_TYPE})
        get_filename_component(QT_IMAGEFORMATS_PATH "${QJPEG_PATH}" PATH)
        if(EXISTS "${QT_IMAGEFORMATS_PATH}/kimg_heif.so")
            pkg_check_modules(HEIF libheif)
            set(LIBHEIF_HEADER "libheif/heif_version.h")
            foreach(dir IN LISTS HEIF_INCLUDE_DIRS)
                if(EXISTS "${dir}/${LIBHEIF_HEADER}")
                    file(READ "${dir}/${LIBHEIF_HEADER}" CONTENTS)
                    string(REGEX MATCH "#define LIBHEIF_PLUGIN_DIRECTORY \"(.+)\"" _ "${CONTENTS}")
                    if(CMAKE_MATCH_1)
                        set(LIBHEIF_PLUGIN_PATH ${CMAKE_MATCH_1})
                        set(APPDIR_LIBHEIF_PLUGIN_PATH "./AppDir/usr/lib/libheif/plugins")
                        file(GLOB LIBHEIF_PLUGINS "${LIBHEIF_PLUGIN_PATH}/*")
                        message(STATUS "AppImage will include libheif plugins from: ${LIBHEIF_PLUGIN_PATH}")
                        set(LIBHEIF_ARGS "--deploy-deps-only=${APPDIR_LIBHEIF_PLUGIN_PATH}")
                        add_custom_target(
                            _appdir_copy_libheif
                            COMMAND ${CMAKE_COMMAND} -E make_directory ${APPDIR_LIBHEIF_PLUGIN_PATH}
                            COMMAND ${CMAKE_COMMAND} -E copy ${LIBHEIF_PLUGINS} ${APPDIR_LIBHEIF_PLUGIN_PATH}
                            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                            COMMENT "Copying libheif plugins to AppDir"
                        )
                        add_dependencies(_appdir_copy_libheif _appdir_create)
                    endif()
                    break()
                endif()
            endforeach()
        endif()
    endif()

    add_custom_target(
        appimage
        # add "AppRun hook" to modify process environment before launching nomacs
        # the tilde (~) makes our hook run last, allowing overrides of other hooks
        COMMAND ${CMAKE_COMMAND} -E make_directory ./AppDir/apprun-hooks/
        COMMAND
            ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/xgd-data/appimage-env.sh ./AppDir/apprun-hooks/~appimage-env.sh
        COMMAND
            ${CMAKE_COMMAND} -E env # linuxdeploy plugins configured with environment variables
            "LD_LIBRARY_PATH=." # find libnomacsCore in CWD
            "LDAI_NO_APPSTREAM=1" # FIXME: support appstream
            "QMAKE=qmake6" # correct qmake for *this* build (even though we don't use it)
            "EXTRA_QT_PLUGINS=waylandcompositor" # wayland support not included by default
            "EXTRA_PLATFORM_PLUGINS=${PLATFORM_PLUGINS}" # wayland platforms
            ${LINUXDEPLOY_BINARY} --appdir AppDir --plugin=qt --output appimage --verbosity=2
            --desktop-file=./AppDir/usr/share/applications/org.nomacs.ImageLounge.desktop ${LIBHEIF_ARGS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building AppImage"
    )
    add_dependencies(appimage _appdir_install_nomacs)
    if(TARGET _appdir_copy_libheif)
        add_dependencies(appimage _appdir_copy_libheif)
    endif()
endif() # AppImage

# generate configuration file
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_LIBS ${DLL_CORE_NAME})
set(NOMACS_INCLUDE_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore
    ${CMAKE_BINARY_DIR}
)

configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)

if(NOT DEFINED ${CMAKE_INSTALL_DATAROOTDIR})
    set(CMAKE_INSTALL_DATAROOTDIR "share") # -> will be installed in /usr/local/share
endif()

string(TIMESTAMP TODAY "%Y-%m-%d")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/xgd-data")
