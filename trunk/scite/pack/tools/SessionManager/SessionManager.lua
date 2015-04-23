--[[--------------------------------------------------
SessionManager
Authors: mozers™
Version: 1.0.1
--]]--------------------------------------------------

require 'shell'
local sessionmanager_path = props['SciteDefaultHome']..'\\tools\\SessionManager\\SessionManager.hta'

local function LoadSession()
	shell.exec('mshta "'..sessionmanager_path..'"', nil, true, false)
	return true
end

local function SaveSession()
	shell.exec('mshta "'..sessionmanager_path..'" '..props['FileName'], nil, true, false)
	return true
end

local function SaveSessionOnQuit()
	props['save.session']=1
	shell.exec('mshta "'..sessionmanager_path..'" QUIT '..props['FileName'], nil, true, false)
end

local function SaveSessionOnQuitAuto()
	local path = ""
	local i = 0
	repeat
		local filename = props['FileName']..'_'..string.sub('0'..i, -2)
		filename = string.gsub(filename,' ','_')
		path = props['scite.userhome']..'\\'..filename..'.session'
		i = i + 1
	until not shell.fileexists(path)
	local session_file = props['scite.userhome']..'\\SciTE.session'
	os_copy (session_file, path)
end

-- Добавляем свой обработчик события OnMenuCommand
AddEventHandler("OnMenuCommand", function(msg, source)
	if tonumber(props['session.manager'])==1 then
		if msg == IDM_SAVESESSION then
			return SaveSession()
		elseif msg == IDM_LOADSESSION then
			return LoadSession()
		end
	end
end)

-- Добавляем свой обработчик события OnFinalise
-- Сохранение текущей сессиии при закрытии SciTE
AddEventHandler("OnFinalise", function()
	if props['FileName'] ~= '' then
		if tonumber(props['session.manager'])==1 then
			if tonumber(props['save.session.manager.on.quit'])==1 then
				if tonumber(props['save.session.on.quit.auto'])==1 then
					SaveSessionOnQuitAuto()
				else
					SaveSessionOnQuit()
				end
			end
		end
	end
end)
