LUALIB=libscite.a

$(LUALIB): ..\..\pack\SciTE.exe
	pexports.exe ..\..\pack\scite.exe > scite.def
	dlltool.exe -d scite.def -l $(LUALIB)

clean:
	-del scite.def
