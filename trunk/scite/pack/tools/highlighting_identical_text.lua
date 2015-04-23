--[[--------------------------------------------------
Highlighting Identical Text
Version: 1.5.3
Author: mozers™, TymurGubayev
------------------------------
Авто подсветка текста, который совпадает с текущим словом или выделением
В файле настроек задается стиль подсветки
Внимание:
В скрипте используются функции из COMMON.lua (EditorMarkText, EditorClearMarks, GetCurrentWord)
------------------------------
Подключение:
Добавить в SciTEStartup.lua строку:
	dofile (props["SciteDefaultHome"].."\\tools\\highlighting_identical_text.lua")

Добавить в файл настроек параметр:
	highlighting.identical.text=0
и переключатель в меню Tools:
	command.checked.139.*=$(highlighting.identical.text)
	command.name.139.*=Highlighting Identical Text
	command.139.*=highlighting_identical_text_switch
	command.mode.139.*=subsystem:lua,savebefore:no

Задать стиль используемых маркеров
(14 - отметка идентичного текста, 15 - отметка текста при превышении max кол-ва вхождений):
	indic.style.14=#CC99FF,box
	indic.style.15=#FF0000,box

Дополнительно можно изменить значение по умолчанию (50) на max кол-во вхождений (0 - без ограничения).
	highlighting.identical.text.max=100
и задать перечень не подсвечиваемых слов для конкретного лексера (или для всех остальных - *)
	highlighting.identical.text.reserved.words.lua=and,break,do,else,elseif,end,false,for,function,if,in,local,nil,not,or,repeat,return,then,true,until,while
--]]----------------------------------------------------

local count_max = 50   -- max кол-во результатов поиска (по умолчанию)
local store_pos        -- переменная для хранения предыдущей позиции курсора
local store_text       -- переменная для хранения предыдущего поискового текста
local mark_ident = 14   -- номер маркера для отметки идентичного текста/слова
local mark_max = 15     -- номер маркера для отметки при превышении max кол-ва вхождений
local chars_count      -- кол-во символов в текущем документе
local reserved_words   -- не подсвечиваемые слова

local max = props['highlighting.identical.text.max']
if max ~= '' then count_max = tonumber(max) end

-- Переключатель подсветки (вкл/выкл) срабатывает из меню Tools
function highlighting_identical_text_switch()
	local prop_name = 'highlighting.identical.text'
	props[prop_name] = 1 - tonumber(props[prop_name])
	EditorClearMarks(mark_ident)
	store_pos, store_text = 0, ''
end

-- Проверка, является ли текущее слово зарезервированным
local function isReservedWord(cur_text)
	if reserved_words == '' then return false end
	for w in string.gmatch(reserved_words, "%w+") do
		if cur_text:lower() == w:lower() then return true end
	end
	return false
end

-- Поиск идентичных слов/текста
local function IdenticalTextFinder()
	local current_pos = editor.CurrentPos
	if current_pos == store_pos then return end
	store_pos = current_pos

	local cur_text = editor:GetSelText()
	if cur_text:find('^%s+$') then return end
	local find_flags = SCFIND_MATCHCASE
	if cur_text == '' then
		cur_text = GetCurrentWord()
		find_flags = find_flags + SCFIND_WHOLEWORD
		if isReservedWord(cur_text) then return end
	end
	if cur_text == store_text then return end
	store_text = cur_text

	EditorClearMarks(mark_ident)
	EditorClearMarks(mark_max)
	----------------------------------------------------------
	local match_table = {}
	local find_start = 0
	repeat
		local ident_text_start, ident_text_end = editor:findtext(cur_text, find_flags, find_start, editor.Length)
		if ident_text_start == nil
		or ident_text_start == ident_text_end then break end
		-- загоняем все результаты поиска в таблицу match_table
		match_table[#match_table+1] = {ident_text_start, ident_text_end}
		if count_max ~= 0 then
			if #match_table > count_max then -- если результатов больше, чем указанное число...
				local err_start, err_end
				if find_flags == SCFIND_MATCHCASE then
					-- маркируем выделенный текст
					err_start = editor.SelectionStart
					err_end = editor.SelectionEnd
					EditorMarkText(err_start, err_end-err_start, mark_max)
					return
				else
					-- маркируем текущее слово
					err_start = editor:WordStartPosition(current_pos, true)
					err_end = editor:WordEndPosition(current_pos, true)
					EditorMarkText(err_start, err_end-err_start, mark_max)
					return
				end
			end
		end
		find_start = ident_text_end + 1
	until false
	----------------------------------------------------------
	if #match_table > 1 then
		for i = 1, #match_table do
			-- Отмечаем все слова, беря данные из таблицы match_table
			EditorMarkText(match_table[i][1], match_table[i][2]-match_table[i][1], mark_ident)
		end
	end

end

AddEventHandler("OnUpdateUI", function()
	if props['FileName'] ~= '' then
		if tonumber(props["highlighting.identical.text"]) == 1 then
			if editor.Length ~= chars_count then
				chars_count = editor.Length
				reserved_words = props['highlighting.identical.text.reserved.words.' .. props['Language']]
				if reserved_words == '' then reserved_words = props['highlighting.identical.text.reserved.words.*'] end
			end
			IdenticalTextFinder()
		end
	end
end)
