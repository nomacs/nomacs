PROJECT(nomacs-plugins)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

add_subdirectory(jasper-1.900.1)
add_subdirectory(qtjp2imageformat)
add_dependencies(qtjp2 libjasper)
