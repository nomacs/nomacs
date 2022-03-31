REM this script signs the exe provided, use:
REM sign nomacs.x64/nomacs.exe cert-password

REM the second argument is assumed to be the certificate's password (for appveyor)
if "%~2" == "" goto end

REM sign with the repositorie's certificate (password has to be provided)

echo online signing...
echo arg: %2
REM signtool sign /t http://timestamp.digicert.com /f ./cert-2018.p12 /fd certHash /p %2 /a %~1
goto end

REM REM set code page to utf-8 (for german umlaute)
REM :local
REM chcp 65001

REM REM sign using the installed certificate
REM signtool sign /n "Technische Universität Wien" /t http://timestamp.digicert.com %~1

:end
