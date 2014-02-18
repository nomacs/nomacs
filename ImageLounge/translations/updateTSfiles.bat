set lupdatePath="C:\Qt\qt-everywhere-opensource-src-4.8.5-x64\bin\lupdate.exe"

for %%f in (*.ts) DO %lupdatePath%  ..\src\ -no-obsolete -ts %%f
pause
