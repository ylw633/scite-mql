--[[--------------------------------------------------
ROCheck.lua
Authors: Midas, VladVRO
Version: 1.1.1
------------------------------------------------------
Скрипт для автоматической установки R/O режима для файлов
с устанав-ми аттрибутами RHS
------------------------------------------------------
Подключение:
  Добавьте в SciTEStartup.lua строку
    dofile (props["SciteDefaultHome"].."\\tools\\ROCheck.lua")
--]]--------------------------------------------------

local function ROCheck()
	-- Получим аттрибуты файла
	local FileAttr = props['FileAttr']
-- Если среди аттрибутов ReadOnly/Hidden/System, и НЕ установлен режим R/O
	if string.find(FileAttr, "[RHS]") and not editor.ReadOnly then
	-- то установим режим R/O
		scite.MenuCommand(IDM_READONLY)
	end
end

-- Добавляем свой обработчик события OnOpen
AddEventHandler("OnOpen", ROCheck)
