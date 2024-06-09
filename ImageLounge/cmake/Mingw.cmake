# placeholder for refining mingw/mxe cross compilation
# unix setup is fine for now
include(cmake/Unix.cmake)

# this isn't coming in on MXE, but it won't break Msys2 build
if(TIFF_FOUND)
    set(TIFF_LIBRARIES "${TIFF_LIBRARIES};tiffxx")
endif()
