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
 GET_FILENAME_COMPONENT(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
 set(QT_ROOT ${QT_QMAKE_PATH}/)
 SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5)
 find_package(Qt5 REQUIRED Widgets Network LinguistTools PrintSupport Concurrent Gui Svg)
 if (NOT Qt5_FOUND)
	message(FATAL_ERROR "Qt5Widgets not found. Check your QT_QMAKE_EXECUTABLE path and set it to the correct location")
 endif()
 add_definitions(-DQT5)
 
endmacro(NMC_FINDQT)

macro(NMC_INSTALL)
	SET(NOMACS_INSTALL_DIRECTORY ${CMAKE_SOURCE_DIR}/../installer CACHE PATH "Path to the installer directory")

	if (MSVC)
		set(PACKAGE_DIR ${NOMACS_INSTALL_DIRECTORY}/packages/${PROJECT_NAME}.${NMC_ARCHITECTURE})
		set(DATA_PACKAGE_DIR ${PACKAGE_DIR}/data/nomacs-${NMC_ARCHITECTURE})
		install(TARGETS ${PROJECT_NAME} ${DLL_CORE_NAME} RUNTIME DESTINATION ${DATA_PACKAGE_DIR} CONFIGURATIONS Release)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/package.xml DESTINATION ${PACKAGE_DIR}/meta CONFIGURATIONS Release)

		if (NOT GLOBAL_READ_BUILD)
			install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release/ DESTINATION ${DATA_PACKAGE_DIR})
		endif()
		
	endif (MSVC)

endmacro(NMC_INSTALL)

macro(NMC_GENERATE_PACKAGE_XML)

	string(TIMESTAMP CURRENT_DATE "%Y-%m-%d")	
	
	set(XML_CONTENT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
	set(XML_CONTENT "${XML_CONTENT}<Package>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<DisplayName>${PROJECT_NAME} [${NMC_ARCHITECTURE}]</DisplayName>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<Description>nomacs for ${NMC_ARCHITECTURE} systems.</Description>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<Version>${NOMACS_FULL_VERSION}</Version>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<ReleaseDate>${CURRENT_DATE}</ReleaseDate>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<Default>true</Default>\n")
	set(XML_CONTENT "${XML_CONTENT}</Package>\n")
	
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/package.xml ${XML_CONTENT})
	
endmacro(NMC_GENERATE_PACKAGE_XML)
