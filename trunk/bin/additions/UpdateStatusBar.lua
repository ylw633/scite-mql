------------------------------------------------------------------------------------------------------
-- UpdateStatusBar.lua																				--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description (Requires: Script Manager):															--
--	this file creates a hook for SciTE that updates the statusbar (used for updating data and time)	--
--	if used the statusbar updates if the UI is updated, margin is clicked, or dwell begins or ends	--
--																									--
-- How to Install:																					--
--	1) place this file in Script Manager's directory (or one of it's subdirectories)				--
--	2) create a property in the SciTEGlobal.properties file called 'update.status.bar'				--
--		if the property is 0 then the hook is not used; if it is 1 then the hook is used			--
--	3) either run SciTE or run Refresh Manager and the hook will become active						--
--																									--
--------------------------------------------------------------------------------------------------]]--

------------------------------------------------------------------------------------------------------
-- UpdateStatusBar()																				--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is the function that updates the statusbar													--
--																									--
-- Parameters:																						--
--	none																							--
--																									--
-- Returns:																							--
--	none																							--
--																									--
--------------------------------------------------------------------------------------------------]]--

function UpdateStatusBar()
	local mode = GetProp("update.status.bar", "0")	-- get the property, uninstalled by default

	if mode == '1' then								-- if the hook is installed
		scite:UpdateStatusBar()						-- yea, it's that easy
	end
end

------------------------------------------------------------------------------------------------------
-- Event Registrations																				--
------------------------------------------------------------------------------------------------------

DwellEvent(UpdateStatusBar, -1, 1)					-- register the event holders
MarginEvent(UpdateStatusBar, -1, 1)					-- never remove this function
UpdateUIEvent(UpdateStatusBar, -1, 1)				-- set it to top priority

------------------------------------------------------------------------------------------------------
