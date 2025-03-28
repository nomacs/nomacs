set(NOMACS_RC src/nomacs.rc) #add resource file when compiling with MSVC
set(VERSION_LIB Version.lib)

# create the targets
set(BINARY_NAME ${PROJECT_NAME})
set(DLL_CORE_NAME ${PROJECT_NAME}Core)
set(LIB_CORE_NAME optimized ${DLL_CORE_NAME}.lib debug ${DLL_CORE_NAME}d.lib)

#binary
link_directories(${OpenCV_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/libs/)
set(CHANGLOG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/changelog.txt)

add_executable(
	${BINARY_NAME} WIN32  MACOSX_BUNDLE 
	${NOMACS_EXE_SOURCES} ${NOMACS_EXE_HEADERS} 
	${NOMACS_QM} ${NOMACS_RC} ${CHANGLOG_FILE}
	)
	
# define that changelog should not be compiled
set_source_files_properties(${CHANGLOG_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(
	${BINARY_NAME} 
	${LIB_CORE_NAME} 
	${EXIV2_LIBS} 
	${LIBRAW_LIBRARIES} 
	${OpenCV_LIBS} 
	${TIFF_LIBRARIES} 
	${QUAZIP_LIBRARIES}
	)
	
set_target_properties(${BINARY_NAME} PROPERTIES COMPILE_FLAGS "-DDK_DLL_IMPORT -DNOMINMAX")

# add DLL
add_library(
	${DLL_CORE_NAME} SHARED 
	${GUI_HEADERS} ${GUI_SOURCES} 
	${CORE_HEADERS} ${CORE_SOURCES} 
	${NOMACS_RCC} ${NOMACS_RC}
	)
target_link_libraries(
	${DLL_CORE_NAME}
	${EXIV2_LIBS} 				# metadata support
	${VERSION_LIB} 				# needed for registering the curren version
	${LIBRAW_LIBRARIES} 		# RAW support (optional)
	${OpenCV_LIBS} 				# image manipulation support (optional)
	${TIFF_LIBRARIES} 			# multip page tiff support (optional)
	${QUAZIP_LIBRARIES}			# ZIP support (optional)
	)

add_dependencies(
	${BINARY_NAME} 
	${DLL_CORE_NAME} 
	)

target_include_directories(${BINARY_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/../3rd-party/build/opencv/3rdparty/zlib")
target_include_directories(${DLL_CORE_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/../3rd-party/build/opencv/3rdparty/zlib")

target_link_libraries(${BINARY_NAME} Qt::Widgets Qt::Gui Qt::Network Qt::PrintSupport Qt::Concurrent Qt::Svg)
target_link_libraries(${DLL_CORE_NAME} Qt::Widgets Qt::Gui Qt::Network Qt::PrintSupport Qt::Concurrent Qt::Svg)

# set(_moc ${CMAKE_CURRENT_BINARY_DIR}/GeneratedFiles)
file(GLOB NOMACS_AUTOMOC "${CMAKE_BINARY_DIR}/*_automoc.cpp ${CMAKE_BINARY_DIR}/moc_.cpp")

# core flags
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/libs/$<CONFIGURATION>)

# Define dlls which should be delay loaded on windows -------------------------------------
set(DELAY_DLL_NAMES
	raw.dll
	exiv2.dll
    )

# dear future me: sorry, for manually defining them - but I have no time right now
set(DELAY_DLL_NAMES_DEBUG 
	opencv_core4110d.dll
	opencv_imgproc4110d.dll
	${DELAY_DLL_NAMES}
	)

set(DELAY_DLL_NAMES_RELEASE
	opencv_core4110.dll
	opencv_imgproc4110.dll
	quazip1-qt6.dll
	${DELAY_DLL_NAMES}
	)

foreach(DLL_NAME ${DELAY_DLL_NAMES_DEBUG})
   set(DELAY_LOAD_DEBUG "${DELAY_LOAD_DEBUG} /DELAYLOAD:${DLL_NAME}")
endforeach()

foreach(DLL_NAME ${DELAY_DLL_NAMES_RELEASE})
   set(DELAY_LOAD_RELEASE "${DELAY_LOAD_RELEASE} /DELAYLOAD:${DLL_NAME}")
endforeach()

set_target_properties(${DLL_CORE_NAME} PROPERTIES LINK_FLAGS_DEBUG ${DELAY_LOAD_DEBUG})
set_target_properties(${DLL_CORE_NAME} PROPERTIES LINK_FLAGS_RELEASE ${DELAY_LOAD_RELEASE})
# Define dlls which should be delay loaded on windows -------------------------------------

set_target_properties(${DLL_CORE_NAME} PROPERTIES COMPILE_FLAGS "-DDK_CORE_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_CORE_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_CORE_NAME}d)
set_target_properties(${DLL_CORE_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_CORE_NAME})

# make RelWithDebInfo link against release instead of debug opencv dlls
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE)

# copy themes
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/themes)
file(COPY ${NOMACS_THEMES} DESTINATION ${CMAKE_BINARY_DIR}/Release/themes/)

# copy DLL files
file(COPY ${EXIV2_BUILD_PATH}/Release/bin/exiv2.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${EXPAT_BUILD_PATH}/Release/libexpat.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${LIBRAW_BUILD_PATH}/Release/raw.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${OpenCV_DIR}/bin/Release/opencv_core4110.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
file(COPY ${OpenCV_DIR}/bin/Release/opencv_imgproc4110.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
if(ENABLE_QUAZIP)
	file(COPY "${DEPENDENCY_PATH}/quazip/quazip/Release/quazip1-qt6.dll" DESTINATION ${CMAKE_BINARY_DIR}/Release/)
endif(ENABLE_QUAZIP)

if (ENABLE_AVIF)
    file(DOWNLOAD "https://github.com/novomesk/qt-avif-image-plugin/releases/latest/download/qavif6.dll" ${CMAKE_BINARY_DIR}/Release/imageformats/qavif6.dll)
endif()

if (ENABLE_JXL)
    file(DOWNLOAD "https://github.com/novomesk/qt-jpegxl-image-plugin/releases/latest/download/qjpegxl6.dll" ${CMAKE_BINARY_DIR}/Release/imageformats/qjpegxl6.dll)
endif()

if (ENABLE_HEIF)
    file(DOWNLOAD "https://github.com/novomesk/qt-heic-image-plugin/releases/latest/download/kimg_heif6.dll" ${CMAKE_BINARY_DIR}/Release/imageformats/kimg_heif6.dll)
    file(DOWNLOAD "https://github.com/novomesk/qt-heic-image-plugin/releases/latest/download/heif.dll" ${CMAKE_BINARY_DIR}/Release/heif.dll)
    file(DOWNLOAD "https://github.com/novomesk/qt-heic-image-plugin/releases/latest/download/libde265.dll" ${CMAKE_BINARY_DIR}/Release/libde265.dll)
    file(DOWNLOAD "https://github.com/novomesk/qt-heic-image-plugin/releases/latest/download/openjp2.dll" ${CMAKE_BINARY_DIR}/Release/openjp2.dll)
    file(DOWNLOAD "https://github.com/novomesk/qt-heic-image-plugin/releases/latest/download/openh264-7.dll" ${CMAKE_BINARY_DIR}/Release/openh264-7.dll)
    file(DOWNLOAD "https://github.com/novomesk/qt-jp2-image-plugin/releases/latest/download/kimg_jp2.dll" ${CMAKE_BINARY_DIR}/Release/imageformats/kimg_jp2.dll)
endif()

if (ENABLE_JXR)
    file(DOWNLOAD "https://github.com/novomesk/qt-jxr-image-plugin/releases/latest/download/kimg_jxr.dll" ${CMAKE_BINARY_DIR}/Release/imageformats/kimg_jxr.dll)
endif()

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
file(COPY ${QT_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/imageformats PATTERN *d.dll EXCLUDE)
file(COPY ${QT_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Debug/imageformats)

file(GLOB QT_EXTRA_IMAGE_FORMATS "${QT_QMAKE_PATH}/../../qtimageformats/plugins/imageformats/*.dll")
file(COPY ${QT_EXTRA_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats PATTERN *d.dll EXCLUDE)
file(COPY ${QT_EXTRA_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/imageformats PATTERN *d.dll EXCLUDE)
file(COPY ${QT_EXTRA_IMAGE_FORMATS} DESTINATION ${CMAKE_BINARY_DIR}/Debug/imageformats)

# *d.dll would exclude qpsd.dll - so copy this manually...
file(GLOB QT_PSD_LIB "${QT_QMAKE_PATH}/../plugins/imageformats/qpsd.dll")
file(COPY ${QT_PSD_LIB} DESTINATION ${CMAKE_BINARY_DIR}/Release/imageformats)

# Platforms
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/platforms)
file(COPY ${QT_QMAKE_PATH}/../plugins/platforms/qwindows.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/platforms/)
file(COPY ${QT_QMAKE_PATH}/../plugins/platforms/qwindows.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/platforms/)
file(COPY ${QT_QMAKE_PATH}/../plugins/platforms/qwindowsd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/platforms/)

# PrintSupport
#file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/printsupport)
#file(COPY ${QT_QMAKE_PATH}/../plugins/printsupport/windowsprintersupport.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/printsupport)
#file(COPY ${QT_QMAKE_PATH}/../plugins/printsupport/windowsprintersupport.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/printsupport)
#file(COPY ${QT_QMAKE_PATH}/../plugins/printsupport/windowsprintersupportd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/printsupport)

	# SVG support
	file(COPY ${QT_QMAKE_PATH}/Qt6Svg.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
	file(COPY ${QT_QMAKE_PATH}/Qt6Svg.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/)
	file(COPY ${QT_QMAKE_PATH}/Qt6Svgd.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/)

	# Themes
	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/styles)
	file(COPY ${QT_QMAKE_PATH}/../plugins/styles/qmodernwindowsstyle.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/styles/)
	file(COPY ${QT_QMAKE_PATH}/../plugins/styles/qmodernwindowsstyle.dll DESTINATION ${CMAKE_BINARY_DIR}/RelWithDebInfo/styles/)
	file(COPY ${QT_QMAKE_PATH}/../plugins/styles/qmodernwindowsstyled.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug/styles/)

# OpenSSL
if (NOT DEFINED ${OPEN_SSL_PATH} )
    set(OPEN_SSL_PATH "C:/OpenSSL-v111-Win64/bin")
    message(STATUS "defaulting open ssl path to ${OPEN_SSL_PATH}")
endif()

if (EXISTS ${OPEN_SSL_PATH})
    file(COPY ${OPEN_SSL_PATH}/libcrypto-1_1-x64.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
    file(COPY ${OPEN_SSL_PATH}/libssl-1_1-x64.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
    message(STATUS "open ssl found...")
endif()

# add default settings file
#file(COPY ${CMAKE_SOURCE_DIR}/src/default.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
#file(COPY ${CMAKE_SOURCE_DIR}/src/default.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
#file(COPY ${CMAKE_SOURCE_DIR}/src/default.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo)

# create settings file for portable version while working
if(NOT EXISTS ${CMAKE_BINARY_DIR}/Debug/settings.ini)
	file(WRITE ${CMAKE_BINARY_DIR}/Debug/settings.ini "")
endif()

# copy translation files after each build
add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory \"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/translations/\")
foreach(QM ${NOMACS_QM})
	add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"${QM}\" \"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/translations/\")
endforeach(QM)

if (Python_FOUND)
    # add version for install scripts
    add_custom_command(
        TARGET ${DLL_CORE_NAME} PRE_BUILD 
        COMMAND ${Python_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/versionupdate.py 
        ${CMAKE_CURRENT_SOURCE_DIR}/../installer/nomacs-setup.wxi
        --copy
        )

    add_custom_command(
        TARGET ${DLL_CORE_NAME} PRE_BUILD 
        COMMAND ${Python_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/versionupdate.py 
        ${CMAKE_CURRENT_SOURCE_DIR}/../installer/nomacs-setup.iss
        --copy
        )
endif()

# set properties for Visual Studio Projects
set(CMAKE_CXX_FLAGS_DEBUG "/Zc:wchar_t /W4 /EHsc ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "/Zc:wchar_t /W4 /O2 /EHsc -DDK_INSTALL -DQT_NO_DEBUG_OUTPUT ${CMAKE_CXX_FLAGS_RELEASE}")

source_group("Generated Files" FILES ${NOMACS_RCC} ${NOMACS_RC} ${NOMACS_QM} ${NOMACS_AUTOMOC})
source_group("Translations" FILES ${NOMACS_TRANSLATIONS})
source_group("Changelog" FILES ${CHANGLOG_FILE})

# generate configuration file
if(DLL_CORE_NAME)
	get_property(CORE_DEBUG_NAME TARGET ${DLL_CORE_NAME} PROPERTY DEBUG_OUTPUT_NAME)
	get_property(CORE_RELEASE_NAME TARGET ${DLL_CORE_NAME} PROPERTY RELEASE_OUTPUT_NAME)
	set(NOMACS_CORE_LIB optimized ${CMAKE_BINARY_DIR}/libs/Release/${CORE_RELEASE_NAME}.lib debug  ${CMAKE_BINARY_DIR}/libs/Debug/${CORE_DEBUG_NAME}.lib)
endif()

set(NOMACS_LIBS ${NOMACS_CORE_LIB})
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui ${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore ${CMAKE_CURRENT_SOURCE_DIR}/src/DkLoader ${CMAKE_BINARY_DIR})
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)

# CMAKE_MAKE_PROGRAM works for VS 2017 too
get_filename_component(VS_PATH ${CMAKE_MAKE_PROGRAM} PATH)
if(CMAKE_CL_64)
	set(VS_PATH "${VS_PATH}/../../../Common7/IDE/Remote Debugger/x64")
else()
	set(VS_PATH "${VS_PATH}/../../Common7/IDE/Remote Debugger/x86")
endif()
