On Error Resume Next
Set WshShell = CreateObject("WScript.Shell")
If WshShell.ExpandEnvironmentStrings("%ProgramFiles(x86)%") <> "%ProgramFiles(x86)%" And InStr(WScript.FullName, WshShell.ExpandEnvironmentStrings("%WINDIR%\SysWOW64")) <> 1 Then
	x = Split(WScript.FullName, "\")
	cmd = WshShell.ExpandEnvironmentStrings("%WINDIR%\SysWOW64\") & x(UBound(x)) & " """ & WScript.ScriptFullName & """"
	WshShell.Run cmd
	WScript.Quit
End If

Set SciTE = CreateObject("SciTE.Helper")
If Err.Number = 0 Then
	If MsgBox("SciTE Helper already register" & vbCrLf & "You want UnRegister it?", vbOKCancel + vbInformation, "UnRegister SciTE Helper") = vbCancel Then WScript.Quit
	WshShell.Run "Regsvr32 /U SciTE.dll"
Else
	If MsgBox("You want to install SciTE Helper?", vbOKCancel + vbInformation, "Register SciTE Helper") = vbCancel Then WScript.Quit
	Set FSO = WScript.CreateObject("Scripting.FileSystemObject")
	if Not FSO.FileExists("SciTE.dll") then MsgBox "File SciTE.dll in current dir" & vbCrLf & "N O T   E X I S T !", vbExclamation, "Error" : WScript.Quit
	ret = WshShell.Run("cmd /c for /f %i in (""msvbvm60.dll"") do if ""%~$PATH:i""=="""" exit /b 1", 0, True)
	if ret <> 0 Then MsgBox "MSVBVM60.DLL not found!", vbExclamation, "Error" : WScript.Quit
	WshShell.Run "Regsvr32 /s SciTE.dll", 0, true
	Set SciTE = CreateObject("SciTE.Helper")
	SciTE.About
End If
WScript.Quit
