YAWL=twl.o twl_cntrls.o twl_data.o twl_dialogs.o twl_ini.o twl_layout.o twl_menu.o twl_modal.o twl_splitter.o twl_toolbar.o

LUA_INCLUDE=../../src/scite/lua/include
CFLAGS=-Os -pedantic -D_USRDLL -DUNICODE -D_WIN32_IE=0x0500
LIBS=-lscite -lgdi32 -lcomdlg32 -lcomctl32 -lshell32
LIBDIR= -L ../lib/

gui.dll: gui_ext.o $(YAWL)
	g++ $(CFLAGS) -s -shared -o gui.dll gui_ext.o $(YAWL) $(LIBDIR) $(LIBS)

gui_ext.o: gui_ext.cpp yawl.h
	g++ $(CFLAGS) -c -I$(LUA_INCLUDE) gui_ext.cpp

clean:
	-del /s *.o >nul

vpath %.cpp yawl

.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

twl.o: yawl/twl.cpp

twl_ini.o: yawl/twl_ini.cpp

twl_cntrls.o: yawl/twl_cntrls.cpp

twl_data.o: yawl/twl_data.cpp

twl_dialogs.o: yawl/twl_dialogs.cpp

twl_layout.o: yawl/twl_layout.cpp

twl_menu.o: yawl/twl_menu.cpp

twl_modal.o: yawl/twl_modal.cpp

twl_splitter.o: yawl/twl_splitter.cpp

twl_toolbar.o: yawl/twl_toolbar.cpp

