# Initialization
AC_INIT([LibRaw], m4_esyscmd([./version.sh]), [info@libraw.org], [], [http://www.libraw.org])
AM_INIT_AUTOMAKE([foreign no-define])
#AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])

# Tools to use
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_ENABLE_SHARED
AC_ENABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_SETUP
AC_SUBST(LIBTOOL_DEPS)

# Config files to generate

CPPFLAGS="$CPPFLAGS -I/usr/local/include"
LDFLAGS="$LDFLAGS -L/usr/local/lib"

AC_CONFIG_FILES([
	Makefile
	libraw.pc
	libraw_r.pc
])

# check if we want OpenMP support
AC_ARG_ENABLE([openmp],
	[  --enable-openmp    Enable OpenMP support],
	[case "${enableval}" in
		yes) 	openmp=true ;;
		no)		openmp=false ;;
		*)		AC_MSG_ERROR([bad value ${enableval} for --enable-openmp]) ;;
	esac],[openmp=true])

if test x$openmp = xtrue ; then
	AX_OPENMP([
		CXXFLAGS="$CXXFLAGS $OPENMP_CFLAGS"
		CFLAGS="$CFLAGS $OPENMP_CFLAGS"
		AC_SUBST([PC_OPENMP],[" $OPENMP_CFLAGS"])		
	],[
		AC_MSG_WARN([OpenMP support cannot be enabled because your system doesn't support it.])		
	])
fi

# check for libjpeg v8
AC_ARG_ENABLE([jpeg],
	[  --enable-jpeg    Enable JPEG support for Lossy compressed DNG files],
	[case "${enableval}" in
		yes)	jpeg=true ;;
		no)	jpeg=false ;;
		*)	AC_MSG_ERROR([bad value ${enableval} for --enable-jpeg]) ;;
	esac],[jpeg=true])

if test x$jpeg = xtrue; then
        AC_CHECK_LIB([jpeg], [jpeg_stdio_src],
                [
                        AC_CHECK_HEADERS([jpeglib.h], [
                                CPPFLAGS="$CPPFLAGS -DUSE_JPEG"
                                LIBS="$LIBS -ljpeg"
                        ], AC_MSG_WARN([no jpeg headers found]))
                ],
                AC_MSG_WARN([libjpeg not found])
        )
fi

if test x$jpeg = xtrue; then
        AC_CHECK_LIB([jpeg], [jpeg_mem_src],
                [
                        AC_CHECK_HEADERS([jpeglib.h], [
                                CPPFLAGS="$CPPFLAGS -DUSE_JPEG8"
                        ], AC_MSG_WARN([no jpeg8 headers found]))
                ],
                AC_MSG_WARN([libjpeg8 not found - disabling decoding JPEG data from memory buffer])
        )
fi

# check for Jasper (JPEG2000) support
AC_ARG_ENABLE([jasper],
	[  --enable-jasper    Enable Jasper (JPEG2000) support for RedCine files],
	[case "${enableval}" in
		yes)	jasper=true ;;
		no)	jasper=false ;;
		*)	AC_MSG_ERROR([bad value ${enableval} for --enable-jasper]) ;;
	esac],[jasper=true])

if test x$jasper = xtrue; then
        AC_CHECK_LIB([jasper], [jas_init],
                [
                        AC_CHECK_HEADERS([jasper/jasper.h], [
                                CPPFLAGS="$CPPFLAGS -DUSE_JASPER"
                                LIBS="$LIBS -ljasper"
                        ], AC_MSG_WARN([no jasper headers found]))
                ],
                AC_MSG_WARN([libjasper not found])
        )
fi



# check if we want LCMS support
AC_ARG_ENABLE([lcms],
	[  --enable-lcms    Enable LCMS support],
	[case "${enableval}" in
		yes)	lcms=true ;;
		no)		lcms=false ;;
		*)		AC_MSG_ERROR([bad value ${enableval} for --enable-lcms]) ;;
	esac],[lcms=true])

if test x$lcms = xtrue; then
	PKG_CHECK_MODULES([LCMS2],[lcms2],[
		CPPFLAGS="$CPPFLAGS $LCMS2_CFLAGS -DUSE_LCMS2"
		LIBS="$LIBS $LCMS2_LIBS"
	        AC_SUBST([PACKAGE_REQUIRES],[lcms2])
	],[
	PKG_CHECK_MODULES([LCMS],[lcms],[
		CPPFLAGS="$CPPFLAGS $LCMS_CFLAGS -DUSE_LCMS"
		LIBS="$LIBS $LCMS_LIBS"
	        AC_SUBST([PACKAGE_REQUIRES],[lcms])
	],[
		AC_MSG_WARN([LCMS support cannot be enabled])
	])
	])
fi

# check if we want build examples
AC_ARG_ENABLE([examples],
	[  --enable-examples    Enable building of examples],
	[case "${enableval}" in
		yes) 	examples=true ;;
		no)		examples=false ;;
		*)		AC_MSG_ERROR([bad value ${enableval} for --enable-examples]) ;;
	esac],[examples=true])
AM_CONDITIONAL([EXAMPLES], [test x$examples = xtrue])

# LibRaw-demosaic-pack-gpl2
AC_ARG_ENABLE([demosaic-pack-gpl2],
  [ --enable-demosaic-pack-gpl2  Enable LibRaw demosaic pack/GPL2 ],
  [case "${enableval}" in
      yes)    dp2=true ;;
      no)     dp2=false ;;
      *)      dp2=true; dp2dir=${enableval} ;;
  esac],[dp2=true])

if test x$dp2 = xtrue ; then
  if test x$dp2dir = x ; then

    if test -f ./LibRaw-demosaic-pack-GPL2/afd_interpolate_pl.c ; then
        # Directory within LibRaw source tree.
	echo "Activated LibRaw demosaic pack/GPL2 in current dir"
        CPPFLAGS="$CPPFLAGS -I./LibRaw-demosaic-pack-GPL2 -DLIBRAW_DEMOSAIC_PACK_GPL2"
    else
        lcount=`find .. -type d -name 'LibRaw-demosaic*GPL2*' | wc -l`
        if test $lcount -eq 1 ; then
                pdir=`find .. -type d -name 'LibRaw-demosaic*GPL2*' | head -1`
	        if test -f  ${pdir}/afd_interpolate_pl.c ; then
           	        echo "Activated LibRaw demosaic pack/GPL2 in $pdir"
                        CPPFLAGS="$CPPFLAGS -I${pdir} -DLIBRAW_DEMOSAIC_PACK_GPL2"
                else
	                AC_MSG_WARN([Unable to find working LibRaw-demosaic-pack-GPL2 in ${pdir}, disabling the feature.\n Use --enable-demosaic-pack-gpl2=directory to override]);	
                fi
         else
	        AC_MSG_WARN([Unable to find working LibRaw-demosaic-pack-GPL2 in ${pdir}, disabling the feature. Use --enable-demosaic-pack-gpl2=directory to override]);	
	  fi


    fi
  else
    # directory set manually
        if test -f ${dp2dir}/afd_interpolate_pl.c ; then
           	echo "Activated LibRaw demosaic pack/GPL2 in $dp2dir"
                CPPFLAGS="$CPPFLAGS -I${dp2dir} -DLIBRAW_DEMOSAIC_PACK_GPL2"
        else
		 AC_MSG_ERROR([${dp2dir} does not contain LibRaw-demosaic-pack-GPL2]);	
	fi
  fi 
fi

# LibRaw-demosaic-pack-gpl3
AC_ARG_ENABLE([demosaic-pack-gpl3],
  [ --enable-demosaic-pack-gpl3  Enable LibRaw demosaic pack/GPL3 ],
  [case "${enableval}" in
      yes)    dp3=true ;;
      no)     dp3=false ;;
      *)      dp3=true; dp3dir=${enableval} ;;
  esac],[dp3=true])

if test x$dp3 = xtrue ; then
  if test x$dp3dir = x ; then

    if test -f ./LibRaw-demosaic-pack-GPL3/CA_correct_RT.cc ; then
        # Directory within LibRaw source tree.
	echo "Activated LibRaw demosaic pack/GPL3 in current dir"
        CPPFLAGS="$CPPFLAGS -I./LibRaw-demosaic-pack-GPL3 -DLIBRAW_DEMOSAIC_PACK_GPL3"
    else
        lcount=`find .. -type d -name 'LibRaw-demosaic*GPL3*' | wc -l`
        if test $lcount -eq 1 ; then
                pdir=`find .. -type d -name 'LibRaw-demosaic*GPL3*'|head -1`
	        if test -f  ${pdir}/CA_correct_RT.cc ; then
           	        echo "Activated LibRaw demosaic pack/GPL3 in $pdir"
                        CPPFLAGS="$CPPFLAGS -I${pdir} -DLIBRAW_DEMOSAIC_PACK_GPL3"
                else
	                AC_MSG_WARN([Unable to find working LibRaw-demosaic-pack-GPL3 in ${pdir}, disabling the feature.\n Use --enable-demosaic-pack-gpl3=directory to override]);	
                fi
         else
	        AC_MSG_WARN([Unable to find working LibRaw-demosaic-pack-GPL3 in ${pdir}, disabling the feature. Use --enable-demosaic-pack-gpl3=directory to override]);	
	  fi


    fi
  else
    # directory set manually
        if test -f ${dp3dir}/CA_correct_RT.cc ; then
           	echo "Activated LibRaw demosaic pack/GPL3 in $dp3dir"
                CPPFLAGS="$CPPFLAGS -I${dp3dir} -DLIBRAW_DEMOSAIC_PACK_GPL3"
        else
		 AC_MSG_ERROR([${dp3dir} does not contain LibRaw-demosaic-pack-GPL3]);	
	fi
  fi 
fi

LIBS="$LIBS -lm -lstdc++"

AC_SUBST([LIBRAW_SHLIB_VERSION],m4_esyscmd([./shlib-version.sh]))
AC_SUBST([LIBRAW_RELEASE_VERSION],m4_esyscmd([./version.sh]))

AC_OUTPUT

