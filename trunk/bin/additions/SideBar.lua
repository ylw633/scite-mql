--[[--------------------------------------------------
SideBar.lua
Authors: Frank Wunderlich, mozers? VladVRO, frs, BioInfo, Tymur Gubayev KimHa
version 1.10.3
------------------------------------------------------
  Note: Needed gui.dll <http://scite-ru.googlecode.com/svn/trunk/lualib/gui/>
  Connection:
   In file SciTEStartup.lua add a line:
	  dofile (props["SciteDefaultHome"].."\\tools\\SideBar.lua")
   Set in a file .properties:
	  command.checked.17.*=$(sidebar.show)
	  command.name.17.*=SideBar
	  command.17.*=SideBar_ShowHide
	  command.mode.17.*=subsystem:lua,savebefore:non

	# Set show(1) or hide(0) to SciTE start
	sidebar.show=1
	# Set default settings for Functions/Procedures List
	sidebar.functions.flags=1
	sidebar.functions.params=1
--]]--------------------------------------------------n
-- you can choose to make it a stand-alone window; just uncomment this line:
local win = false
-- local _DEBUG = true
local _show_flags = tonumber(props['sidebar.functions.flags']) == 1
local _show_params = tonumber(props['sidebar.functions.params']) == 1

--И«ѕЦ abbrev list
local abbrev_list = {}
--И«ѕЦtree nodes
local tree_nodes  = {}
local file_mask = '*.*'

local tab_index = 0
local panel_width = 200
local win_height = props['position.height']
if win_height == '' then win_height = 600 end

local style   = props['style.*.32']
local foreground = props['sidebar.foreground'];
local background = props['sidebar.background'];
if background == '' then
	background = style:match('back:(#%x%x%x%x%x%x)')
end
if background ~= '' then
	if foreground == '' then
		foreground = style:match('fore:(#%x%x%x%x%x%x)')
	end
	if foreground == nil then foreground = '' end
end
----------------------------------------------------------
-- Common functions
----------------------------------------------------------
local function ReplaceWithoutCase(text, s_find, s_rep)
	local i, j = 1
	local replaced = nil
	repeat
		i, j = text:lower():find(s_find:lower(), j, true)
		if j == nil then return text, replaced end
		text = text:sub(1, i-1)..s_rep..text:sub(j+1)
		replaced = true
	until false
end

local function ShowCompactedLine(line_num)
	local function GetFoldLine(ln)
		while editor.FoldExpanded[ln] do ln = ln-1 end
		return ln
	end
	while not editor.LineVisible[line_num] do
		local x = GetFoldLine(line_num)
		editor:ToggleFold(x)
		line_num = x - 1
	end
end

local function OpenFile(filename)
	if filename:match(".session$") ~= nil then
		filename = filename:gsub('\\','\\\\')
		--scite.Perform ("loadsession:"..filename)
	else
		--±аВлОКМвЈ¬РиТЄЧЄ»»ІЕДЬґтїЄЦРОДОДјюјР/ОДјюГы
		filename = charset_iconv('','utf-8',filename)
		scite.Open(filename)
	end
	gui.pass_focus()
end

if _DEBUG then
local nametotime = {} -- maps names to starttimes
	_DEBUG = {}

	_DEBUG.timerstart = function (name)
		nametotime[name] = os.clock()
	end -- _DEBUG.timerstart

	_DEBUG.timer = function (name,...)
		if nametotime[name] then
			local d = os.clock() - nametotime[name]
			print(name,('%.5fs'):format(d),...)
		end
		return d
	end -- _DEBUG.timer

	_DEBUG.timerstop = function (name,...)
		local d = _DEBUG.timer(name,...)
		nametotime[name] = nil
		return d
	end --_DEBUG.timerstop

else
	_DEBUG = {}
	local empty = function (...) end
	_DEBUG.timer, _DEBUG.timerstart, _DEBUG.timerstop = empty, empty, empty
end

----------------------------------------------------------
-- Create panels
----------------------------------------------------------
tab0 = gui.panel(panel_width + 18)

local memo_path = gui.memo()
tab0:add(memo_path, "top", 22)

local list_dir_height = win_height/3
if list_dir_height <= 0 then list_dir_height = 320 end

local list_functions = gui.list(true)
list_functions:add_column("Functions/Procedures", 400)
tab0:add(list_functions, "bottom", list_dir_height)
if background then list_functions:set_list_colour(foreground,background) end

local list_dir = gui.list()
tab0:client(list_dir)
if background then list_dir:set_list_colour(foreground,background) end

tab0:context_menu {
	'New File\tCtrl+Alt+N|FileMan_NewFile',
	'Change Dir|FileMan_ChangeDir',
	'', -- separator
	'Refresh|FileMan_Refresh',
	'Show All|FileMan_MaskAllFiles',
	'Only current ext|FileMan_MaskOnlyCurrentExt',
	'', -- separator
	'Copy to...|FileMan_FileCopy',
	'Move to...|FileMan_FileMove',
	'Rename|FileMan_FileRename',
	'Delete\tDelete|FileMan_FileDelete',
	'', -- separator
	--'Execute|FileMan_FileExec',
	--'Exec with Params|FileMan_FileExecWithParams',
	'Sort by Order|Functions_SortByOrder',
	'Sort by Name|Functions_SortByName',
	--'', -- separator
	'Show/Hide Flags|Functions_ToggleFlags',
	'Show/Hide Parameters|Functions_ToggleParams',
	'', -- separator
	'Hide SideBar\tCtrl+0|SideBar_ShowHide',
}
-------------------------
tab1 = gui.panel(panel_width + 18)
local list_project = gui.tree(true)
tab1:client(list_project)
if background then list_project:set_tree_colour(foreground,background) end

tab1:context_menu {
	'Open Dir As Project|Open_Project_Dir',
	'Refresh...|Project_Refresh_Tree',
	'New File|Project_NewFile',
	'Rebuild Tags|Project_Build_Tags',
	'',
	'SVN Update Ўэ|SVN_Update',
	'SVN Commit Ўь|SVN_Commit',
	'SVN Add Ј«Ј«|SVN_Add',
	'',
	'Hide SideBar|SideBar_ShowHide',
}

------------------------

tab2 = gui.panel(panel_width + 18)
local list_abbrev = gui.list(true)
list_abbrev:add_column("Abbrev", 60)
list_abbrev:add_column("Expansion", 600)
tab2:client(list_abbrev)
if background then list_abbrev:set_list_colour(foreground,background) end

tab2:context_menu {
	'Edit Abbrev|Edit_Abbrev_File',
	'',
	'Hide SideBar|SideBar_ShowHide',
}

-------------------------
local win_parent
if win then
	win_parent = gui.window "Side Bar"
else
	win_parent = gui.panel(panel_width)
end

local tabs = gui.tabbar(win_parent)
tabs:add_tab("Files", tab0)
tabs:add_tab("Project", tab1)
tabs:add_tab("Abbrev", tab2)
win_parent:client(tab2)
win_parent:client(tab1)
win_parent:client(tab0)

if tonumber(props['sidebar.show'])==1 then
	local position = 'right'
	if props['sidebar.position'] ~= nil then
		position = props['sidebar.position'];
	end
	if win then
		win_parent:size(panel_width + 24, 600)
		win_parent:show()
	else
		gui.set_panel(win_parent,position)
	end
end
----------------------------------------------------------
-- tab0:memo_path   Path and Mask
----------------------------------------------------------
local current_path = ''

local function FileMan_ShowPath()
	local rtf = [[{\rtf\ansi\ansicpg1251{\fonttbl{\f0\fcharset65001 Arial;}}{\colortbl;\red0\green0\blue255;\red255\green0\blue0;}\f0\fs16]]
	local path = '\\cf1'..current_path:gsub('\\', '\\\\')
	local mask = '\\cf2'..file_mask..'}'
	memo_path:set_text(rtf..path..mask)
end

----------------------------------------------------------
-- tab0:list_dir   File Manager
----------------------------------------------------------
local function FileMan_ListFILL()
	if current_path == '' then return end
	local folders = gui.files(current_path..'*', true)
	if not folders then return end
	list_dir:clear()
	list_dir:add_item ('[..]', {'..','d'})
	table.sort(folders)
	for i, d in ipairs(folders) do
		if d ~='.svn' then -- escape SVN folder
			list_dir:add_item(d .. '', {d,'d'})
		end
	end
	local files = gui.files(current_path..file_mask)
	table.sort(files)
	if files then
		for i, filename in ipairs(files) do
			list_dir:add_item(filename .. '', {filename})
		end
	end
	list_dir:set_selected_item(0)
	FileMan_ShowPath()
end

local function FileMan_GetSelectedItem()
	local idx = list_dir:get_selected_item()
	if idx == -1 then return '' end
	local data = list_dir:get_item_data(idx)
	local dir_or_file = data[1]
	local attr = data[2]
	return dir_or_file, attr
end

function FileMan_ChangeDir()
	local newPath = gui.select_dir_dlg('Please change current directory', current_path)
	if newPath == nil then return end
	if newPath:match('[\\/]$') then
		current_path = newPath
	else
		current_path = newPath..'\\'
	end
	FileMan_ListFILL()
end

function FileMan_MaskAllFiles()
	file_mask = '*.*'
	FileMan_ListFILL()
end

function FileMan_Refresh()
	FileMan_ListFILL()
end

function FileMan_MaskOnlyCurrentExt()
	local filename, attr = FileMan_GetSelectedItem()
	if filename == '' then return end
	if attr == 'd' then return end
	file_mask = '*.'..filename:gsub('.+%.','')
	FileMan_ListFILL()
end

function FileMan_FileCopy()
	local filename = FileMan_GetSelectedItem()
	if filename == '' or filename == '..' then return end
	local path_destination = gui.select_dir_dlg("Copy to...")
	if path_destination == nil then return end
	os_copy(current_path..filename, path_destination..'\\'..filename)
	FileMan_ListFILL()
end

function FileMan_FileMove()
	local filename = FileMan_GetSelectedItem()
	if filename == '' or filename == '..' then return end
	local path_destination = gui.select_dir_dlg("Move to...")
	if path_destination == nil then return end
	os.rename(current_path..filename, path_destination..'\\'..filename)
	FileMan_ListFILL()
end

function FileMan_FileRename()
	local filename = FileMan_GetSelectedItem()
	if filename == '' or filename == '..' then return end
	local filename_new = shell.inputbox("Rename", "Enter new file name:", filename, function(name) return not name:match('[\\/:|*?"<>]') end)
	if filename_new == nil then return end
	if filename_new ~= '' and filename_new ~= filename then
		os.rename(current_path..filename, current_path..filename_new)
		FileMan_ListFILL()
	end
end

function FileMan_NewFile()
	local filename_selected , attr = FileMan_GetSelectedItem()
	if filename_selected == '' or attr == 'd' then
		filename = 'newfile.*'
	else
		filename = 'newfile.'..filename_selected:gsub('.+%.','')
	end
	local filename_new = shell.inputbox("New File", "Enter new file name:", filename, function(name) return not name:match('[\\/:|*?"<>]') end)
	if filename_new ~= nil then
		temp = assert(io.open(current_path..filename_new,'w'))
		if(temp ~= nil) then
			io.close(temp)
			FileMan_ListFILL()
			OpenFile(current_path..filename_new)
		end
	end
end

function FileMan_FileDelete()
	local filename, attr = FileMan_GetSelectedItem()
	if filename == '' then return end
	if attr == 'd' then return end
	if shell.msgbox("Are you sure you want to DELETE this file?\n"..filename, "DELETE", 4+256) == 6 then
	-- if gui.message("Are you sure you want to DELETE this file?\n"..filename, "query") then
		os.remove(current_path..filename)
		FileMan_ListFILL()
	end
end

local function FileMan_FileExecWithSciTE(cmd, mode)
	local p0 = props["command.0.*"]
	local p1 = props["command.mode.0.*"]
	props["command.name.0.*"] = 'tmp'
	props["command.0.*"] = cmd
	if mode == nil then mode = 'console' end
	props["command.mode.0.*"] = 'subsystem:'..mode..',savebefore:no'
	scite.MenuCommand(9000)
	props["command.0.*"] = p0
	props["command.mode.0.*"] = p1
end

local function FileMan_OpenItem()
	local dir_or_file, attr = FileMan_GetSelectedItem()
	if dir_or_file == '' then return end
	if attr == 'd' then
		gui.chdir(dir_or_file)
		if dir_or_file == '..' then
			local new_path = current_path:gsub('(.*\\).*\\$', '%1')
			if not gui.files(new_path..'*',true) then return end
			current_path = new_path
		else
			current_path = current_path..dir_or_file..'\\'
		end
		FileMan_ListFILL()
	else
		OpenFile(current_path..dir_or_file)
	end
end

list_dir:on_double_click(function()
	FileMan_OpenItem()
end)

list_dir:on_key(function(key)
	if key == 13 then -- Enter
		FileMan_OpenItem()
	elseif key == 8 then -- BackSpace
		list_dir:set_selected_item(0)
		FileMan_OpenItem()
	elseif key == 46 then -- Delete
		FileMan_FileDelete()
	elseif key == 45 then -- Insert
		Favorites_AddFile()
	end
end)

----------------------------------
-- Project Functions
----------------------------------

function Project_Fill_Tree(node,path)
	if path == '' then return end
	if tree_nodes[node] == nil then
		--path = path:gsub('\\','\\\\');
		if node==0 then
			--add_tree(int parent,string caption,bool hasChildren,mixed data)
			node = list_project:add_tree(node,path,true,{path,'d'})
			tree_nodes[0] = true
			tree_nodes['root'] = node
		end
		local folders = gui.files(path..'/*', true)
		if folders then
			for i, d in ipairs(folders) do
				if d ~= '.svn' then -- escape SVN folder
					list_project:add_tree(node,d,true,{path.."/"..d,'d'})
				end
			end
			folders = nil
			tree_nodes[node] = true
		end
		local files = gui.files(path..'/'..file_mask)
		if files then
			for i, filename in ipairs(files) do
				list_project:add_tree(node,filename,false,{path.."/"..filename,'f'})
			end
			files = nil
			tree_nodes[node] = true
		end
		list_project:expand(tree_nodes['root'],true)
	end
end

function Project_Tree_Clear()
	list_project:clear_tree()
	tree_nodes = {}
end

function Project_Build_Tags()
	if (props['ProjectPath'] ~= nil and props['ProjectPath'] ~= '') then
		scite.MenuCommand(1143)
		reset_tags()
	end
end

function Open_Project_Dir()
	local Path = gui.select_dir_dlg('Please select a directory')
	if Path == nil then return end
	Project_Save_Path(Path)
	Project_Tree_Clear()
	Project_Fill_Tree(0,Path)
	if tonumber(props['project.opendir.buildtags']) == 1 then
		Project_Build_Tags()
	end
end

function Project_NewFile()
	filename = 'newfile.*'
	local filename_new = shell.inputbox("New File", "Enter new file name:", filename, function(name) return not name:match('[\\/:|*?"<>]') end)
	if filename_new ~= nil then
		temp = assert(io.open(current_path..filename_new,'w'))
		if(temp ~= nil) then
			io.close(temp)
			Project_Refresh_Tree()
			OpenFile(current_path..filename_new)
		end
	end
end

function Project_Save_Path(path)
	if path =='' then end
	local file = io.open(props["SciteUserHome"] .."\\SciTE.project","w")
	props['ProjectPath'] = path
	if(file ~= nil) then
		file:write(path)
		file:close()
	end
end

function Project_Get_Store_Path()
	local file = io.open(props["SciteUserHome"] .."\\SciTE.project", "r")
	if(file ~= nil) then
		local ourline = file:read()
		if ourline ~=nil then
			props['ProjectPath'] = ourline
			return ourline
		end
		file:close()
	end
	return ''
end

function Project_Refresh_Tree()
	Project_Tree_Clear()
	Project_Fill_Tree(0,Project_Get_Store_Path())
end

function Project_GetSelectedItem()
	local data = list_project:get_tree_data()
	if data then
		local dir_or_file = data[1]
		local attr = data[2]
		return dir_or_file, attr
	else
		return nil,'n'
	end
end

function SVN_Update()
	local dir_or_file, attr = Project_GetSelectedItem()
	if dir_or_file ~= nil then
		svn_exec('update',dir_or_file)
	end
end

function SVN_Commit()
	local dir_or_file, attr = Project_GetSelectedItem()
	if dir_or_file ~= nil then
		svn_exec('commit',dir_or_file)
	end
end

function SVN_Add()
	local dir_or_file, attr = Project_GetSelectedItem()
	if dir_or_file ~= nil then
		svn_exec('add',dir_or_file)
	end
end

-----------------------------
-- Project Events
-----------------------------

list_project:on_select(function(item)
	if item then
		local data = list_project:get_tree_data(item)
		if data[2] == 'd' then
			Project_Fill_Tree(item,data[1])
			current_path = data[1] .. '\\'
		end
	end
end)

list_project:on_double_click(function(item)
	local data = list_project:get_tree_data(item)
	if data[2] == 'f' then
		OpenFile(data[1])
	else
		Project_Fill_Tree(item,data[1])
	end
end)

list_project:on_key(function(key)
	if key==13 then
		--local item = list_project:get_selected_tree()
		local data = list_project:get_tree_data()
		if data[2] == 'f' then
			OpenFile(data[1])
		else
			--to do something with dir
			--Project_Fill_Tree(item,data[1])
			list_project:toggle()
		end
	--elseif key==17 then
	--	local item = list_project:get_tree_parent()
	--	list_project:toggle(item)
	end
end)

----------------------------------------------------------
-- tab0:list_functions   Functions/Procedures
----------------------------------------------------------
local table_functions = {}
-- 1 - function names
-- 2 - line number
-- 3 - function paramaters with parentheses
local _sort = 'order'
local _backjumppos -- store position if jumping

local Lang2lpeg = {}
do
	local P, V, Cg, Ct, Cc, S, R, C, Carg, Cf, Cb, Cp, Cmt = lpeg.P, lpeg.V, lpeg.Cg, lpeg.Ct, lpeg.Cc, lpeg.S, lpeg.R, lpeg.C, lpeg.Carg, lpeg.Cf, lpeg.Cb, lpeg.Cp, lpeg.Cmt

	--@todo: переписать ?использованием lpeg.Cf
	local function AnyCase(str)
		local res = P'' --empty pattern to start with
		local ch, CH
		for i = 1, #str do
			ch = str:sub(i,i):lower()
			CH = ch:upper()
			res = res * S(CH..ch)
		end
		assert(res:match(str))
		return res
	end

	local PosToLine = function (pos) return editor:LineFromPosition(pos) end

--v------- common patterns -------v--
	-- basics
	local EOF = P(-1)
	local BOF = P(function(s,i) return (i==1) and 1 end)
	local NL = P"\n"-- + P"\f" -- pattern matching newline, platform-specific. \f = page break marker
	local AZ = R('AZ','az')+"_"
	local N = R'09'
	local ANY =  P(1)
	local ESCANY = P'\\'*ANY + ANY
	local SINGLESPACE = S'\n \t\r\f'
	local SPACE = SINGLESPACE^1

	-- simple tokens
	local IDENTIFIER = AZ * (AZ+N)^0 -- simple identifier, without separators

	local Str1 = P'"' * ( ESCANY - (S'"'+NL) )^0 * (P'"' + NL)--NL == error'unfinished string')
	local Str2 = P"'" * ( ESCANY - (S"'"+NL) )^0 * (P"'" + NL)--NL == error'unfinished string')
	local STRING = Str1 + Str2

	-- c-like-comments
	local line_comment = '//' * (ESCANY - NL)^0*NL
	local block_comment = '/*' * (ESCANY - P'*/')^0 * (P('*/') + EOF)
	local COMMENT = (line_comment + block_comment)^1

	local SC = SPACE + COMMENT
	local IGNORED = SPACE + COMMENT + STRING
	-- special captures
	local cp = Cp() -- pos capture, Carg(1) is the shift value, comes from start_code_pos
	local cl = cp/PosToLine -- line capture, uses editor:LineFromPosition
	local par = C(P"("*(1-P")")^0*P")") -- captures parameters in parentheses
--^------- common patterns -------^--
	do --v----- MQL ------v--
		-- define local patterns
		local keywords = P'if'+P'else'+P'switch'+P'case'+P'while'+P'for'
		local nokeyword = -(keywords)
		local type = P"static "^-1*P"const "^-1*P"enum "^-1*P'*'^-1*IDENTIFIER*P'*'^-1
		local funcbody = P"{"*(ESCANY-P"}")^0*P"}"
		-- redefine common patterns
		local IDENTIFIER = P'*'^-1*P'~'^-1*IDENTIFIER
		IDENTIFIER = IDENTIFIER*(P"::"*IDENTIFIER)^-1
		-- create flags:
		type = Cg(type,'')
		-- create additional captures
		local I = nokeyword*C(IDENTIFIER)*cl
		-- definitions to capture:
		local funcdef = nokeyword*Ct((type*SC^1)^-1*I*SC^0*par*SC^0*(#funcbody))
		local classconstr = nokeyword*Ct((type*SC^1)^-1*I*SC^0*par*SC^0*P':'*SC^0*IDENTIFIER*SC^0*(P"("*(1-P")")^0*P")")*SC^0*(#funcbody)) -- this matches smthing like PrefDialog::PrefDialog(QWidget *parent, blabla) : QDialog(parent)
		-- resulting pattern, which does the work
		local patt = (classconstr + funcdef + IGNORED^1 + IDENTIFIER + ANY)^0 * EOF

		Lang2lpeg['MQL'] = lpeg.Ct(patt)
	end --^----- MQL ------^--
	
	do --v------- asm -------v--
		-- redefine common patterns
		local SPACE = S' \t'^1
		local NL = P"\r\n"

		local IGNORED = (ESCANY - NL)^0 * NL -- just skip line by line

		-- define local patterns
		local p = P"proc"
		local F = P"FRAME"
		-- create flags:
		F = Cg(F*Cc(true),'F')
		-- create additional captures
		I = C(IDENTIFIER)*cl
		-- definitions to capture:
		local par = C((ESCANY - NL)^0)
		local def1 = I*SPACE*(p+F)
		local def2 = p*SPACE*I*P','^-1
		local def = (SPACE+P'')*Ct((def1+def2)*(SPACE*par)^-1)*NL
		-- resulting pattern, which does the work
		local patt = (def + IGNORED + 1)^0 * EOF

		Lang2lpeg.Assembler = lpeg.Ct(patt)
	end --do --^------- ASM -------^--

	do --v------- Lua -------v--
		-- redefine common patterns
		local IDENTIFIER = IDENTIFIER*(P'.'*IDENTIFIER)^0*(P':'*IDENTIFIER)^-1
		-- LONG BRACKETS
		local long_brackets = #(P'[' * P'='^0 * P'[') *
			function (subject, i1)
				local level = _G.assert( subject:match('^%[(=*)%[', i1) )
				local _, i2 = subject:find(']'..level..']', i1, true)  -- true = plain "find substring"
				return (i2 and (i2+1)) or #subject+1--error('unfinished long brackets')
				-- ^ if unfinished long brackets then capture till EOF (at #subject+1)
		end
		local LUALONGSTR = long_brackets

		local multi  = P'--' * long_brackets
		local single = P'--' * (1 - NL)^0 * NL
		local COMMENT = multi + single
		local SC = SPACE + COMMENT

		local IGNORED = SPACE + COMMENT + STRING + LUALONGSTR

		-- define local patterns
		local f = P"function"
		local l = P"local"
		-- create flags
		l = Cg(l*SC^1*Cc(true),'l')^-1
		-- create additional captures
		I = C(IDENTIFIER)*cl
		-- definitions to capture:
		local funcdef1 = l*f*SC^1*I*SC^0*par -- usual function declaration
		local funcdef2 = l*I*SC^0*"="*SC^0*f*SC^0*par -- declaration through assignment
		local def = Ct(funcdef1 + funcdef2)
		-- resulting pattern, which does the work
		local patt = (def + IGNORED^1 + IDENTIFIER + 1)^0 * (EOF) --+ error'invalid character')

		Lang2lpeg.Lua = lpeg.Ct(patt)
	end --do --^------- Lua -------^--

	do --v----- Pascal ------v--
		-- redefine common patterns
		local IDENTIFIER = IDENTIFIER*(P'.'*IDENTIFIER)^0
		local STRING = P"'" *( ANY - (P"'"+NL) )^0 *(P"'"+NL) --NL == error'unfinished string')
		--^ there's no problem with pascal strings with double single quotes in the middle, like this:
		--  'first''second'
		--  in the loop, STRING just matches the 'first'-part, and then the 'second'.

		local multi1  = P'(*' *(1-P'*)')^0 * (P'*)' + EOF)--unfinished long comment
		local multi2  = P'{' *(1-P'}')^0 * (P'}' + EOF)--unfinished long comment
		local single = P'//' * (1 - NL)^0 * NL
		local COMMENT = multi1 + multi2 + single

		local SC = SPACE + COMMENT
		local IGNORED = SPACE + COMMENT + STRING

		-- define local patterns
		local f = AnyCase"function"
		local p = AnyCase"procedure"
		local c = AnyCase"constructor"
		local d = AnyCase"destructor"
		local restype = AZ^1
		-- create flags:
		-- f = Cg(f*Cc(true),'f')
		restype = Cg(C(restype),'')
		p = Cg(p*Cc(true),'p')
		c = Cg(c*Cc(true),'c')
		d = Cg(d*Cc(true),'d')
		-- create additional captures
		local I = C(IDENTIFIER)*cl
		-- definitions to capture:
		local procdef = Ct((p+c+d)*SC^1*I*SC^0*par^-1)
		local funcdef = Ct(f*SC^1*I*SC^0*par^-1*SC^0*P':'*SC^0*restype*SC^0*P';')
		-- resulting pattern, which does the work
		local patt = (procdef + funcdef + IGNORED^1 + IDENTIFIER + 1)^0 * EOF

		Lang2lpeg.Pascal = lpeg.Ct(patt)
	end --^----- Pascal ------^--

	do --v----- C++ ------v--
		-- define local patterns
		local keywords = P'if'+P'else'+P'switch'+P'case'+P'while'
		local nokeyword = -(keywords*SC^1)
		local type = P"static "^-1*P"const "^-1*P"enum "^-1*P'*'^-1*IDENTIFIER*P'*'^-1
		local funcbody = P"{"*(ESCANY-P"}")^0*P"}"
		-- redefine common patterns
		local IDENTIFIER = P'*'^-1*P'~'^-1*IDENTIFIER
		IDENTIFIER = IDENTIFIER*(P"::"*IDENTIFIER)^-1
		-- create flags:
		type = Cg(type,'')
		-- create additional captures
		local I = C(IDENTIFIER)*cl
		-- definitions to capture:
		local funcdef = nokeyword*Ct((type*SC^1)^-1*I*SC^0*par*SC^0*(#funcbody))

		-- resulting pattern, which does the work
		local patt = (funcdef + IGNORED^1 + IDENTIFIER + ANY)^0 * EOF

		Lang2lpeg['C++'] = lpeg.Ct(patt)
	end --^----- C++ ------^--

	do --v----- JS ------v--
		-- redefine common patterns
		local NL = NL + P"\f"
		local regexstr = P'/' * (ESCANY - (P'/' + NL))^0*(P'/' * S('igm')^0 + NL)
		local STRING = STRING + regexstr
		-- define local patterns
		local f = P"function"
		local funcbody = P"{"*(ESCANY-P"}")^0*P"}"
		-- create additional captures
		local I = C(IDENTIFIER)*cl
		-- definitions to capture:
		local funcdef = Ct(f*SC^1*I*SC^0*par*SC^0*(#funcbody))

		-- resulting pattern, which does the work
		local patt = (funcdef + IGNORED^1 + IDENTIFIER + 1)^0 * EOF

		Lang2lpeg.JScript = lpeg.Ct(patt)
	end --^----- JS ------^--

	do --v----- VB ------v--
		-- redefine common patterns
		local STRING = P'"' * (ANY - (P'"' + NL))^0*(P'"' + NL)
		local COMMENT = (P"'" + P"REM ") * (ANY - NL)^0*NL
		local SC = SPACE
		-- define local patterns
		local f = AnyCase"function"
		local p = AnyCase"property"
			local let = AnyCase"let"
			local get = AnyCase"get"
			local set = AnyCase"set"
		local s = AnyCase"sub"
		-- create flags:
		-- f = Cg(f*Cc(true),'f')
		local restype = (P"As"+P"as")*SPACE*Cg(C(AZ^1),'')
		let = Cg(let*Cc(true),'pl')
		get = Cg(get*Cc(true),'pg')
		set = Cg(set*Cc(true),'ps')
		p = p*SC^1*(let+get+set)
		-- create additional captures
		local I = C(IDENTIFIER)*cl
		-- definitions to capture:
		f = f*SC^1*I*SC^0*par
		p = p*SC^1*I*SC^0*par
		s = s*SC^1*I*SC^0*par
		local def = Ct((f + s + p)*(SPACE*restype)^-1)
		-- resulting pattern, which does the work
		local patt = (def + IGNORED^1 + IDENTIFIER + 1)^0 * EOF

		Lang2lpeg.VisualBasic = lpeg.Ct(patt)
	end --^----- VB ------^--

	do --v------- Python -------v--
		-- redefine common patterns
		local SPACE = S' \t'^1
		local IGNORED = (ESCANY - NL)^0 * NL -- just skip line by line
		-- define local patterns
		local c = P"class"
		local d = P"def"
		-- create flags:
		c = Cg(c*Cc(true),'class')
		-- create additional captures
		I = C(IDENTIFIER)*cl
		-- definitions to capture:
		local def = (c+d)*SPACE*I
		def = (SPACE+P'')*Ct(def*SPACE^-1*par)*SPACE^-1*P':'
		-- resulting pattern, which does the work
		local patt = (def + IGNORED + 1)^0 * EOF

		Lang2lpeg.Python = lpeg.Ct(patt)
	end --do --^------- Python -------^--

	do --v------- nnCron -------v--
		-- redefine common patterns
		local IDENTIFIER = (ANY - SPACE)^1
		local SPACE = S' \t'^1
		local IGNORED = (ESCANY - NL)^0 * NL -- just skip line by line
		-- define local patterns
		local d = P":"
		-- create additional captures
		I = C(IDENTIFIER)*cl
		-- definitions to capture:
		local def = d*SPACE*I
		def = Ct(def*(SPACE*par)^-1)*IGNORED
		-- resulting pattern, which does the work
		local patt = (def + IGNORED + 1)^0 * EOF

		Lang2lpeg.nnCron = lpeg.Ct(patt)
	end --do --^------- nnCron -------^--

	do --v------- CSS -------v--
		-- helper
		local function clear_spaces(s)
			return s:gsub('%s+',' ')
		end
		-- redefine common patterns
		local IDENTIFIER = (ANY - SPACE)^1
		local NL = P"\r\n"
		local SPACE = S' \t'^1
		local IGNORED = (ANY - NL)^0 * NL -- just skip line by line
		local par = C(P"{"*(1-P"}")^0*P"}")/clear_spaces -- captures parameters in parentheses
		-- create additional captures
		I = C(IDENTIFIER)*cl
		-- definitions to capture:
		local def = Ct(I*SPACE*par)--*IGNORED
		-- resulting pattern, which does the work
		local patt = (def + IGNORED + 1)^0 * EOF

		Lang2lpeg.CSS = lpeg.Ct(patt)
	end --do --^------- CSS -------^--

	do --v----- * ------v--
		-- redefine common patterns
		local NL = P"\r\n"+P"\n"+P"\f"
		local SC = S" \t\160" -- бе?по?тия чт?за символ ?кодо?160, но он встречается ?SciTEGlobal.properties непосредственн?посл?[Warnings] 10 ра?
		local COMMENT = P'#'*(ANY - NL)^0*NL
		-- define local patterns
		local somedef = S'fFsS'*S'uU'*S'bBnN'*AZ^0 --пытаем? поймат?чт?нибудь, похоже?на определени?функци?..
		local section = P'['*(ANY-P']')^1*P']'
		-- create flags
		local somedef = Cg(somedef, '')
		-- create additional captures
		local I = C(IDENTIFIER)*cl
		section = C(section)*cl
		local tillNL = C((ANY-NL)^0)
		-- definitions to capture:
		local def1 = Ct(somedef*SC^1*I*SC^0*(par+tillNL))
		local def2 = (NL+BOF)*Ct(section*SC^0*tillNL)*NL

		-- resulting pattern, which does the work
		local patt = (def2 + def1 + COMMENT + IDENTIFIER + 1)^0 * EOF
		-- local patt = (def2 + def1 + IDENTIFIER + 1)^0 * EOF -- чуть медленне?

		Lang2lpeg['*'] = lpeg.Ct(patt)
	end --^----- * ------^--

end

local Lang2CodeStart = {
	['Pascal']='^IMPLEMENTATION$',
}

local Lexer2Lang = {
	['asm']='Assembler',
	['cpp']='C++',
	['js']='JScript',
	['vb']='VisualBasic',
	['vbscript']='VisualBasic',
	['css']='CSS',
	['pascal']='Pascal',
	['php']='Php',
	['python']='Python',
	['lua']='Lua',
	['nncrontab']='nnCron',
	['mql']='MQL',
}

local Ext2Lang = {}
do -- Fill_Ext2Lang
	local patterns = {
		[props['file.patterns.asm']]='Assembler',
		[props['file.patterns.cpp']]='C++',
		[props['file.patterns.wsh']]='JScript',
		[props['file.patterns.vb']]='VisualBasic',
		[props['file.patterns.wscript']]='VisualBasic',
		['*.css']='CSS',
		[props['file.patterns.pascal']]='Pascal',
		[props['file.patterns.php']]='Php',
		[props['file.patterns.py']]='Python',
		[props['file.patterns.lua']]='Lua',
		[props['file.patterns.nncron']]='nnCron',
		[props['file.patterns.mql']]='MQL',
	}
	for i,v in pairs(patterns) do
		for ext in (i..';'):gfind("%*%.([^;]+);") do
			Ext2Lang[ext] = v
		end
	end
end -- Fill_Ext2Lang

local function GetFlagsAndCut(findString)
	local findString = findString
	local t = {}
	findString,f = ReplaceWithoutCase(findString, "Sub ", "") -- VB
	t["s"] = f and true
	findString,f = ReplaceWithoutCase(findString, "Function ", "") -- JS, VB,...
	t["f"] = f and true
	findString,f = ReplaceWithoutCase(findString, "Procedure ", "") -- Pascal
	t["p"] = f and true
	findString,f = ReplaceWithoutCase(findString, "Proc ", "") -- C
	t["p"] = t.p or (f and true)
	findString,f = ReplaceWithoutCase(findString, "Property Let ", "") -- VB
	t["pl"] = f and true
	findString,f = ReplaceWithoutCase(findString, "Property Get ", "") -- VB
	t["pg"] = f and true
	findString,f = ReplaceWithoutCase(findString, "Property Set ", "") -- VB
	t["ps"] = f and true
	findString,f = ReplaceWithoutCase(findString, "CLASS ", "") -- Phyton
	t["c"] = f and true
	findString,f = ReplaceWithoutCase(findString, "DEF ", "") -- Phyton
	t["d"] = f and true
	return findString, t
end

local function Functions_GetNames()
	_DEBUG.timerstart('Functions_GetNames')
	table_functions = {}
	if editor.Length == 0 then return end

	local ext = props["FileExt"]:lower() -- a bit unsafe...
	local lang = Ext2Lang[ext]

	local start_code = Lang2CodeStart[lang]
	local lpegPattern = Lang2lpeg[lang]
	if not lpegPattern then
		-- lang = Lexer2Lang[editor.LexerLanguage]
		start_code = Lang2CodeStart[lang]
		lpegPattern = Lang2lpeg[lang]
		if not tablePattern then
			start_code = Lang2CodeStart['*']
			lpegPattern = Lang2lpeg['*']
		end
	end
	local textAll = editor:GetText()
	local start_code_pos = 0
	if start_code then
		start_code_pos = editor:findtext(start_code, SCFIND_REGEXP)
	end

	-- lpegPattern = nil
	table_functions = lpegPattern:match(textAll, start_code_pos+1) -- 2nd arg is the symbol index to start with

	_DEBUG.timerstop('Functions_GetNames','lpeg')
end

local function Functions_ListFILL()
	if tonumber(props['sidebar.show'])~=1 or tab_index~=0 then return end
	if _sort == 'order' then
		table.sort(table_functions, function(a, b) return a[2] < b[2] end)
	else
		table.sort(table_functions, function(a, b) return a[1]:lower() < b[1]:lower() end)
	end
	-- remove duplicates
	for i = #table_functions, 2, -1 do
		if table_functions[i][2] == table_functions[i-1][2] then
			table.remove (table_functions, i)
		end
	end
	list_functions:clear()

	local function emptystr(...) return '' end
	local function GetParams (funcitem)
		return (funcitem[3] and ' '..funcitem[3]) or ''
	end
	local function GetFlags (funcitem)
		local res = ''
		local add = ''
		for flag,value in pairs(funcitem) do
			if type(flag)=='string' then
				if type(value)=='string' then	add = flag .. value
				elseif type(value)=='number' then add = flag..':'..value
				else add = flag end
				res = res .. '['.. add ..']'
			end
		end
		if res~='' then res = res .. ' ' end
		return res or ''
	end
	if not _show_params then GetParams = emptystr end
	if not _show_flags then GetFlags = emptystr end

	local function fixname (funcitem)
		return GetFlags(funcitem)..funcitem[1]..GetParams(funcitem)
	end
	for _, a in ipairs(table_functions) do
		list_functions:add_item(fixname(a), a[2])
	end
end

function Functions_SortByOrder()
	_sort = 'order'
	Functions_ListFILL()
end

function Functions_SortByName()
	_sort = 'name'
	Functions_ListFILL()
end

function Functions_ToggleParams ()
	_show_params = not _show_params
	Functions_ListFILL()
end

function Functions_ToggleFlags ()
	_show_flags = not _show_flags
	Functions_ListFILL()
end

local function Functions_GotoLine()
	local sel_item = list_functions:get_selected_item()
	if sel_item == -1 then return end
	local pos = list_functions:get_item_data(sel_item)
	if pos then
		ShowCompactedLine(pos)
		editor:GotoLine(pos)
		gui.pass_focus()
	end
end

list_functions:on_double_click(function()
	Functions_GotoLine()
end)

list_functions:on_key(function(key)
	if key == 13 then -- Enter
		Functions_GotoLine()
	end
end)

----------------------------------------------------------
-- tab2:list_abbrev   Abbreviations
----------------------------------------------------------
local function Abbreviations_ListFILL()
	local function ReadAbbrev(file)
		local abbrev_file = io.open(file)
		if abbrev_file then
			for line in abbrev_file:lines() do
				if line ~= '' then
					local _abr, _exp = line:match('^([^#].-)=(.+)')
					if _abr ~= nil then
						list_abbrev:add_item({_abr, _exp}, {_abr, _exp})
						abbrev_list[_abr] = _exp
					else
						local import_file = line:match('^import%s+(.+)')
						if import_file ~= nil then
							ReadAbbrev(file:match('.+[\\/]')..import_file)
						end
					end
				end
			end
			abbrev_file:close()
		end
	end
	list_abbrev:clear()
	--local abbrev_filename = props['SciteDefaultHome'] .. '/abbrevs/' .. props['FileExt'] .. '.abbrev'
	local abbrev_filename = props['AbbrevPath']
	ReadAbbrev(abbrev_filename)
end

local function Abbreviations_InsertExpansion()
	local begin = 0
	local sel_item = list_abbrev:get_selected_item()
	if sel_item == -1 then return end
	local abbrev = list_abbrev:get_item_data(sel_item)
	editor:BeginUndoAction()
	editor:AddText(abbrev[1])
	scite.MenuCommand(IDM_ABBREV)
	editor:EndUndoAction()
	gui.pass_focus()
	editor:CallTipCancel()
end

local function Abbreviations_ShowExpansion()
	local sel_item = list_abbrev:get_selected_item()
	if sel_item == -1 then return end
	local list = list_abbrev:get_item_data(sel_item)
	local expansion = list[2]
	expansion = expansion:gsub('\\\\','\4'):gsub('\\r','\r'):gsub('(\\n','\n'):gsub('\\t','\t'):gsub('\4','\\')
	editor:CallTipCancel()
	editor:CallTipShow(editor.CurrentPos, expansion)
end

function Edit_Abbrev_File()
	--scite.Open(props['SciteDefaultHome'] .. '/abbrevs/' .. props['FileExt'] .. '.abbrev')
	scite.Open(props['AbbrevPath'])
end


list_abbrev:on_double_click(function()
	Abbreviations_InsertExpansion()
end)

list_abbrev:on_select(function()
	Abbreviations_ShowExpansion()
end)

list_abbrev:on_key(function(key)
	if key == 13 then -- Enter
		Abbreviations_InsertExpansion()
	end
end)

----------------------------------------------------------
-- Events
----------------------------------------------------------
local function OnSwitch()
	_DEBUG.timerstart('OnSwitch')
	if tab0:bounds() then -- visible FileManager
		local path = props['FileDir']
		if path == '' then return end
		path = path:gsub('\\$','')..'\\'
		if path ~= current_path then
			current_path = path
			FileMan_ListFILL()
		end
		Functions_GetNames()
		Functions_ListFILL()
	elseif tab1:bounds() then -- visible Project
		Project_Fill_Tree(0,Project_Get_Store_Path())
	elseif tab2:bounds() then -- visible Abbrev
		Abbreviations_ListFILL()
	end
	if not tab2:bounds() then
		editor:CallTipCancel()
	end
	_DEBUG.timerstop('OnSwitch')
end

tabs:on_select(function(ind)
	tab_index=ind
	OnSwitch()
end)

function SideBar_ShowHide()
	local position = 'right'
	if props['sidebar.position'] ~= nil then
		position = props['sidebar.position'];
	end
	if tonumber(props['sidebar.show'])==1 then
		if win then
			win_parent:hide()
		else
			gui.set_panel()
		end
		gui.pass_focus()
		props['sidebar.show']=0
	else
		if win then
			win_parent:show()
		else
			gui.set_panel(win_parent,position)
		end
		props['sidebar.show']=1
		OnSwitch()
	end
end

local function OnDocumentCountLinesChanged(def_line_count)
	if tab0:bounds() then -- visible Function
		local cur_line = editor:LineFromPosition(editor.CurrentPos)
		for i = 1, #table_functions do
			local table_line = table_functions[i][2]
			if table_line > cur_line then
				table_functions[i][2] = table_line + def_line_count
			end
		end
		Functions_ListFILL()
	end
end

-- Add user event handler OnSwitchFile
local old_OnSwitchFile = OnSwitchFile
function OnSwitchFile(file)
	local result
	if old_OnSwitchFile then result = old_OnSwitchFile(file) end
	OnSwitch()
	return result
end

-- Add user event handler OnOpen
local old_OnOpen = OnOpen
function OnOpen(file)
	local result
	if old_OnOpen then result = old_OnOpen(file) end
	OnSwitch()
	return result
end

-- Add user event handler OnKey
local line_count = 0
local old_OnKey = OnKey
function OnKey(key, shift, ctrl, alt, char)
	local result
	if old_OnKey then result = old_OnKey(key, shift, ctrl, alt, char) end
	if (editor.Focus) then
		local line_count_new = editor.LineCount
		local def_line_count = line_count_new - line_count
		if def_line_count ~= 0 then
			OnDocumentCountLinesChanged(def_line_count)
			line_count = line_count_new
		end
	end
	return result
end

-- Add user event handler OnSave
local old_OnSave = OnSave
function OnSave(file)
	local result
	if old_OnSave then result = old_OnSave(file) end
	Functions_GetNames()
	Functions_ListFILL()
	return result
end

-- Add user event handler OnSendEditor
local old_OnSendEditor = OnSendEditor
function OnSendEditor(id_msg, wp, lp)
	local result
	if old_OnSendEditor then result = old_OnSendEditor(id_msg, wp, lp) end
	return result
end

-- Add user event handler OnFinalise
local old_OnFinalise = OnFinalise
function OnFinalise()
	local result
	if old_OnFinalise then result = old_OnFinalise() end
	Favorites_SaveList()
	return result
end

----------------------------------------------------------
-- Go to function definition
----------------------------------------------------------
local function Func2Line(funcname)
	if not next(table_functions) then
		Functions_GetNames()
	end
	for i = 1, #table_functions do
		if funcname == table_functions[i][1] then
			return table_functions[i][2]
		end
	end
end

local function JumpToFuncDefinition()
	local funcname = props['CurrentWord']
	local line = Func2Line(funcname)
	if line then
		_backjumppos = editor.CurrentPos
		editor:GotoLine(line)
		return true
	end
	return false
end

local function JumpBack()
	if not _backjumppos then return false end
	editor:GotoPos(_backjumppos)
	_backjumppos = nil
	return true
end

-- Add user event handler OnDoubleClick
local old_OnDoubleClick = OnDoubleClick
function OnDoubleClick(shift, ctrl, alt)
	local result
	if old_OnDoubleClick then result = old_OnDoubleClick(shift, ctrl, alt) end
	if shift then
		if JumpToFuncDefinition() then return true end
	end
	return result
end

-- Add user event handler OnKey
local old_OnKey = OnKey
function OnKey(key, shift, ctrl, alt, char)
	local result
	if old_OnKey then result = old_OnKey(key, shift, ctrl, alt, char) end
	if (editor.Focus) then
		--if ctrl and key == 188 and JumpBack() then return true end --char == ','
		--if ctrl and key == 190 and JumpToFuncDefinition() then return true end --char == '.'
	else
		if ctrl and alt and key == 78 then FileMan_NewFile() end -- ctrl+alt+N
		if key ==27 and tonumber(props['sidebar.show']) == 1 then
			gui.pass_focus()
			return 0
		end
	end
	return result
end

--Active tab first,then switching...
function SideBar_Switch_Tabs(option)
	if tonumber(props['sidebar.show']) ~= 1 then return end
	local index = tabs:tab_selected()
	local curtab = eval("tab"..index)
	if editor.Focus then
		win_parent:client(curtab)
	else
		if option ~= '' and tonumber(option) < 0 then
			index = index - 1
		else
			index = index + 1
		end
		if index == tabs:tab_count() then
			index = 0
		elseif index < 0 then
			index = tabs:tab_count() - 1
		end
		tabs:set_tab(index)
	end
end

AddEventHandler("OnUpdateUI", function()
	if tonumber(props['sidebar.dock'])==1 then
		if tonumber(props['sidebar.show'])==1 then
			if win then
				win_parent:hide()
			else
				gui.set_panel()
			end
			gui.pass_focus()
			props['sidebar.show']=0
		end
	end
end)