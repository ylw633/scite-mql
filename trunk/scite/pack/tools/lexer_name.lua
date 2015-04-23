--[[--------------------------------------------------
lexer_name.lua
Authors: mozers™, VladVRO
version 1.1.4
------------------------------------------------------
Показ имени текущего лексера в строке статуса

Подключение:
В файл SciTEStartup.lua добавьте строку:
  dofile (props["SciteDefaultHome"].."\\tools\\lexer_name.lua")
включите scite.lexer.name в статусную строку:
  statusbar.text.1=Line:$(LineNumber) Col:$(ColumnNumber) [$(scite.lexer.name)]
--]]--------------------------------------------------

local last_lexer
local function SetPropLexerName()
	if props['FileName'] == '' then return end
	local cur_lexer = props['Language']
	if cur_lexer ~= last_lexer then
		if cur_lexer == "hypertext" then
			props["scite.lexer.name"] = "html"
    elseif props['FileExt'] == 'mq4' or props['FileExt'] == 'mqh' then
      props['scite.lexer.name'] = 'mql'
		else
			props["scite.lexer.name"] = cur_lexer
		end
		last_lexer = cur_lexer
	end
end

-- Добавляем свой обработчик события OnUpdateUI
AddEventHandler("OnUpdateUI", SetPropLexerName)

-- Добавляем свой обработчик события OnSwitchFile
AddEventHandler("OnSwitchFile", SetPropLexerName)
