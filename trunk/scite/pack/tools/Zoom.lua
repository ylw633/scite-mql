--[[-------------------------------------------------
Zoom.lua
Version: 1.2.3
Authors: mozers™, Дмитрий Маслов
-----------------------------------------------------
Обработка стандартной команды Zoom
Вместе с отображаемыми шрифтами, масштабируется и выводимый на принтер шрифт
Изменяет значение пользовательской переменной font.current.size, используемой для отображения текщего размера шрифта в строке состояния
Изменяет значения параметров (magnification, print.magnification, output.magnification) сохраняемых с помощью save_settings.lua
-----------------------------------------------------
Для подключения добавьте в файл .properties:
  statusbar.text.1=$(font.current.size)px
в файл SciTEStartup.lua:
  dofile (props["SciteDefaultHome"].."\\tools\\Zoom.lua")
--]]-------------------------------------------------

local function ChangeFontSize(zoom)
	if output.Focus then
		props["output.magnification"] = output.Zoom
	else
		props["magnification"] = zoom
		props["print.magnification"] = zoom
		if props["pane.accessible"] == '1' then
			editor.PrintMagnification = zoom
		end
		local font_current_size = props["style.*.32"]:match("size:(%d+)")
		props["font.current.size"] = font_current_size + zoom -- Used in statusbar
		scite.UpdateStatusBar()
	end
end

-- Добавляем свой обработчик события OnSendEditor
AddEventHandler("OnSendEditor", function(id_msg, wp, lp)
	if id_msg == SCI_SETZOOM then
		ChangeFontSize(lp)
	end
end)
