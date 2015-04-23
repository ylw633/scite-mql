LUALIB=-L ../lib/ -lscite
DEFFILE=scite.def
LDFLAG=--strip-all -shared

ALL: lua51.dll lua5.1.dll

$(DEFFILE):..\..\pack\SciTE.exe
	pexports ..\..\pack\SciTE.exe > $(DEFFILE)

lua51:$(DEFFILE)
	dlltool --input-def $(DEFFILE) --output-exp lua51

lua5.1:$(DEFFILE)
	dlltool --input-def $(DEFFILE) --output-exp lua5.1

lua51.dll: lua51
	ld.exe $(LDFLAG) --output lua51.dll lua51 $(LUALIB)

lua5.1.dll: lua5.1
	ld.exe $(LDFLAG) --output lua5.1.dll lua5.1 $(LUALIB)

clean:
	-del $(DEFFILE)
	-del lua51
	-del lua5.1
