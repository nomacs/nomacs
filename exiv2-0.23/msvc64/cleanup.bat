setlocal

set X=exiv2\build      && if EXIST %X% rmdir/s/q %X%
set X=exiv2\Win32      && if EXIST %X% rmdir/s/q %X%
set X=exiv2\x64        && if EXIST %X% rmdir/s/q %X%

set X=exiv2lib\build   && if EXIST %X% rmdir/s/q %X%
set X=exiv2lib\Win32   && if EXIST %X% rmdir/s/q %X%
set X=exiv2lib\x64     && if EXIST %X% rmdir/s/q %X%

set X=expat\build      && if EXIST %X% rmdir/s/q %X%
set X=expat\Win32      && if EXIST %X% rmdir/s/q %X%
set X=expat\x64        && if EXIST %X% rmdir/s/q %X%

set X=xmpsdk\build     && if EXIST %X% rmdir/s/q %X%
set X=xmpsdk\Win32     && if EXIST %X% rmdir/s/q %X%
set X=xmpsdk\x64       && if EXIST %X% rmdir/s/q %X%

set X=zlib\build       && if EXIST %X% rmdir/s/q %X%
set X=zlib\Win32       && if EXIST %X% rmdir/s/q %X%
set X=zlib\x64         && if EXIST %X% rmdir/s/q %X%

set X=zlib123\build    && if EXIST %X% rmdir/s/q %X%
set X=zlib123\Win32    && if EXIST %X% rmdir/s/q %X%
set X=zlib123\x64      && if EXIST %X% rmdir/s/q %X%

set X=zlib125\build    && if EXIST %X% rmdir/s/q %X%
set X=zlib125\Win32    && if EXIST %X% rmdir/s/q %X%
set X=zlib125\x64      && if EXIST %X% rmdir/s/q %X%

del/s *.ncb
del/s *.sdf
del/s *.vcxproj
del/s *%USERNAME%*
del/s *.filters

rmdir/s/q bin

endlocal

