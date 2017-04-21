REM dir to current directory
cd %~dp0

REM sign exe
"C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool" sign /f D:\coding\TU-code-signing.p12  /t http://timestamp.digicert.com .\packages\nomacs.x64\data\nomacs-x64\nomacs.exe

"C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool" sign /f D:\coding\TU-code-signing.p12  /t http://timestamp.digicert.com .\packages\nomacs.x86\data\nomacs-x86\nomacs.exe

REM update package verison numbers
cscript /nologo ./package-incrementer-batch.vbs ./packages package.xml package-incrementer.vbs

REM update repo
D:\Qt\QtIFW2.0.5\bin\repogen.exe -p ./packages --update repository

REM create offline installer
D:\Qt\QtIFW2.0.5\bin\binarycreator.exe -c config-beta\config-offline.xml -p packages -r config/resources.qrc nomacs-setup-beta-offline

REM use for final online installer
D:\Qt\QtIFW2.0.5\bin\binarycreator.exe -n -c config-beta\config.xml -p packages -r config/resources.qrc nomacs-setup-beta

"C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool" sign /f D:\coding\TU-code-signing.p12  /t http://timestamp.digicert.com nomacs-setup-beta.exe

pause