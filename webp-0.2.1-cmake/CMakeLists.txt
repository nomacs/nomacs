cmake_minimum_required(VERSION 2.8.8)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(webp C)

set(VERSION 1.9.9)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

#set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-O3"
#	-Wextra -Wold-style-definition 
#	-Wmissing-prototypes
#	-Wmissing-declarations
#	-Wdeclaration-after-statement
#)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wall)

#-DUNICODE

ADD_DEFINITIONS(-DNDEBUG -DWEBP_USE_THREAD)
#ADD_DEFINITIONS(-DWEBP_EXPERIMENTAL_FEATURES)

FILE(GLOB WEBP_DEC_SRCS
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/dec/*c
)

FILE(GLOB WEBP_DSP_SRCS
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/*c
)

#LIST(REMOVE_ITEM WEBP_DSP_SRCS
 # src/dsp/dec_neon.c
  #src/dsp/dec_sse2.c
  #src/dsp/enc_sse2.c
  #src/dsp/upsampling_sse2.c  
  #)

FILE(GLOB WEBP_ENC_SRCS
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/enc/*c
)

FILE(GLOB WEBP_UTILS_SRCS
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*c
)

FILE(GLOB WEBP_MUX_SRCS
	RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/mux/*c
)

add_library(webp STATIC ${WEBP_DEC_SRCS} ${WEBP_DSP_SRCS} ${WEBP_ENC_SRCS} ${WEBP_UTILS_SRCS})
target_link_libraries(webp m pthread)

add_library(webpmux STATIC ${WEBP_MUX_SRCS})
target_link_libraries(webpmux m pthread)


add_executable(vwebp sample/vwebp.c sample/example_util.c)
target_link_libraries(vwebp webp webpmux freeglut glu32 opengl32 m pthread)
