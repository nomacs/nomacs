
 REM Visual Studio File Version Incrementer.
 REM this file edits the FILEVERSION and Value "FileVersion" strings of a visual studio rc file
 
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
    WScript.Echo "incrementer.vbs Please specify a valid path to the input file."
	WScript.Quit
end if

Set objFSO = CreateObject("Scripting.FileSystemObject")

REM 'How to read a file
filePath = WScript.Arguments(0)
Set inFile = objFSO.OpenTextFile(filePath, 1, True)
Set outFile = objFSO.OpenTextFile(filePath & ".tmp", 2, True)

Do Until inFile.AtEndOfStream
    oLine = inFile.ReadLine

	oLine = IncrementVersionString(oLine)
	oLine = IncrementVersion(oLine)
	
	outFile.writeLine oLine
Loop

REM replace file
inFile.Close
outFile.Close
objFSO.DeleteFile(filePath)
objFSO.MoveFile filePath & ".tmp", filePath

REM increment this line: VALUE "FileVersion", "3.1.0.42"
Function IncrementVersionString(oLine)

	line = Split(oLine, ",")
	    
	REM searching for this line: VALUE "FileVersion", "3.1.0.43"
	If (UBound(line) = 1) Then
	
		ver = Split(line(1), ".")
	
		If (Trim(line(0)) = "VALUE ""FileVersion""") And (UBound(ver) = 3) Then
			verc = Replace(ver(3), """", "")
			
			newLine = Replace(line(1), ver(3), CStr(CInt(verc)+1) + """")
			
			REM set edited line
			oLine = Replace(oLine, line(1), newLine)
			
			WScript.Echo "[Version Incrementer] version updated: " + newLine
		End If
	End If

	IncrementVersionString = oLine
	
End Function

REM increment this line: FILEVERSION 3,1,0,43
Function IncrementVersion(oLine)

	line = Split(Trim(oLine), " ")
	    
	REM searching for this line: FILEVERSION 3,1,0,43
	If (UBound(line) = 1) Then
	
		ver = Split(line(1), ",")
	
		If (line(0) = "FILEVERSION") And (UBound(ver) = 3) Then
			
			newLine = Replace(line(1), ver(3), CStr(CInt(ver(3))+1))
			
			REM set edited line
			oLine = Replace(oLine, line(1), newLine)
			
		End If
	End If

	IncrementVersion = oLine
	
End Function