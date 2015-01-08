# this macro is copied from OpenCV - OpenCVUtils.cmake
# Provides an option that the user can optionally select.
# Can accept condition to control when option is available for user.
# Usage:
#   option(<option_variable> "help string describing the option" <initial value or boolean expression> [IF <condition>])
macro(NMC_OPTION variable description value)
  set(__value ${value})
  set(__condition "")
  set(__varname "__value")
  foreach(arg ${ARGN})
    if(arg STREQUAL "IF" OR arg STREQUAL "if")
      set(__varname "__condition")
    else()
      list(APPEND ${__varname} ${arg})
    endif()
  endforeach()
  unset(__varname)
  if("${__condition}" STREQUAL "")
    set(__condition 2 GREATER 1)
  endif()

  if(${__condition})
    if("${__value}" MATCHES ";")
      if(${__value})
        option(${variable} "${description}" ON)
      else()
        option(${variable} "${description}" OFF)
      endif()
    elseif(DEFINED ${__value})
      if(${__value})
        option(${variable} "${description}" ON)
      else()
        option(${variable} "${description}" OFF)
      endif()
    else()
      option(${variable} "${description}" ${__value})
    endif()
  else()
    unset(${variable} CACHE)
  endif()
  unset(__condition)
  unset(__value)
endmacro(NMC_OPTION)

# Searches for Qt with the required components
macro(NMC_FINDQT)

	# set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	# find the Qt
	if (NOT ENABLE_QT5)
		set(QT_MIN_VERSION "4.7.0")
		set(QT_USE_GUI 1)
		set(QT_USE_QTNETWORK 1)
		if(ENABLE_UPNP)
			find_package(Qt4 REQUIRED QtCore QtGui QtNetwork QtXml)
		else()
			 find_package(Qt4 REQUIRED QtCore QtGui QtNetwork)
		endif(ENABLE_UPNP)
		
		find_package(Qt4 REQUIRED QtCore QtGui QtNetwork)
		
		set(QT_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY})
		include(${QT_USE_FILE})
	else()
		if(NOT QT_QMAKE_EXECUTABLE)
			find_program(QT_QMAKE_EXECUTABLE NAMES "qmake" "qmake-qt5" "qmake.exe")
		endif()
		if(NOT QT_QMAKE_EXECUTABLE)
			message(FATAL_ERROR "you have to set the path to the Qt5 qmake executable")
		endif()
		
		set(CMAKE_INCLUDE_CURRENT_DIR ON)
		message(STATUS "qmake path: ${QT_QMAKE_EXECUTABLE}")
		GET_FILENAME_COMPONENT(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
		set(CMAKE_PREFIX_PATH ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5Widgets ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5Network ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5LinguistTools ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5PrintSupport ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5Concurrent)
		message(STATUS "path: ${QT_QMAKE_PATH}")
		message(STATUS "cmake path: ${CMAKE_PREFIX_PATH}")
		
		find_package(Qt5Widgets)
		find_package(Qt5Network)
		find_package(Qt5LinguistTools)
		find_package(Qt5PrintSupport)
		find_package(Qt5Concurrent)
		
		set(QT_INCLUDES ${Qt5Widgets_INCLUDE_DIRS} ${Qt5LinguistTools_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5PrintSupport_INCLUDE_DIRS} ${Qt5Concurrent_INCLUDE_DIRS})
		set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Concurrent_LIBRARIES} ${Qt5Core_LIBARIES} ${Qt5Gui_LIBRARIES} ${Qt5PrintSupport_LIBRARIES} ${Qt5Widgets_LIBRARIES})

		message(STATUS "QT_INCLUDES: ${QT_INCLUDES}")
		add_definitions(-DQT5)
	endif(NOT ENABLE_QT5)
	
endmacro(NMC_FINDQT)

# enables webp
# macro(NMC_ENABLE_WEBP)
	# SET(WEBP_INCLUDE_DIR "")
	# SET(WEBP_SOURCE "")
	# if(ENABLE_WEBP)
		# add_definitions(-DNDEBUG -DWEBP_USE_THREAD)
	
		# file(GLOB WEBP_DEC_SRCS
			# RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			# ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/dec/*c
		# )
		
		# file(GLOB WEBP_DEMUX_SRCS
			# RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			# ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/demux/*c
		# )
	
		# file(GLOB WEBP_DSP_SRCS
			# RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			# ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/dsp/*c
		# )
	
		# file(GLOB WEBP_ENC_SRCS
			# RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			# ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/enc/*c
		# )
	
		# file(GLOB WEBP_UTILS_SRCS
			# RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			# ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/utils/*c
		# )
	
		# file(GLOB WEBP_MUX_SRCS
			# RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			# ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/mux/*c
		# )
		# set(WEBP_SOURCE ${WEBP_DEC_SRCS} ${WEBP_DEMUX_SRCS} ${WEBP_DSP_SRCS} ${WEBP_ENC_SRCS} ${WEBP_UTILS_SRCS} ${WEBP_MUX_SRC})
		# set(WEBP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src)
		# add_definitions(-DWITH_WEBP)
	# endif(ENABLE_WEBP)
# endmacro(NMC_ENABLE_WEBP)


