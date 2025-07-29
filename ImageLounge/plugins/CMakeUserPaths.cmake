# If you want to use prefix paths with cmake, copy and rename this file to CMakeUser.txt
# Do not add this file to GIT!

# set your preferred paths
if(CMAKE_CL_64)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/Qt-5.14.1/bin")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/coding/3rd-party/opencv/build2019-x64")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/coding/nomacs/nomacs/build2019-x64/")
else()
endif()
