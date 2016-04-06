 REM Package XML version incrementer.
 REM this file edits the <version> strings of a package.xml file
 
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
    WScript.Echo "package-incrementer.vbs Please specify a valid path to the input file (package.xml)."
	WScript.Quit
end if

Set objFSO = CreateObject("Scripting.FileSystemObject")

REM 'How to read a file
filePath = WScript.Arguments(0)
Set inFile = objFSO.OpenTextFile(filePath, 1, True)
Set outFile = objFSO.OpenTextFile(filePath & ".tmp", 2, True)

Do Until inFile.AtEndOfStream
    oLine = inFile.ReadLine

	oLine = IncrementVersion(oLine)
	oLine = updateReleaseDate(oLine)
	
	outFile.writeLine oLine
Loop

REM replace file
inFile.Close
outFile.Close
objFSO.DeleteFile(filePath)
objFSO.MoveFile filePath & ".tmp", filePath

WScript.Quit(1)

REM searching for this line: <Version>2.0.0</Version>
Function IncrementVersion(oLine)

	line = Split(Trim(oLine), ">")
	    		
	REM searching for this line: <Version>2.0.0</Version>
	If (UBound(line) = 2) Then
	
		ver = Replace(line(1), "</Version", "")
		ver = Split(ver, ".")
		
		set regExp = New RegExp
		regExp.Global = True
		regExp.Pattern = "^\s+|\s+$"
		tCmd = regExp.Replace(line(0), "")

		REM check if we really have the correct line
		If (tCmd = "<Version" And (UBound(ver) = 2)) Then
			
			REM for nomacs we use versions like 3.1.0-3
			nver = Split(ver(2), "-")
			
			if UBound(nver) = 1 Then
				vrep = nver(1)
			Else
				vrep = ver(2)
			End If
			
			newLine = Replace(line(1), vrep & "<", CStr(CInt(vrep)+1) & "<")
			
			
			REM set edited line
			oLine = Replace(oLine, line(1), newLine)
		End If
	End If

	IncrementVersion = oLine
	
End Function

REM searching for this line: <Version>2.0.0</Version>
Function updateReleaseDate(oLine)

	line = Split(Trim(oLine), ">")
	    		
	REM searching for this line: <ReleaseDate>2016-04-06</ReleaseDate>
	If (UBound(line) = 2) Then
	
		oldDate = Replace(line(1), "</ReleaseDate", "")
		
		set regExp = New RegExp
		regExp.Global = True
		regExp.Pattern = "^\s+|\s+$"
		tCmd = regExp.Replace(line(0), "")

		REM 
		If (tCmd = "<ReleaseDate") Then
			
			
			newDate = Year(Date()) & "-" & Right("0" & Month(Date()),2) & "-" & Right("0" & Day(Date()),2)
			newLine = Replace(line(1), oldDate, newDate)
			
			REM set edited line
			oLine = Replace(oLine, line(1), newLine)
		End If
	End If

	updateReleaseDate = oLine
	
End Function