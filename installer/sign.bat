echo off
REM this script signs the exe provided, use:
REM sign nomacs.x64/nomacs.exe cert-password

REM the second argument is assumed to be the certificate's password (for appveyor)
if "%~2"=="" (
    goto LocalCert
) else (
    goto RepoCert
)

:LocalCert

REM set code page to utf-8 (for german umlaute)
chcp 65001

REM sign using the installed certificate
signtool sign /n "Technische Universität Wien" /t http://timestamp.digicert.com %~1

goto End

:RepoCert

REM sign with the repositorie's certificate (password has to be provided)
signtool sign /t http://timestamp.digicert.com /f ./cert-2018.p12 /p %2 /a %~1

goto End

:End