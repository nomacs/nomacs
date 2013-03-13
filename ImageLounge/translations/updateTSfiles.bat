set lupdatePath="C:\Qt\qt-everywhere-opensource-src-4.8.4-msvc2012-x86\bin\lupdate.exe"

for %%f in (*.ts) DO %lupdatePath%  ..\src\ -ts %%f
pause