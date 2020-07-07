echo off

REM set your WiX path
SET PATH=%PATH%;"C:\Program Files (x86)\WiX Toolset v3.11\bin";"C:\Program Files (x86)\Windows Kits\10\bin\x64\";
SET ARCH=x64

call sign .\nomacs.%ARCH%\nomacs.exe %~1

REM harvest dlls
heat.exe dir .\nomacs.%ARCH% -o HarvestedFiles.wxs -scom -frag -srd -sreg -gg -cg ApplicationResources -dr BIN_DIR_REF

REM create meta object
candle.exe -arch %ARCH% nomacs-setup-versioned.wxs nomacs-ui.wxs HarvestedFiles.wxs

REM make setup (might take a few seconds)
light.exe -ext WixUIExtension nomacs-setup-versioned.wixobj nomacs-ui.wixobj HarvestedFiles.wixobj -b ./nomacs.%ARCH% -out nomacs-setup-%ARCH%.msi

REM sign the setup
call sign .\nomacs-setup-%ARCH%.msi %~1

REM clean up
del *.wixobj
del *.wixpdb
del HarvestedFiles.wxs