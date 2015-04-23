--[[--------------------------------------------------
LuaInspectInstall.lua
Authors: mozers™, Tymur Gubayev
Version: 1.4.0
------------------------------------------------------
Служит для подключения LuaInspect <http://lua-users.org/wiki/LuaInspect>
Срабатывает при наличии параметра luainspect.path в .properties файле
В каталог, указанный этим параметром следует распаковать LuaInspect.
Реально испольуется только содержимое подкаталогов lib и metalualib.
Все остальное можно спокойно удалять.
--]]--------------------------------------------------

function scite_GetProp(key, default)
	local val = props[key]
	if val and val ~= '' then return val
	else return default end
end

----------------------------------------
local name_id_map = {}
function scite_Command(tbl)
	local function get_num(pat, cmd)
		for num = 0, 19 do
			if props['command.name.'..num..'.'..pat] == '' or props['command.'..num..'.'..pat] == cmd
			then return num
			end
		end
	end
	local name, cmd, pattern, shortcut = tbl:match('([^|]*)|([^|]*)|([^|]*)|([^|]*)')
	local num = get_num(pattern, cmd)
	props['command.name.'..num..'.'..pattern] = name
	props['command.'..num..'.'..pattern] = cmd
	props['command.mode.'..num..'.'..pattern] = 'subsystem:lua,savebefore:no'
	props['command.shortcut.'..num..'.'..pattern] = shortcut
	name_id_map[name] = 9000 + num
end

----------------------------------------
function scite_MenuCommand(cmd)
	if type(cmd) == 'string' then
		cmd = name_id_map[cmd]
		if not cmd then return end
	end
	scite.MenuCommand(cmd)
end

----------------------------------------
local old_h
local user_id = 13
local separator = string.char(1)
function scite_UserListShow(list, start, fn)
	local pane = editor
	if not pane.Focus then pane = output end
	pane.AutoCSeparator = string.byte(separator)
	pane:UserListShow(user_id, table.concat(list, separator, start))
	pane.AutoCSeparator = string.byte(' ')
	if old_h then RemoveEventHandler ("OnUserListSelection", old_h) end
	local h = function(tp, sel_value)
		if tp==user_id then
			return fn(sel_value)
		end
	end
	old_h = AddEventHandler("OnUserListSelection", h, "RunOnce")
end

----------------------------------------
function use_Script_Lexer(isUse)
	if isUse then
		props['lexer.*.lua']='script_lua'
		props['command.checked.0.*.lua']=0
		props['command.checked.1.*.lua']=1
	else
		props['lexer.*.lua']='lua'
		props['command.checked.0.*.lua']=1
		props['command.checked.1.*.lua']=0
	end
end

----------------------------------------
local LUAINSPECT_PATH = props["luainspect.path"]
package.path = package.path .. ";" .. LUAINSPECT_PATH .. "\\metalualib\\?.lua"
package.path = package.path .. ";" .. LUAINSPECT_PATH .. "\\lib\\?.lua"
require "luainspect.scite" : install()
