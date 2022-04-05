set lupdatePath="C:\Qt\5.15.2\msvc2019_64\bin\lupdate.exe"

%lupdatePath%  ..\..\src\ -no-obsolete -ts nomacs.ts

for %%f in (*.ts) DO %lupdatePath%  ..\..\src\ -no-obsolete -ts %%f

