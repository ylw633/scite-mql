-- editing colour specifications in property files.
-- you will need extman to use this directly; otherwise comment out the scite_Command
-- line and make a command pointing to edit_colour in the old-fashioned way.
scite_Command 'Edit Colour|edit_colour|Ctrl+Shift+C'

function edit_colour ()
    local function getch (i)
	    return editor:textrange(i,i+1)
	end
	local function hexdigit (c)
	    return c:find('[0-9A-F]')==1
	end
	local i = editor.CurrentPos
	-- let's find the extents of this colour field...
	local ch = getch(i)
	while i > 0 and ch ~= '#' and hexdigit(ch) do
		i = i - 1
		ch = getch(i)
	end
	if i == 0 then return end
	local istart = i
	i = i + 1 -- skip the '#'
	while hexdigit(getch(i)) do i = i + 1 end
	-- extract the colour!
	local colour = editor:textrange(istart,i)
	colour = gui.colour_dlg(colour)
	if colour then -- replace the colour in the document
		editor:SetSel(istart,i)
		editor:ReplaceSel(colour)
	end
end
