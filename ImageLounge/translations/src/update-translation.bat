set lupdatePath="C:\Qt\5.14.2\msvc2017_64\bin\lupdate.exe"

%lupdatePath%  ..\..\src\ -no-obsolete -ts nomacs.ts

for %%f in (*.ts) DO %lupdatePath%  ..\..\src\ -no-obsolete -ts %%f
pause

