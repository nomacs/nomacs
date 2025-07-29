# If you want to use prefix paths with cmake, copy and rename this file to CMakeUser.txt
# Do not add this file to GIT!

# set your preferred paths
if(CMAKE_CL_64)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "D:/Qt/qt-everywhere-opensource-src-5.8.0-x64/qtbase/bin/")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "D:/coding/3rd-party/opencv/build2017-x64/")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "D:/coding/nomacs/build2017-x64/")
else()
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "D:/Qt/qt-everywhere-opensource-src-5.8.0-x86/qtbase/bin/")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "D:/coding/3rd-party/opencv/build2017-x86/")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "D:/coding/nomacs/build2017-x86/")
endif()
