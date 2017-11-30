set lupdatePath="D:\Qt\qt-everywhere-opensource-src-5.9.1-x64\qttools\bin\lupdate.exe"

%lupdatePath%  ..\src\ -no-obsolete -ts nomacs_en.ts


REM for %%f in (*.ts) DO %lupdatePath%  ..\src\ -no-obsolete -ts %%f
pause

