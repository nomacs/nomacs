# Searches for Qt with the required components
macro(NMC_FINDQT)
	
	unset(QT_QTCORE_LIBRARY CACHE)
	unset(QT_QTGUI_LIBRARY CACHE)
	unset(QT_QTNETWORK_LIBRARY CACHE)
	unset(QT_QTMAIN_LIBRARY CACHE)

	# set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	if(NOT QT_QMAKE_EXECUTABLE)
		find_program(QT_QMAKE_EXECUTABLE NAMES "qmake" "qmake-qt5" "qmake.exe")
	endif()
	if(NOT QT_QMAKE_EXECUTABLE)
		message(FATAL_ERROR "you have to set the path to the Qt5 qmake executable")
	endif()
	
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	# message(STATUS "qmake path: ${QT_QMAKE_EXECUTABLE}")
	GET_FILENAME_COMPONENT(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
	set(CMAKE_PREFIX_PATH ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5Widgets ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5Network ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5LinguistTools ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5PrintSupport ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5Concurrent)
	# message(STATUS "path: ${QT_QMAKE_PATH}")
	# message(STATUS "cmake path: ${CMAKE_PREFIX_PATH}")
	
	unset(Qt5Widgets_FOUND CACHE)
	find_package(Qt5Widgets)
	find_package(Qt5Network)
	find_package(Qt5LinguistTools)
	find_package(Qt5PrintSupport)
	find_package(Qt5Concurrent)
	
	set(QT_INCLUDES ${Qt5Widgets_INCLUDE_DIRS} ${Qt5LinguistTools_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5PrintSupport_INCLUDE_DIRS} ${Qt5Concurrent_INCLUDE_DIRS})
	set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Concurrent_LIBRARIES} ${Qt5Core_LIBARIES} ${Qt5Gui_LIBRARIES} ${Qt5PrintSupport_LIBRARIES} ${Qt5Widgets_LIBRARIES})
	if (NOT Qt5Widgets_FOUND)
		message(FATAL_ERROR "Qt5Widgets not found. Check your QT_QMAKE_EXECUTABLE path and set it to the correct location")
	endif()
	# message(STATUS "QT_INCLUDES: ${QT_INCLUDES}")
	add_definitions(-DQT5)
	
endmacro(NMC_FINDQT)

# Searches for Qt with the required components
macro(NMC_FIND_OPENCV)

# search for opencv
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_OPT CACHE)
unset(OpenCV_CONFIG_PATH CACHE)
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_LIB_DIR_OPT CACHE)
unset(OpenCV_LIBRARY_DIRS CACHE)

find_package(OpenCV 2.1.0 REQUIRED core imgproc)
if(OpenCV_VERSION VERSION_LESS 2.4.0 AND OpenCV_FOUND) # OpenCV didn't allow to define packages before version 2.4.0 ... nomacs was linking against all libs even if they were not compiled -> error
	string(REGEX REPLACE "\\." "" OpenCV_SHORT_VERSION ${OpenCV_VERSION})
	set(OpenCV_LIBS "debug;opencv_imgproc${OpenCV_SHORT_VERSION}d;optimized;opencv_imgproc${OpenCV_SHORT_VERSION};debug;opencv_core${OpenCV_SHORT_VERSION}d;optimized;opencv_core${OpenCV_SHORT_VERSION};" FORCE)
endif()
if(OpenCV_VERSION VERSION_GREATER 2.4.7 AND OpenCV_FOUND) # OpenCV cmake does not define optimized and debug libs any longer (reallyrelease not working), thus define them ourselves
	unset(OpenCV_LIBS)
	string(REGEX REPLACE "\\." "" OpenCV_SHORT_VERSION ${OpenCV_VERSION})
	foreach(lib ${OpenCV_FIND_COMPONENTS_})
		set(OpenCV_LIBS "${OpenCV_LIBS}debug;${lib}${OpenCV_SHORT_VERSION}d.lib;optimized;${lib}${OpenCV_SHORT_VERSION}.lib;" FORCE)
	endforeach()
endif()
SET(OpenCV_LIBRARY_DIRS ${OpenCV_LIBRARY_DIRS} ${OpenCV_LIB_DIR_DBG} ${OpenCV_LIB_DIR_OPT} ${OpenCV_DIR}/lib/${OpenCV_LIB_DIR_DBG} ${OpenCV_DIR}/lib/${OpenCV_LIB_DIR_OPT})
if(NOT OpenCV_FOUND)
	message(FATAL_ERROR "OpenCV not found.") 
else()
	add_definitions(-DWITH_OPENCV)
endif()

if(${OpenCV_VERSION} EQUAL "2.1.0")
	add_definitions(-DDISABLE_LANCZOS)
endif()

# unset include directories since OpenCV sets them global
get_property(the_include_dirs  DIRECTORY . PROPERTY INCLUDE_DIRECTORIES)
list(REMOVE_ITEM the_include_dirs ${OpenCV_INCLUDE_DIRS})
set_property(DIRECTORY . PROPERTY INCLUDE_DIRECTORIES ${the_include_dirs})

endmacro(NMC_FIND_OPENCV)