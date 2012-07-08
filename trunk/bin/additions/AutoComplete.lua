-- Dynamically generate autocomplete lists from possible identifiers in any file.

local IGNORE_CASE = true
-- Number of chars to type before the autocomplete list appears:
local MIN_PREFIX_LEN = 3
-- Length of shortest word to add to the autocomplete list
local MIN_IDENTIFIER_LEN = 5
-- A list of string patterns for finding suggestions for the autocomplete menu.
local IDENTIFIER_PATTERNS = {"[%a_$][%w_]+", "[%a_$][%w_.]*[%w_]", "[%a_$][%w_-]*[%w_]"}


local list_names = {}
local notempty = next


if IGNORE_CASE then
    normalize = string.lower
else
    normalize = function(word) return word end
end


function buildNames()
    list_names = {}
    local text = editor:GetText()
    for i, pattern in ipairs(IDENTIFIER_PATTERNS) do
        for word in string.gmatch(text, pattern) do
            if string.len(word) >= MIN_IDENTIFIER_LEN then
                list_names[word] = true
            end
        end
    end
end


function handleChar()
    if not editor:AutoCActive() then
        editor.AutoCIgnoreCase = IGNORE_CASE            
        local pos = editor.CurrentPos
        local startPos = editor:WordStartPosition(pos, true)
        local len = pos - startPos
        if len >= MIN_PREFIX_LEN then
            local prefix = editor:textrange(startPos, pos)
            local menuItems = {}
            for name, v in pairs(list_names) do
                if normalize(string.sub(name, 1, len)) == normalize(prefix) then 
                    table.insert(menuItems, name)
                end
            end
            if notempty(menuItems) then
                table.sort(menuItems)
                editor:AutoCShow(len, table.concat(menuItems, " "))
            end
        end
    end
end

-- Event handlers
--OnChar       = handleChar
--OnSave       = buildNames
--OnSwitchFile = buildNames
--OnOpen       = buildNames
