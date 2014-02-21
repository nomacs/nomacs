@ECHO OFF

REM  Copyright (c) 2013, Gilles Caulier, <caulier dot gilles at gmail dot com>
REM 
REM  Redistribution and use is allowed according to the terms of the BSD license.
REM  For details see the accompanying COPYING-CMAKE-SCRIPTS file.

REM adjust setup here if necessary
SET INSTALL_PREFIX=C:\
SET BUILD_TYPE=debugfull

IF NOT EXIST "build" md "build"

cd "build"

REM Microsoft Visual C++ command line compiler.
cmake -G "NMake Makefiles" . ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX% ^
      -Wno-dev ^
      -DENABLE_OPENMP=ON ^
      -DENABLE_LCMS=ON ^
      -DENABLE_EXAMPLES=ON ^
      -DENABLE_RAWSPEED=ON ^
      -DRAWSPEED_RPATH=../RawSpeed.libraw ^
      -DCHECKOUT_RAWSPEED=ON ^
      -DRAWSPEED_REVISION=HEAD ^
      -DENABLE_DEMOSAIC_PACK_GPL2=ON ^
      -DDEMOSAIC_PACK_GPL2_RPATH=../LibRaw-demosaic-pack-GPL2/ ^
      -DCHECKOUT_DEMOSAIC_PACK_GPL2=ON ^
      -DDEMOSAIC_PACK_GPL2_REVISION=HEAD ^
      -DENABLE_DEMOSAIC_PACK_GPL3=ON ^
      -DDEMOSAIC_PACK_GPL3_RPATH=../LibRaw-demosaic-pack-GPL3/ ^
      -DCHECKOUT_DEMOSAIC_PACK_GPL3=ON ^
      -DDEMOSAIC_PACK_GPL3_REVISION=HEAD ^
      -DENABLE_DCRAW_DEBUG=ON ^
      ..
