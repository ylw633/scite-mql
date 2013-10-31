------------------------------------------------------------------------------------------------------
-- SelectStyle.lua																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description (Requires: Script Manager):															--
--	this file creates a hook for SciTE that extends the selection of text to a style				--
--	if used a selected text will extend through a whole continuous style, if ctrl is held down		--
--	this adds more flexibility then other select style functions because now you can do both		--
--																									--
-- How to Install:																					--
--	1) place this file in Script Manager's directory (or one of it's subdirectories)				--
--	2) create a property in the SciTEGlobal.properties file called 'ctrl.sel.style'					--
--		if the property is 0 then the hook is not used; if it between 1 then the hook is used		--
--	3) either run SciTE or run Refresh Manager and the hook will become active						--
--																									--
--------------------------------------------------------------------------------------------------]]--

------------------------------------------------------------------------------------------------------
-- SelectStyle()																					--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	this is the function that extends a selection throughout a style								--
--		if you've never used this kind of function before then install it and select this info box	--
--		if you notice, it selected the entire box, not just the line								--
--		note it does not select the text 'SelectStyle()' because it's a different style				--
--																									--
-- Parameters:																						--
--	none																							--
--																									--
-- Returns:																							--
--	none																							--
--																									--
--------------------------------------------------------------------------------------------------]]--

function SelectStyle()
	local mode = GetProp("ctrl.sel.style", "0")		-- get the property, uninstalled by default

	if CtrlKeyDown() and mode == '1' and editor.Focus == true then
		local pos = editor.CurrentPos				-- get the current position
		local style = editor.StyleAt[pos]			-- retrieve the current style (0 is valid)

		local pre = pos								-- used to determine the start
		while pre > -1 and editor.StyleAt[pre] == style do
			pre = pre - 1							-- loop to the beginning of it
		end

		local pro = pos								-- used to determine the end
		local len = editor.Length-1					-- the length of the file

		if pre+1 == pos then return end				-- we're at the end, don't bother

		while pro < len and editor.StyleAt[pro] == style do
				pro = pro + 1						-- loop to the end of it
		end

		editor:SetSel(pre+1, pro)					-- select the entire style
	end
end

------------------------------------------------------------------------------------------------------
-- Event Registrations																				--
------------------------------------------------------------------------------------------------------

DoubleClickEvent(SelectStyle, -1, 3)				-- set it to semi-high priority

------------------------------------------------------------------------------------------------------
