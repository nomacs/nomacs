# - FindMSGFMT.cmake for Hugin 0.7
# file taken from rosegarden
#
# Find the gnu gettext msgfmt and msgmerge programs and define
# a macro that calls it to add message translations
#
# Defined variables:
#  MSGFMT_FOUND
#  MSGFMT_EXECUTABLE
#  MSGMERGE_FOUND
#  MSGMERGE_EXECUTABLE
#
# Macro:
#  ADD_TRANSLATIONS
#
# mod for Windows build 07Nov2007 TKSharpless --
#   look in C:\MinGW after std. Unix places, that's where 
#   gettext automatically installed itself on my system.
# TODO find where else on Windows, Mac, etc gettext might be
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF(MSGFMT_EXECUTABLE)
    SET(MSGFMT_FOUND TRUE)
ELSE(MSGFMT_EXECUTABLE)
    FIND_PROGRAM(MSGFMT_EXECUTABLE NAMES msgfmt gmsgfmt msgfmt.exe
                                   PATHS /bin /usr/bin /usr/local/bin c:/MinGW/bin ${SOURCE_BASE_DIR}/gettext/bin)
    IF(MSGFMT_EXECUTABLE)
        SET(MSGFMT_FOUND TRUE)
    ELSE(MSGFMT_EXECUTABLE)
        IF(NOT MSGFMT_FIND_QUIETLY)
            IF(MSGFMT_FIND_REQUIRED)
                MESSAGE(FATAL_ERROR "msgfmt program couldn't be found")
            ENDIF(MSGFMT_FIND_REQUIRED)
        ENDIF(NOT MSGFMT_FIND_QUIETLY)
    ENDIF(MSGFMT_EXECUTABLE)
    MARK_AS_ADVANCED(MSGFMT_EXECUTABLE)
ENDIF (MSGFMT_EXECUTABLE)

IF(MSGMERGE_EXECUTABLE)
    SET(MSGMERGE_FOUND TRUE)
ELSE(MSGMERGE_EXECUTABLE)
    FIND_PROGRAM(MSGMERGE_EXECUTABLE NAMES msgmerge gmsgmerge msgmerge.exe
                                     PATHS /bin /usr/bin /usr/local/bin c:/MinGW/bin ${SOURCE_BASE_DIR}/gettext/bin)
    IF(MSGMERGE_EXECUTABLE)
        SET(MSGMERGE_FOUND TRUE)
    ELSE(MSGMERGE_EXECUTABLE)
        IF(NOT MSGMERGE_FIND_QUIETLY)
            IF(MSGMERGE_FIND_REQUIRED)
                MESSAGE(FATAL_ERROR "msgmerge program couldn't be found")
            ENDIF(MSGMERGE_FIND_REQUIRED)
        ENDIF(NOT MSGMERGE_FIND_QUIETLY)
    ENDIF(MSGMERGE_EXECUTABLE)
    MARK_AS_ADVANCED(MSGMERGE_EXECUTABLE)
ENDIF (MSGMERGE_EXECUTABLE)

MACRO(UPDATE_TRANSLATIONS _baseName)
    SET(_outputs)
    FOREACH(_file ${ARGN})
        GET_FILENAME_COMPONENT(_file_we ${_file} NAME_WE)
        SET(_out  "${CMAKE_CURRENT_SOURCE_DIR}/${_file_we}.po")
        SET(_ref "${CMAKE_CURRENT_SOURCE_DIR}/${_baseName}.pot")
        ADD_CUSTOM_COMMAND(OUTPUT ${_out}
                           COMMAND ${MSGMERGE_EXECUTABLE} ${_out} ${_ref}
                           DEPENDS ${_ref})
        SET(_outputs ${_outputs} ${_out})
    ENDFOREACH(_file)
    SET(MSGMERGE_TARGET update_translations_${_baseName})
    ADD_CUSTOM_TARGET(${MSGMERGE_TARGET} ALL DEPENDS ${_outputs})
ENDMACRO(UPDATE_TRANSLATIONS)

MACRO(ADD_TRANSLATIONS _baseName)
    SET(_outputs)
    FOREACH(_file ${ARGN})
        GET_FILENAME_COMPONENT(_file_we ${_file} NAME_WE)
        SET(_out "${CMAKE_CURRENT_BINARY_DIR}/${_file_we}.gmo")
        SET(_in  "${CMAKE_CURRENT_SOURCE_DIR}/${_file_we}.po")
        ADD_CUSTOM_COMMAND(
            OUTPUT ${_out}
            COMMAND ${MSGFMT_EXECUTABLE} -o ${_out} ${_in}
            DEPENDS ${_in} )
        INSTALL(FILES ${_out}
            DESTINATION ${LOCALEDIR}/${_file_we}/LC_MESSAGES/
            RENAME ${_baseName}.mo )
        SET(_outputs ${_outputs} ${_out})
    ENDFOREACH(_file)
    SET(MSGFMT_TARGET translations${_baseName})
    ADD_CUSTOM_TARGET(${MSGFMT_TARGET} ALL DEPENDS ${_outputs})
ENDMACRO(ADD_TRANSLATIONS)
