--[[--------------------------------------------------
goto_line.lua
Author: mozers™
version: 1.1.1
------------------------------------------------------
При переходе на линию, текущее положение курсора на экране сохраняется
для чего текст прокручивается на нужное количество строк.
Теперь вам не придется искать курсор по всему экрану!

  Connection:
   In file SciTEStartup.lua add a line:
      dofile (props["SciteDefaultHome"].."\\tools\\goto_line.lua")
--]]--------------------------------------------------
local bypass = false

local function GotoLine(line)
	bypass = true
	local linecur = editor:LineFromPosition(editor.CurrentPos)
	local linecur_onscreen = linecur - editor.FirstVisibleLine
	editor:GotoLine(line)
	local line_onscreen = line - editor.FirstVisibleLine
	if line_onscreen ~= linecur_onscreen then
		editor:LineScroll(0, line_onscreen - linecur_onscreen)
	end
	bypass = false
end

AddEventHandler("OnSendEditor", function(id_msg, wp, lp)
	if id_msg == SCI_GOTOLINE and not bypass then
		GotoLine(wp)
	end
end)
