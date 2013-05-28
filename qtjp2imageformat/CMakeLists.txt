# works for windows
# untested (and should NOT work :( ) for linux/unix

PROJECT(QTJP2)

# don't know if we need this
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# find the Qt
set(QT_MIN_VERSION "4.7.0")
set(QT_USE_GUI 1)
set(QT_USE_QTNETWORK 1)
find_package(Qt4 REQUIRED QtCore QtGui)

include(${CMAKE_CURRENT_SOURCE_DIR}/../jasper-1.900.1/libjasper.cmake)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src/
	${QT_INCLUDES}
	${LIBJASPER_INCLUDE_DIRECTORY}
)




file(GLOB_RECURSE QTJP2_SOURCES "plugin/*.cpp" "src/*.cpp")
file(GLOB_RECURSE QTJP2_HEADERS "plugin/*.h" "src/*.h")

add_library(qtjp2 SHARED ${QTJP2_SOURCES} ${QTJP2_HEADERS})
target_link_libraries(qtjp2 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY})
target_link_libraries(qtjp2 debug ${LIBJASPER_DEBUG_LIB})
target_link_libraries(qtjp2 optimized ${LIBJASPER_RELEASE_LIB})


# if(MSVC)
	# add_custom_command(TARGET qtjp2 POST_BUILD COMMAND xcopy \"${CMAKE_CURRENT_BINARY_DIR}/Release/qtjp2.dll\" \"${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release/imageformats\")
	# add_custom_command(TARGET qtjp2 POST_BUILD COMMAND xcopy \"${CMAKE_CURRENT_BINARY_DIR}/Release/qtjp2.lib\" \"${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release/imageformats\")
	# add_custom_command(TARGET qtjp2 POST_BUILD COMMAND xcopy \"${CMAKE_CURRENT_BINARY_DIR}/Debug/qtjp2.dll\" \"${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release/imageformats\")
	# add_custom_command(TARGET qtjp2 POST_BUILD COMMAND xcopy \"${CMAKE_CURRENT_BINARY_DIR}/Debug/qtjp2.lib\" \"${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release/imageformats\")
# endif(MSVC)	

if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Release/qtjp2.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/qtjp2.lib  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../bin/Release/imageformats CONFIGURATIONS Release)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/qtjp2.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/qtjp2.lib  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../bin/Debug/imageformats CONFIGURATIONS Debug)
endif(MSVC)	

# ADD_CUSTOM_TARGET(install_qtjp2 ALL
    # COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake)
# ADD_DEPENDENCIES(install_qtjp2 qtjp2)