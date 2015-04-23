--[[----------------------------------------------------------------------------
HighlightLinks v1.5.3
Автор: VladVRO, mozers

- Подсветка линков в тексте (автоматическая или по команде меню)
- Выделение всего линка двойным кликом на нем
- Открытие линка в броузере двойным кликом с зажатой клавишей Ctrl.

Внимание:
Скрипт работает только в версии SciTE-Ru.
В скрипте используются функции из COMMON.lua (EditorMarkText, EditorClearMarks)
и внешней библиотеки shell (shell.exec)
-----------------------------------------------

Подключение:
Добавить в SciTEStartup.lua строку:
  dofile (props["SciteDefaultHome"].."\\tools\\HighlightLinks.lua")
Настройка:
Создать пункт меню:
  command.name.137.*=Highlight Links
  command.137.*=HighlightLinks
  command.mode.137.*=subsystem:lua,savebefore:no
Задать стиль маркера для подсветки линка:
  indic.style.13=#0000FF,plain
Задать файлы для которых при открытии и при сохранении файла будет автоматически
выполняться подсветка:
в виде списка имен лексеров через запятую (для файлов без лексера имя null)
  highlight.links.lexers=null
или списка расширений файлов через запятую:
  highlight.links.exts=txt,htm

Дополнительный параметр, позволяющий менять маску поиска линков:
  highlight.links.mask=https*://[\w_&%?.\-@$+=*~/]+
--]]----------------------------------------------------------------------------

require "shell"

local mark_number = 13
local default_link_mask = "https*://[^ \t\r\n\"\']+"

function HighlightLinks()
	local link_mask = props['highlight.links.mask']
	if link_mask == '' then link_mask = default_link_mask end
	EditorClearMarks(mark_number)
	local flag = SCFIND_REGEXP
	local s,e = editor:findtext(link_mask, flag, 0)
	while s do
		EditorMarkText(s, e-s, mark_number)
		s,e = editor:findtext(link_mask, flag, e+1)
	end
end

local function select_highlighted_link()
	local p = editor.CurrentPos
	if scite.SendEditor(SCI_INDICATORVALUEAT, mark_number, p) == 1 then
		local s = scite.SendEditor(SCI_INDICATORSTART, mark_number, p)
		local e = scite.SendEditor(SCI_INDICATOREND, mark_number, p)
		if s and e then
			editor:SetSel(s,e)
			return true
		end
	end
	return false
end
AddEventHandler("OnDoubleClick", function(shift, ctrl, alt)
	if editor.Focus then
		if select_highlighted_link() and ctrl then
			local url = editor:GetSelText()
			shell.exec(url)
		end
	end
end)

local function auto_highlight()
	local list_lexers = props['highlight.links.lexers']
	local list_exts = props['highlight.links.exts']
	if (list_lexers ~= '' and string.find(','..list_lexers..',', ','..props['Language']..',')) or
	   (list_exts ~= '' and string.find(','..list_exts..',', ','..props['FileExt']..','))
	then
		HighlightLinks()
	end
end
AddEventHandler("OnOpen", auto_highlight)
AddEventHandler("OnSave", auto_highlight)
