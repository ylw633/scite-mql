--[[--------------------------------------------------
Recode.lua
Authors: mozers
Version: 3.0.0
------------------------------------------------------
Description: Преобразует кодировку открытого файла к указанной
Использует публичную функцию change_codepage_ru из CodePage.lua
------------------------------------------------------
Connection:
 Set in a file .properties:
	command.name.23.*=Convert to DOS-866
	command.23.*=dostring cp_out=866 dofile(props["SciteDefaultHome"].."\\tools\\Recode.lua")
	command.mode.23.*=subsystem:lua,savebefore:no
--]]--------------------------------------------------

local cp_in = editor:codepage()
if cp_in ~= cp_out then
	editor.TargetStart = 0
	editor.TargetEnd = editor.Length
	local text = editor:GetText()
	if text then
		-- Перекодировка
		if cp_in ~= 65001 then
			text = text:to_utf8(cp_in)
		end
		if cp_out ~= 65001 then
			text = text:from_utf8(cp_out)
		end
		editor:ReplaceTarget(text)

		-- Смена кодировки отображения
		if cp_out==65001 then
			scite.MenuCommand(IDM_ENCODING_UCOOKIE)
		else
			if cp_in==866 or cp_out==866 then
				change_codepage_ru() -- public function of CodePage.lua
			else
				scite.MenuCommand(IDM_ENCODING_DEFAULT)
			end
		end
	end
end
