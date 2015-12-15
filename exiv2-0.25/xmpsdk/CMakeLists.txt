# CMake build system for exiv2 library and executables
# Copyright 2012-     Robin Mills      <robin@clanmills.com>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml at gmx dot de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(../CMake_msvc.txt)
msvc_runtime_configure(${EXIV2_ENABLE_SHARED})

if( POLICY CMP0026 )
    cmake_policy(SET CMP0026 OLD) # Something to do with location
endif()

FOREACH(_currentfile ${XMPSRC})
    # http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    IF(NOT MSVC AND NOT CYGWIN AND NOT MSYS)
        SET_SOURCE_FILES_PROPERTIES(${_currentfile} PROPERTIES COMPILE_FLAGS "-fPIC")
    ENDIF(NOT MSVC AND NOT CYGWIN AND NOT MSYS)
ENDFOREACH()

IF( EXIV2_ENABLE_XMP AND EXIV2_ENABLE_LIBXMP )
    ADD_LIBRARY( xmp STATIC ${XMPSRC} )
    GET_TARGET_PROPERTY( XMPLIB xmp LOCATION )
    TARGET_LINK_LIBRARIES(xmp ${EXPAT_LIBRARIES})
    # SET( XMPLIB ${XMPLIB} PARENT_SCOPE )
    # INSTALL(TARGETS xmp ${INSTALL_TARGET_STANDARD_ARGS} )
ENDIF()

# That's all Folks!
##
