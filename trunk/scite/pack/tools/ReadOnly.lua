--[[----------------------------------------------------------------------------
ReadOnly.lua
Author: VladVRO, mozers
version 1.3.1

Наглядная установка/снятие режима "только для чтения"
индикация режима ReadOnly в строке состояния, стилем поля
с нумерацией строк и отключением мерцания курсора

Подключение:
В файл SciTEStartup.lua добавьте строку:
  dofile (props["SciteDefaultHome"].."\\tools\\ReadOnly.lua")
включите scite.readonly в статусную строку:
  statusbar.text.1=Line:$(LineNumber) Col:$(ColumnNumber) [$(scite.readonly)]
задайте в файле .properties цвет фона поля с нумерацией строк в режиме "только для чтения":
  style.back.readonly=#FFEEEE
--]]----------------------------------------------------------------------------

local function SetReadOnly(ro)
	if ro then
		if props["normal.style.saved"] == "" then
			props["normal.style.saved"] = "1"
			props["caret.period.normal"] = props["caret.period"]
			props["caret.width.normal"] = props["caret.width"]
			props["style.*.33.normal"] = props["style.*.33"]
		end

		props["caret.period"] = 0
		props["caret.width"] = 0
		if props["style.back.readonly"] ~= "" then
			props["style.*.33"] = props["style.*.33"]..",back:"..props["style.back.readonly"]
		end
		scite.Perform("reloadproperties:")

		props["scite.readonly"] = "VIEW"

	else
		if props["scite.readonly"] == "VIEW" then
			props["style.*.33"] = props["style.*.33.normal"]
			props["caret.period"] = props["caret.period.normal"]
			props["caret.width"] = props["caret.width.normal"]
			scite.Perform("reloadproperties:")
		end

		props["scite.readonly"] = "EDIT"
	end

	scite.UpdateStatusBar()
end

-- Добавляем свой обработчик события OnSwitchFile
AddEventHandler("OnSwitchFile", function(file)
	SetReadOnly(editor.ReadOnly)
end)

-- Добавляем свой обработчик события OnOpen
AddEventHandler("OnOpen", function(file)
	SetReadOnly(editor.ReadOnly)
end)

-- Добавляем свой обработчик события, возникающего при вызове пункта меню "Read-Only"
AddEventHandler("OnSendEditor", function(id_msg, wp, lp)
	if id_msg == SCI_SETREADONLY then
		SetReadOnly(wp~=0)
	end
end)
