------------------------------------------------------------------------------------------------------
-- ScriptManager.lua																				--
--[[--------------------------------------------------------------------------------------------------
--																									--
-- Description:																						--
--	although the code in this file is commented, it isn't the real documentation for Script Manager	--
--	for Script Manager's documentation, (installation, usage, and more): 'ScriptManagerDoc.html'	--
--																									--
--------------------------------------------------------------------------------------------------]]--


-- local variables for Script Manager ----------------------------------------------------------------
local cmdmax = 10									-- the command number (leave the first 10 alone)
local commands = {}									-- holds all commands properties used
local events = {}									-- unlike extman, one list contains everything
local keys = {alt = 0, ctrl = 0, shift = 0}			-- this is used to determine control keys
local lexers = {									-- table containing all of SciTE's (v1.75) lexers
	"null", "python", "cpp", "hypertext", "xml", "perl", "sql", "vb", "props", "errorlist",
	"makefile", "batch", "xcode", "latex", "lua", "diff", "conf", "pascal", "ave", "ada", "lisp",
	"ruby", "eiffel", "eiffelkw", "tcl", "nncrontab", "bullant", "vbscript", "asp", "php", "baan",
	"matlab", "scriptol", "asm", "cppnocase", "fortran", "f77", "css", "pov", "lout", "escript", "ps",
	"nsis", "mmixal", "clarion", "clarionnocase", "lot", "yaml", "tex", "metapost", "powerbasic",
	"forth", "erlang", "octave", "mssql", "verilog", "kix", "gui4 cli", "specman", "au3", "apdl",
	"bash", "asn1", "vhdl", "caml", "blitzbasic", "purebasic", "haskell", "phpscript", "tads3",
	"rebol", "smalltalk", "flagship", "csound", "freebasic", "inno", "opal", "spice", "d", "cmake",
	"gap", "PL/M", "progress", "abaqus", "asy", "r"
}
local loadscript = {}								-- determines if a script was already loaded
local menucmd = {									-- holds all the SciTE menu commands (MenuCommand)
	["New"] = IDM_NEW, ["Open"] = IDM_OPEN, ["Open..."] = IDM_OPEN,
	["Open Selected Filename"] = IDM_OPENSELECTED, ["Revert"] = IDM_REVERT, ["Close"] = IDM_CLOSE,
	["Save"] = IDM_SAVE, ["Save As"] = IDM_SAVEAS, ["Save As..."] = IDM_SAVEAS,
	["Save a Copy"] = IDM_SAVEACOPY, ["Save a Copy..."] = IDM_SAVEACOPY,
	["Code Page Proper"] = IDM_ENCODING_DEFAULT, ["UCS-2 Big Endian"] = IDM_ENCODING_UCS2BE,
	["UCS-2 Little Endian"] = IDM_ENCODING_UCS2LE, ["UTF-8 with BOM"] = IDM_ENCODING_UTF8,
	["UTF-8"] = IDM_ENCODING_UCOOKIE, ["As HTML"] = IDM_SAVEASHTML,
	["As HTML..."] = IDM_SAVEASHTML, ["As RTF"] = IDM_SAVEASRTF, ["As RTF..."] = IDM_SAVEASRTF,
	["As PDF"] = IDM_SAVEASPDF, ["As PDF..."] = IDM_SAVEASPDF, ["As LaTeX"] = IDM_SAVEASTEX,
	["As LaTeX..."] = IDM_SAVEASTEX, ["As XML"] = IDM_SAVEASXML, ["As XML..."] = IDM_SAVEASXML,
	["Page Setup"] = IDM_PRINTSETUP, ["Page Setup..."] = IDM_PRINTSETUP,
	["Print"] = IDM_PRINT, ["Print..."] = IDM_PRINT, ["Load Session"] = IDM_LOADSESSION,
	["Load Session..."] = IDM_LOADSESSION, ["Save Session"] = IDM_SAVESESSION,
	["Save Session..."] = IDM_SAVESESSION, ["Exit"] = IDM_QUIT, ["Undo"] = IDM_UNDO,
	["Redo"] = IDM_REDO, ["Cut"] = IDM_CUT, ["Copy"] = IDM_COPY, ["Paste"] = IDM_PASTE,
	["Duplicate"] = IDM_DUPLICATE, ["Delete"] = IDM_CLEAR, ["Select All"] = IDM_SELECTALL,
	["Copy as RTF"] = IDM_COPYASRTF, ["Match Brace"] = IDM_MATCHBRACE,
	["Select to Brace"] = IDM_SELECTTOBRACE, ["Show Calltip"] = IDM_SHOWCALLTIP,
	["Complete Symbol"] = IDM_COMPLETE, ["Complete Word"] = IDM_COMPLETEWORD,
	["Expand Abbreviation"] = IDM_ABBREV, ["Insert Abbreviation"] = IDM_INS_ABBREV,
	["Block Comment or Uncomment"] = IDM_BLOCK_COMMENT, ["Box Comment"] = IDM_BOX_COMMENT,
	["Stream Comment"] = IDM_STREAM_COMMENT, ["Make Selection Uppercase"] = IDM_UPRCASE,
	["Make Selection Lowercase"] = IDM_LWRCASE, ["Join"] = IDM_JOIN, ["Split"] = IDM_SPLIT,
	["Find"] = IDM_FIND, ["Find..."] = IDM_FIND, ["Find Next"] = IDM_FINDNEXT,
	["Find Previous"] = IDM_FINDNEXTBACK, ["Find in Files"] = IDM_FINDINFILES,
	["Find in Files..."] = IDM_FINDINFILES, ["Replace"] = IDM_REPLACE, ["Replace..."] = IDM_REPLACE,
	["Incremental Search"] = IDM_INCSEARCH, ["Incremental Search..."] = IDM_INCSEARCH,
	["Go to"] = IDM_GOTO, ["Go to..."] = IDM_GOTO, ["Next Bookmark"] = IDM_BOOKMARK_NEXT,
	["Previous Bookmark"] = IDM_BOOKMARK_PREV, ["Toggle Bookmark"] = IDM_BOOKMARK_TOGGLE,
	["Clear All Bookmarks"] = IDM_BOOKMARK_CLEARALL, ["Toggle current fold"] = IDM_EXPAND,
	["Toggle all folds"] = IDM_TOGGLE_FOLDALL, ["Full Screen"] = IDM_FULLSCREEN,
	["Tool Bar"] = IDM_VIEWTOOLBAR, ["Tab Bar"] = IDM_VIEWTABBAR, ["Status Bar"] = IDM_VIEWSTATUSBAR,
	["Whitespace"] = IDM_VIEWSPACE, ["End of Line"] = IDM_VIEWEOL,
	["Indentation Guides"] = IDM_VIEWGUIDES, ["Line Numbers"] = IDM_LINENUMBERMARGIN,
	["Margin"] = IDM_SELMARGIN, ["Fold Margin"] = IDM_FOLDMARGIN, ["Output"] = IDM_TOGGLEOUTPUT,
	["Parameters"] = IDM_TOGGLEPARAMETERS, ["Compile"] = IDM_COMPILE, ["Build"] = IDM_BUILD,
	["Go"] = IDM_GO, ["Stop Executing"] = IDM_STOPEXECUTE, ["Next Message"] = IDM_NEXTMSG,
	["Previous Message"] = IDM_PREVMSG, ["Clear Output"] = IDM_CLEAROUTPUT,
	["Switch Pane"] = IDM_SWITCHPANE, ["Always On Top"] = IDM_ONTOP,
	["Open Files Here"] = IDM_OPENFILESHERE, ["Vertical Split"] = IDM_SPLITVERTICAL,
	["Wrap"] = IDM_WRAP, ["Wrap Output"] = IDM_WRAPOUTPUT, ["Read-Only"] = IDM_READONLY,
	["CR + LF"] = IDM_EOL_CRLF, ["CR"] = IDM_EOL_CR, ["LF"] = IDM_EOL_LF,
	["Convert Line End Characters"] = IDM_EOL_CONVERT, ["Change Indentation Settings"] = IDM_TABSIZE,
	["Change Indentation Settings..."] = IDM_TABSIZE, ["Use Monospaced Font"] = IDM_MONOFONT,
	["Open Local Options File"] = IDM_OPENLOCALPROPERTIES,
	["Open Directory Options File"] = IDM_OPENDIRECTORYPROPERTIES,
	["Open User Options File"] = IDM_OPENUSERPROPERTIES,
	["Open Global Options File"] = IDM_OPENGLOBALPROPERTIES,
	["Open Abbreviations File"] = IDM_OPENABBREVPROPERTIES,
	["Open Lua Startup Script"] = IDM_OPENLUAEXTERNALFILE, ["Previous"] = IDM_PREVFILE,
	["Next"] = IDM_NEXTFILE, ["Close All"] = IDM_CLOSEALL, ["Save All"] = IDM_SAVEALL,
	["Help"] = IDM_HELP, ["Sc1 Help"] = IDM_HELP_SCITE, ["About Sc1"] = IDM_ABOUT,
	["SciTE Help"] = IDM_HELP_SCITE, ["About SciTE"] = IDM_ABOUT
}
local ostime = {os.clock(), 0}						-- this is used to determine double clicks
local shortcuts = {}								-- holds all the Tools menu shortcuts used
local userlist = {0, 0}								-- holds all registered user list types (3 - ??)
------------------------------------------------------------------------------------------------------


-- the main functions that run Script Manager --------------------------------------------------------
local function AddEvent(event, func, remove, priority, ...)
	local data = {}									-- the data table we will insert
	if func == 'once' then							-- this is extman's method for removing
		func = remove								-- put the function in the right spot
		remove = 1									-- extman doesn't enable more than one
	end

	if not priority then							-- priority doesn't exist, set it to 5
		priority = 5
	elseif tonumber(priority) < 1 or tonumber(priority) > 10 then
		priority = 5								-- see if it's out of bounds
	end

	if not remove or remove < -1 then				-- we want it to run forever
		remove = -1
	elseif remove == 0 then							-- stupid, we can't do anything now
		return										-- exit out
	end

	if type(remove) == 'string' then				-- fix the type of remove
		remove = tostring(remove)
	end

	if type(priority) == 'number' then				-- fix the type of priority
		priority = tostring(priority)
	end

	if arg.n > 0 then								-- see if there were extra parameters
		arg.n = nil									-- erase the argument count
		data = {func, remove, unpack(arg)}			-- unpack the args into the table for later
	else
		data = {func, remove}						-- just make a standard table
	end

	if not events[event] then						-- see if an ? event was registered
		events[event] = {}							-- make a new table
	end

	if not events[event][priority] then				-- create another table if needed
		events[event][priority] = {}
	end

	local defined = false							-- it wasn't defined yet
	for _, prev in pairs(events[event][priority]) do-- find if the command was set already
		for x, old in pairs(prev) do
			if old == data[1] then					-- it was defined already (by function)
				defined = true						-- mark it as so
				break								-- break out of the loop
			end
		end
	end

	if not defined and func then					-- if it wasn't defined yet and it exists
		table.insert(events[event][priority], data)	-- register the event
	end
end
local function DoEvent(event, ...)
	local ret = {}									-- return values

	if events[event] then							-- an event has to be registered first
		for p, priority in pairs(events[event]) do	-- this sorts the priority
			for d, data in pairs(priority) do		-- search the data to get the function
				local func = data[1]				-- now we have the function
				local remove = data[2]				-- now we have the remove parameter
				local msg = nil						-- the return message

				if data[2] == 0 then				-- if the remove == 0 then remove it
					priority[d] = nil				-- erase the data
					table.remove(priority, d)		-- delete the entry altogether

					if # priority == 0 then			-- get rid of this table if it's empty
						table.remove(priority)
					end

					if # events[event] == 0 then	-- erase this too if it's empty
						table.remove(events[event])	-- remove the table
					end
				elseif data[2] ~= -1 then			-- if the value isn't -1 then subtract
					data[2] = data[2] - 1			-- subtract the counter
				end

				if data[3] == 'extman' then			-- that means scite_OnWord was called format it
					_, msg = pcall(func, {ch = arg[4], word = arg[1], startp = arg[2], endp = arg[3]})
				else
					if arg.n and arg.n > 0 then		-- if there are arguments, send them
						_, msg = pcall(func, unpack(arg))
					else
						_, msg = pcall(func)
					end
				end

				if not _ then						-- if there was an error then show it
					print(msg)						-- but continue on with everything else
				end

				if msg then							-- if there was a message
					table.insert(ret, msg)			-- save the message
				end
			end
		end

		if ret then									-- see if there are return values
			return unpack(ret)						-- return them if so
		else
			return false							-- return false, don't affect SciTE
		end
	end
end
local function LoadScripts()
	path = GetManagerPath()							-- get the manager's path
	scripts = GetFiles(path, '*.lua')				-- get the lua files

	for _, script in pairs(scripts) do				-- _ is there because it's not needed
		if FileExists(script) then					-- check if it exists
			RunScript(script)						-- safely run the script
		end
	end
end
function RefreshManager()
	for _, name in pairs(commands) do
		local numpat = '.'..name[4]..name[2]		-- get the way it was defined

		props["command.name"..numpat] = nil			-- remove the command properties from the menu
		props["command"..numpat] = nil				-- with this addition, nothing will leak over
		props["command.subsystem"..numpat] = nil	-- after SciTE has been reset (after this runs)
		props["command.mode"..numpat] = nil
		props["command.shortcut"..numpat] = nil
	end

	props["ext.lua.reset"] = 1						-- SciTE resets when the function is run
end
function RunCommand(name, func, ...)
	local exist = "attempt to call a nil value"		-- the message if it doesn't exist
	local ret, msg = pcall(func, unpack(arg))		-- run the function safely

	if not ret and msg == exist then				-- there was an error (it doesn't exist)
		local numpat = '.'..commands[name][4]..commands[name][2]
		local shortcut = props["command.shortcut"..numpat]

		menucmd[name] = nil							-- remove the menu command value

		props["command.name"..numpat] = nil			-- remove the command properties
		props["command"..numpat] = nil
		props["command.subsystem"..numpat] = nil
		props["command.mode"..numpat] = nil
		props["command.shortcut"..numpat] = nil

		if shortcuts["backup: "..shortcut] then		-- attempt to restore a backup's shortcut
			local backup = shortcuts["backup: "..shortcut]
			local value = backup[1]					-- get the first in line

			table.remove(backup, 1)					-- remove the item

			shortcuts[shortcut] = value
			shortcuts["backup: "..shortcut] = backup-- update the actual table

			local numpat = '.'..commands[value][4]..commands[value][2]
			props["command.shortcut"..numpat] = shortcut
		else
			shortcuts[shortcut] = nil				-- remove the shortcut data
		end

		commands[name] = nil						-- everything is now gone
	end
end
function SetCommand(name, func, ext, shortcut, mode)
	local _ext, _shortcut, _mode = 0, 0, 0			-- set the defaults for if they exist

	if not ext then _ext = 1 end					-- if ext is nil
	if not shortcut then _shortcut = 2 end			-- if shortcut is nil
	if not mode then _mode = 4 end					-- if mode is nil

	local option = _ext + _shortcut + _mode			-- this sets the option for what to do

	local _shortcuts = {							-- all the patterns that indicate a shortcut
		"Alt%+", "Ctrl%+", "Shift%+", "F[0-9]+"
	}

	local _modes = {								-- all the patterns that indicate a mode
		"auto", "filter:", "groupundo:", "no",		-- of course this doesn't have 'subsystem'
		"prompt", "quiet:", "replaceselection:", "savebefore:", "yes"
	}

	if option ~= 0 then								-- if it's 0 then everything is present
		if option == 1 then
			ext = '*.*'								-- set the default file pattern

		elseif option == 2 then
			shortcut = ''							-- set the default shortcut

		elseif option == 3 then
			ext = '*.*'								-- set the default file pattern
			shortcut = ''							-- set the default shortcut

		elseif option == 4 then
			local find

			for x = 1, # _shortcuts do				-- check if ext should be shortcut
				if find ~= nil then break end
				find = string.find(ext, _shortcuts[x])
			end

			if not find then						-- if no shortcut was found (ext == ext)
				for x = 1, # _modes do				-- check if shortcut should be mode
					if find ~= nil then break end
					find = string.find(shortcut, _modes[x])
				end

				if not find then					-- if no mode was found set the default mode
					mode = 'savebefore:no'			-- set the default mode
				else								-- a mode was found
					mode = shortcut					-- set the mode
					shortcut = ''					-- set the default shortcut
				end
			else									-- a shortcut was found
				mode = shortcut						-- shortcut has to be mode, so set it
				shortcut = ext						-- set the shortcut
				ext = '*.*'							-- set the default file pattern
			end

		elseif option == 5 then
			ext = '*.*'								-- set the default file pattern
			mode = 'savebefore:no'					-- set the default mode

		elseif option == 6 then
			local find

			for x = 1, # _shortcuts do				-- check if ext should be shortcut
				if find ~= nil then break end
				find = string.find(ext, _shortcuts[x])
			end

			if not find then						-- if no shortcut was found
				for x = 1, # _modes do				-- check if ext should be mode
					if find ~= nil then break end
					find = string.find(ext, _modes[x])
				end

				if not find then					-- if no mode was found (set the defaults)
					shortcut = ''
					mode = 'savebefore:no'			-- set the default mode
				else								-- a mode was found
					mode = ext						-- set the mode
					ext = '*.*'						-- set the default file pattern

					if not shortcut then			-- if shortcut doesn't exist, set it to default
						shortcut = ''				-- set the default shortcut
					end
				end
			else									-- a shortcut was found
				shortcut = ext						-- set the shortcut
				ext = '*.*'							-- set the default file pattern

				if not mode then					-- if mode doesn't exist, set it to default
					mode = 'savebefore:no'			-- set the default mode
				end
			end

		elseif option == 7 then
			ext = '*.*'								-- set the default file pattern
			shortcut = ''							-- set the default shortcut
			mode = 'savebefore:no'					-- set the default mode
		end
	end

	if ext == '*.*' then							-- format file pattern so it works
		ext = '.*'
	else
		ext = '.'..ext
	end

	local defined = false							-- set it as false as a default
	for x = 10, cmdmax do							-- determine if the command was defined
		if commands[name] ~= nil then
			defined = true							-- the command was already defined
		end
	end

	local cmdnum = cmdmax							-- this is the default to use

	local delete = {}								-- used to find anything deleted
	for _, x in pairs(commands) do					-- look for anything deleted
		table.insert(delete, x[4])					-- grab all the command values defined
	end
	local holes = table.concat(delete, '|')			-- setup a string '##|##|...'

	for x = 10, cmdmax do							-- go through all the values
		if not string.find(holes, tostring(x)) then
			cmdnum = x								-- set the command number to the hole
			break									-- we're done
		end
	end

	if not defined then								-- if the function wasn't defined yet
		local multi = string.find(ext, ';')			-- see if more than one file pattern is used
		local global = string.find(ext, '.*')		-- this is used in every file pattern

		if multi and global then					-- maybe someone messed up
			ext = '.*'								-- make it strictly global
			multi = nil								-- fix multi for later
		end

		local numpat = '.'..cmdnum..ext				-- set the number.filepattern

		menucmd[name] = cmdnum+1100					-- define the menu command value
		commands[name] = {func, ext, mode, cmdnum}	-- define all the properties

		if string.find(func, ' ') and not string.find(func, '- -') then
			func = string.gsub(func, " ", ", ")		-- fixes problems with arguments
		end

		local add = "'"..name.."', "..func			-- combine them so it's shorter

		props["command.name"..numpat] = name
		props["command"..numpat] = 'dostring dostring("pcall(RunCommand, '..add..')")'
		props["command.subsystem"..numpat] = '3'	-- obviously we have to set it as lua
		props["command.mode"..numpat] = mode

		if cmdnum == cmdmax then					-- as long as no hole was filled
			cmdmax = cmdmax + 1						-- increase the number for a new function
		end

		if shortcut ~= '' then						-- see if a shortcut was defined
			if not shortcuts[shortcut] then			-- the shortcut wasn't defined yet
				props["command.shortcut"..numpat] = shortcut
				shortcuts[shortcut] = name			-- define it for later
			else									-- this shortcut was defined
				local backup = "backup: "..shortcut	-- define a backup table

				if not shortcuts[backup] then		-- there is not a list yet
					shortcuts[backup] = {name}		-- define it
				else								-- just append it to the end
					table.insert(shortcuts[backup], name)
				end
			end
		end
	end
end
------------------------------------------------------------------------------------------------------


-- SciTE / extman / Script Manager event registerers -------------------------------------------------
function BeforeSaveEvent(func, remove, priority)
	AddEvent("beforesave", func, remove, priority)
end
function CharEvent(func, remove, priority)
	AddEvent("char", func, remove, priority)
end
function ClearEvent(func, remove, priority)
	AddEvent("clear", func, remove, priority)
end
function CloseEvent(func, remove, priority)
	AddEvent("close", func, remove, priority)
end
function DoubleClickEvent(func, remove, priority)
	AddEvent("doubleclick", func, remove, priority)
end
function DwellEndEvent(func, remove, priority)
	AddEvent("dwellend", func, remove, priority)
end
function DwellEvent(func, remove, priority)
	AddEvent("dwellstart", func, remove, priority)
	AddEvent("dwellend", func, remove, priority)
end
function DwellStartEvent(func, remove, priority)
	AddEvent("dwellstart", func, remove, priority)
end
function EditorLineEvent(func, remove, priority)
	AddEvent("editorline", func, remove, priority)
end
function KeyEvent(func, remove, priority)
	AddEvent("key", func, remove, priority)
end
function MarginEvent(func, remove, priority)
	AddEvent("marginclick", func, remove, priority)
	AddEvent("margindoubleclick", func, remove, priority)
end
function MarginClickEvent(func, remove, priority)
	AddEvent("marginclick", func, remove, priority)
end
function MarginDoubleClickEvent(func, remove, priority)
	AddEvent("margindoubleclick", func, remove, priority)
end
function OpenEvent(func, remove, priority)
	AddEvent("open", func, remove, priority)
end
function OpenSwitchEvent(func, remove, priority)
	AddEvent("open", func, remove, priority)
	AddEvent("switchfile", func, remove, priority)
end
function OutputLineEvent(func, remove, priority)
	AddEvent("outputline", func, remove, priority)
end
function SaveEvent(func, remove, priority)
	AddEvent("save", func, remove, priority)
end
function SavePointLeftEvent(func, remove, priority)
	AddEvent("savepointleft", func, remove, priority)
end
function SavePointReachedEvent(func, remove, priority)
	AddEvent("savepointreached", func, remove, priority)
end
function StartUpEvent(func, remove, priority)
	AddEvent("startup", func, remove, priority)
end
function SwitchFileEvent(func, remove, priority)
	AddEvent("switchfile", func, remove, priority)
end
function UpdateUIEvent(func, remove, priority)
	AddEvent("updateui", func, remove, priority)
end
function WordEvent(func, remove, priority)
	AddEvent("word", func, remove, priority)
end
------------------------------------------------------------------------------------------------------


-- SciTE / extman / Script Manager event handlers ----------------------------------------------------
function OnBeforeSave(file)
	return DoEvent("beforesave", file)
end
function OnChar(char)
	if char == '\n' then
		if editor.Focus == true then				-- this is an OnEditorLine
			local pos = editor.CurrentPos			-- this is extman's code
			local row = editor:LineFromPosition(pos)-1
			local line = string.sub(editor:GetLine(row), 1, -3)

			return OnEditorLine(line)				-- call the event handler
		elseif output.Focus == true then			-- this just looks better
			local pos = output.CurrentPos			-- this is extman's code
			local row = output:LineFromPosition(pos)-1
			local line = string.sub(output:GetLine(row), 1, -3)

			return OnOutputLine(line) 				-- call the event handler
		end
	elseif string.find(char, '[%c%p%s]') then		-- if it was a space or punctuation character
		local pos = editor.CurrentPos-1				-- this was the character we entered
		local word = true							-- if it's a word character this is true
		local x, y = pos, pos						-- defines the start and end of the word
		local spring = editor:textrange(x,y+1)		-- the character that sprung this

		while word ~= nil do						-- as long as it's a word
			x = x - 1								-- goto the previous letter

			local char = editor.CharAt[x]			-- get the character

			if char < 0 then						-- it's a special character
				char = char + 256					-- for some reason it is 256 less than
			end										-- it's normal value

			char = string.char(char)
			word = string.find(char, '[^%c%p%s]')	-- test if it's a word character
		end

		local text = editor:textrange(x+1,y+1)

		if text ~= spring then						-- if the text wasn't the character
			text = string.sub(text, 1, -2)			-- get rid of the last characeter (spring)

			return OnWord(text, x+1, y, spring)		-- run the OnWord event handler
		end
	end

	return DoEvent("char", char)					-- we will still call OnChar
end
function OnClear()
	return DoEvent("clear")
end
function OnClose(file)
	return DoEvent("close", file)
end
function OnDoubleClick()
	return DoEvent("doubleclick")
end
function OnDwellEnd()
	return DoEvent("dwellend")
end
function OnDwellStart(pos, word)
	if word == '' then								-- this is an OnDwellEnd
		return OnDwellEnd()							-- call the event handler
	else
		return DoEvent("dwellstart", pos, word)
	end
end
function OnEditorLine(line)
	return DoEvent("editorline", line)
end
function OnKey(key, shift, ctrl, alt)
	if alt == true then keys.alt = os.clock() end	-- record the single special key presses
	if ctrl == true then keys.ctrl = os.clock() end
	if shift == true then keys.shift = os.clock() end

	return DoEvent("key", key, shift, ctrl, alt)
end
function OnMarginClick()
	local time = os.clock()							-- get the current time

	ostime[0] = ostime[1]							-- move the last entry back
	table.insert(ostime, 1, time)					-- add the time to the table

	old = ostime[0]									-- size was formed before the addition
	new = ostime[1]									-- get the new time too

	if new-old < 0.5 then							-- were the clicks less than a half second apart
		return OnMarginDoubleClick()				-- call the event handler
	else
		return DoEvent("marginclick")
	end
end
function OnMarginDoubleClick()
	return DoEvent("margindoubleclick")
end
function OnOpen(file)
	return DoEvent("open", file)
end
function OnOutputLine(line)
	return DoEvent("outputline", line)
end
function OnSave(file)
	return DoEvent("save", file)
end
function OnSavePointLeft()
	return DoEvent("savepointleft")
end
function OnSavePointReached()
	return DoEvent("savepointreached")
end
function OnStartUp()
	if os.clock() < 1 then							-- SciTE just started
		return DoEvent("startup")					-- call the handler
	end
end
function OnSwitchFile(file)
	return DoEvent("switchfile", file)
end
function OnUpdateUI()
	return DoEvent("updateui")
end
function OnUserListSelection(style, item)
	_, msg = pcall(userlist[style], item)			-- run the function in protected mode

	if not _ then									-- still display the error
		print(msg)
	end
end
function OnWord(word, posx, posy, char)
	return DoEvent("word", word, posx, posy, char)
end
------------------------------------------------------------------------------------------------------


-- utility functions, that make programming quicker and easier ---------------------------------------
function AltKeyDown()
	return os.clock() - keys.alt < 0.075			-- this determines the alt key is held down
end
function AppendFile(file, text)
	file = string.gsub(file, '/', '\\')				-- we're in windows, fix it

	if FileExists(file) then						-- see if it exists first
		local handle = io.open(file, "a")			-- get the file in append mode
		handle:write(text or '')					-- default just in case
		handle:flush()

		handle:close()								-- close the file
		return true									-- return success
	else
		return false								-- it doesn't exist
	end
end
function CtrlKeyDown()
	return os.clock() - keys.ctrl < 0.075			-- this determines the ctrl key is held down
end
function CurrentFile()
	return props["FilePath"]						-- return the file path
end
function FileExists(file)
	local handle = io.open(file)					-- open the file

	if not handle then								-- the file doesn't exist
		return false
	else											-- the file does exist
		handle:close()								-- we have to close it
		return true
	end
end
function GetDirectories(path, exclude)
	local dir = 'dir /ad/b/s "'						-- include subdirectories, exclude file names
	local directories = {}
	local tmpfile = "\\SciTE.scripts"				-- the temporary file that receives the names

	path = string.gsub(path, '/', '\\')				-- we're in windows, fix it

	os.execute(dir..path..'" > '..tmpfile)			-- get all the directories
	local file = io.open(tmpfile)

	if not file then return end						-- we're done it there was an error

	if not exclude then								-- if there was not an exclude
		exclude = '*'								-- input something impossible
	end

	local ptr = 1
	local excludes = {}								-- format the exclude parameter

	if string.find(exclude, ';') then				-- there are multiple excluded paths used
		repeat										-- loop through all of the items
			local pos = string.find(exclude, ';', ptr)

			if pos then
				local add = string.sub(exclude, ptr, pos-1)

				table.insert(excludes, add)			-- add it to the list
				ptr = pos+1							-- update the pointer
			end
		until not pos

		local add = string.sub(exclude, ptr)
		table.insert(excludes, add)					-- add the last item (after it's fixed)
	else
		table.insert(excludes, exclude)				-- add it to the table
	end

	for line in file:lines() do						-- go through the list
		for x = 1, # excludes do					-- loop through all the file patterns
			local pattern = excludes[x]
			local find = false

			if pattern ~= '*' then					-- this will never be there
				local sub = string.sub(line, string.len(path)+2)
				find = string.find(sub, pattern)	-- find the pattern
			end

			if not find then
				table.insert(directories, line)		-- add it to the table
			end
		end
	end

	return directories								-- return the directory list
end
function GetFiles(path, mask)
	local dir = 'dir /a-d/b/s "'					-- include subdirectories, exclude directory names
	local files = {}

	path = string.gsub(path, '/', '\\')				-- we're in windows, fix it
	local tmpfile = path.."SciTE.scripts"				-- the temporary file that receives the names

	if not mask and string.find(path, '*') then		-- * can't be in the name, assume it's scite_Files
		local ast = string.find(path, '*')

		mask = string.sub(path, ast)				-- set the mask
		path = string.sub(path, 1, ast-2)			-- set the path (get rid of the end '\')
	end

	os.execute(dir..path..'" > '..tmpfile)			-- get all the files
	local file = io.open(tmpfile)

	if not file then return end						-- we're done it there was an error

	if not mask then								-- no mask was given, find all
		mask = '*.*'
	end

	local ptr = 1
	local masks = {}								-- format the mask parameter

	if string.find(mask, ';') then					-- there are multiple files used
		repeat										-- loop through all of the items
			local pos = string.find(mask, ';', ptr)	-- look for the separator

			if pos then
				local add = string.sub(mask, ptr, pos-1)
				local add = string.gsub(add, '*', '', 1)

				table.insert(masks, add)			-- add it to the list
				ptr = pos+1							-- update the pointer
			end
		until not pos

		local add = string.gsub(string.sub(mask, ptr), '*', '', 1)
		table.insert(masks, add)					-- add the last item (after it's fixed)
	else
		local add = string.gsub(mask, '*', '', 1)	-- fix the way it's written
		table.insert(masks, add)					-- add it to the table
	end

	for line in file:lines() do						-- go through the list
		for x = 1, # masks do						-- loop through all the file patterns
			local pattern = '%'..masks[x]..'$'		-- this makes sure there's no glitches
			local find = true						-- fixes it if the pattern is '*.*'

			if pattern ~= '%.*$' then				-- we're looking for everything ignore filter
				find = string.find(line, pattern)	-- find the pattern
			end

			if find then
				table.insert(files, line)			-- add it to the table
			end
		end
	end

	return files									-- return the file list
end
function GetLexerName()
	local lex = editor.Lexer						-- this is the current lexer

	if not lex or lex == 0 then						-- if it's nil
		lex = '*'									-- return it as global
		return lex									-- return this value
	end

	return lexers[lex]								-- return the lexer name
end
function GetManagerPath()
	local default = props["ext.lua.startup.script"]	-- the Script Manager's default path
	default = string.gsub(default, '\\[%w]+[.][%w]+$', '')

	return GetProp("ext.lua.script.directory", default)
end
function GetProp(prop, default)
	local val = props[prop]							-- get the property

	if not default then								-- default wasn't defined
		default = ''								-- return a default for default :)
	end

	if val and val ~= '' then
		return val									-- it exists, return it
	else
		return default								-- it doesn't exist, return default
	end
end
function MenuCommand(command)
	local idm = nil									-- used to do the menu command

	if type(command) == "string" then				-- it might be the actual name
		idm = menucmd[command]						-- load the constant

		if string.find(command, "buffer%[[0-9]+%]") then
			local len = string.len(command)			-- ignore the '-1' the first buffer is 1
			idm = 1200 + string.sub(command, 8, len-1) - 1
		end

		if string.find(command, "language:[ ]*.+") then
			local menu = props["menu.language"]		-- get the menu languages
			local lang = string.sub(command, 10)	-- this gets rid of 'language:'

			menu = string.gsub(menu, '&', '')		-- this would make things difficult
			lang = string.gsub(lang, '^[ ]+', '')	-- eliminate preceding spaces

			local pos = string.find(menu, lang)		-- get the language position
			local prev = string.sub(menu, 1, pos-1)	-- get all the text before pos
			local _, x = string.gsub(prev, '|', '')	-- just count how many times it was replaced

			idm = 1400 + ((x/3)+1) - 1				-- ignore the math

			if lang == '' then						-- fixes a glitch that selects the first item
				idm = nil							-- there's nothing to do
			end
		end
	else											-- the command is assumed as an IDM_ constant
		idm = command								-- just switch it over
	end

	if idm then										-- if one exists then
		scite.MenuCommand(idm)						-- do the menu command
	end
end
function ReadFile(file)
	file = string.gsub(file, '/', '\\')				-- we're in windows, fix it

	if FileExists(file) then						-- see if it exists first
		local handle = io.open(file)				-- get the file
		local text = handle:read("*all")			-- get all the text

		handle:close()								-- close the file
		return text									-- return the text
	else
		return false								-- it doesn't exist
	end
end
function RequireScript(script)
	if not string.find(script, '[/\\]')	then		-- we assume it's in the Script Manager directory
		manager = GetManagerPath()					-- get the manager's path
		script = manager..'\\'..script				-- combine the path
	else
		script = string.gsub(script, '/', '\\')		-- we're in windows, fix it
	end

	if not loadscript[string.lower(script)] then	-- if it wasn't loaded yet
		RunScript(script)							-- safely run the script
	end
end
function RunScript(script)
	local handler = {								-- contains all the default SciTE event handlers
		"OnBeforeSave", "OnChar", "OnClear", "OnClose", "OnDoubleClick", "OnDwellStart",
		"OnKey", "OnMarginClick", "OnOpen", "OnSave", "OnSavePointLeft",
		"OnSavePointReached", "OnSwitchFile", "OnUpdateUI","OnUserListSelection"
	}

	if not string.find(script, '[/\\]')	then		-- we assume it's in the Script Manager directory
		manager = GetManagerPath()					-- get the manager's path
		script = manager..'\\'..script				-- combine the path
	else
		script = string.gsub(script, '/', '\\')		-- we're in windows, fix it
	end

	if FileExists(script) then						-- check to see if it exists
		if string.find(script, "ScriptManager.lua$") == nil then
			if string.find(script, "extman.lua$") == nil then
				local find
				local text = ReadFile(script)		-- read the text of the file

				for x = 1, # handler do				-- go through every handler
					if find ~= nil then break end	-- something was found so we won't run it
					if not text then break end		-- don't continue if the file is empty

					local word
					local search = '[ ]+=[ ]+[%w]+'
					local subs, sube = string.find(text, handler[x]..search)

					if subs then					-- cut everything out so we see the substitute
						subs = subs + string.len(handler[x])
						word = string.sub(text, subs, sube)
						word = string.gsub(word, '[ ]+=[ ]+', '')
					end

					if not word then				-- if no substitution was found search normal
						find = string.find(text, "function "..handler[x])
					else							-- if there was search for that 'handler'
						find = string.find(text, "function "..word)
					end
				end

				if not find and not loadscript[string.lower(script)] then
					loadscript[script] = true

					local _, msg = pcall(function() dofile(script) end)

					if not _ then					-- if there is an error in a file when it's run
						print(msg)					-- then Refresh Manager won't appear, so run the
					end								-- script in protected mode and show the message

					return true						-- return success
				end
			end
		end
	else
		return nil									-- return that it doesn't exist
	end

	return false									-- return that it wasn't loaded
end
function ShiftKeyDown()
	return os.clock() - keys.shift < 0.075			-- this determines the shift key is held down
end
function UserListShow(data, func, first, last, split)
	if type(first) == 'function' then		-- fix it if it's from extman
		func, first = first, func			-- switch them
	end

	if data and func then					-- we need a data and func parameter
		local pane = editor					-- determine where to show it
		local id = # userlist + 1			-- this is the new id we will register

		userlist[id] = func					-- define the function

		if output.Focus == true then		-- set the pane to output if needed
			pane = output
		end

		if type(data) == 'string' then		-- we'll assume that it's correct
			pane:UserListShow(id, data)
		elseif type(data) == 'table' then	-- we will format it for you
			local list = ''					-- the list that will be displayed

			if not first or first < 1 or first == -1 then
				first = 1					-- set first to default
			end

			if not last or last > # data or last == -1 then
				last = # data				-- set last to default
			end

			if not split or string.find(split, "%w") then
				split = ';'					-- set the default split
			end

			for x = first, last do			-- loop through the entire list
				list = list..data[x]..split	-- setup the list (this is different from extman)
			end								-- (but with the next line, it's the same result
			list = string.sub(list, 1, string.len(list) - 1)

			pane.AutoCSeparator = string.byte(split)
			pane:UserListShow(id, list)		-- display the list
			pane.AutoCSeparator = string.byte(' ')
		end
	end
end
function WriteFile(file, text)
	file = string.gsub(file, '/', '\\')				-- we're in windows, fix it

	if FileExists(file) then						-- see if it exists first
		local handle = io.open(file, "w")			-- get the file in write mode
		handle:write(text or '')					-- default just in case
		handle:flush()

		handle:close()								-- close the file
		return true									-- return success
	else
		return false								-- it doesn't exist
	end
end
------------------------------------------------------------------------------------------------------


-- links of extman functions to Script Manager (enables compatibility) -------------------------------
function scite_Command(data)
	if type(data) == 'string' then					-- we don't want a string
		data = {data}								-- put it in a table for now
	end

	for _, cmd in pairs(data) do					-- loop through all the commands
		local ptr, x = 1							-- just ptr is defined
		local params = {}							-- the table that will hold the parameters

		cmd, x = string.gsub(cmd, '[ ]*|[ ]*', '\n')-- replace the separators with enters
		cmd = cmd..'\n'								-- this makes searching later easier

		for y = 1, x+1 do							-- get all the parameters
			local pos = string.find(cmd, '\n', ptr)

			if pos then								-- as long as we found something
				params[y] = string.sub(cmd, ptr, pos - 1)
				ptr = pos + 1
			end
		end

		SetCommand(unpack(params))
	end
end
scite_CurrentFile = CurrentFile
scite_Directories = GetDirectories
scite_dofile = RunScript
scite_FileExists = FileExists
scite_Files = GetFiles
scite_GetProp = GetProp
scite_OnBeforeSave = BeforeSaveEvent
scite_OnChar = CharEvent
scite_OnClear = ClearEvent
scite_OnClose = CloseEvent
scite_OnDoubleClick = DoubleClickEvent
scite_OnEditorLine = EditorLineEvent
scite_OnDwellStart = DwellStartEvent
scite_OnKey = KeyEvent
scite_OnMarginClick = MarginClickEvent
scite_MenuCommand = MenuCommand
scite_OnOpen = OpenEvent
scite_OnOpenSwitch = OpenSwitchEvent
scite_OnOutputLine = OutputLineEvent
scite_OnSave = SaveEvent
scite_OnSavePointLeft = SavePointLeftEvent
scite_OnSavePointReached = SavePointReachedEvent
scite_OnSwitchFile = SwitchFileEvent
scite_OnUpdateUI = UpdateUIEvent
scite_require = RequireScript
scite_UserListShow = UserListShow
function scite_OnWord(func, remove)
	AddEvent("word", func, remove, nil, "extman")
end
------------------------------------------------------------------------------------------------------


-- Script Manager's startup routine (runs on opening) ------------------------------------------------
LoadScripts()
props["ext.lua.reset"] = 0							-- this will help in avoiding problems
if not props["dwell.period"] then					-- this property isn't defined in SciTEDoc.html
	props["dwell.period"] = 400						-- we need to define it for OnDwellStart and etc
end
SetCommand("Refresh Manager", "RefreshManager", "Ctrl+F5")
OnStartUp()											-- call the OnStartUp event
------------------------------------------------------------------------------------------------------
