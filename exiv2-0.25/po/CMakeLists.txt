# CMake build system for exiv2 library and executables
# Copyright 2008      by Patrick Spendrin <ps_ml@gmx.de>
# Copyright 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# automatically include all po files in the directory
FILE(GLOB PO_FILES *.po)

if ( NOT MSVC )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

UPDATE_TRANSLATIONS(exiv2 ${PO_FILES})

ADD_TRANSLATIONS(exiv2 ${PO_FILES})

ADD_CUSTOM_TARGET( pot-update
                   COMMAND xgettext -d ${PACKAGE_NAME}
                                    -s
                                    -D ${CMAKE_CURRENT_SOURCE_DIR}/..
                                    -o ${PACKAGE_NAME}.pot
                                    -p ${CMAKE_CURRENT_SOURCE_DIR}/
                                    --from-code=UTF-8 
                                    --files-from=POTFILES.in
                                    -j
                                    --keyword=_
                                    --package-name=${PACKAGE_NAME}
                                    --package-version=${PACKAGE_VERSION}
                                    --copyright-holder=${PACKAGE_COPYRIGHT}
                                    --msgid-bugs-address=${PACKAGE_BUGREPORT}
                 )
