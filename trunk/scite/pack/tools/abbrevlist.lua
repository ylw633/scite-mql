--[[--------------------------------------------------
abbrevlist.lua
Authors: Dmitry Maslov, frs, mozers™, Tymur Gubayev
version 3.4.17
------------------------------------------------------
  Если при вставке расшифровки аббревиатуры (Ctrl+B) не нашлось точного соответствия,
  то выводится список соответствий начинающихся с этой комбинации символов.
  Возможен автоматический режим работы (появление списка без нажатия на Ctrl+B).
  Он включается параметром abbrev.lexer.auto=3,
        где lexer - имя соответствующего лексера,
              а 3 - min длина введенной строки при которой она будет анализироваться как аббревиатура

  Если в расшифровке аббревиатуры задано несколько курсорных меток, то после вставки расшифровки курсор устанавливается на первую из них.
  На все остальные устанавливаются невидимые метки, переход по которым осуществляется клавишей Tab.
  При установке параметра abbrev.multitab.clear.manual=1 скрипт не очищает метки табуляторов после перемещения на них по Tab. Их пользователь удаляет вручную комбинацией Ctrl+Tab.
  Параметр abbrev.multitab.indic.style=#FF6600,diagonal позволяет показывать метки табуляторов заданным стилем (значения задаются так же как в параметрах indic.style.number)
  Установка параметра abbrev.lexer.ignore.comment=1 разрешает скрипту игнорировать символ комментария в файлах аббревиатур для указанных лексеров (т.е. все закомментированные строки будут восприниматься как обычные аббревиатуры с начальным символом #)
  Параметром abbrev.list.width можно задать максимальную ширину раскрывающегося списка расшифровок аббревиатур (в символах)
  Параметром abbrev.lexer.prev.chars можно задать индивидуальный для избранного лексера список символов, отличный от дефолтового набора ' ([{<', которые предшествуют аббревиатуре. Учтите, что эти символы уже нельзя будет включать в состав аббревиатур!

  Предупреждение:
  Встроенные функции SciTE (Ctrl+B, Ctrl+Shift+R), которые заменяет скрипт, работают совершенно иначе!
  Поэтому файлы сокращений от оригинального SciTE подлежат внимательному пересмотру.

  Подключение:
    В файл SciTEStartup.lua добавьте строку:
    dofile (props["SciteDefaultHome"].."\\tools\\abbrevlist.lua")
--]]--------------------------------------------------

local table_abbr_exp = {}     -- полный список аббревиатур и расшифровок к ним
local table_user_list = {}    -- список подходящих к текущему тексту аббревиатур и расшифровок к ним
local get_abbrev = true       -- признак того, что этот список надо пересоздать
local chars_count_min = 0     -- min длина введенной строки при которой она будет анализироваться
local sep = '\1'              -- разделитель для строки раскрывающегося списка
local typeUserList = 11       -- идентификатор раскрывающегося списка
local smart_tab = 0           -- кол-во дополнительных позиций табуляции (невидимых маркеров)
local cr = string.char(1)     -- символ для временной подмены метки курсора |
local clearmanual = tonumber(props['abbrev.multitab.clear.manual']) == 1
local abbrev_length = 0       -- длина аббревиатуры
local prev_chars = ' ([{<'    -- символы которые предшествуют аббревиатуре

-- Возвращает номер свободного маркера и присваивает ему атрибут "невидимый"
local function SetHiddenMarker()
	for indic_number = 0, 31 do
		local mark = props["indic.style."..indic_number]
		if mark == "" then
			local indic_style = props["abbrev.multitab.indic.style"]
			if indic_style == '' then
				props["indic.style."..indic_number] = "hidden"
			else
				props["indic.style."..indic_number] = indic_style
			end
			return indic_number
		end
	end
end
local num_hidden_indic = SetHiddenMarker()   -- номер маркера позиций курсора (для обхода по TAB)

-- Чтение всех подключенных abbrev-файлов в таблицу table_abbr_exp
local function CreateExpansionList()
	local abbrev_filename = props["AbbrevPath"]
	if abbrev_filename == '' then return end
	table_abbr_exp = ReadAbbrevFile(abbrev_filename) or {}
	for k,v in pairs(table_abbr_exp) do
		v.abbr = v.abbr:utf8upper()
		v.exp = v.exp:gsub('\t','\\t')
	end
end

-- Вставка расшифровки, из раскрывающегося списка
local function InsertExpansion(expansion, abbrev_length)
	if not abbrev_length then abbrev_length = 0 end
	editor:BeginUndoAction()
	-- удаление введенной аббревиатуры с сохранением выделения
	local sel_start, sel_end = editor.SelectionStart - abbrev_length, editor.SelectionEnd - abbrev_length
	if abbrev_length > 0 then
		editor:remove(sel_start, editor.SelectionStart)
		editor:SetSel(sel_start, sel_end)
		abbrev_length = 0
	end
	-- вставка расшифровки c заменой всех меток курсора | (кроме первой) на символ cr
	expansion = expansion:gsub("|", cr):gsub(cr..cr, "||"):gsub(cr, "|", 1)
	local _, tab_count = expansion:gsub(cr, cr) -- определяем кол-во дополнительных меток курсора
	local before_length = editor.Length
	scite.InsertAbbreviation(expansion)
	--------------------------------------------------
	if tab_count>0 then -- если есть дополнительные метки курсора
		local start_pos = editor.CurrentPos
		local end_pos = sel_end + editor.Length - before_length
		if clearmanual then
			EditorMarkText(start_pos-1, 1, num_hidden_indic)
		else
			EditorClearMarks(num_hidden_indic) -- если от предыдущей вставки остались маркеры (пользователь заполнил не все поля), то удаляем их
		end

		repeat -- убираем символы # из расшифровки, ставя вместо них невидимые маркеры
			local tab_start = editor:findtext(cr, 0, end_pos, start_pos)
			if not tab_start then break end
			editor:GotoPos(tab_start+1)  editor:DeleteBack()
			EditorMarkText(tab_start-1, 1, num_hidden_indic)
			end_pos = tab_start-1
		until false

		editor:GotoPos(start_pos)
		smart_tab = tab_count -- разрешаем особую обработку нажатия на TAB (по событию OnKey)
	end
	--------------------------------------------------
	editor:EndUndoAction()
end
-- export global
scite_InsertAbbreviation = InsertExpansion

-- Показ списка из расшифровок, соответствующих введенной аббревиатуре
local function ShowExpansionList(event_IDM_ABBREV)
	if get_abbrev then -- при открытии и переключении вкладки
		-- определяем символы которые предшествуют аббревиатуре
		prev_chars = props['abbrev.'..props['Language']..'.prev.chars']
		if prev_chars == '' then prev_chars = ' ([{<' end
		prev_chars = '['..prev_chars:gsub(' ', 's'):gsub('(.)', '\\%1')..']'
	end
	local sel_start = editor.SelectionStart
	local line_start_pos = editor:PositionFromLine(editor:LineFromPosition(sel_start))
	-- ищем начало сокращения - первый пробельный символ
	local abbrev_start = editor:findtext(prev_chars, SCFIND_REGEXP, sel_start-1, line_start_pos) --@ `-1` in `sel_start-1` is propably wrong, but so you can make "& =bla" abbreviation
	abbrev_start = abbrev_start and abbrev_start+1 or line_start_pos

	local abbrev = editor:textrange(abbrev_start, sel_start)
	abbrev_length = #abbrev
	if abbrev_length == 0 then return event_IDM_ABBREV end
	-- если длина вероятной аббревиатуры меньше заданного кол-ва символов то выходим
	if not event_IDM_ABBREV and abbrev_length < chars_count_min then return true end

	-- если мы переключились на другой файл, то строим таблицу table_abbr_exp заново
	if get_abbrev then
		CreateExpansionList()
		get_abbrev = false
	end
	if #table_abbr_exp == 0 then return event_IDM_ABBREV end

	local cp = editor:codepage()
	if cp ~= 65001 then abbrev = abbrev:to_utf8(cp) end
	abbrev = abbrev:utf8upper()
	table_user_list = {}
	 -- выбираем из table_abbr_exp только записи соответствующие этой аббревиатуре
	for i = 1, #table_abbr_exp do
		if table_abbr_exp[i].abbr:find(abbrev, 1, true) == 1 then
			table_user_list[#table_user_list+1] = {table_abbr_exp[i].abbr, table_abbr_exp[i].exp}
		end
	end
	if #table_user_list == 0 then return event_IDM_ABBREV end
	-- если мы используем Ctrl+B (а не автоматическое срабатывание)
	if (event_IDM_ABBREV)
		-- и если найден единственный вариант расшифровки
		and (#table_user_list == 1)
		-- и аббревиатура полностью соответствует введенной
		and (abbrev == table_user_list[1][1])
			-- то вставку производим немедленно
			then
				InsertExpansion(table_user_list[1][2], abbrev_length)
				return true
	end

	-- показываем раскрывающийся список из расшифровок, соответствующих введенной аббревиатуре
	local tmp = {}
	local list_width = tonumber(props['abbrev.list.width']) or -1
	for i = 1, #table_user_list do
		tmp[#tmp+1] = table_user_list[i][2]:sub(1, list_width)
	end
	local table_user_list_string = table.concat(tmp, sep):gsub('%?', ' ')
	if cp ~= 65001 then table_user_list_string = table_user_list_string:from_utf8(cp) end
	local sep_tmp = editor.AutoCSeparator
	editor.AutoCSeparator = string.byte(sep)
	editor:UserListShow(typeUserList, table_user_list_string)
	editor.AutoCSeparator = sep_tmp
	return true
end

------------------------------------------------------
AddEventHandler("OnMenuCommand", function(msg)
	if msg == IDM_ABBREV then
		return ShowExpansionList(true)
	end
end)

AddEventHandler("OnChar", function()
	chars_count_min = tonumber(props['abbrev.'..props['Language']..'.auto']) or tonumber(props['abbrev.*.auto']) or 0
	if chars_count_min ~= 0 then
		return ShowExpansionList(false)
	end
end)

AddEventHandler("OnKey", function(key, shift, ctrl, alt)
	if editor.Focus and smart_tab > 0 and key == 9 then -- TAB=9
		if not (shift or ctrl or alt) then
			for i = editor.CurrentPos, editor.Length do
				if editor:IndicatorValueAt(num_hidden_indic, i)==1 then
					editor:GotoPos(i+1)
					if not clearmanual then
						EditorClearMarks(num_hidden_indic, i, 1) -- после перехода на позицию заданную маркером, этот маркер удаляем
						smart_tab = smart_tab - 1
					end
					return true
				end
			end
		elseif ctrl and not (shift or alt) then
			EditorClearMarks(num_hidden_indic)
			smart_tab = 0
			return true
		end
	end
end)

AddEventHandler("OnUserListSelection", function(tp, sel_value, sel_item_id)
	if tp == typeUserList then
		InsertExpansion(table_user_list[sel_item_id][2], abbrev_length)
	end
end)

AddEventHandler("OnSwitchFile", function()
	get_abbrev = true
end)

AddEventHandler("OnOpen", function()
	get_abbrev = true
end)
