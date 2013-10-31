------------------------------------------------------------------------------------------------------
-- FastFiles.lua																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description (Requires: Script Manager):															--
--	this file creates a hook for SciTE that speeds up files larger then the max file size			--
--	if used then the buffered draw and two phase draw properties are changed to speed up the editor	--
--																									--
-- How to Install:																					--
--	1) place this file in Script Manager's directory (or one of it's subdirectories)				--
--	2) create a property in the SciTEGlobal.properties file called 'fast.files'						--
--		if the property is 0 then the hook is not used; if it between 1 then the hook is used		--
--		the 'max.file.size' property must be set in order for this to work							--
--	3) either run SciTE or run Refresh Manager and the hook will become active						--
--																									--
--------------------------------------------------------------------------------------------------]]--

------------------------------------------------------------------------------------------------------
-- FastFiles(file)																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is the function speeds up the file opened or switched to if needed							--
--	the actual properties don't change, so regular files still have your properties					--
--																									--
-- Parameters:																						--
--	file - not needed, just the file name															--
--																									--
-- Returns:																							--
--	none																							--
--																									--
--------------------------------------------------------------------------------------------------]]--

function FastFiles(file)
	local max = tonumber(GetProp("max.file.size", 0))
	local mode = GetProp("fast.files", "0")			-- get the property, uninstalled by default
	local size = tonumber(scite.SendEditor(SCI_GETLENGTH))

	local bufdraw = GetProp("buffered.draw", "1")
	local twophase = GetProp("two.phase.draw", "1")

	if mode == '1' and max and max > 0 then			-- if it's installed and there's a max limit
		if max >= size then
			editor.BufferedDraw = bufdraw			-- set buffered.draw to normal
			editor.TwoPhaseDraw = twophase			-- set two.phase.draw to normal
		else
			editor.BufferedDraw = false				-- disable buffered.draw to speed up file
			editor.TwoPhaseDraw = false				-- disable two.phase.draw to speed up file
		end
	end
end

------------------------------------------------------------------------------------------------------
-- Event Registrations																				--
------------------------------------------------------------------------------------------------------

OpenSwitchEvent(FastFiles, -1, 1)					-- we want this to load as soon as possible

------------------------------------------------------------------------------------------------------
