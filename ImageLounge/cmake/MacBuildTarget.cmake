if(ENABLE_PLUGINS)
	add_definitions(-DWITH_PLUGINS)
endif()

#taken from https://cmake.org/Wiki/CMake_RPATH_handling to avoid warning
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "")

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")

# create the targets
set(BINARY_NAME ${PROJECT_NAME})
set(DLL_CORE_NAME ${PROJECT_NAME}Core)

#binary
link_directories(${LIBRAW_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS} ${EXIV2_LIBRARY_DIRS} ${CMAKE_BINARY_DIR})
add_executable(${BINARY_NAME} WIN32  MACOSX_BUNDLE ${NOMACS_EXE_SOURCES} ${NOMACS_EXE_HEADERS} ${NOMACS_QM} ${NOMACS_TRANSLATIONS} ${NOMACS_RC} ${QUAZIP_SOURCES})
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

# add core
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
	${LIBQPSD_LIBRARY}) 

target_link_libraries(${BINARY_NAME} Qt::Widgets Qt::Gui Qt::Network Qt::PrintSupport Qt::Concurrent Qt::Svg)
target_link_libraries(${DLL_CORE_NAME} Qt::Widgets Qt::Gui Qt::Network Qt::PrintSupport Qt::Concurrent Qt::Svg)
if(${QT_VERSION_MAJOR} VERSION_GREATER_EQUAL "6")
    target_link_libraries(${BINARY_NAME} Qt6::Core5Compat)
    target_link_libraries(${DLL_CORE_NAME} Qt6::Core5Compat)
endif()

# core flags
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/libs)
set_target_properties(${DLL_CORE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/libs)

set_target_properties(${DLL_CORE_NAME} PROPERTIES COMPILE_FLAGS "-DDK_CORE_DLL_EXPORT -DNOMINMAX")
set_target_properties(${DLL_CORE_NAME} PROPERTIES DEBUG_OUTPUT_NAME ${DLL_CORE_NAME}d)
set_target_properties(${DLL_CORE_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${DLL_CORE_NAME})

# mac's bundle install
set_target_properties(${BINARY_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macosx/Info.plist.in")
set(MACOSX_BUNDLE_ICON_FILE nomacs.icns)
set(MACOSX_BUNDLE_INFO_STRING "${BINARY_NAME} ${NOMACS_VERSION}")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.nomacs")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_NAME "${BINARY_NAME}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_COPYRIGHT "(c) Nomacs team")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/macosx/nomacs.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

install(TARGETS ${BINARY_NAME} ${DLL_CORE_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX})

# create a "transportable" bundle - all libs into the bundle: "make bundle" after make install
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/macosx/bundle.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/bundle.cmake @ONLY)
add_custom_target(bundle ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/bundle.cmake)

# generate configuration file
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_LIBS ${DLL_CORE_NAME})
set(NOMACS_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/DkGui ${CMAKE_CURRENT_SOURCE_DIR}/src/DkCore ${CMAKE_BINARY_DIR})
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)
