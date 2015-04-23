--[[--------------------------------------------------
FileManager.lua
Demonstrates a Useful Side Bar for SciTE
------------------------------------------------------
  Needed gui.dll <http://mysite.mweb.co.za/residents/sdonovan/scite/gui_ext.zip>
  Connection:
   In file SciTEStartup.lua add a line:
     dofile (props["SciteDefaultHome"].."\\tools\\FileManager.lua")
--]]--------------------------------------------------

local current_path = props['FileDir']
local file_path = props['FilePath']
local file_mask = '*.*'
local my_panel_width = 150
-- you can choose to make it a stand-alone window; just uncomment this line:
-- local win = true

require 'gui'
-- BUG: After this command in SciTE it is impossible to input text on russian :(

-----------------------------------------------------------
-- Panel:
if win then
	my_panel = gui.window "FileManager"
	my_panel:size(my_panel_width + 24, 600)
else
	my_panel = gui.panel(my_panel_width + 18)
	gui.set_panel(my_panel,"right")
end

function all_files() -- note not local function!
	file_mask = '*.*'
	fill_list_dir()
end

function current_ext() -- note not local function!
	file_mask = '*.'..props['FileExt']
	fill_list_dir()
end

my_panel:context_menu {
	'Show all files|all_files',
	'Only current ext|current_ext',
}

------------------------------
-- Memo: Path and Mask
text_path = gui.memo()
my_panel:add(text_path, "top", 22)

local function show_path()
	local rtf = '{\\rtf{\\fonttbl{\\f0\\fcharset0 MS Shell Dlg;}}{\\colortbl ;\\red0\\green0\\blue255;  \\red255\\green0\\blue0;}\\f0\\fs16'
	local path = '\\cf1'..string.gsub(current_path, '\\', '\\\\')..'\\\\'
	local mask = '\\cf2'..file_mask..'}'
	text_path:set_text(rtf..path..mask)
end

-----------------------------------------------------------
-- List: Folders and Files
local list_dir = gui.list(false) -- list without heading
my_panel:add(list_dir,"top",400)

function fill_list_dir() -- note not local function!
	list_dir:clear()

	local folders = gui.files(current_path..'\\*', true)
	-- add (caption, {data array})
	list_dir:add_item ('[..]', {'..','d'})
	for i, d in ipairs(folders) do
		list_dir:add_item('['..d..']', {d,'d'})
	end

	-- note that gui.files will not return a table if there were no contents!
	local files = gui.files(current_path..'\\'..file_mask)
	if files then
		for i, filename in ipairs(files) do
			-- add (caption, {data array})
			list_dir:add_item(filename, {filename})
		end
	end

	show_path()
end

list_dir:on_double_click(function(idx)
	if idx 	~= -1 then
		local data = list_dir:get_item_data(idx)
		local dir_or_file = data[1]
		local attr = data[2]
		-- print (attr, dir_or_file)
		if attr == 'd' then
			gui.chdir(dir_or_file)
			if dir_or_file == '..' then
				current_path = string.gsub(current_path,"(.*)\\.*$", "%1")
			else
				current_path = current_path..'\\'..dir_or_file
			end
			fill_list_dir()
		else
			scite.Open(current_path..'\\*'..dir_or_file)
			editor.Focus = true
		end
	end
end)

-----------------------------------------------------------
-- List: Bookmarks
local list_bookmarks = gui.list(true)
list_bookmarks:add_column("Bookmarks", my_panel_width)
my_panel:add(list_bookmarks, "top", 200)

local function text_restrict(text)
	return text:gsub('%s+', ' ')
end

local function togge_list_bookmarks(line_number)
	local line_text = text_restrict(editor:GetLine(line_number))
	-- is this line already in the list?
	for i = 0, list_bookmarks:count() - 1 do
		if list_bookmarks:get_item_text(i) == line_text then
			list_bookmarks:delete_item(i)
			return
		end
	end
	-- add (caption, {data array})
	list_bookmarks:add_item(line_text, {file_path, line_number})
end

function fill_list_bookmarks()
	local session_file = io.open(props['SciteUserHome']..'\\SciTE.session')
	local n = nil
	if session_file then
		for line in session_file:lines() do
			if n ~= nil then
				local bmk = string.match(line, 'buffer%.'..n..'%.bookmarks=(.*)')
				if bmk ~= nil then
					for line_number in string.gmatch(bmk, "%d+") do
						local caption = text_restrict(editor:GetLine(line_number-1))
						-- add (caption, {data array})
						list_bookmarks:add_item(caption, {file_path, line_number-1})
					end
					break
				end
			end
			if n == nil then n = string.match(line, 'buffer%.(%d+)%.path='..file_path) end
		end
		session_file:close()
	end
end

list_bookmarks:on_double_click(function(idx)
	if idx 	~= -1 then
		local pos = list_bookmarks:get_item_data(idx)
		if pos then
			scite.Open(pos[1])
			editor:GotoLine(pos[2])
			editor.Focus = true
		end
	end
end)

-----------------------------------------------------------
-- only win mode
if win then my_panel:show() end
-----------------------------------------------------------

-----------------------------------------------------------
local function on_switch ()
	file_path = props['FilePath']
	local path = props['FileDir']
	if path == '' then return end
	if path ~= current_path then
		current_path = path
		fill_list_dir()
	end
end

-----------------------------------------------------------

-- Add user event handler OnCommand (Call function togge_list_bookmarks())
local old_OnCommand = OnCommand
function OnCommand (msg, source)
	local result
	if old_OnMenuCommand then result = old_OnMenuCommand(msg, source) end
	if msg == IDM_BOOKMARK_TOGGLE then
		togge_list_bookmarks(editor:LineFromPosition(editor.CurrentPos))
	end
	return result
end

-- Add user event handler OnSwitchFile (Call function on_switch())
local old_OnSwitchFile = OnSwitchFile
function OnSwitchFile(file)
	local result
	if old_OnSwitchFile then result = old_OnSwitchFile(file) end
	on_switch()
	return result
end

-- Add user event handler OnOpen (Call function on_switch())
local old_OnOpen = OnOpen
function OnOpen(file)
	local result
	if old_OnOpen then result = old_OnOpen(file) end
	on_switch()
	fill_list_bookmarks()
	return result
end
