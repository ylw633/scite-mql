--[[----------------------------------------------------------
SciTE GUI Extensions for Windows by Steve Donovan and RU-Team
Examples of some of the functions
--]]----------------------------------------------------------

-- Set:
require("gui")

-- NOTE: It's important that any controls be created immediately after the form, so they pick up the correct parent window!

----------
-- WINDOW
----------
-- Set:
window1 = gui.window("title")
window1:size(width, height)
window1:position(x, y)
window1:show()
window1:ontop(true) -- Note: There is no. This - the wish.
window1:hide()
window1:context_menu {'item1|func1', 'item2|func2'} -- Note: Need to use global functions!

-- Get:
visible, x, y, width, height = window1:bounds()

-- Event:
window1:on_show(shown) -- Note: Revision: 1604
window1:on_close() -- Note: Revision: 1604

---------
-- PANEL
---------
-- Set:
panel1 = gui.panel(width)
gui.set_panel(panel1, "right") -- "right" or "left". Note: This set and show the panel.
gui.set_panel() -- remove the panel and hide it (Revision 741)
-- or
window_or_panel1:client(panel2)
panel1:size(width, height) -- Note: Changes either height or width (one of parameters is ignored)
panel1:context_menu {'item1|func1', 'item2|func2'} -- Note: Need to use global functions!

-- Get:
visible, x, y, width, height = panel1:bounds()

----------
-- TABBAR
----------
-- Set:
tabbar1 = gui.tabbar(window_or_panel1)

-- Add:
tabbar1:add_tab("Tab1 heading", list1) -- list1, memo_text1 or panel2

-- Event:
tabbar1:on_select(function(index) print(index) end)

--------
-- LIST
--------
-- Set:
list1 = gui.list(true, true) -- List heading show (true => visiable), multiselect items (false => enable)
window_or_panel1:add(list1, "top", height) -- "top" or "bottom"
-- or
window_or_panel1:add(list1, "left", width) -- "left" or "right"
-- or
window_or_panel1:client(list1)
list1:size(width, height) -- Note: Changes either height or width (one of parameters is ignored)
list1:set_list_colour("#FFFFFF", "#000000") -- foreground, background
list1:context_menu {'item1|func1', 'item2|func2'} -- Note: Revision 1484
list1:set_selected_item(index) -- Note: Revision 747
list1:selected_count() -- Note: Revision 1467
list1:get_selected_items() -- Note: Revision 1467

-- Add:
list1:add_column('Title1', width) -- Note: If gui.list(true)
list1:add_item ({'Caption1','Caption2'}, {data1, data2}) -- data or table, string, function

-- Change:
list1:delete_item(index)
list1:insert_item(index, {'Caption1','Caption2'}, {data1, data2}) -- data or table, string, function

-- Get:
list1_count = list1:count()
data = list1:get_item_data(index) -- Note: Check index ~= -1
text = list1:get_item_text(index) -- Note: Check index ~= -1. BUG: Returned contain only first entry!
index = list1:get_selected_item() -- Note: Revision 747
visible, x, y, width, height = list1:bounds()

-- Event:
list1:on_select(function(index) print(index) end)
list1:on_double_click(function(index) print(index) end)
list1:on_key(function(key) print(key) end) -- Note: Revision: 747
list1:on_focus(setfocus) -- Note: Revision: 1604

-------------
-- MEMO TEXT
-------------
-- Set:
memo1 = gui.memo()
window_or_panel1:add(memo1, "top", height) -- "top" or "bottom"
-- or
window_or_panel1:add(memo1, "left", width) -- "left" or "right"
-- or
window_or_panel1:client(memo1)
memo1:size(width, height) -- Note: Changes either height or width (one of parameters is ignored)
memo1:set_text('{\\rtf{\\fonttbl{\\f0\\fcharset0 Helv;}}\\f0\\fs16'..'sample text'..'}')
memo1:set_memo_colour("#FFFFFF", "#000000") -- foreground, background

-- Get:
visible, x, y, width, height = memo1:bounds()
text = memo1:get_text() -- Note: Revision: 1518

-- Event:
memo1:on_key(function(key) print(key) end) --  -- Note: Revision: 1518