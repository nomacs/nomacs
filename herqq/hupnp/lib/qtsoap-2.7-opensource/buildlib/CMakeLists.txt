set(QtSoap_SRCS
../src/qtsoap.cpp
)
 
qt4_wrap_cpp(QtSoap_MOC_SRCS ../src/qtsoap.h)
 
include(${QT_USE_FILE})

add_library(QtSolutions_SOAP-2.7 SHARED ${QtSoap_SRCS} ${QtSoap_MOC_SRCS})
set_target_properties(QtSolutions_SOAP-2.7 PROPERTIES COMPILE_FLAGS "-DQT_QTSOAP_EXPORT") 
set_target_properties(QtSolutions_SOAP-2.7 PROPERTIES
VERSION "1.0.0"
SOVERSION "1.0.0"
)
target_link_libraries(QtSolutions_SOAP-2.7
${QT_LIBRARIES}
${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY}
)

set_target_properties(QtSolutions_SOAP-2.7 PROPERTIES DEBUG_OUTPUT_NAME QtSoap27d)
set_target_properties(QtSolutions_SOAP-2.7 PROPERTIES RELEASE_OUTPUT_NAME QtSoap27)

install(TARGETS QtSolutions_SOAP-2.7
RUNTIME DESTINATION bin
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib COMPONENT Devel)