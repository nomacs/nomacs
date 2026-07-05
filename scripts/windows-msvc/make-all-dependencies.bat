@echo off

call make-000-pkgconf.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-001-zlib.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-002-jpeg-turbo.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-003-openjpeg.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-004-dav1d.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-005-lcms2.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-006-libjxl.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-007-libaom.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-008-libwebp.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-009-libavif.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-010-openexr.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-011-libtiff.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-012-opencv.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-013-libexpat.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-014-exiv2.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-015-libraw.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-016-quazip.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-017-libde265.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-018-openh264.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-019-x264.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-020-libheif.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-021-extra-cmake-modules.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-022-jxrlib.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-023-karchive.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

call make-024-kimageformats.bat
if %ERRORLEVEL% NEQ 0 (
  exit /b 1
)

echo Success!!!
