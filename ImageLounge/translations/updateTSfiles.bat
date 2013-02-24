set lupdatePath="C:\Qt\4.8.2\qt-everywhere-opensource-src-4.8.2\bin\lupdate.exe"

for %%f in (*.ts) DO %lupdatePath%  ..\src\ -ts %%f
pause