------------------------------------------------------------------------------------------------------
-- CallTip+.lua																						--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description (Requires: Script Manager):															--
--	this file creates new lexer-based properties for SciTE that controls the calltip window			--
--																									--
-- How to Install:																					--
--	1) place this file in Script Manager's directory (or one of it's subdirectories)				--
--	2) this file is automatically installed, but the program only runs if the properties are used	--
--		calltip.[lexer].back - changes the background color for the calltip window					--
--		calltip.[lexer].fore - changes the foreground color for the calltip window					--
--		calltip.[lexer].hlt - changes the highlighted parameter color for the calltip window		--
--		calltip.[lexer].tabs - sets the length of a tab in a calltip (default = 0 [no tab])			--
--	3) either run SciTE or run Refresh Manager and the hook will become active						--
--																									--
--------------------------------------------------------------------------------------------------]]--

local defaultprop = {								-- these are the default property values
	"#ffffff",										-- the default background color
	"#808080",										-- the default foreground color
	"#00007f",										-- the default highlight color
	"0"												-- the default tab size
}

local proplist = {									-- these are the new properties available
	"calltip.[lexer].back",							-- sets the background color
	"calltip.[lexer].fore",							-- sets the foreground color
	"calltip.[lexer].hlt",							-- sets the highlight color
	"calltip.[lexer].tabs"							-- sets the tab length (0 = no tab)
}

------------------------------------------------------------------------------------------------------
-- FixHexValue(val)																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is local function converts a hex string to a rgb number value (#FFFFFF -> 268435455)		--
--	checks to see if its a hex string just in case													--
--																									--
-- Parameters:																						--
--	val - the string that contains the hex string													--
--																									--
-- Returns:																							--
--	the number value in rgb form																	--
--	if there's an error, it returns the given string												--
--																									--
--------------------------------------------------------------------------------------------------]]

local function FixHexValue(val)
	if string.sub(val, 1, 1) ~= '#' then			-- lets be nice and add it on
		val = '#'..val
	end

	if not string.find(val, '#%x%x%x%x%x%x') then	-- we need 6 hex digits
		if val == '#' then return '' end

		return val									-- return the string
	else
		local str = ''								-- setup a string

		for x = 6, 2, -2 do							-- perform a loop to get the characters
			if x > 2 then y = 1 else y = 2 end

			str = str..string.sub(val, x, x+y)		-- get the hexadecimal digits
		end

		return tonumber(str, 16)					-- return the fixed number
	end
end

------------------------------------------------------------------------------------------------------
-- CustomCallTip(file)																				--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is the function that changes the view of the calltip window								--
--																									--
-- Parameters:																						--
--	file - not needed, just the file name															--
--																									--
-- Returns:																							--
--	none																							--
--																									--
--------------------------------------------------------------------------------------------------]]--

function CustomCallTip(file)
	local newprop = {}								-- define a new property list
	local lex = GetLexerName()						-- this enables different lexer settings

	for x = 1, # proplist do						-- convert the '[lexer]' to the current lexer
		local sub = '%[lexer%]'						-- search for this
		newprop[x] = string.gsub(proplist[x], sub, lex)
	end

	for x = 1, # newprop - 1 do						-- loop through everything but tabs
		local hex = GetProp(newprop[x])				-- get the value

		if FixHexValue(hex) == hex then				-- there was an error
			newprop[x] = FixHexValue(defaultprop[x])-- there won't be an error now
		else
			newprop[x] = FixHexValue(hex)			-- get the property
		end
	end

	newprop[4] = tonumber(GetProp(newprop[4], defaultprop[4]))

	editor.CallTipBack = newprop[1]					-- set all the properties
	editor.CallTipFore = newprop[2]
	editor.CallTipForeHlt = newprop[3]
	editor.CallTipUseStyle = newprop[4]
end

------------------------------------------------------------------------------------------------------
-- Event Registrations																				--
------------------------------------------------------------------------------------------------------

OpenSwitchEvent(CustomCallTip, -1, 3)				-- this isn't shown right away, but keep it high

------------------------------------------------------------------------------------------------------
