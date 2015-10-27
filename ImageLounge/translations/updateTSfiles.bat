set lupdatePath="D:\Qt\qt-everywhere-opensource-src-5.5.0-x86\qtbase\bin\lupdate.exe"

%lupdatePath%  ..\src\ -no-obsolete -ts nomacs_en.ts


REM for %%f in (*.ts) DO %lupdatePath%  ..\src\ -no-obsolete -ts %%f
pause

