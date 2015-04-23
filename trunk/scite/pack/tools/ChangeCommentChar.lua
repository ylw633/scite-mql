--[[--------------------------------------------------
ChangeCommentChar.lua
Authors: VladVRO, mozers™
Version: 1.1.0
------------------------------------------------------
Подставляет адекватный символ комментария для файлов обрабатываемых лексером props
(*.properties;*.abbrev;*.session;*.ini;*.inf;*.reg;*.url;*.cfg;*.cnf;*.aut;*.m3u)
Подставляет адекватный символы потокового комментария для файлов *.php
------------------------------------------------------
Connection:
 In file SciTEStartup.lua add a line:
    dofile (props["SciteDefaultHome"].."\\tools\\ChangeCommentChar.lua")
--]]--------------------------------------------------

local function ChangeCommentChar()
	function IsINI()
		local ini = {'ini', 'inf', 'reg'}
		local ext = props['FileExt']:lower()
		for _, x in pairs(ini) do
			if x == ext then return true end
		end
		return false
	end
	if props['Language'] == 'props' then
		if IsINI() then
			props['comment.block.props']=';'
		else
			props['comment.block.props']='#'
		end
	elseif props['Language'] == 'hypertext' then
		if props['FileExt']:lower() == 'php' then
			props['comment.stream.start.hypertext']='/*'
			props['comment.stream.end.hypertext']='*/'
		else
			props['comment.stream.start.hypertext']='<!--'
			props['comment.stream.end.hypertext']='-->'
		end
	end
end

-- Добавляем свой обработчик события OnSwitchFile
AddEventHandler("OnSwitchFile", ChangeCommentChar)

-- Добавляем свой обработчик события OnOpen
AddEventHandler("OnOpen", ChangeCommentChar)
