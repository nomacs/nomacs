set lupdatePath="C:\Qt\Qt5.11.1-x64\bin\lupdate.exe"

%lupdatePath%  ..\..\src\ -no-obsolete -ts nomacs.ts

for %%f in (*.ts) DO %lupdatePath%  ..\..\src\ -no-obsolete -ts %%f
pause

