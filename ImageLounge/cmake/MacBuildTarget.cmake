if(ENABLE_PLUGINS)
	add_definitions(-DWITH_PLUGINS)
endif()

# macOS install names for dylibs will include @rpath/ prefix.
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling#default-rpath-settings
set(MACOSX_RPATH TRUE)
set(INSTALL_NAME_DIR "@executable_path/../Frameworks")

# /usr/local is occupied by homebrew usually, not a good default
set(CMAKE_INSTALL_PREFIX "/Applications")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")

# create the targets
set(BINARY_NAME ${PROJECT_NAME})
set(DLL_CORE_NAME ${PROJECT_NAME}Core-${NOMACS_FULL_VERSION})

set(NOMACS_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/macosx/nomacs.icns")

set(MACOSX_FILETYPES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/macosx/filetypes.xml")

# binary
link_directories(${LIBRAW_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS} ${EXIV2_LIBRARY_DIRS} ${CMAKE_BINARY_DIR})
add_executable(
	${BINARY_NAME} MACOSX_BUNDLE
	${NOMACS_EXE_SOURCES}
	${NOMACS_EXE_HEADERS}
	${NOMACS_QM}
	${NOMACS_TRANSLATIONS}
	${NOMACS_RC}
	${NOMACS_ICON_FILE}
	${NOMACS_THEMES}
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

# add core dll
add_library(
	${DLL_CORE_NAME} SHARED
	${GUI_SOURCES} ${GUI_HEADERS}
	${CORE_SOURCES} ${CORE_HEADERS}
	${NOMACS_RCC} ${NOMACS_RC}
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

add_dependencies(
	${BINARY_NAME}
	${DLL_CORE_NAME}
	${QUAZIP_DEPENDENCY}
	${LIBQPSD_LIBRARY}
	)

target_link_libraries(${BINARY_NAME} Qt::Widgets Qt::Gui Qt::Network Qt::PrintSupport Qt::Concurrent Qt::Svg)
target_link_libraries(${DLL_CORE_NAME} Qt::Widgets Qt::Gui Qt::Network Qt::PrintSupport Qt::Concurrent Qt::Svg)

# core flags
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs) # just create that it exists and we get no warning from linker
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/libs)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/libs)

set_target_properties(${DLL_CORE_NAME} PROPERTIES COMPILE_FLAGS "-DDK_CORE_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_CORE_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_CORE_NAME}d)
set_target_properties(${DLL_CORE_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_CORE_NAME})

# mac's bundle install

set_target_properties(${BINARY_NAME} PROPERTIES MACOSX_BUNDLE ON)
set_target_properties(${BINARY_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macosx/Info.plist.in")
set(MACOSX_BUNDLE_ICON_FILE "nomacs.icns")
set(MACOSX_BUNDLE_INFO_STRING "${BINARY_NAME} ${NOMACS_VERSION}")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.nomacs")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_NAME "${BINARY_NAME}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_COPYRIGHT "(c) Nomacs team")
file(READ ${MACOSX_FILETYPES_FILE} MACOSX_BUNDLE_FILETYPES_XML) # refresh with "make filetypes" on the guest system

set_source_files_properties(${NOMACS_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${NOMACS_THEMES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/themes)
set_source_files_properties(${NOMACS_QM} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/translations)

# FIXME: to install nomacs requires "make bundle" before "make install", libraries won't be found otherwise
#install(TARGETS ${BINARY_NAME} ${DLL_CORE_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS ${BINARY_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})

# install plugins in the bundle, note this path is hardcoded in nomacs as well
set(PLUGINS_BUNDLE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.app/Contents/PlugIns/nomacs")

# generate configuration file
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_LIBS ${DLL_CORE_NAME})
set(NOMACS_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui ${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore ${CMAKE_BINARY_DIR})
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)

# create macOS bundle
# Thanks to https://the-codeslinger.com/2023/02/04/create-macos-app-bundle-with-app-icon-and-dmg-using-cmake-and-qt6/
get_target_property(_qmake_executable Qt::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

add_custom_target(bundle
	COMMAND ${MACDEPLOYQT_EXECUTABLE} ${BINARY_NAME}.app -always-overwrite -dmg
	DEPENDS ${BINARY_NAME}
	COMMENT "Building portable bundle and dmg")

add_custom_target(
	filetypes
	COMMAND ./${BINARY_NAME}.app/Contents/MacOS/nomacs --list-formats plist "${MACOSX_FILETYPES_FILE}"
	COMMAND rm -r ./${BINARY_NAME}.app/Contents/Info.plist
	COMMAND echo filetypes updated, re-run \"make\" to update Info.plist
	DEPENDS ${BINARY_NAME}
	COMMENT "Generating filetypes.xml")

# this macro must appear after add_subdirectory(<plugins-path>)
macro(NMC_BUNDLE_COPY_PLUGINS)

	# find plugin targets, not exposed in any cmake variable I could find
	# so use this helper
	set_property(GLOBAL PROPERTY COLLECTED_TARGETS "")
	collect_dir_targets("${PLUGINS_DIR}")
	get_property(PLUGINS_TARGETS GLOBAL PROPERTY COLLECTED_TARGETS)

	# naming convention for plugins output is "libxxx.dylib"
	set(PLUGINS_FILES "")
	foreach(plugin_target ${PLUGINS_TARGETS})
		list(APPEND PLUGINS_FILES "${CMAKE_CURRENT_BINARY_DIR}/plugins/lib${plugin_target}.dylib")
	endforeach()

	# use rsync to deref the symlink and keep the same filename
	add_custom_target(
		  copy_bundle_plugins ALL
			COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGINS_BUNDLE_DIR}"
			COMMAND rsync -aL ${PLUGINS_FILES} "${PLUGINS_BUNDLE_DIR}")

	# make our target run last, after compiling plugins
	add_dependencies(copy_bundle_plugins ${BINARY_NAME})
	add_dependencies(copy_bundle_plugins ${DLL_CORE_NAME})

	foreach(plugin_target ${PLUGINS_TARGETS})
		add_dependencies(copy_bundle_plugins ${plugin_target})
	endforeach()

endmacro()
