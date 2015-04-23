local path,name,ext
----[[
tt = gui.toolbar("toolbar!", {
	"FILEOPEN:switch to header|switch_header",
	"PASTE:create a new header|new_header",
})
tt:position(500,300)

local header_ext = {
	h = true, hpp = true, hh = true, hxx = true
}

local source_ext = {
	c = true, cpp = true, cc = true, cxx = true
}

function set_paths ()
	path = props['FileDir']
	name = props['FileName']
    ext = props['FileExt']
end

function try_open (exts)
    for ext,v in pairs(exts) do
	    local file = path..'\\'..name..'.'..ext
		local f = io.open(file,'r')
		if f then
			f:close()
			scite.Open(file)
			return
		end
	end
end

function switch_header ()
	set_paths()
	if header_ext[ext] then
		try_open(source_ext)
	elseif source_ext[ext] then
		try_open(header_ext)
	end
end

function new_header ()
	set_paths()
	name = gui.prompt_value("Give header base name",name)
	local file = path..'\\'..name..'.h'
    local f = io.open(file,'w')
	local guard = '__'..name:upper()..'_H'
	f:write('#ifndef '..guard..'\n')
	f:write('#define '..guard..'\n\n')
	f:write('#endif\n')
	f:close()
	scite.Open(file)
end

local function on_switch ()
    set_paths()
	if header_ext[ext] or source_ext[ext] then -- is a C/C++ file
		tt:show()
	else
		tt:hide()
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
--]]
