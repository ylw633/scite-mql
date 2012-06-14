------------------------------------------------------------------------------------------------------
-- FoldSymbols+.lua																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description (Requires: Script Manager):															--
--	this file creates the ability for users to easily create new folding symbols for SciTE			--
--	note: because this script works with pixmaps, an error can close SciTE and cause alot of issues	--
--	therefore, I made this as safe and smart as possible, however, I am not responsible for any		--
--	errors that occur, but I am confident in saying that if used correctly, there won't be errors	--
--																									--
--	you will have to define each item, even if you don't want a picture for it (just make it blank)	--
--	if you don't understand what property does what, just fiddle around with it, it's really simple	--
--																									--
-- How to Install:																					--
--	1) place this file in Script Manager's directory (or one of it's subdirectories)				--
--	2) this file is automatically installed, but it only runs if the fold.symbols property is > 3	--
--		if fold.symbols is above 3 then the program looks for the following properties: (xpm images)--
--		fold.[number].plus.connect, fold.[number].minus.connect, fold.[number].tcurve,				--
--		fold.[number].lcurve, fold.[number].vline, fold.[number].plus, and fold.[number].minus		--
--	3) either run SciTE or run Refresh Manager and the hook will become active						--
--																									--
--------------------------------------------------------------------------------------------------]]--

local proplist = {									-- these are the new properties available
		"fold.[number].plus.connect",				-- also SC_MARKNUM_FOLDEREND
		"fold.[number].minus.connect",				-- also SC_MARKNUM_FOLDEROPENMID
		"fold.[number].tcurve",						-- also SC_MARKNUM_FOLDERMIDTAIL
		"fold.[number].lcurve",						-- also SC_MARKNUM_FOLDERTAIL
		"fold.[number].vline",						-- also SC_MARKNUM_FOLDERSUB
		"fold.[number].plus",						-- also SC_MARKNUM_FOLDER
		"fold.[number].minus"						-- also SC_MARKNUM_FOLDEROPEN
}

------------------------------------------------------------------------------------------------------
-- CustomFold(file)																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is the function obtains and draws the new fold symbols										--
--	note: the value of the properties has to be the xpm image in order for this to work				--
--																									--
-- Parameters:																						--
--	file - not needed, just the file name															--
--																									--
-- Returns:																							--
--	none																							--
--																									--
--------------------------------------------------------------------------------------------------]]--

function CustomFold(file)
	local newprop = {}								-- create a new property list table
	local mode = GetProp("fold.symbols", "1")		-- get the property, set it to 1 by default

	if mode > '3' then								-- use the custom symbols
		local exist = true							-- everything is good by default

		local header = '^/%* XPM %*/'				-- we have to try our best here (this is risky)
		local name = 'static char[ ]+*[ ]+[^ %[%]]+%[%]={'
		local number = '"[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+1",'
		local body = '.*};$'						-- this is the best format we can find

		local valid = header..name..number..body

		for x = 1, # proplist do					-- set the [number] section
			newprop[x] = string.gsub(proplist[x], '%[number%]', mode)

			local xpm = GetProp(newprop[x])			-- get the property

			if xpm == '' or not string.find(xpm, valid) then
				exist = false						-- mark it as a problem
				break								-- we're done
			else									-- set the property value
				newprop[x] = GetProp(newprop[x])	-- we know it exists, no default needed
			end
		end

		if not exist then							-- temporarily change it to default
			style1 = { 5, 5, 5, 5, 5, 8, 7 }		-- ignore the values it's fold.symbols = 1
			fore, back = 268435455, 0				-- this sets the right colors

			for x = 1, 7 do							-- loop through all the values
				editor:MarkerSetFore(x+24, fore)
				editor:MarkerSetBack(x+24, back)
				editor:MarkerDefine(x+24, style1[x])
			end
		else
			for x = 1, 7 do 						-- loop through all the values
				editor:MarkerDefinePixmap(x+24, newprop[x])
			end
		end
	else
		return false								-- continue as normal
	end
end

------------------------------------------------------------------------------------------------------
-- Event Registrations																				--
------------------------------------------------------------------------------------------------------

OpenSwitchEvent(CustomFold, -1, 1)				-- set it high, because it's shown on opening

------------------------------------------------------------------------------------------------------
