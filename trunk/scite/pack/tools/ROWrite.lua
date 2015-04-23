--[[--------------------------------------------------
ROWrite.lua
Authors: Midas, VladVRO
Version: 1.1.2
------------------------------------------------------
Скрипт для поддержки сохранения RO/Hidden/System файлов
------------------------------------------------------
Подключение:
  Добавьте в SciTEStartup.lua строку
    dofile (props["SciteDefaultHome"].."\\tools\\ROWrite.lua")
--]]--------------------------------------------------

require 'shell'

local function iif (expresion, onTrue, onFalse)
	if (expresion) then return onTrue; else return onFalse; end
end

local function BSave(FN)
	-- Получим аттрибуты файла.
	local FileAttr = props['FileAttr']
	props['FileAttrNumber'] = 0
	if string.find(FileAttr, '[RHS]') then --  Если в файл нельзя записать, то спросим
		if shell.msgbox(scite.GetTranslation("This file is read-only. Save anyway?\nFile attributes:").." "..FileAttr, "SciTE", 65)==1 then
			-- сохраним текущии, затем снимем все аттрибуты
			local FileAttrNumber, err = shell.getfileattr(FN)
			if (FileAttrNumber == nil) then
				print("> "..err)
				props['FileAttrNumber'] = 32 + iif(string.find(FileAttr,'R'),1,0) + iif(string.find(FileAttr,'H'),2,0) + iif(string.find(FileAttr,'S'),4,0)
			else
				props['FileAttrNumber'] = FileAttrNumber
			end
			shell.setfileattr(FN, 2080)
		end
	end
end

local function AfterSave(FN)
	-- Если была сохранена строка с аттрибутами, то установим их
	local FileAttrNumber = tonumber(props['FileAttrNumber'])
	if FileAttrNumber ~= nil and FileAttrNumber > 0 then
		shell.setfileattr(FN, FileAttrNumber)
	end
end

-- Добавляем свой обработчик события OnBeforeSave
AddEventHandler("OnBeforeSave", BSave)

-- Добавляем свой обработчик события OnSave
AddEventHandler("OnSave", AfterSave)
