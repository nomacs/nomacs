# If you want to use prefix paths with cmake, copy and rename this file to CMakeUser.cmake
# Do not add this file to GIT!

# set your preferred Qt Library path
IF (CMAKE_CL_64)
	SET(CMAKE_PREFIX_PATH "D:/Qt/qt-everywhere-opensource-src-5.5.0-x64/qtbase/bin/")
ELSE ()
	SET(CMAKE_PREFIX_PATH "D:/Qt/qt-everywhere-opensource-src-5.5.0-x86/qtbase/bin/")
ENDIF ()

# set your preferred OpenCV Library path
IF (CMAKE_CL_64)
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/VSProjects/OpenCV/build2015x64")
ELSE ()
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/VSProjects/OpenCV/build2015x86")
ENDIF ()

# set your preferred HUpnp path
IF (CMAKE_CL_64)
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/VSProjects/nomacs.git/herqq/build2012x64")
ELSE ()
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/VSProjects/nomacs.git/herqq/build2012x86")
ENDIF ()
