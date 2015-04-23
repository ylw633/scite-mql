--[[
Eng: Creating SVN commands submenu in tab context menu
Rus: —оздает в контекстном меню таба (вкладки) подменю дл€ команд SVN
Version: 1.3.0
Author: VladVRO, arhohryakov

Using:
Add next line to lua startup file (SciTEStartup.lua):
	dofile ("svn_menu.lua")
]]
--++++++++++++++++++++++++++++++++++++++++++++++++++++++++
require 'shell'

-- SVN menu
local SVNContectMenu =
	"||"..
	"SVN|POPUPBEGIN|"..
	"$(FileMenuCommands)"..
	"$(BranchMenuCommands)"..
	"$(RootMenuCommands)"..
	"SVN|POPUPEND|"
local FileMenuCommands =
	"Update|9181|"..
	"CommitЕ|9182|"..
	"RevertЕ|9183|"..
	"Diff|9184|"..
	"Show log|9186|"
local NewFileMenuCommands =
	"AddЕ|9185|"
local RootMenuCommands =
	"||"..
	"Update All|9190|"..
	"Commit AllЕ|9191|"..
	"Show log for All|9192|"..
	"Check for modifications for All|9193|"
local BranchMenuCommands =
	"||"..
	"Update 'trunk'|9187|"..
	"Commit 'trunk'Е|9188|"..
	"Show log for 'trunk'|9189|"

local function update_svn_menu()
	local menu = props["user.tabcontext.menu.*"]
	local filedir = props["FileDir"]
	local svnroot = ""
	local svnbranch = ""
	local isSVN = false
	local svnSign
	-- test SVN context
  repeat
		if shell.fileexists(filedir.."\\.svn") then
			isSVN = true
			svnSign = "."
			break
		elseif shell.fileexists(filedir.."\\_svn") then
			isSVN = true
			svnSign = "_"
			break
		end
		-- move to parent folder
		filedir = string.match(filedir, "(.*)\\")
  until (filedir == nil)
	
	if isSVN then
		-- file in SVN context
		svnroot = filedir
		local filemenu = NewFileMenuCommands.."||"..FileMenuCommands
		local branchmenu = ""
		local child = ""
		-- find SVN branch/trunk and root
		repeat
			local _,_,parent,name = string.find(svnroot, "(.*)\\([^\\]+)")
			if name == "trunk" then
				svnbranch = svnroot
				branchmenu = BranchMenuCommands
			elseif name == "branches" then
				svnbranch = child
				local _,_,branchname = string.find(svnbranch, ".*\\([^\\]+)")
				branchmenu = string.gsub(BranchMenuCommands, "trunk", branchname)
			end
			if parent then
				if shell.fileexists(parent.."\\"..svnSign.."svn") then
					child = svnroot
					svnroot = parent
				else
					break
				end
			end
		until not parent
		-- set menu
		if not string.find(menu,"|||SVN|") then
			menu = menu.."||SVN||"
		end
		props["user.tabcontext.menu.*"] =
			string.gsub(menu, "||SVN|.*", 
			string.gsub(string.gsub(string.gsub(SVNContectMenu,
			"$%(FileMenuCommands%)", filemenu),
			"$%(BranchMenuCommands%)", branchmenu),
			"$%(RootMenuCommands%)", RootMenuCommands))
	else
		-- no SVN context
		if string.find(menu,"|||SVN|") then
			props["user.tabcontext.menu.*"] = string.gsub(menu, "||SVN|.*", "")
		end
	end
	-- set variables for SVN menu
	props["SVNRoot"] = svnroot
	props["SVNCurrentBranch"] = svnbranch
end

-- ƒобавл€ем свой обработчик событи€ OnOpen
AddEventHandler("OnOpen", update_svn_menu)

-- ƒобавл€ем свой обработчик событи€ OnSwitchFile
AddEventHandler("OnSwitchFile", update_svn_menu)
