# If you want to use prefix paths with cmake, copy and rename this file to CMakeUserPaths.cmake
# Do not add this file to GIT!

IF (CMAKE_CL_64)

	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/Qt5.11.1-x64/bin")
ELSE()

	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/Qt5.11.1-x86/bin")
ENDIF()
