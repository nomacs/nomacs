# ----------------------------------------------------------------------------
# macro get_git_version_string
#
# sets the variable NOMACS_VCSVERSION from git, if available.
#
# if no git was found NOMACS_VCSVERSION is ""
# otherwise NOMACS_VCSVERSION is a a string "(git: 3.6.1-48-g3494189-dirty)"
#
# The version string components are:
# - last 3 letter version tag, e.g. '3.6.1'
# - number of commits since the tag, e.g. '48'
# - a short ref of the current commit, e.g. 'g394189'
# - a tag indicating a dirty build repo, e.g. '-dirty'
#
# ----------------------------------------------------------------------------
find_package(Git)

macro(get_git_version_string)
if(GIT_FOUND)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe --tags --always --dirty --match "[0-9].[0-9].[0-9]*"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE NOMACS_VCSVERSION
    RESULT_VARIABLE GIT_RESULT
    #ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT GIT_RESULT EQUAL 0)
    set(NOMACS_VCSVERSION "")
  else()
      set(NOMACS_VCSVERSION "(git: ${NOMACS_VCSVERSION})")
  endif()
elseif(NOT DEFINED NOMACS_VCSVERSION)
  # We don't have git:
  set(NOMACS_VCSVERSION "")
endif()

endmacro()
