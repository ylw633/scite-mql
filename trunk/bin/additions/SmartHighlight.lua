------------------------------
-- Smart Highlights Words
------------------------------

local function isWord(all_text, cur_text, text_start, text_end)
	local word_pattern = '[%w_]+'
	if not cur_text:match(word_pattern) then return false end
	if  ( (text_start==1)       or (not all_text:sub(text_start-1, text_start-1):match(word_pattern)) ) and
		( (text_end==#all_text) or (not all_text:sub(text_end+1, text_end+1):match(word_pattern)) )    then
			return true
	end
	return false
end

function SmartHighlight()
	local style_number = ParseMarkStyle('smart.highlight.style')
	local currentPos = editor.CurrentPos
	local all_text   = editor:GetText()
	local matchcase  = tonumber(ifnil('smart.highlight.matchcase',1)) > 0
	local min_len = tonumber(ifnil('smart.highlight.minlen',1))
	local max_num = tonumber(ifnil('smart.highlight.maxnum',100))
	local text = props['CurrentSelection']
	if text == '' then text= editor:GetSelText() end
	text = text:gsub('[\t\r\n ]+','')
	local sel_start = editor.SelectionStart
	local sel_end = editor.SelectionEnd

	--clear marks first
	EditorClearMarks(style_number)
	if isWord(all_text, text, sel_start + 1,sel_end) then
		--local s,e = editor:findtext(text, flag, 0) -- very slowly
		local s,e = all_text:find(text, 1, true)
		local count = 1
		if s ~= nil then
			while s do
				if count > max_num then break end
				--s,e = editor:findtext(text, flag, e + 1) -- very slowly
				if isWord(all_text,text,s,e) then
					EditorMarkText(s - 1, e - s + 1, style_number)
				end
				s,e = all_text:find(text, e + 1, true)
				count = count + 1
			end
		end
		--scite.SendEditor(SCI_SETCURRENTPOS, currentPos)
	end
end
------------------------------
-- Add event
------------------------------
AddEventHandler("OnUpdateUI", function()
	if tonumber(ifnil('smart.highlight',0)) < 1 --[[or editor.Length > 500000]] then
		return
	end
	SmartHighlight()
end)