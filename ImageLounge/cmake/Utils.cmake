# Searches for Qt with the required components
macro(NMC_FINDQT)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC OFF)
	# set(CMAKE_INCLUDE_CURRENT_DIR ON)
 
	if (MSVC)
		if(NOT QT_QMAKE_EXECUTABLE)
		find_program(QT_QMAKE_EXECUTABLE NAMES "qmake" "qmake-qt5" "qmake.exe")
		endif()
		if(NOT QT_QMAKE_EXECUTABLE)
		message(FATAL_ERROR "you have to set the path to the Qt5 qmake executable")
		endif()
		
		message(STATUS "QMake found: ${QT_QMAKE_EXECUTABLE}")
		get_filename_component(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
	 endif()

	 set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5)
	 	 
	 find_package(Qt5 ${QT5_MIN_VERSION} REQUIRED COMPONENTS Core Widgets Network LinguistTools PrintSupport Concurrent Gui Svg)
	 
	if (MSVC)
		find_package(Qt5 ${QT5_MIN_VERSION} REQUIRED WinExtras)
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
		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release/ DESTINATION ${PACKAGE_DIR})
		
	endif (MSVC)

endmacro(NMC_INSTALL)

macro(NMC_COPY_FILES)

# copy all themes
add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory \"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/themes/\")

foreach(CSS ${NOMACS_THEMES})
	message(STATUS "${CSS} added...")
	add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"${CSS}\" \"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/themes/\")
endforeach()

endmacro(NMC_COPY_FILES)