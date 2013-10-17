logfile = "c:\tmp\testoutput.txt"
cDir = left(WScript.ScriptFullName,(Len(WScript.ScriptFullName))-(len(WScript.ScriptName)))
workingdir = "C:\VSProjects\nomacs.git\build2012\"


Set wshShell = WScript.CreateObject("Wscript.Shell")

REM build x86
buildCmd = ""
buildCmd = addCmd(buildCmd, """C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"" x86")
buildCmd = addCmd(buildCmd, "cd " & workingdir & " & dir")
buildCmd = addCmd(buildCmd, "devenv.exe nomacs.sln /build ReallyRelease /out " & logfile &"2")

runCmd buildCmd, "Building Failed"


REM make NSIS
cmd = ""
cmd = addCmd(cmd, """C:\Program Files (x86)\NSIS\makensis.exe"" /DBUILD_DIR=""" & workingdir & "\ReallyRelease"" ..\ImageLounge\createWindowsInstaller.nsi")
runCmd cmd, "Building NSIS Package failed"

Function addCmd(oldCmd, newCmd)
REM WScript.Echo Len(oldCmd)
	if Len(oldCmd) > 0 then
		REM addCmd = "cmd /c "
	REM else 
		addCmd = oldCmd & " && "
	end if
	addCmd = addCmd & newCmd & " >> " & logfile
End Function

Function runCmd(cmd, errorMessage)
	WScript.Echo "cmd /c """ & cmd & """"
	x = wshShell.Run("cmd /c """ & cmd & """",1,1)
	if x > 0 then
		WScript.Echo errorMessage
	end if
End Function
REM MsgBox("Status: " & Return)