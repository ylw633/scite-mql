'  Add this block for run only with 32bit WScript|CScript
'  Вставка для того, чтобы скрипт запускался только в 32bit версии интерпретаторов WScript|CScript
'  ===============================================
Set WshShell = CreateObject("WScript.Shell")
If WshShell.ExpandEnvironmentStrings("%ProgramFiles(x86)%") <> "%ProgramFiles(x86)%" And InStr(WScript.FullName, WshShell.ExpandEnvironmentStrings("%WINDIR%\SysWOW64")) <> 1 Then
	x = Split(WScript.FullName, "\")
	cmd = WshShell.ExpandEnvironmentStrings("%WINDIR%\SysWOW64\") & x(UBound(x)) & " """ & WScript.ScriptFullName & """"
	WshShell.Run cmd
	WScript.Quit
End If

'  EXAMPLES AND DESCRIPTIONS OF ALL AVAILABLE METHODS SCITE HELPER
'  ПРИМЕРЫ И ОПИСАНИЕ ВСЕХ ДОСТУПНЫХ МЕТОДОВ SCITE HELPER
'  ===============================================
'  In the beginning we create object
'  Вначале создаем объект
On Error Resume Next
Set SciTE = CreateObject("SciTE.Helper")
If Err.Number <> 0 Then
	WScript.Echo "Please install SciTE Helper before!"
	WScript.Quit 1
End If
On Error GoTo 0

' Печать строки в окне консоли (без префикса и перевода строки)
' Writes string to the output pane (no prefix, no newlines)
SciTE.Trace("Example of all available methods:\n\n")

' SciTE window size and position
SciTE.Trace ("position.Left = " & SciTE.Left & "\n")
SciTE.Trace ("position.top = " & SciTE.Top & "\n")
SciTE.Trace ("position.width = " & SciTE.Width & "\n")
SciTE.Trace ("position.height = " & SciTE.Height & "\n\n")

'  Get all text with active page
'  Извлекаем весь текст с активной страницы
all_text = SciTE.GetText

SciTE.Send ("find:scite")
'  Get only selected text with active page
'  Извлекаем только выделенный текст с активной страницы
sel_text = SciTE.GetSelText
SciTE.Trace ("Selected text: """ & sel_text & """\n")

'  Replace selected on active page text on our
'  Заменяем выделенный на активной странице текст на наш
'~ SciTE.ReplaceSel ("<http://scite-ru.org>")

'  Run command use SciTE Lua Scripting Extension
'  Запускаем LUA команду и получаем результат
CurrentPos = SciTE.LUA("editor.CurrentPos")
SciTE.Trace ("editor.CurrentPos = " & CurrentPos & "\n")

'  Задаем ключ в property и его значение
'  Set the value of a property
value = WScript.FullName
SciTE.Props("my.key") = WScript.Name

'  Читаем значение заданного ключа
'  Return the value of a property
prop = SciTE.Props ("my.key")
SciTE.Trace ("my.key = " & prop & "\n")

'  Send actions use SciTE Director Interface
'  List of all available commands - in file SciTEDirector.html
'  Посылаем команду используя SciTE Director Interface
'  Список всех доступных команд - в файле SciTEDirector.html
filename  = SciTE.Send ("askfilename:")
filename = Replace(filename,"\","\\")
SciTE.Trace (filename & "\n")

'  Run internal menu command SciTE (call "About" window)
'  List of all available commands - in file SciTE.h
'  Вызываем внутреннюю команду меню SciTE (окошко "О программе")
'  Список всех доступных команд - в файле SciTE.h
SciTE.MenuCommand (902)

'  Ну как же без этого :)
SciTE.About()
