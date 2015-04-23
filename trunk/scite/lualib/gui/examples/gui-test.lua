-- here are various possibilities with SciTE-GUI
-- just uncomment the bits you want to see!

--[[
w = gui.window('hello')
--w = gui.panel(200)
w:size(200,300)
w:position(500,40)
ls = gui.list(true)
ls2 = gui.list(true)
txt = gui.memo()
ls:add_column('Firstname',100)
ls:add_column('Lastname',100)
ls:add_item {'jan','botha'}
ls:add_item {'joan','taylor'}
ls:add_item {'billy','jean'}
ls2:add_column('OK',100)
ls2:add_column('NONE',100)
ls2:add_item {'bonzo','dog'}
w:add(ls,"top",70)
--w:add(txt,"top",70)
w:add(ls2,"client")
function joy ()
    print 'that was happy, really!'
end

function sorrow ()
    print 'not so nice...'
end

w:context_menu {
	'some joy|joy',
	'some pain|sorrow',
}

w:show()
--gui.set_panel(w)
--]]
--[[
w = gui.window('hello')
w:size(200,200)
w:position(400,40)
ls = gui.list(true)
ls2 = gui.list(true)
txt = gui.memo()
t = gui.tabbar(w)
t:add_tab("list!",ls)
t:add_tab("another",ls2)
t:add_tab("text",txt)
t:on_select(function(idx)
	print('tab',idx)
end)
ls:add_column('Firstname',100)
ls:add_column('Lastname',100)
ls:add_item {'jan','botha'}
ls:add_item {'joan','taylor'}
ls:on_double_click (print)
ls:on_select(function(i)
	print('selected',i)
end)

ls2:add_column('OK',100)
ls2:add_column('NONE',100)
ls2:add_item {'bonzo','dog'}

w:client(ls)

function joy ()
    print 'that was happy, really!'
end

function sorrow ()
    print 'not so nice...'
end

w:context_menu {
	'some joy|joy',
	'some pain|sorrow',
}
w:show()

w2 = gui.window "next"
w2:size(200,200)
w2:position(400,240)
wls = gui.list()
wls:add_item "stuff"
wls:add_item "nonsense"
w2:client(wls)
w2:show()

tt = gui.toolbar("toolbar!", {
	"watch.bmp:watch this!|joy",
	"stop.bmp:stop doin that!|sorrow",
	"run.bmp:run,run,run|IDM_OPEN",
},16,"c:\\Program Files\\scite")
tt:position(500,300)

--]]

