local t = {}
-- t.tag_start, t.tag_end, t.paired_start, t.paired_end  -- positions
-- t.begin, t.finish  -- contents of tags (when copying)
local old_current_pos

function CopyTags()
	if t.tag_start == nil then
		print("Error : "..scite.GetTranslation("Move the cursor on a tag to copy it!"))
		return
	end
	local tag = editor:textrange(t.tag_start, t.tag_end+1)
	if t.paired_start~=nil then
		local paired = editor:textrange(t.paired_start, t.paired_end+1)
		if t.tag_start < t.paired_start then
			t.begin = tag
			t.finish = paired
		else
			t.begin = paired
			t.finish = tag
		end
	else
		t.begin = tag
		t.finish = nil
	end
end

function PasteTags()
	if t.begin~=nil then
		if t.finish~=nil then
			local sel_text = editor:GetSelText()
			editor:ReplaceSel(t.begin..sel_text..t.finish)
			if sel_text == '' then
				editor:GotoPos(editor.CurrentPos - #t.finish)
			end
		else
			editor:ReplaceSel(t.begin)
		end
	end
end

function DeleteTags()
	if t.tag_start~=nil then
		editor:BeginUndoAction()
		if t.paired_start~=nil then
			if t.tag_start < t.paired_start then
				editor:SetSel(t.paired_start, t.paired_end+1)
				editor:DeleteBack()
				editor:SetSel(t.tag_start, t.tag_end+1)
				editor:DeleteBack()
			else
				editor:SetSel(t.tag_start, t.tag_end+1)
				editor:DeleteBack()
				editor:SetSel(t.paired_start, t.paired_end+1)
				editor:DeleteBack()
			end
		else
			editor:SetSel(t.tag_start, t.tag_end+1)
			editor:DeleteBack()
		end
		editor:EndUndoAction()
	else
		print("Error : "..scite.GetTranslation("Move the cursor on a tag to delete it!"))
	end
end

function highlighting_paired_tags_switch()
	local prop_name = 'hypertext.highlighting.paired.tags'
	props[prop_name] = 1 - tonumber(props[prop_name])
	EditorClearMarks(0)
	EditorClearMarks(2)
end

local function PairedTagsFinder()
	local current_pos = editor.CurrentPos
	if current_pos == old_current_pos then return end
	old_current_pos = current_pos

	EditorClearMarks(0)
	EditorClearMarks(2)

	t.tag_start = nil
	t.tag_end = nil
	t.paired_start = nil
	t.paired_end = nil

	local tag_start = editor:findtext("[<{}>]", SCFIND_REGEXP, current_pos, 0)
	if tag_start == nil then return end
	if editor.CharAt[tag_start] ~= 60  and editor.CharAt[tag_start] ~= 123 then return end
	t.style = editor.StyleAt[tag_start+1]
	if t.style ~= 1 and t.style ~= 2 then return end
	if tag_start == t.tag_start then return end
	t.tag_start = tag_start
	 

	local tag_end = editor:findtext("[<{}>]", SCFIND_REGEXP, current_pos, editor.Length)
	if tag_end == nil then return end
	if editor.CharAt[tag_end] ~= 62 and editor.CharAt[tag_end] ~= 125 then t.tag_end = nil return end
	t.tag_end = tag_end

	if editor.CharAt[t.tag_end-1] == 47 then
		-- paint in green
		EditorMarkText(t.tag_start, t.tag_end - t.tag_start + 1, 0)
		return
	end

	local dec, find_end
	if editor.CharAt[t.tag_start+1] == 47 then
		dec, find_end = -1, 0
	else
		dec, find_end =  1, editor.Length
	end

	-- Find paired tag
	local begin_pos,end_pos = editor:findtext("\\w+", SCFIND_REGEXP, t.tag_start, t.tag_end)
	if begin_pos == nil or end_pos == nil then return end
	local tag = editor:textrange(begin_pos,end_pos)
	local count = 1
	local find_start = t.tag_start+dec
	
	repeat
		t.paired_start, t.paired_end = editor:findtext("[<{]/*"..tag.."[^}>]*", SCFIND_REGEXP, find_start, find_end)
		if t.paired_start == nil then break end
		if editor.CharAt[t.paired_start+1] == 47 then
			count = count - dec
		else
			count = count + dec
		end
		if count == 0 then break end
		find_start = t.paired_start + dec
	until false

	if t.paired_start ~= nil then
		-- paint in green
		EditorMarkText(t.tag_start, t.tag_end - t.tag_start + 1, 0)
		EditorMarkText(t.paired_start, t.paired_end - t.paired_start + 1, 0)
	else
		-- paint in Red
		EditorMarkText(t.tag_start, t.tag_end - t.tag_start + 1, 2)
	end
end

AddEventHandler("OnUpdateUI", function()
	if props['FileName'] ~= '' then
		if tonumber(props["hypertext.highlighting.paired.tags"]) == 1 then
			if editor:GetLexerLanguage() == "hypertext" or editor:GetLexerLanguage() == "xml" then
				PairedTagsFinder()
			end
		end
	end
end)