REM set your inno setup path
SET PATH=%PATH%;"C:\Users\markus\AppData\Local\Programs\Inno Setup 6";"C:\Program Files (x86)\Windows Kits\10\bin\x64\";
SET ARCH=x64

REM set code page to utf-8 (for german umlaute)
chcp 65001

REM sign nomacs
signtool sign /n "Technische Universität Wien" /t http://timestamp.digicert.com .\nomacs.x64\nomacs.exe

REM compile
ISCC.exe ./nomacs-setup.iss

REM sign the setup
signtool sign /n "Technische Universität Wien" /t http://timestamp.digicert.com .\nomacs-setup-%ARCH%.exe
