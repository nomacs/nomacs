 REM Package version incrementer batch.
 REM usage: package-incrementer-batch.vbs <root path> <file name>
  
 REM Copyright (C) 2016 Markus Diem <markus@nomacs.org>

 REM This file is part of nomacs.

 REM nomacs is free software: you can redistribute it and/or modify
 REM it under the terms of the GNU General Public License as published by
 REM the Free Software Foundation, either version 3 of the License, or
 REM (at your option) any later version.

 REM nomacs is distributed in the hope that it will be useful,
 REM but WITHOUT ANY WARRANTY; without even the implied warranty of
 REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 REM GNU General Public License for more details.

 REM You should have received a copy of the GNU General Public License
 REM along with this program.  If not, see <http://www.gnu.org/licenses/>.
 

if WScript.Arguments.Count = 0 then
    WScript.Echo "package-incrementer-batch.vbs Please specify a valid root folder."
	WScript.Quit
end if

if WScript.Arguments.Count = 1 then
    WScript.Echo "package-incrementer-batch.vbs Please specify a file name which will be filtered."
	WScript.Quit
end if

if WScript.Arguments.Count = 2 then
    WScript.Echo "package-incrementer-batch.vbs Please specify another vbs which will be called for all relevant files."
	WScript.Quit
end if

Set objFSO = CreateObject("Scripting.FileSystemObject")

REM 'How to read a file
filePath = WScript.Arguments(0)
fileName = WScript.Arguments(1)
scriptName = WScript.Arguments(2)

Call IndexFolder(filePath)

Function IndexFolder (filePath)

    Set fs = CreateObject("Scripting.FileSystemObject")
    Set f = fs.GetFolder(filePath)
    Set sf = f.SubFolders
    
	For Each folder in sf
		Call IndexFolder(folder)
    Next
	
	Set files = f.Files
	For Each file in files
		Call findFile(file)
	Next

End Function

Function findFile(filePath)

	If NOT (InStr(filePath, fileName) = 0) Then
		
		Dim objShell
		Set objShell = Wscript.CreateObject("WScript.Shell")

		' call the script
		objShell.Run scriptName & " " & filePath

		' Using Set is mandatory
		Set objShell = Nothing
		WScript.Echo "updating: " & filePath
	End If
	

End Function
