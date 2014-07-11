
#create the targets
set(BINARY_NAME ${CMAKE_PROJECT_NAME})
link_directories(${LIBRAW_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS} ${EXIV2_LIBRARY_DIRS})
add_executable(${BINARY_NAME} WIN32 MACOSX_BUNDLE ${NOMACS_SOURCES} ${NOMACS_UI} ${NOMACS_MOC_SRC} ${NOMACS_RCC} ${NOMACS_HEADERS} ${NOMACS_RC} ${NOMACS_QM} ${NOMACS_TRANSLATIONS} ${LIBQPSD_SOURCES} ${LIBQPSD_HEADERS} ${LIBQPSD_MOC_SRC} ${WEBP_SOURCE})
target_link_libraries(${BINARY_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} ${EXIV2_LIBRARIES} ${LIBRAW_LIBRARIES} ${OpenCV_LIBS} ${VERSION_LIB} ${TIFF_LIBRARY})

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	SET_TARGET_PROPERTIES(${BINARY_NAME} PROPERTIES LINK_FLAGS -fopenmp)
endif()

# installation
#  binary
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX})
#  desktop file
install(FILES nomacs.desktop DESTINATION share/applications)
#  icon
install(FILES src/img/nomacs.png DESTINATION share/pixmaps)
#  translations
install(FILES ${NOMACS_QM} DESTINATION share/nomacs/translations)
#  manpage
install(FILES Readme/nomacs.1 DESTINATION share/man/man1)


# "make dist" target
string(TOLOWER ${CMAKE_PROJECT_NAME} CPACK_PACKAGE_NAME)
set(CPACK_PACKAGE_VERSION "${NOMACS_VERSION}")
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;refresh-copyright-and-license.pl;build;release;")
set(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
include(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)


# generate configuration file
set(NOMACS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NOMACS_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})
configure_file(${NOMACS_SOURCE_DIR}/nomacs.cmake.in ${CMAKE_BINARY_DIR}/nomacsConfig.cmake)
