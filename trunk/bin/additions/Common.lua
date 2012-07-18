------------------------------------------
--     PACKAGE Path                     --
------------------------------------------
package.cpath = props["SciteDefaultHome"].."\\additions\\Lib\\?.dll;" .. package.cpath
require 'lpeg'
require 'gui'
require 'shell'
require 'iconv'

------------------------------------------
-- Handle Scite Events
------------------------------------------
function AddEventHandler(EventName, Handler)
	local prime_Event = _G[EventName]
	if prime_Event ~= nil then
		_G[EventName] = function (...)
			return prime_Event(...) or Handler(...)
		end
	else
		_G[EventName] = function (...)
			return Handler(...)
		end
	end
end

function OnUserListSelection(tp,str)
	print(tp,str)
end

function GetCurrentWord()
	local current_pos = editor.CurrentPos
	return editor:textrange(editor:WordStartPosition(current_pos, true),
							editor:WordEndPosition(current_pos, true))
end

------------------------------------------
--      SciTE Event Functions          --
------------------------------------------
function OnOpen(filename)
end

function OnClose(filename)
end

function  OnSwitchFile(filename)
end

function OnSave(filename)
end

function OnBeforeSave(filename)
end

function OnChar(c)
	AutoCompleteWord(1)
end

function OnKey(key, shift, ctrl, alt)
end

function OnSavePointReached()
end

function OnSavePointLeft()
end

function OnDwellStart()
end

function OnDoubleClick()
	--print(GetStartWord())
end

function OnMarginClick()
	--scite.MenuCommand(IDM_BOOKMARK_TOGGLE)
end

function OnUserListSelection(listType, text)
end

function OnMenuCommand(msg, source)
	--[[repeat
		--case:
		--default:
		break
	until true]]--
end

function OnStatusBarClick(part,t)
	if part==1 then
		local eol = editor.EOLMode + 1;
		if eol > SC_EOL_LF then
			eol = SC_EOL_CRLF
		end
		scite.SendEditor(SCI_SETEOLMODE,eol)
		scite.UpdateStatusBar(false)
	elseif part==2 then
		if editor.CodePage == SC_CP_UTF8 then
			scite.MenuCommand(IDM_ENCODING_DEFAULT)
		else
			scite.MenuCommand(IDM_ENCODING_UCOOKIE)
		end
	elseif part==3 then
		scite.MenuCommand(1125)
	elseif part==4 then
		--SideBar_ShowHide()
		scite.MenuCommand(1109)
	end
end

------------------------------------------
-- Custom Common Functions              --
------------------------------------------

--------------------------
-- 编码转换
--------------------------
function charset_iconv(in_charset, out_charset, text)
  local cd = iconv.new(out_charset .. "//TRANSLIT", in_charset)
  local cd = iconv.open(in_charset, out_charset)
  assert(cd, "Failed to create a converter object.")
  local text_out, err = cd:iconv(text)

  if err == iconv.ERROR_INCOMPLETE then
	print("ICONV ERROR: Incomplete input.")
  elseif err == iconv.ERROR_INVALID then
	print("ICONV ERROR: Invalid input.")
  elseif err == iconv.ERROR_NO_MEMORY then
	print("ICONV ERROR: Failed to allocate memory.")
  elseif err == iconv.ERROR_UNKNOWN then
	print("ICONV ERROR: There was an unknown error.")
  end
  return text_out
end

--------------------------
-- 编辑颜色
--------------------------
function edit_colour ()
	local function get_prevch (i)
		return editor:textrange(i-1,i)
	end
	local function get_nextch (i)
		return editor:textrange(i,i+1)
	end
	local function hexdigit(c)
		return c:find('[0-9a-fA-F]')==1
	end
	local i = editor.CurrentPos
	-- let's find the extents of this colour field...
	local ch = get_prevch(i)
	-- 先向前查找颜色编码
	while i > 0 and ch ~= '#' and hexdigit(get_prevch(i)) do
		i = i - 1
		--ch = get_prevch(i)
	end
	if i == 0 then return end
	local istart = i
	-- skip the '#'
	if ch == '#' then
		istart = istart - 1
	end
	if get_nextch(i) == '#' then
		i = i+1
	end
	--反向查找颜色编码
	while hexdigit(get_nextch(i)) do
		i = i + 1
	end
	-- extract the colour!
	local colour = editor:textrange(istart,i)
	colour = gui.colour_dlg(colour)
	if colour then -- replace the colour in the document
		editor:SetSel(istart,i)
		editor:ReplaceSel(colour)
	end
end

--------------------------
-- 检查UTF8编码,暂无应用
--------------------------
function DetectUTF8()
	local text = editor:GetText()
	local cont = lpeg.R("\128\191")   -- continuation byte
	local utf8 = lpeg.R("\0\127")^1
			+ (lpeg.R("\194\223") * cont)^1
			+ (lpeg.R("\224\239") * cont * cont)^1
			+ (lpeg.R("\240\244") * cont * cont * cont)^1
	local latin = lpeg.R("\0\127")^1
	local searchpatt = latin^0 * utf8 ^1 * -1
	if searchpatt:match(text) then
		scite.MenuCommand(IDM_ENCODING_UCOOKIE)
	end
end

----------------------------------
-- 全局调用检查编码函数，暂无应用
----------------------------------
function CheckUTF()
	if props["utf8.check"] == "1" then
		if editor.CodePage ~= SC_CP_UTF8 then
			DetectUTF8()
		end
	end
end
--------------------------
-- 转换输出区码
--------------------------
function switch_encoding()
	--editor:BeginUndoAction()
	editor:SelectAll()
	editor:Copy()
	if editor.CodePage == SC_CP_UTF8 then
		scite.MenuCommand(IDM_ENCODING_DEFAULT)
	else
		scite.MenuCommand(IDM_ENCODING_UCOOKIE)
	end
	editor:Paste()
	scite.SendOutput(SCI_SETCODEPAGE, editor.CodePage)
	--editor:EndUndoAction()
end
---------------------------
-- 滚屏并保持光标原屏幕位置
---------------------------
function screen_up()
	editor:LineScrollUp()
	editor:LineUp()
end

function screen_down()
	editor:LineScrollDown()
	editor:LineDown()
end
-------------------------
-- 使用Shell执行SVN命令
-------------------------
function svn_exec(cmd,path)
	local svnexec = props['ext.subversion.path']
	if cmd == nil then cmd = 'update' end
	if path == nil or path == '' then path = props['FileDir'] end
	local command = "\""..svnexec.."\" /command:"..cmd.." /path:\""..path.."\" /notempfile /closeonend:0"
	shell.exec(command)
end
------------------------------------------
-- 为hypertext lexer的文档添加html注释
------------------------------------------
function add_html_comment()
	local old_comment_start = props['comment.stream.start.hypertext']
	local old_comment_end   = props['comment.stream.end.hypertext']
	props['comment.stream.start.hypertext'] = '<!--'
	props['comment.stream.end.hypertext']   = '-->'
	scite.MenuCommand(IDM_STREAM_COMMENT)
	props['comment.stream.start.hypertext'] = old_comment_start
	props['comment.stream.end.hypertext']   = old_comment_end
end
------------------------------------------
-- EditorMarkText
------------------------------------------
function EditorMarkText(start, length, style_number)
	local current_mark_number = scite.SendEditor(SCI_GETINDICATORCURRENT)
	scite.SendEditor(SCI_SETINDICATORCURRENT, style_number)
	scite.SendEditor(SCI_INDICATORFILLRANGE, start, length)
	scite.SendEditor(SCI_SETINDICATORCURRENT, current_mark_number)
end
------------------------------------------
-- EditorClearMarks
------------------------------------------
function EditorClearMarks(style_number, start, length)
	local _first_style, _end_style, style
	local current_mark_number = scite.SendEditor(SCI_GETINDICATORCURRENT)
	if style_number == nil then
		_first_style, _end_style = 0, 31
	else
		_first_style, _end_style = style_number, style_number
	end
	if start == nil then
		start, length = 0, editor.Length
	end
	for style = _first_style, _end_style do
		scite.SendEditor(SCI_SETINDICATORCURRENT, style)
		scite.SendEditor(SCI_INDICATORCLEARRANGE, start, length)
	end
	scite.SendEditor(SCI_SETINDICATORCURRENT, current_mark_number)
end

------------------------------------------
-- Get Start Word
------------------------------------------
function GetStartWord()
	local current_pos = editor.CurrentPos
	return editor:textrange(editor:WordStartPosition(current_pos, true),current_pos)
end
------------------------------------------
-- AutoCompleteWord
------------------------------------------
function AutoCompleteWord(len)
	if len == nil then len = 3 end
	if props['autocompleteword.automatic'] == '1' then return end
	local currentPos = editor.CurrentPos
	local startPos = editor:WordStartPosition(currentPos, true)
	local wordLen = currentPos - startPos
	if wordLen >= len then
		scite.MenuCommand(IDM_COMPLETE)
		if not editor:AutoCActive() then
			scite.MenuCommand(IDM_COMPLETEWORD)
		end
	end
end

------------------------------------------
-- unwind_protect
------------------------------------------
local function unwind_protect(thunk,cleanup)
	local ok,res = pcall(thunk)
	if cleanup then cleanup() end
	if not ok then error(res,0) else return res end
end
------------------------------------------
-- with_open_file
------------------------------------------
local function with_open_file(name,mode)
	return function(body)
	local f = assert(io.open(name,mode))
	return unwind_protect(function()return body(f) end,
		function()return f and f:close() end)
	end
end
------------------------------------------
-- os_copy
------------------------------------------
function os_copy(source_path,dest_path)
	return with_open_file(source_path,"rb") (function(source)
		return with_open_file(dest_path,"wb") (function(dest)
			assert(dest:write(assert(source:read("*a"))))
			return true
		end)
	end)
end
------------------------------------------
-- ifnil
------------------------------------------
function ifnil(prop, def)
	local val = props[prop]
	if val == nil or val == '' then
		return def
	else
		return val
	end
end
------------------------------------------
-- Translate color from RGB to win
------------------------------------------
local function encodeRGB2WIN(color)
	if color == nil then return nil end
	if string.sub(color,1,1)=="#" and string.len(color)>6 then
		return tonumber(string.sub(color,6,7)..string.sub(color,4,5)..string.sub(color,2,3), 16)
	else
		return color
	end
end
------------------------------------------
-- Translate INDIC_*
------------------------------------------
local function GetStyle(mark_string)
	local mark_style_table = {
		plain    = INDIC_PLAIN,    squiggle = INDIC_SQUIGGLE,
		tt       = INDIC_TT,       diagonal = INDIC_DIAGONAL,
		strike   = INDIC_STRIKE,   hidden   = INDIC_HIDDEN,
		roundbox = INDIC_ROUNDBOX, box      = INDIC_BOX
	}
	return mark_style_table[mark_string]
end
------------------------------------------
-- Init Mark Style
------------------------------------------
local function InitMarkStyle(mark_number, mark_style, color, alpha)
	editor.IndicStyle[mark_number] = mark_style
	editor.IndicFore[mark_number]  = encodeRGB2WIN(color)
	editor.IndicAlpha[mark_number] = alpha
	editor.IndicUnder[mark_number] = true
end
------------------------------------------
-- Parse Mark Style From Prop String
------------------------------------------
function ParseMarkStyle(prop_string)
	local mark = props[prop_string]
	local ret_number = 8
	if mark ~= "" then
		local mark_number= tonumber(mark:match("%d+")) or 30
		local mark_color = mark:match("#%x%x%x%x%x%x") or (props["find.mark"]):match("#%x%x%x%x%x%x") or "#0F0F0F"
		local mark_style = GetStyle(mark:match("%l+")) or INDIC_ROUNDBOX
		local alpha_fill = tonumber((mark:match("%@%d+") or ""):sub(2)) or 30
		InitMarkStyle(mark_number, mark_style, mark_color, alpha_fill)
		ret_number = mark_number
	end
	return ret_number
end

function ReadAbbrevFile(file, abbr_table)
	--[[------------------------------------------
	@usage: for l in scite_io_lines('c:\\some.file') do print(l) end
	  alternative:
	f = io.open('s:\\some.file')
	for l in scite_io_lines(f) do print(l) end
	--]]------------------------------------------
	local function scite_io_lines(file)
		local line_iter = type(file)=='string' and io.lines(file) or file:lines()
		local scite_iter = function()
			local line = line_iter()
			if not line then return end
			-- start [SciTE]
			while string.sub(line,-1)=='\\' do
				line = string.sub(line,1,-2)..line_iter()
			end
			-- end [SciTE]
			return line
		end
		return scite_iter
	end
	--------------------------------------------
	local abbrev_file, err, errcode = io.open(file)
	if not abbrev_file then return abbrev_file, err, errcode end

	local abbr_table = abbr_table or {}
	local ignorecomment = tonumber(props['abbrev.'..props['Language']..'.ignore.comment'])==1
	for line in scite_io_lines(abbrev_file) do
		if line ~= '' and (ignorecomment or line:sub(1,1) ~= '#' ) then
			local _abr, _exp = line:match('^(.-)=(.+)')
			if _abr then
				abbr_table[#abbr_table+1] = {abbr=_abr, exp=_exp}
			else
				local import_file = line:match('^import%s+(.+)')
				-- если обнаружена запись import, то рекурсивно вызываем эту же функцию
				if import_file then
					ReadAbbrevFile(file:match('.+\\')..import_file, abbr_table)
				end
			end
		end
	end
	abbrev_file:close()
	return abbr_table
end