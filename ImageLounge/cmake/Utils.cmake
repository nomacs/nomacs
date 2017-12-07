# Searches for Qt with the required components
macro(NMC_FINDQT)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC OFF)
	# set(CMAKE_INCLUDE_CURRENT_DIR ON)
 
	 if(NOT QT_QMAKE_EXECUTABLE)
		find_program(QT_QMAKE_EXECUTABLE NAMES "qmake" "qmake-qt5" "qmake.exe")
	 endif()
	 if(NOT QT_QMAKE_EXECUTABLE)
		message(FATAL_ERROR "you have to set the path to the Qt5 qmake executable")
	 endif()
	 
	 message(STATUS "QMake found: ${QT_QMAKE_EXECUTABLE}")
	 get_filename_component(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
	 
	 set(QT_ROOT ${QT_QMAKE_PATH}/)
	 set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5)
	 	 
	 find_package(Qt5 REQUIRED  Core Widgets Network LinguistTools PrintSupport Concurrent Gui Svg)
	 
	if (MSVC)
		find_package(Qt5 REQUIRED WinExtras)
	endif()
	 
	 if (NOT Qt5_FOUND)
		message(FATAL_ERROR "Qt5Widgets not found. Check your QT_QMAKE_EXECUTABLE path and set it to the correct location")
	 endif()
	 add_definitions(-DQT5)
	 
endmacro(NMC_FINDQT)

macro(NMC_INSTALL)
	set(NOMACS_INSTALL_DIRECTORY ${CMAKE_SOURCE_DIR}/../installer/ CACHE PATH "Path to the installer directory")

	if (MSVC)
		set(PACKAGE_DIR ${NOMACS_INSTALL_DIRECTORY}/${PROJECT_NAME}.${NMC_ARCHITECTURE})
		install(TARGETS ${PROJECT_NAME} ${DLL_CORE_NAME} RUNTIME DESTINATION ${PACKAGE_DIR} CONFIGURATIONS Release)

		if (NOT GLOBAL_READ_BUILD)
			install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release/ DESTINATION ${PACKAGE_DIR})
		endif()
		
	endif (MSVC)

endmacro(NMC_INSTALL)
