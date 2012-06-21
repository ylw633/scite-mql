-----------------------------------------
--ShowCalltip
-----------------------------------------

local function GetCurrentWord()
	local current_pos = editor.CurrentPos
	return editor:textrange(editor:WordStartPosition(current_pos, true),
							editor:WordEndPosition(current_pos, true))
end

local function ShowCalltip()
	local word = GetCurrentWord()
	if #word < 1 then return end
	for api_filename in string.gmatch(props["APIPath"], "[^;]+") do
		if api_filename ~= '' then
			local api_file = io.open(api_filename)
			if not api_file then return end
			for line in api_file:lines() do
				local _start, _end, calltip = line:find('^('..word..'[^%w%.%_%:].+)')
				if _start == 1 then
					editor:CallTipCancel()
					editor:CallTipShow(editor.CurrentPos, calltip:gsub('\\n','\n'))
					editor:CallTipSetHlt(0, #word)
					break
				end
			end
			api_file:close()
		end
	end
end

---------------------------------------
-- Add user event handler OnMenuCommand
---------------------------------------
local old_OnMenuCommand = OnMenuCommand
function OnMenuCommand (msg, source)
	local result
	if old_OnMenuCommand then result = old_OnMenuCommand(msg, source) end
	if msg == IDM_SHOWCALLTIP then
		ShowCalltip()
	end
	return result
end