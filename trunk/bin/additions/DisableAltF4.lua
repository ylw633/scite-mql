------------------------------------------------------------------------------------------------------
-- DisableAltF4.lua																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description (Requires: Script Manager):															--
--	this file creates a hook for SciTE that nulls the alt+f4 keycode (I find it useful)				--
--	if used the alt+f4 keycode will have no effect													--
--																									--
-- How to Install:																					--
--	1) place this file in Script Manager's directory (or one of it's subdirectories)				--
--	2) create a property in the SciTEGlobal.properties file called 'disable.alt.f4'					--
--		if the property is 0 then the hook is not used; if it between 1 then the hook is used		--
--	3) either run SciTE or run Refresh Manager and the hook will become active						--
--																									--
--------------------------------------------------------------------------------------------------]]--

------------------------------------------------------------------------------------------------------
-- DisableAltF4(key, shift, ctrl, alt)																--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is the function that nulls the alt+f4 keycode												--
--																									--
-- Parameters:																						--
--	key - the keycode of the key pressed															--
--	shift - bool: true if used																		--
--	ctrl - bool: true if used																		--
--	alt - bool: true if used																		--
--																									--
-- Returns:																							--
--	false - don't block the keycode (SciTE ends)													--
--	true - block the keycode (interrupt processing)													--
--																									--
--------------------------------------------------------------------------------------------------]]--

function DisableAltF4(key, shift, ctrl, alt)
	local mode = GetProp("disable.alt.f4", "0")		-- get the property, uninstalled by default
	local f4 = 115									-- the f4 keycode

	if key == f4 and alt == true then				-- the keycode was used
		if mode == '0' then
			return false							-- end SciTE
		elseif mode == '1' then
			return true								-- continue SciTE
		end
	end
end

------------------------------------------------------------------------------------------------------
-- Event Registrations																				--
------------------------------------------------------------------------------------------------------

KeyEvent(DisableAltF4, -1, 1)						-- if it's not really high, there could be issues

------------------------------------------------------------------------------------------------------
