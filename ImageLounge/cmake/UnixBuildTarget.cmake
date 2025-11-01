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
    ${QUAZIP_SOURCES}
)
target_link_libraries(
    ${BINARY_NAME}
    ${DLL_CORE_NAME}
    ${EXIV2_LIBRARIES}
    ${LIBRAW_LIBRARIES}
    ${OpenCV_LIBS}
    ${TIFF_LIBRARIES}
    ${QUAZIP_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${LIBQPSD_LIBRARY}
)

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
)

set_property(
    TARGET ${DLL_CORE_NAME}
    PROPERTY VERSION ${NOMACS_VERSION_MAJOR}.${NOMACS_VERSION_MINOR}.${NOMACS_VERSION_PATCH}
)
set_property(TARGET ${DLL_CORE_NAME} PROPERTY SOVERSION ${NOMACS_VERSION_MAJOR})

add_dependencies(${BINARY_NAME} ${DLL_CORE_NAME} ${QUAZIP_DEPENDENCY} ${LIBQPSD_LIBRARY})

target_link_libraries(
    ${BINARY_NAME}
    Qt::Widgets
    Qt::Gui
    Qt::Network
    Qt::PrintSupport
    Qt::Concurrent
    Qt::Svg
)
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
# FIXME: download correct image for host arch
set(LINUXDEPLOY_URL "https://api.github.com/repos/linuxdeploy/linuxdeploy/releases/latest")
set(LINUXDEPLOY_FILE "${CMAKE_BINARY_DIR}/linuxdeploy.AppImage")
add_custom_command(
    OUTPUT ${LINUXDEPLOY_FILE}
    COMMAND curl --silent --location ${LINUXDEPLOY_URL} > release.json
    COMMAND grep browser_download_url release.json | grep x86_64.AppImage | cut -d\\\" -f4 > download.url
    COMMAND cat download.url | xargs curl --location --output ${LINUXDEPLOY_FILE}
    COMMAND chmod +x ${LINUXDEPLOY_FILE}
    COMMENT "Downloading latest linuxdeploy AppImage"
)

set(LINUXDEPLOY_QT_URL "https://api.github.com/repos/linuxdeploy/linuxdeploy-plugin-qt/releases/latest")
set(LINUXDEPLOY_QT_FILE "${CMAKE_BINARY_DIR}/linuxdeploy-plugin-qt.AppImage")
add_custom_command(
    OUTPUT ${LINUXDEPLOY_QT_FILE}
    COMMAND curl --silent --location ${LINUXDEPLOY_QT_URL} > release.json
    COMMAND grep browser_download_url release.json | grep x86_64.AppImage | cut -d\\\" -f4 > download.url
    COMMAND cat download.url | xargs curl --location --output ${LINUXDEPLOY_QT_FILE}
    COMMAND chmod +x ${LINUXDEPLOY_QT_FILE}
    COMMENT "Downloading latest linuxdeploy-plugin-qt AppImage"
)

add_custom_target(
    appimage
    DEPENDS ${LINUXDEPLOY_FILE} ${LINUXDEPLOY_QT_FILE}
    COMMAND rm -rf AppDir
    COMMAND mkdir -p AppDir/usr
    COMMAND cmake --install . --prefix ./AppDir/usr
    COMMAND
        ${CMAKE_COMMAND} -E env # linuxdeploy plugins configured with environment variables
        "LD_LIBRARY_PATH=." # find libnomacsCore in CWD
        "LDAI_NO_APPSTREAM=1" # FIXME: support appstream
        "QMAKE=qmake6" # correct qmake for *this* build (even though we don't use it)
        "EXTRA_QT_PLUGINS=waylandcompositor" # wayland support not included by default
        "EXTRA_PLATFORM_PLUGINS=libqwayland-egl.so\\;libqwayland-generic.so" # wayland platforms
        ${LINUXDEPLOY_FILE} --appdir AppDir --plugin qt --output appimage --verbosity=2
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building AppImage"
)

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
