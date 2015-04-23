-- this opens any URL found inside a file on double-click.
-- note the gotcha: the _current linenumber_ after the double click
-- is just past the double-clicked line.
-- (Requires extman)

scite_OnDoubleClick(function()
    local lno = editor:LineFromPosition(editor.CurrentPos)
    local line = editor:GetLine(lno-1)
    local url = line:match('(http://[%w_/%.]+)')
    if not url then
        url = line:match('(file://[%w_/%.:]+)')
    end
    if url then
        gui.run(url)
    end
end)
