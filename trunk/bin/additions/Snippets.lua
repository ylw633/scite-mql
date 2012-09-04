------------------------------------------------
-- snippets,与abbrev不同的是，支持tab键定位
-- 适合循环语句和比较长且会有重复的参数，如果有
-- 重复的参数，将数字设定成一样即可
------------------------------------------------
local in_snippet
local total_num
local current_num
local begin_pos
local end_pos
local snippet_pairs={}

local function init_snippet()
	in_snippet  = false
	total_num   = 0
	current_num = 0
	begin_pos   = -1
	end_pos     = -1
	snippet_pairs = {}
end

local function get_snippets(abbrev)
	--init snippets for php example:
	local snippets = {
		--['for'] = 'for (%{1} = 0; %{1} < %{2}; %{1}++) {\n    %{3}\n}%{end}',
		--['class'] = 'class %{1} {\n    function __construct() {\n         %{2}\n    }\n}%{end}',
		--['func'] = 'function %{1}(%{2}) \n{\n    %{3}\n}%{end}',
		--['foreach'] ='foreach (%{1} as %{2}) \n{\n    %{3}\n}%{end}';
		--['if'] ='if (%{1}) \n{\n    %{2}\n}%{end}';
	}
	if abbrev == '' then return nil end
	--read from php snippets file
	--it's depend on props['FileExt']
	--local snip_filename = props['SciteDefaultHome'] .. '/snippets/' .. props['FileExt'] .. '.snip'
	local snip_filename = props['SciteDefaultHome'] .. '/snippets/' .. props['Language'] .. '.snip'
	local function get_snippets_from_file(file)
		local snip_file = io.open(file)
		if snip_file then
			for line in snip_file:lines() do
				if line ~= '' then
					local _abr, _exp = line:match('^([^#].-)=(.+)')
					if _abr ~= nil then
						if _abr == abbrev then
						   return _exp ;
					   end
					else
						local import_file = line:match('^import%s+(.+)')
						if import_file ~= nil then
							return get_snippets_from_file(file:match('.+[\\/]')..import_file)
						end
					end
				end
			end
			snip_file:close()
		end
	end
	local snip = get_snippets_from_file(snip_filename)
	if snip ~= nil then
		return snip
	end
	return snippets[abbrev]
end

------------------------------------------------
-- snippet tab函数
------------------------------------------------
function do_snippet()
	if in_snippet then
		--print(editor.CurrentPos,begin_pos,end_pos)
		--如果当前位置不在代码范围内则重置
		if editor.CurrentPos > end_pos or editor.CurrentPos < begin_pos then
			--init_snippet()
			--return false
			return expand_snippet()
		end
		current_num = current_num + 1
		if total_num == nil then total_num = 0 end
		local end_str = snippet_pairs['0']
		if current_num > total_num then
			if end_str ~= nil then
				local b, e = editor:findtext(end_str, 0, begin_pos)
				if b ~= nil and e ~= nil then
					editor:SetSel(b, e)
					editor:ReplaceSel()
				end
			end
			expand_snippet()
			return true
		end
		--editor:ClearSelections()
		local str = snippet_pairs[''..current_num..'']
		local begin = begin_pos
		local has_main = false
		-- 寻找所有 str 并选中它
		while true do
			local b, e = editor:findtext(str, 0, begin, end_pos)
			if b ~= nil then
				--if b > end_pos then break end
				if not has_main then
					editor:SetSelection(b, e)
					has_main = true
				else
					editor:AddSelection(b, e)
				end
				begin = e
			else
				break
			end
		end
		return has_main
	end
	return false
end

------------------------------------------------
-- snippet command函数
------------------------------------------------
function expand_snippet()
	local word = GetStartWord()--props['CurrentWord']
	local snippet = get_snippets(word)
	local curline = editor:LineFromPosition(editor.CurrentPos)
	local linendpos = editor.LineEndPosition[curline]
	local enabled  = linendpos == editor.CurrentPos
	init_snippet() --先进行初始化
	if snippet ~= nil and enabled then
		--撤消队列
		editor:BeginUndoAction()
		editor:DelWordLeft()
		--editor:ReplaceSel('')
		total_num = 0
		local pattern = '%%{([%d]+):([^}]+)}'
		for n,f in string.gmatch(snippet,pattern) do
			if snippet_pairs[n] == nil then
				if n ~= '0' then
					total_num = total_num + 1
				end
				snippet_pairs[n] = f
			end
		end
		snippet = string.gsub(snippet,pattern,'%2')
		current_num   = 0
		--记录开始位置
		begin_pos     = editor.CurrentPos
		local curline = scite.SendEditor(SCI_LINEFROMPOSITION, begin_pos)
		local indent  = scite.SendEditor(SCI_GETLINEINDENTATION, curline)
		local lines   = 0
		snippet,lines = string.gsub(snippet, '\\n', '\n')
		snippet = snippet:gsub('\\r','\r'):gsub('\\t','    ')
		editor:AddText(snippet)
		editor:EndUndoAction()
		for i = 1, lines do
			local line_indent = scite.SendEditor(SCI_GETLINEINDENTATION, curline + i)
			scite.SendEditor(SCI_SETLINEINDENTATION, curline + i, indent + line_indent)
		end
		--记录结束位置
		end_pos = editor.CurrentPos
		in_snippet = true
		do_snippet()
		return true
	end
	return false
end

local old_OnKey = OnKey
function OnKey(k, shift, ctrl, alt, char)
	local result
	if old_OnKey then result = old_OnKey(k, shift, ctrl, alt, char) end
	-- Tab 键
	local keycode = tonumber(props['command.snippet.shortcut'])
	if k == keycode then
		if in_snippet then
			return do_snippet()
		else
			return expand_snippet()
		end
	end
	return result
end

local old_OnSwitchFile = OnSwitchFile
function OnSwitchFile(file)
	local result
	if old_OnSwitchFile then result = old_OnSwitchFile(file) end
	init_snippet()
	return result
end

local old_OnMenuCommand = OnMenuCommand
function OnMenuCommand (msg, source)
	local result
	if old_OnMenuCommand then result = old_OnMenuCommand(msg, source) end
	if msg == IDM_UNDO and in_snippet then
		init_snippet()
	end
	return result
end