--[[--------------------------------------------------
Script Text Lexer
Authors: Tymur Gubayev
Version: 1.1.1
------------------------------------------------------
Description:	text-"lexer": colors latin chars, 
	national chars and highlights links. There's
	also possibility to 
--]]--------------------------------------------------

local function TextLexer(styler)
	local S_DEFAULT = 0
	local S_IDENTIFIER = 1
	local S_LINK = 2
	local S_NATIONAL_CHARS = 3

	local IsIdentifier = function (c)
		return c:find('^%a+$') ~= nil
	end

	local IsLink = IsURI or function (c)
		return c:find('[:%w_&%?.%%-@%$%+=%*~%/]')-- ~= nil
	end

	local national_chars = props['chars.accented']
	local IsNational_Char = function (c)
		if national_chars ~= '' then
			return c:find('['..national_chars..']') ~= nil --@todo: this wont work well for UTF
		else
			return
		end
	end

	-- print("Styling: ", styler.startPos, styler.lengthDoc, styler.initStyle)
	styler:StartStyling(styler.startPos, styler.lengthDoc, styler.initStyle)
	local styler_endPos = styler.startPos + styler.lengthDoc

	while styler:More() do
		local stst = styler:State()
		local c = styler:Current()
		-- Exit state if needed
		if stst == S_IDENTIFIER then
			if not IsIdentifier(c) then -- End of identifier
				-- local identifier = styler:Token()
				styler:SetState(S_DEFAULT)
			end
		elseif stst == S_NATIONAL_CHARS and not IsNational_Char(c) then
			styler:SetState(S_DEFAULT)
		--[[--links are processed at once, so the state cannot be LINK
			elseif stst == LINK and not IsLink(c) then
			styler:SetState(DEFAULT)]]
		end

		local n -- link special var
		-- Enter state if needed
		if styler:State() == S_DEFAULT then
			local s = editor:textrange(styler.Position(), styler_endPos) --@todo: optimize: only current line
			n = IsLink(s)
			if n then
				-- print(n,s:sub(1,n),'\n\t',s)
				styler:SetState(S_LINK)
				for i = 1,n do styler:Forward() end
				styler:SetState(S_DEFAULT)
			elseif IsNational_Char(c) then
				styler:SetState(S_NATIONAL_CHARS)
			elseif IsIdentifier(c) then
				styler:SetState(S_IDENTIFIER)
			end
		end

		-- if current text is a link, styler:Forward() is already called
		if not n then styler:Forward() end
	end
	styler:EndStyling()
end

AddEventHandler("OnStyle", function(styler)
	if styler.language == "script_text" then
		TextLexer(styler)
	end
end)

AddEventHandler("OnHotSpotReleaseClick", function(ctrl)
	if --editor.Lexer == 0 and 
	props['Language'] == "script_text" then
		local URL = GetCurrentHotspot()
		-- check if URL is like "a@b.c"
		if URL:find('^%w+@') then
			URL = "mailto:"..URL
		end
		shell.exec(URL)
	end
end)

