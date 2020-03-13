# If you want to use prefix paths with cmake, copy and rename this file to CMakeUser.txt
# Do not add this file to GIT!

# set your preferred paths
IF (CMAKE_CL_64)
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/Qt5.11.1-x64/bin")
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/coding/3rd-party/opencv/build2019-x64/")
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/coding/nomacs/build2019-x64/")
ELSE()
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/qt-everywhere-opensource-src-5.8.0-x86/qtbase/bin/")
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/coding/3rd-party/opencv/build2019-x86/")
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/coding/nomacs/build2019-x86/")	
ENDIF()
