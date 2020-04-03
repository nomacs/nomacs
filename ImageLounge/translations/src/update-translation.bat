set lupdatePath="C:\Qt\Qt-5.14.1\bin\lupdate.exe"

%lupdatePath%  ..\..\src\ -no-obsolete -ts nomacs.ts

for %%f in (*.ts) DO %lupdatePath%  ..\..\src\ -no-obsolete -ts %%f
pause

