------------------------------------------------
--  自动注释
------------------------------------------------
local definition = {['function'] = {};['variable'] = {};
['doc'] = {'access', 'author', 'copyright', 'license', 'package', 'param', 'return', 'throws', 'var', 'version'}}
focus = {['start'] = 0;['end'] = 0}
access = {['public'] = true;['private'] = true;['protected'] = true;['var'] = true}

function findDefinedFunctionParams(params)
	local struct = {}
	--在编辑器中寻找
	for class, var, val in string.gmatch(params, "(%w*)%s*\$([_a-zA-Z0-9]+)%s*=*%s*([%w'\"]*)") do
		if string.len(class) > 0 then
			table.insert(struct, class .. " $" .. var)
		else
			local varType = 'mixed'
			if val then varType = parseVarType(val) end
			table.insert(struct, varType .. " $" .. var)
		end
	end
	return struct;
end

function parseVarType(val)
	local varType = 'mixed'
	if val then
		if tonumber(val) ~= nil then
			varType = 'int'
		elseif val:lower() == 'true' or val:lower() == 'false' then
			varType = 'bool'
		elseif val:match('^array') then
			varType = 'array'
		elseif val:match('^[\'"]') then
			varType = 'string'
		end
	end
	return varType
end

local function formatEditorDoc(str, tab, line)
	scite.SendEditor(SCI_INSERTTEXT, editor.CurrentPos, str)

	local selStart = editor:PositionFromLine(line) + string.len(tab) + 3
	local selEnd = selStart + string.len("description...")

	scite.SendEditor(SCI_SETSELECTIONSTART, selStart)
	scite.SendEditor(SCI_SETSELECTIONEND, selEnd)
end

local function checkDoc(char)
	if "\n" ~= char and "\r" ~= char and " " ~= char then
		return
	--解决\r\n时2次注释的问题
	elseif 0 == editor.EOLMode and "\r" == char then
		return
	end

	if " " == char and editor.CurrentPos >= 3 and '/**' == editor:textrange(editor.CurrentPos - 4, editor.CurrentPos - 1) then
		local str = ''
		if editor:LineFromPosition(editor.CurrentPos)+1 == editor.LineCount then
			str = "End of file ".. props['FileNameExt']
		else
			str = "description..."
		end
		scite.SendEditor(SCI_INSERTTEXT, editor.CurrentPos, str.." */")
		scite.SendEditor(SCI_SETSELECTIONSTART, editor.CurrentPos)
		scite.SendEditor(SCI_SETSELECTIONEND, editor.CurrentPos + string.len(str))
		return
	end

	local line = editor:LineFromPosition(editor.CurrentPos)
	local lineStart = editor:PositionFromLine(line - 1)
	local prevLine = editor:GetLine(line - 1)
	local lineEnd = lineStart

	if nil ~= prevLine then
		lineEnd = lineEnd + string.len(prevLine)
	end

	local nextLine = nil
	local eol = "\n"
	local eolLength = 1

	if 0 == editor.EOLMode then
		eol = "\r\n"
		eolLength = 2
	end

	if 1 == editor.EOLMode then
		eol = "\r"
	end

	if(lineEnd < lineStart + eolLength + 3) then
		return
	end

	local tab = ''
	local tabLen = 0
	for findTab in string.gmatch(prevLine, '(%s*)[^%s]*') do
		tab = findTab
		tabLen = string.len(findTab)
		break
	end

	local author = ''
	local siteurl= ''
	if props['document.author'] ~= nil and props['document.author'] ~= '' then
		author = props['document.author']
	else
		author = os.getenv('USERNAME')
	end

	if props['document.siteurl'] ~= nil then
		siteurl = props['document.siteurl']
	end

	if ("\n" == char or "\r" == char) and lineEnd >= 3 and '/**' == editor:textrange(lineEnd - 3 - eolLength, lineEnd - eolLength) then
		preChar = char
		if editor.LineCount <= line then
			nextLine = nil
		else
			nextLine = editor:GetLine(line + 1)
		end

		local str = ''
		local trim = ''

		if editor.CurrentPos <= editor:PositionFromLine(line) then
			trim = tab
		end

		if nextLine ~= nil then
			for access1, access2, functionName in string.gmatch(nextLine, "(%w*)%s*(%w*)%s*function%s+[&]*([_a-zA-Z0-9]+)%s*\(.*\)") do
				if functionName ~= nil then
					local iaccess = ''
					if true == rawget(access, access2) then
						iaccess = access2
					else
						iaccess = access1
					end

					if iaccess == '' then
						iaccess = 'global'
					end

					local struct = findDefinedFunctionParams(nextLine)

					str = trim .. " * description..." .. eol
					str = str .. tab .. " * " .. eol

					if string.len(iaccess) > 0 then
						str = str .. tab .. " * @access " .. iaccess .. eol
					end

					for key, class in pairs(struct) do
						str = str .. tab .. " * @param " .. class .. eol
					end

					str = str .. tab .. " * @return void" .. eol
					str = str .. tab .. " */"

					formatEditorDoc(str, tab, line)
					return
				end
			end
		end

		if nextLine ~= nil then
			for className in string.gmatch(nextLine, "class%s+([_a-zA-Z0-9]+)") do
				if className ~= nil then
					str = trim .. " * description..." .. eol
					str = str .. tab .. " * " .. eol
					str = str .. tab .. " * @author ".. author .. eol
					--str = str .. tab .. " * @category  " .. eol
					str = str .. tab .. " * @package " .. className .. eol
					str = str .. tab .. " */"

					formatEditorDoc(str, tab, line)
					return
				end
			end
		end

		if nextLine ~= nil then
			for access1, access2, variableName in string.gmatch(nextLine, "(%w*)%s*(%w*)%s+\$([_a-zA-Z0-9]+)") do
				if variableName ~= nil then
					local iaccess = ''
					if true == rawget(access, access2) then
						iaccess = access2
					else
						iaccess = access1
					end
					--var to be public access
					if iaccess == string.lower('var') then
						iaccess = 'public'
					end
					str = trim .. " * description..." .. eol
					str = str .. tab .. " * " .. eol
					str = str .. tab .. " * @access " .. iaccess .. eol
					str = str .. tab .. " * @var unknown" .. eol
					str = str .. tab .. " */"

					formatEditorDoc(str, tab, line)
					return
				end
			end
		end


		if nextLine ~= nil then
			local realLine = string.gsub(nextLine, "%s*", "");
		else
			local realLine = '';
		end

		if realLine ~= nil then
			str = trim .. " * description..." .. eol
			str = str .. tab .. " * " .. eol
			str = str .. tab .. " */"
			formatEditorDoc(str, tab, line)
			return
		else
			str = trim .. " * description..." .. eol
			str = str .. tab .. " * " .. eol
			str = str .. tab .. " * @author ".. author .. eol
			--str = str .. tab .. " * @category Project" .. eol
			--str = str .. tab .. " * @package None" .. eol
			--str = str .. tab .. " * @copyright Copyright(c) ".. os.date('%Y') ..' '.. siteurl .. eol
			str = str .. tab .. " * @version $Id$" .. eol --配合 svn:keywords 产生版本修改信息
			str = str .. tab .. " */"

			formatEditorDoc(str, tab, line)
			return
		end
	end

	if ("\n" == char or "\r" == char) and lineEnd >= 2 and '*' == editor:textrange(lineStart + tabLen, lineStart + tabLen + 1) and '/' ~= editor:textrange(lineStart + tabLen + 1, lineStart + tabLen + 2) then
		if 0 == editor.EOLMode then
			editor:InsertText(editor.CurrentPos, '* ')
			editor:GotoPos(editor.CurrentPos + 2)
		else
			editor:InsertText(editor.CurrentPos, tab .. '* ')
			editor:GotoPos(editor.CurrentPos + 2 + tabLen)
		end
	end

end

--------------------------------------------------------
--  转到定义
--  先在本页内查找变量,然后利用find_ctag索引查找所有定义
--------------------------------------------------------
function GotoDefinition()
	local word = props['CurrentWord']
	if word == nil or word=='' then
		--print("No word selected.")
		return
	end
	local text = editor:GetText()
	--[[func = string.find(text, "[Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn][ ]+[&]*" .. word) --defined function
	cls  = string.find(text, "[Cc][Ll][Aa][Ss][Ss][ ]+" .. word) --defined class
	const= string.find(text, "[Dd][Ee][Ff][Ii][Nn][Ee][(]['\"]"..word.."['\"]") --defined const]]--
	if string.find(word,"^[$]") then place = string.find(text, word) else place = nil end
	--[[if func then
		place = func
	elseif cls then
		place = cls
	elseif const then
		place = const
	elseif var then
		place = var
	else
		place = nil
	end]]--
	if place then
		-- mark current line to be able to jump back
		-- editor:MarkerAdd(editor:LineFromPosition(editor.CurrentPos),1)
		set_mark() --标记当前位置
		editor:GotoLine(editor:LineFromPosition(place))
	else
		--print("Unable to find definition: ".. word)
		find_ctag(word)
	end
end

function php_select_to_quote()
	-- ''  ""
	-- 'abc'  "abc"
	local right_pos = editor.CurrentPos
	local right_chr = string.char(editor.CharAt[right_pos])
	local left_pos  = right_pos - 1
	local left_chr  = string.char(editor.CharAt[left_pos])
	if left_chr == right_chr then
		return false
	end
	if right_chr == '"' or right_chr == "'" then
		editor:SearchAnchor()
		local pos = editor:SearchPrev(0, right_chr) + 1
		editor:SetSel(pos, right_pos)
	end
	if left_chr == '"' or left_chr == "'" then
		editor:SearchAnchor()
		local pos = editor:SearchNext(0, left_chr)
		editor:SetSel(left_pos + 1, pos)
	end
end

------------------------------------------------
--  Localhost浏览
------------------------------------------------
function View_In_Localhost()
	local file_path = props['FilePath']
	local localhost = props['localhost.path']
	local host_path = props['localhost.root']
	local this_path = ''
	if localhost == '' or host_path == '' then
		print("Please set web path in [hypertext.properties]")
		return
	elseif string.find(localhost,'http://') == nil then
		localhost = 'http://'..localhost
	end

	if string.find(string.lower(file_path),string.lower(host_path)) ~= nil then
		this_path = string.gsub(string.lower(file_path),string.lower(host_path),localhost)
	else
		this_path = file_path
		local copy_name = '\\'..props['FileName']..'.temp.'..props['FileExt']
		if shell.msgbox("Copy file '"..file_path .."' to '"..host_path..copy_name.."'?", "Copy file to web root", 4) == 6 then
			if os_copy(file_path,host_path..copy_name) then
				this_path = localhost..copy_name
			end
		end
	end
	this_path = string.gsub(this_path,"\\","/")
	if this_path ~= nil then
		local rs,info = shell.exec(this_path)
	    if(rs ~= true) then
			print(this_path,info)
		end
	end
end

------------------------------------------------
-- 事件处理
------------------------------------------------
local old_OnChar = OnChar
function OnChar(c)
	local result
	if old_OnChar then result = old_OnChar(c) end
	local toClose = {
		--['('] = ')',
		['['] = ']',
		['{'] = '}',
		['"'] = '"',
		["'"] = "'"
	}
	local toPass = {
		--[')'] = '(',
		[']'] = '[',
		['}'] = '{'
	}

	--当前行信息
	local curline = editor:LineFromPosition(editor.CurrentPos)
	local linendpos = editor.LineEndPosition[curline]

	-- 自动闭合
	if toClose[c] then
		local chr = editor.CharAt[editor.CurrentPos + 1]
		if chr > 0 and toClose[c] ~= string.char(chr) then
			editor:InsertText(editor.CurrentPos, toClose[c])
			--if linendpos < editor.CurrentPos+1 and c ~= '{' and c ~= '(' then
			--	editor:InsertText(editor.CurrentPos+1, ';')
			--end
		end
	end
	-- 智能越过
	if toPass[c] then
		local chr = editor.CharAt[editor.CurrentPos - 2]
		if (editor.CharAt[editor.CurrentPos - 2] < 0) then return end
		if chr > 0 and toPass[c] == string.char(chr) then
			if linendpos >= editor.CurrentPos + 1 then
				editor:GotoPos(editor.CurrentPos + 1)
				scite.SendEditor(SCI_DELETEBACK);
			end
		end
	end
	-- 增加注释
	checkDoc(c)
	return result
end

local old_OnKey = OnKey
function OnKey(k, shift, ctrl, alt, char)
	local result
	if old_OnKey then result = old_OnKey(k, shift, ctrl, alt, char) end
	local toDelete = {
		['('] = ')',
		['['] = ']',
		['{'] = '}',
		['"'] = '"',
		["'"] = "'"
	}
	-- 自动删除。8 是回退键
	if k == 8 then
		local pos = editor.CurrentPos - 1
		local chr = editor.CharAt[pos]
		if chr < 0 then
			return
		end
		chr = string.char(chr)
		thr = editor.CharAt[pos + 1]
		if thr < 0 then
			return
		end
		local mth = string.char(thr)
		if toDelete[chr] and toDelete[chr] == mth then
			editor:SetSel(pos, pos + 2)
			editor:ReplaceSel('')
			return true
		end
	-- 回车键
	elseif k == 13 then
		local left_char = editor.CharAt[editor.CurrentPos - 1]
		local right_char = editor.CharAt[editor.CurrentPos]
		if left_char < 0 or right_char < 0 then
			return false
		end
		local left  = string.char(left_char)
		local right = string.char(right_char)
		if (left == '{' and right == '}') or
			(left == '(' and right == ')' or
			left == '[' and right == ']') then
			local line = editor:LineFromPosition(editor.CurrentPos)
			scite.SendEditor(SCI_NEWLINE)
			scite.SendEditor(SCI_NEWLINE)
			editor.LineIndentation[line + 2] = editor.LineIndentation[line]
			if left ~= '{' then
				editor.LineIndentation[line + 1] = editor.LineIndentation[line] + 4
			end
			editor:GotoPos(editor.LineEndPosition[line + 1])
			return true
		end
		return false
	end
	return result
end
