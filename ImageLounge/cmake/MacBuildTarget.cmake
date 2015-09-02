
#creae the targets
set(BINARY_NAME ${CMAKE_PROJECT_NAME})
link_directories(${LIBRAW_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS} ${EXIV2_LIBRARY_DIRS})
add_executable(${BINARY_NAME} WIN32 MACOSX_BUNDLE ${NOMACS_SOURCES} ${NOMACS_UI} ${NOMACS_RESOURCES} ${NOMACS_HEADERS} ${NOMACS_RC} ${NOMACS_QM} ${NOMACS_TRANSLATIONS} ${LIBQPSD_SOURCES} ${LIBQPSD_HEADERS} ${WEBP_SOURCE} ${QUAZIP_SOURCES} ${ZLIB_LIBRARIES})
target_include_directories(${BINARY_NAME} PRIVATE  ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${BINARY_NAME} ${QT_LIBRARIES} ${EXIV2_LIBRARIES} ${LIBRAW_LIBRARIES} ${OpenCV_LIBS} ${VERSION_LIB} ${TIFF_LIBRARIES} ${ZLIB_LIBRARIES})

if (ENABLE_QT5)
  qt5_use_modules(${BINARY_NAME} Widgets Gui Network LinguistTools PrintSupport Concurrent)
ENDIF()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")

# mac's bundle install
set_target_properties(${BINARY_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/macosx/Info.plist.in")
set(MACOSX_BUNDLE_ICON_FILE nomacs.icns)
set(MACOSX_BUNDLE_INFO_STRING "${BINARY_NAME} ${NOMACS_VERSION}")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.nomacs")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_NAME "${BINARY_NAME}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${NOMACS_VERSION}")
set(MACOSX_BUNDLE_COPYRIGHT "(c) Nomacs team")
set_source_files_properties(${CMAKE_SOURCE_DIR}/macosx/nomacs.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

install(TARGETS ${BINARY_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})

# create a "transportable" bundle - all libs into the bundle: "make bundle" after make install
configure_file(${CMAKE_SOURCE_DIR}/macosx/bundle.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/bundle.cmake @ONLY)
add_custom_target(bundle ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/bundle.cmake)

# generate configuration file
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)
