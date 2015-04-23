--[[--------------------------------------------------
ShowCalltip.lua
Show calltip for current word
Authors: mozers™, TymurGubayev
version 1.2.4
------------------------------------------------------
Выводит всплывающую подсказку по слову на котором стоит курсор
  по команде меню "Показать подсказку" (Ctrl+Shift+Space) 
  (если, конечно, соответствующий каллтип задан в api-файле).
Если вывод подсказки возможен внутренними средствами SciTE,
  то скриптовый вариант игнорируется.
------------------------------------------------------
Для подключения добавьте в свой SciTEStartup.lua строку:
  dofile (props["SciteDefaultHome"].."\\tools\\ShowCalltip.lua")

ВНИМАНИЕ: В скрипте используются функции из COMMON.lua (string.pattern, GetCurrentWord)
--]]--------------------------------------------------

local function ShowCalltip()
	local word = GetCurrentWord()
	if #word < 1 then return end
	for api_filename in string.gmatch(props["APIPath"], "[^;]+") do
		if api_filename ~= '' then
			local api_file = io.open(api_filename)
			if api_file then
				for line in api_file:lines() do
					local _start, _end, calltip = line:find('^('..word:pattern()..'[^%w%.%_%:].+)')
					if _start == 1 then
						editor:CallTipCancel()
						local cp = editor:codepage()
						if cp ~= 65001 then calltip=calltip:from_utf8(cp) end
						editor:CallTipShow(editor.CurrentPos, calltip:gsub('\\n','\n'))
						editor:CallTipSetHlt(0, #word)
						break
					end
				end
				api_file:close()
			end
		end
	end
end

AddEventHandler("OnMenuCommand", function(msg, source)
	if msg == IDM_SHOWCALLTIP then
		ShowCalltip()
	end
end)
