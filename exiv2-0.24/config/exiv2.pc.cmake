prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${prefix}/lib
includedir=${prefix}/include

Name: exiv2
Description: Exif and IPTC metadata library and tools
Version: @PACKAGE_VERSION@
URL: @PACKAGE_URL@
Libs: -L${libdir} -lexiv2 @EXV_LIB_STATIC@
Cflags: -I${includedir}
