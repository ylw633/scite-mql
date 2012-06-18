------------------------------------------------------------------------------------------------------
-- AutoComplete+.lua																				--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description (Requires: Script Manager):															--
--	this file creates new lexer-based properties for SciTE that controls the autocompletion window	--
--	note: because this script works with pixmaps, an error can close SciTE and cause alot of issues	--
--	therefore, I made this as safe and smart as possible, however, I am not responsible for any		--
--	errors that occur, but I am confident in saying that if used correctly, there won't be errors	--
--																									--
-- How to Install:																					--
--	1) place this file in Script Manager's directory (or one of it's subdirectories)				--
--	2) this file is automatically installed, but the program only runs if the properties are used	--
--		autocomplete.[lexer].row - changes the number of rows for the autocomplete window			--
--		autocomplete.[lexer].[number] - enables pixmaps to be used easily (number = 0 - 9)			--
--	3) either run SciTE or run Refresh Manager and the hook will become active						--
--																									--
--------------------------------------------------------------------------------------------------]]--

local proplist = {									-- these are the new properties available
	"autocomplete.[lexer].rows",					-- sets the number of rows to display (default: 5)
	"autocomplete.[lexer].[number]"					-- creates pixmaps (0-9) 0 is the default pixmap
}

------------------------------------------------------------------------------------------------------
-- CustomAutoComplete(file)																			--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is the function that searches for the properties and changes the autocomplete properties	--
--																									--
-- Parameters:																						--
--	file - not needed, just the file name															--
--																									--
-- Returns:																							--
--	none																							--
--																									--
--------------------------------------------------------------------------------------------------]]--

function CustomAutoComplete(file)
	local newprop = {}								-- define a new property list
	local lex = GetLexerName()						-- this enables different lexer settings

	for x = 1, # proplist do						-- convert the '[lexer]' to the current lexer
		local sub = '%[lexer%]'						-- search for this
		newprop[x] = string.gsub(proplist[x], sub, lex)
	end

	local rows = tonumber(GetProp(newprop[1], "5"))	-- get the property, or the default
	if rows < 1 or rows > 50 then					-- let's be reasonible here
		rows = 5
	end
	editor.AutoCMaxHeight = rows					-- set the row height (max)

	for x = 0, 9 do									-- expand the '[number]' property
		local base = "autocomplete."..lex			-- set the base of the property
		table.insert(newprop, base..'.'..tostring(x))
	end
	table.remove(newprop, 2)						-- get rid of the [number] item

	editor:ClearRegisteredImages()					-- clear all the registered images

	for x = 2, # newprop do							-- format all the number properties
		local header = '^/%* XPM %*/'				-- we have to try our best here (this is risky)
		local name = 'static char[ ]+*[ ]+[^ %[%]]+%[%]={'
		local number = '"[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+1",'
		local body = '.*};$'						-- this is the best format we can find

		local valid = header..name..number..body

		newprop[x] = string.gsub(newprop[x], '%$%((.+)%)', '\1')
		local xpm = GetProp(props[newprop[x]])

		if xpm ~= '' and string.find(xpm, valid) then
			if x == 2 then x = 1 end					-- just fix it so it can be -1
			editor:RegisterImage(x-2, xpm)				-- register the pixmap
		end
	end
end

------------------------------------------------------------------------------------------------------
-- Event Registrations																				--
------------------------------------------------------------------------------------------------------

OpenSwitchEvent(CustomAutoComplete, -1, 2)			-- this isn't shown right away, but keep it high

------------------------------------------------------------------------------------------------------
