# - Find libssh
# Find the native SSH headers and libraries.
#
#  SSH_INCLUDE_DIRS - where to find libssh.h, etc.
#  SSH_LIBRARIES    - List of libraries when using libssh.
#  SSH_FOUND        - True if libssh found.
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Look for the header file.
FIND_PATH(SSH_INCLUDE_DIR NAMES libssh/libssh.h)

# Look for the library.
FIND_LIBRARY(SSH_LIBRARY NAMES ssh libssh)

# handle the QUIETLY and REQUIRED arguments and set SSH_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SSH DEFAULT_MSG SSH_LIBRARY SSH_INCLUDE_DIR)

# Copy the results to the output variables.
IF(SSH_FOUND)
  SET(SSH_LIBRARIES ${SSH_LIBRARY})
  SET(SSH_INCLUDE_DIRS ${SSH_INCLUDE_DIR})
ELSE(SSH_FOUND)
  SET(SSH_LIBRARIES)
  SET(SSH_INCLUDE_DIRS)
ENDIF(SSH_FOUND)

MARK_AS_ADVANCED(SSH_INCLUDE_DIR SSH_LIBRARY)
