-- Demonstrates a Useful Side Bar for SciTE
-- you can choose to make it a stand-alone window; just uncomment the two lines
-- involving 'gui.window' and comment out the 'w = gui.panel' line.

local append = table.insert
local current_path = props['FileDir']
local ext = props['FileExt']

-- w = gui.window "Files"
-- w:size(100,400)
local w = gui.panel(100)
local ls = gui.list(true)
local dirs = gui.list(true)
local bookmarks = gui.list(true)
gui.set_panel(w,"right")
ls:size(100,100)
ls:add_column("Files",100);
dirs:add_column("Directories",100)
bookmarks:add_column("Bookmarks",100)
w:add(dirs,"top",150)
w:add(bookmarks,"top",150)
w:client(ls)
w:show()

function name_of (f)
    return f:match('([^\\]+)%.%w+$')
end

local dirsep = '\\'

local function makepath (f)
    return current_path..dirsep..f
end

function fill ()
    local mask_base = makepath('*.')
	local mask = mask_base..current_ext
	local files = gui.files(mask)
    local same_ext = true
	ls:clear()
    -- note that gui.files will not return a table if there were no contents!
    if not files then
        files = gui.files(mask_base..'*')
        same_ext = false
    end
	if files then
		for i,f in ipairs(files) do
            local name = f
            if same_ext then name = name_of(name) end
			ls:add_item(name,f)
		end
	end
	local dirlist = gui.files(makepath('*'),true)
	dirs:clear()
	dirs:add_item ('[..]','..')
	for i,d in ipairs(dirlist) do
		dirs:add_item('['..d..']',d)
	end
end

ls:on_double_click(function(idx)
	if idx 	~= -1 then
		local file = ls:get_item_data(idx)
		scite.Open(makepath(file))
	end
end)

dirs:on_double_click(function(idx)
	if idx ~= -1 then
		local dir = dirs:get_item_data(idx)
		gui.chdir(dir)
		current_path = current_path..dirsep..dir
		fill()
	end
end)


function OnCommand (id)
    if id == IDM_BOOKMARK_TOGGLE then
        local line = editor:GetCurLine()
        -- is this line already in the list?
        local inserting = true
        for i = 0,bookmarks:count() - 1 do
            if bookmarks:get_item_text(i) == line then
                bookmarks:delete_item(i)
                inserting = false
                break
            end
        end
        if inserting then
            local lno = editor:LineFromPosition(editor.CurrentPos)
            bookmarks:add_item(line,{props['FilePath'],lno})
        end
    end
end

bookmarks:on_double_click(function(idx)
    local pos = bookmarks:get_item_data(idx)
    if pos then
        scite.Open(pos[1])
        editor:GotoLine(pos[2])
    end
end)

local function on_switch ()
	local path = props['FileDir']
	local ext = props['FileExt']
    if path == '' then return end
	if path ~= current_path or ext ~= current_ext then
		current_path = path
		current_ext = ext
		fill()
	end
end

local oldOnSwitchFile = OnSwitchFile
local oldOnOpen = OnOpen

function OnSwitchFile(file)
	on_switch()
	if oldOnSwitchFile then oldOnSwitchFile(file) end
end

function OnOpen(file)
	on_switch()
	if oldOnOpen then oldOnOpen(file) end
end

