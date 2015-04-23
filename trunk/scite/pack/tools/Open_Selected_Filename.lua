--[[-------------------------------------------------
Open_Selected_Filename.lua
Authors: mozers™, VladVRO
Version: 1.8.0
-----------------------------------------------------
Замена команды "Открыть выделенный файл"
В отличии от встроенной команды SciTE, понимающей только явно заданный путь и относительные пути
обрабатывает переменные SciTE, переменные окружения, конструкции LUA, неполные пути.
При открытии файла с помощью двойного клика мыши на его имени при нажатой клавише Ctrl, в случае неполных путей, поиск будет производится только если не нажата клавиша Shift.
Если файл не найден, то выводится предложение о его создании.
-----------------------------------------------------
Подключение:
Добавьте в SciTEStartup.lua строку
dofile (props["SciteDefaultHome"].."\\tools\\Open_Selected_Filename.lua")

Параметром open.selected.filename.minlength можно задать минимальную длину выделенной строки, которая будет анализироваться как возможное имя файла.
По умолчанию open.selected.filename.minlength=4

Параметр open.filename.by.click=1 разрешает открытие файла с помощью двойного клика мыши на его имени при нажатой клавише Ctrl
-----------------------------------------------------
Connection:
In file SciTEStartup.lua add a line:
dofile (props["SciteDefaultHome"].."\\tools\\Open_Selected_Filename.lua")
--]]-------------------------------------------------

require 'shell'
require 'gui'
local open_selected_filename_minlength = tonumber(props['open.selected.filename.minlength']) or 4

-- Ищет файл в текущем и дочерних каталогах
local function FindFileDown(filename)
	local findfile
	local function DIR(path)
		local files = gui.files(path..'\\'..filename)
		if files then
			for _, file in pairs(files) do
				if filename:find('\\') then file = filename:gsub('[^\\]*$','')..file end
				findfile = path..'\\'..file
				return
			end
		end
		local folders = gui.files(path..'\\*', true)
		if folders then
			for _, folder in pairs(folders) do
				DIR(path..'\\'..folder)
			end
		end
	end
	DIR(props['FileDir'])
	return findfile
end

-- Ищет файл в родительских каталогах
local function FindFileUp(filename)
	local path = props['FileDir']
	repeat
		path = path:gsub('\\[^\\]+$', '')
		filepath = path..'\\'..filename
		if shell.fileexists(filepath) then return filepath end
	until #path < 3
end

-- Возвращает абсолютный путь к файлу
local function GetOpenFilePath(text, shift)
	-- Example: $(SciteDefaultHome)\tools\RestoreRecent.js
	local pattern_sci = '^$[(](.-)[)]'
	local _, _, scite_var = string.find(text,pattern_sci)
	if scite_var then
		return string.gsub(text, pattern_sci, props[scite_var])
	end

	-- Example: %APPDATA%\Opera\Opera\profile\opera6.ini
	local pattern_env = '^[%%](.-)[%%]'
	local _, _, os_env = string.find(text, pattern_env)
	if os_env then
		return string.gsub(text, pattern_env, os.getenv(os_env))
	end

	-- Example: props["SciteDefaultHome"].."\\tools\\Zoom.lua"
	local pattern_props = '^props%[%p(.-)%p%]%.%.%p(.*)%p'
	local _, _, scite_prop1, scite_prop2 = string.find(text, pattern_props)
	if scite_prop1 then
		return props[scite_prop1]..scite_prop2
	end

	if not shift then -- поиск производится только если не нажата клавиша Shift
		-- Example: ..\LuaLib\re.lua
		local files = FindFileDown(text)
		if files then
			return files
		end

		-- Example: ..\languages\css.properties
		local filepath = FindFileUp(text)
		if filepath then
			return filepath
		end
	end
end

local function GetSelText()
	local pane = editor.Focus and editor or output
	local text = pane:GetSelText()
	return text:to_utf8(pane:codepage())
end

local function GetClickedText()
	local pane = editor.Focus and editor or output
	local cur_line, pos_cur = pane:GetCurLine()
	local pos_start, pos_end = 0, 0
	local q = "'"
	local no_filepath_chars = '%s=:,*?<>|"'..q  -- символы, недопутимые в имени файла
	local no_filepath_end_char = '.\\/[('       -- символы, недопутимые в последнем символе имени файла
	local re = '%a?:?\\?[^'..no_filepath_chars..']+[^'..no_filepath_chars..no_filepath_end_char..']'
	repeat
		pos_end = pos_end + 1
		pos_start, pos_end = cur_line:find(re, pos_end)
		if not pos_start then return end
	until (pos_start-1 <= pos_cur) and (pos_end >= pos_cur)
	local line_start_pos = pane:PositionFromLine(pane:LineFromPosition(pane.CurrentPos))
	pane:SetSel(line_start_pos+pos_start-1, line_start_pos+pos_end)
	return cur_line:sub(pos_start,pos_end):to_utf8(pane:codepage())
end

local function OpenSelectedFilename(text, shift)
	if #text < open_selected_filename_minlength then return end
	if text:find('^file://') then return end
	if text:find('^https*://') then return end
	if text:find(':%d+$') then return end
	text = text:gsub('/', '\\')
	local filename = GetOpenFilePath(text, shift) or text
	if not (filename:find('^%a:\\') or filename:find('\\\\')) then
		filename = props['FileDir']..'\\'..filename
	end
	if shell.fileexists(filename) then
		scite.Open (filename)
	else
		-- Создание нового файла
		local alert = scite.GetTranslation('File')..' "'..filename..'" '..scite.GetTranslation('does not exist\nYou want to create a file with that name?')
		if shell.msgbox(alert, scite.GetTranslation('Create New File'), 4+256) == 6 then
			local folder = filename:gsub('\\[^\\]-$', '')
			if not shell.fileexists(folder) then
				shell.exec('cmd /c md "'..folder..'"', nil, true, true)
			end
			local warning_couldnotopenfile_disable = props['warning.couldnotopenfile.disable']
			props['warning.couldnotopenfile.disable'] = 1
			scite.Open(filename)
			props['warning.couldnotopenfile.disable'] = warning_couldnotopenfile_disable
		end
	end
	return true
end

AddEventHandler("OnMenuCommand", function(msg, source)
	if msg == IDM_OPENSELECTED then
		return OpenSelectedFilename(GetSelText())
	end
end)

AddEventHandler("OnDoubleClick", function(shift, ctrl, alt)
	if ctrl and not alt and props["open.filename.by.click"] == "1" then
		return OpenSelectedFilename(GetClickedText(), shift)
	end
end)
