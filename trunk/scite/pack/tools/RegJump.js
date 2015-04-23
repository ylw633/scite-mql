/*
Registry Jump
Version: 2.0
Author: mozers™
------------------------------------------------
Открывает выделенную ветвь в редакторе реестра
Понимает записи вида:
  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control
  HKLM\SYSTEM\CurrentControlSet\Control
  HKLM\\SYSTEM\\CurrentControlSet\\Control
Подключение:
command.name.78.*=Registry Jump
command.78.*=wscript "$(SciteDefaultHome)\tools\RegJump.js"
command.input.78.*=$(CurrentSelection)
command.mode.78.*=subsystem:windows,replaceselection:no,savebefore:no,quiet:yes
command.shortcut.78.*=Ctrl+Alt+J
*/

var key = WScript.StdIn.ReadAll();
if (key === "") {
	WScript.Quit();
}
var WshShell = new ActiveXObject("WScript.Shell");

var LastKey = 'HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit\\Lastkey';
key = key.replace(/^HKLM\\/,'HKEY_LOCAL_MACHINE\\');
key = key.replace(/^HKCR\\/,'HKEY_CLASSES_ROOT\\');
key = key.replace(/^HKCU\\/,'HKEY_CURRENT_USER\\');
key = key.replace(/\\\\/g,'\\');
key = WshShell.RegRead(LastKey).match(/^[^\\]+/) + '\\' + key;

WshShell.RegWrite (LastKey, key,'REG_SZ');
WshShell.Run('regedit -m', 1, false);
WScript.Quit();

