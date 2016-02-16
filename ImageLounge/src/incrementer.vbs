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