REM Be sure to run this in a VS Native Tools environment
REM "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"

SET MYPATH=%~dp0

call %mypath%/make-opencv.bat
call %mypath%/make-expat.bat
call %mypath%/make-exiv2.bat