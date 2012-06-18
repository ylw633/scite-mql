------------------------------------------------------------------------------------------------------
-- QuickOutput.lua																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description (Requires: Script Manager):															--
--	this file creates a hook for SciTE that closes and/or clears the output window					--
--	if used the output would do it's action on a doubleclick to the output while holding ctrl down	--
--																									--
-- How to Install:																					--
--	1) place this file in Script Manager's directory (or one of it's subdirectories)				--
--	2) create a property in the SciTEGlobal.properties file called 'quick.output'					--
--		if the property is 0 then the hook is not used; if it between 1 and 3 then the hook is used	--
--		if 1 then close the output, if 2 then clear the output, if 3 the do both					--
--	3) either run SciTE or run Refresh Manager and the hook will become active						--
--																									--
--------------------------------------------------------------------------------------------------]]--

------------------------------------------------------------------------------------------------------
-- QuickOutput()																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is the function that performs the function on the output window							--
--																									--
-- Parameters:																						--
--	none																							--
--																									--
-- Returns:																							--
--	none																							--
--																									--
--------------------------------------------------------------------------------------------------]]--

function QuickOutput()
	local mode = GetProp("quick.output", "0")		-- get the property, uninstalled by default

	if CtrlKeyDown() and output.Focus == true then	-- ctrl key is down and the click is in the output
		if mode == '1' or mode == '3' then			-- close the output window
			MenuCommand('Output')					-- also IDM_TOGGLEOUTPUT
		end

		if mode == '2' or mode == '3' then			-- clear the output window
			output:ClearAll()						-- we want to close before clear (if 3)
		end

		if editor.Focus == true then				-- this could affect select style functions
			MenuCommand("Switch Pane")				-- set focus to the output; also IDM_SWITCHPANE
		end
	end
end

------------------------------------------------------------------------------------------------------
-- Event Registrations																				--
------------------------------------------------------------------------------------------------------

DoubleClickEvent(QuickOutput, -1, 3)				-- set it to semi-high priority

------------------------------------------------------------------------------------------------------
