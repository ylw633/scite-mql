--[[--------------------------------------------------
RestoreRecent.lua
Authors: mozers™
Version: 1.2.4
------------------------------------------------------
Description:
  Restore position, bookmarks, folds at opening recent file
  Восстанавливает позицию курсора, букмарки и фолдинг при повторном открытии файла
  (используются данные файла SciTE.session, который должен быть подключен директивой
  import home\SciTE.session
  Параметр save.session.recent=1 позволяет использовать для сохранения и восстановления данных файл SciTE.recent
  В этом случае позиция курсора, букмарки и фолдинг восстанавливается у ВСЕХ открывавыемых файлов.
  Эта опция работает при наличии скрипта RestoreRecent.js, расположенном в том же каталоге что и данный скрипт.

  Также скрипт автоматически сворачивает все секции при открытии файлов
  с расширениями заданными параметром fold.on.open.ext
------------------------------------------------------
Connection:
 In file SciTEStartup.lua add a line:
    dofile (props["SciteDefaultHome"].."\\tools\\RestoreRecent.lua")

 Set in a file .properties (optional):
    save.session.recent=1
    fold.on.open.ext=properties,ini
--]]--------------------------------------------------
require 'shell'

----------------------
-- ON STARTUP SCITE --
----------------------
local buffers = {} -- масив {номер_файла, {имя_параметра, значение_параметра} }
local opened = {} -- массив в который заносятся пути ко всем открываемым файлам

-- Чтение параметров SciTE.session в таблицу buffers (параметры доступны, поскольку файл подключен директивой import)
local function ReadSessionToTable()
	for i = 1, props['buffers'] do
		local path = props['buffer.'..i..'.path']
		if path ~= '' then
			-- если элемент таблицы отсутствует, то создаем его (как вложенную таблицу)
			if buffers[i] == nil then buffers[i] = {} end
			buffers[i].path = path
		else
			break
		end

		local position = props['buffer.'..i..'.position']
		if position ~= '' then buffers[i].position = position end

		local bookmarks = props['buffer.'..i..'.bookmarks']
		if bookmarks ~= '' then buffers[i].bookmarks = bookmarks end

		local folds = props['buffer.'..i..'.folds']
		if folds ~= '' then buffers[i].folds = folds end
	end
end

-- Чтение параметров SciTE.recent в таблицу buffers
local function ReadRecentToTable()
	local recent_file = io.open(props['SciteUserHome']..'\\SciTE.recent')
	if recent_file then
		local pattern = "buffer%.(%d+)%.(%a+)=(.+)"
		for line in recent_file:lines() do
			if #line > 10 then
				-- номер_файла, имя_параметра, значение:
				local num, prop, value = string.match (line, pattern)
				if num ~= nil then
					num = tonumber(num)
					-- если элемент таблицы отсутствует, то создаем его (как вложенную таблицу)
					if buffers[num] == nil then buffers[num] = {} end
					buffers[num][prop] = value
				end
			end
		end
		recent_file:close()
	end
end

-- В зависимости от наличия параметра save.session.recent=1 выбираем
-- данными из какого файла будем заполнять таблицу buffers
if tonumber(props['save.session.recent']) == 1 then
	ReadRecentToTable() -- данные читаются из файла SciTE.recent
else
	ReadSessionToTable() -- данные берутся из параметров SciTE.session
end

------------------
-- ON OPEN FILE --
------------------

-- Проверка наличия в таблице buffers записи о текущем файле
local function CheckSession()
	for i = 1, #buffers do
		-- при наличии записи возвращаем массив всех параметров (и их значений) для этого файла
		if buffers[i]['path']:lower() == props['FilePath']:lower() then return buffers[i] end
	end
end

-- Восстановление позиции курсора, букмарков и фолдинга для заданного файла
local function Restore(file)
	local FileParams = CheckSession() -- проверка наличия данных о файле в таблице buffers
	if FileParams ~= nil then
		-- Restore folding
		if tonumber(props['session.folds']) == 1 then
			local folds = FileParams['folds']
			if folds ~= nil then
				for line_num in string.gmatch(folds, "%d+") do
					line_num = tonumber(line_num)-1
					if editor.FoldExpanded[line_num] then
						editor:ToggleFold(line_num)
					end
				end
			end
		end
		-- Restore bookmarks
		if tonumber(props['session.bookmarks']) == 1 then
			local bookmarks = FileParams['bookmarks']
			if bookmarks ~= nil then
				for line_num in string.gmatch(bookmarks, "%d+") do
					editor:MarkerAdd(tonumber(line_num)-1, 1)
				end
			end
		end
		-- Restore position
		if tonumber(props['save.position']) == 1 then
			local pos = FileParams['position']
			if pos ~= nil then
				editor:GotoPos(pos-1)
			end
		end
	else
		local toggle_foldall_ext = props['fold.on.open.ext']:lower()
		local current_ext = props['FileExt']:lower()
		for ext in toggle_foldall_ext:gmatch("%w+") do
			if current_ext == ext then scite.MenuCommand (IDM_TOGGLE_FOLDALL) end
		end
	end
end

AddEventHandler("OnOpen", function(file)
	if tonumber(props['save.session']) == 1 then
		if file ~= '' then opened[file] = true end
	end
end)

AddEventHandler("OnUpdateUI", function()
	local file = props["FilePath"]
	if opened[file] then
		Restore(file)
		opened[file] = nil
	end
end)

-----------------------
-- ON FINALISE SCITE --
-----------------------

AddEventHandler("OnFinalise", function()
	if props['FileName'] ~= '' then
		if tonumber(props['save.session.recent']) == 1 then
			-- Запуск вспомогательного скрипта для сохранения данных в SciTE.recent
			-- (заодно в ком.строке передаем в вызываемый скрипт данные о местоположении SciteUserHome)
			local script_dir = debug.getinfo(1, "S").source:gsub('^@(.+\\).-$', '%1')
			script_dir = script_dir
			local cmd = 'wscript "'..script_dir..'RestoreRecent.js" "'..props["SciteUserHome"]..'"'
			shell.exec(cmd, nil, true, false)
		end
	end
end)
