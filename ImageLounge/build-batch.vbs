Set oShell = WScript.CreateObject("WScript.shell")

cDir = left(WScript.ScriptFullName,(Len(WScript.ScriptFullName))-(len(WScript.ScriptName)))
workingdir = "C:\VSProjects\nomacs\build2012x86\"

REM x86 build vars
cmdx86 = """C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"" x86"
chdir = "cd " & workingdir & " & dir "

build = "devenv.exe nomacs.sln /build ReallyRelease /Out " & cDir & "log.txt"
debugStr = "timeout /T 10"

REM make NSIS
cmdnsis = """C:\Program Files (x86)\NSIS\makensis.exe"" /DBUILD_DIR=""" & workingdir & "\ReallyRelease"" ..\ImageLounge\createWindowsInstaller.nsi"

Return = oShell.Run(cmdx86 & " && " & chdir & " && " & build & " && " & cmdnsis & " && " & debugStr)


REM MsgBox("Status: " & Return)