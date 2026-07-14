set(NOMACS_RC src/nomacs.rc) #add resource file when compiling with MSVC
set(VERSION_LIB Version.lib)

# create the targets
set(BINARY_NAME ${PROJECT_NAME})
set(DLL_CORE_NAME ${PROJECT_NAME}Core)
set(LIB_CORE_NAME optimized ${DLL_CORE_NAME}.lib debug ${DLL_CORE_NAME}d.lib)

#binary
set(CHANGLOG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/changelog.txt)

add_executable(
    ${BINARY_NAME}
    WIN32
    MACOSX_BUNDLE
    ${NOMACS_EXE_SOURCES}
    ${NOMACS_EXE_HEADERS}
    ${NOMACS_QM}
    ${NOMACS_RC}
    ${CHANGLOG_FILE}
)

# define that changelog should not be compiled
set_source_files_properties(${CHANGLOG_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(${BINARY_NAME} ${LIB_CORE_NAME})

set_target_properties(${BINARY_NAME} PROPERTIES COMPILE_FLAGS "-DDK_DLL_IMPORT -DNOMINMAX")

# add DLL
add_library(
    ${DLL_CORE_NAME}
    SHARED
    ${GUI_HEADERS}
    ${GUI_SOURCES}
    ${CORE_HEADERS}
    ${CORE_SOURCES}
    ${NOMACS_RCC}
    ${NOMACS_RC}
)
target_link_libraries(
    ${DLL_CORE_NAME}
    ${EXIV2_LINK_LIBRARIES} # metadata support
    ${VERSION_LIB} # needed for registering the curren version
    ${LIBRAW_LINK_LIBRARIES} # RAW support (optional)
    ${OpenCV_LIBS} # image manipulation support (optional)
    ${TIFF_LINK_LIBRARIES} # multip page tiff support (optional)
    $ENV{NOMACS_DEPENDENCIES}/lib/tiffxx.lib
    ${QUAZIP_LIBRARIES} # ZIP support (optional)
)

add_dependencies(${BINARY_NAME} ${DLL_CORE_NAME})

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

# set(_moc ${CMAKE_CURRENT_BINARY_DIR}/GeneratedFiles)
file(GLOB NOMACS_AUTOMOC "${CMAKE_BINARY_DIR}/*_automoc.cpp ${CMAKE_BINARY_DIR}/moc_.cpp")

set_target_properties(${DLL_CORE_NAME} PROPERTIES COMPILE_FLAGS "-DDK_CORE_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_CORE_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_CORE_NAME})

# make RelWithDebInfo link against release instead of debug opencv dlls
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE)

# copy themes
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/themes)
file(COPY ${NOMACS_THEMES} DESTINATION ${CMAKE_BINARY_DIR}/Release/themes/)

# copy DLL files
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/zlib1.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/jpeg62.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/brotlicommon.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/brotlidec.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/brotlienc.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/exiv2.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/libexpat.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/lcms2.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/raw.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/opencv_core500.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/opencv_flann500.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/opencv_geometry500.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/opencv_imgproc500.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/libsharpyuv.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/liblzma.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/zstd.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
if(ENABLE_TIFF)
    file(COPY "$ENV{NOMACS_DEPENDENCIES}/bin/tiff.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release/)
    file(COPY "$ENV{NOMACS_DEPENDENCIES}/bin/libwebp.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release/)
endif(ENABLE_TIFF)
if(ENABLE_QUAZIP)
    file(COPY "$ENV{NOMACS_DEPENDENCIES}/bin/quazip1-qt6.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release/)
endif(ENABLE_QUAZIP)

file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/OpenEXR-3_4.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/IlmThread-3_4.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/OpenEXRCore-3_4.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/Iex-3_4.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/Imath-3_2.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)

file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/heif.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/libde265.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/openh264-7.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)

file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/openjp2.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)

file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/avif.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/dav1d.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)

file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/jxl.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/jxl_threads.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/jxl_cms.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)

file(COPY $ENV{NOMACS_DEPENDENCIES}/bin/KF6Archive.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)

# copy Qt libs
file(COPY ${QT_QMAKE_PATH}/Qt6Widgets.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${QT_QMAKE_PATH}/Qt6Gui.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${QT_QMAKE_PATH}/Qt6Core.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${QT_QMAKE_PATH}/Qt6Core5Compat.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${QT_QMAKE_PATH}/Qt6Network.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${QT_QMAKE_PATH}/Qt6PrintSupport.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${QT_QMAKE_PATH}/Qt6Concurrent.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)

# copy additional Qt files
# add image plugins
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/imageformats)
file(GLOB QT_IMAGE_FORMATS "${QT_QMAKE_PATH}/../plugins/imageformats/*.dll")
file(COPY ${QT_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats PATTERN *d.dll EXCLUDE)

file(GLOB QT_EXTRA_IMAGE_FORMATS "${QT_QMAKE_PATH}/../../qtimageformats/plugins/imageformats/*.dll")
file(COPY ${QT_EXTRA_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats PATTERN *d.dll EXCLUDE)

file(GLOB KDE_IMAGE_FORMATS "$ENV{NOMACS_DEPENDENCIES}/lib/plugins/imageformats/kimg_*.dll")
file(COPY ${KDE_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats)

# Icon Engines (SVG Icons)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/iconengines)
file(GLOB QT_IMAGE_FORMATS "${QT_QMAKE_PATH}/../plugins/iconengines/*.dll")
file(COPY ${QT_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Release/iconengines PATTERN *d.dll EXCLUDE)

# Platforms
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/platforms)
file(COPY ${QT_QMAKE_PATH}/../plugins/platforms/qwindows.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/platforms/)

# SVG support
file(COPY ${QT_QMAKE_PATH}/Qt6Svg.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)

# Themes
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/styles)
file(COPY ${QT_QMAKE_PATH}/../plugins/styles/qmodernwindowsstyle.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/styles/)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/tls)
file(GLOB QT_TLS_PLUGINS "${QT_QMAKE_PATH}/../plugins/tls/*.dll")
file(COPY ${QT_TLS_PLUGINS} DESTINATION ${CMAKE_BINARY_DIR}/Release/tls PATTERN *backendd.dll EXCLUDE)

# copy translation files after each build
add_custom_command(
    TARGET ${BINARY_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory \"${CMAKE_BINARY_DIR}/Release/translations/\"
)
foreach(QM ${NOMACS_QM})
    add_custom_command(
        TARGET ${BINARY_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy \"${QM}\" \"${CMAKE_BINARY_DIR}/Release/translations/\"
    )
endforeach(QM)

if(Python_FOUND)
    # add version for install scripts
    add_custom_command(
        TARGET ${DLL_CORE_NAME}
        PRE_BUILD
        COMMAND
            ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/versionupdate.py
            ${CMAKE_CURRENT_SOURCE_DIR}/../installer/nomacs-setup.wxi --copy
    )

    add_custom_command(
        TARGET ${DLL_CORE_NAME}
        PRE_BUILD
        COMMAND
            ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/versionupdate.py
            ${CMAKE_CURRENT_SOURCE_DIR}/../installer/nomacs-setup.iss --copy
    )
endif()

# set properties for Visual Studio Projects
set(CMAKE_CXX_FLAGS_DEBUG "/MP /Zc:wchar_t /W4 /w14265 /w14826 /w15038 /EHsc ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE
    "/MP /Zc:wchar_t /W4 /w14265 /w14826 /w15038 /O2 /EHsc -DDK_INSTALL -DQT_NO_DEBUG_OUTPUT ${CMAKE_CXX_FLAGS_RELEASE}"
)

source_group("Generated Files" FILES ${NOMACS_RCC} ${NOMACS_RC} ${NOMACS_QM} ${NOMACS_AUTOMOC})
source_group("Translations" FILES ${NOMACS_TRANSLATIONS})
source_group("Changelog" FILES ${CHANGLOG_FILE})

set(NOMACS_LIBS ${DLL_CORE_NAME})
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_INCLUDE_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DkLoader
    ${CMAKE_BINARY_DIR}
)
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)
