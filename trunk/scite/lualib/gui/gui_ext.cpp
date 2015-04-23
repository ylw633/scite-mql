/* gui_ext.cpp
This is a simple set of predefined GUI windows for SciTE,
built using the YAWL library.
Steve Donovan, 2007.
  */
#include <windows.h>
#include "yawl.h"
#include <string.h>
#include <vector>
#include <io.h>
#include <direct.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

const char* WINDOW_CLASS = "WINDOW*";
const int BUFFER_SIZE = 2*0xFFFF;
static TWin* s_parent = NULL;
static TWin* s_last_parent = NULL;

#define EQ(s1,s2) (wcscmp((s1),(s2))==0)

// ==========================================================================
enum { SURROGATE_LEAD_FIRST = 0xD800 };
enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
enum { SURROGATE_TRAIL_LAST = 0xDFFF };

static unsigned int UTF8Length(const wchar_t *uptr, size_t tlen) {
	size_t len = 0;
	for (size_t i = 0; i < tlen && uptr[i];) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			len++;
		} else if (uch < 0x800) {
			len += 2;
		} else if ((uch >= SURROGATE_LEAD_FIRST) &&
			(uch <= SURROGATE_TRAIL_LAST)) {
			len += 4;
			i++;
		} else {
			len += 3;
		}
		i++;
	}
	return len;
}

static void UTF8FromUTF16(const wchar_t *uptr, size_t tlen, char *putf, size_t len) {
	int k = 0;
	for (size_t i = 0; i < tlen && uptr[i];) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			putf[k++] = static_cast<char>(uch);
		} else if (uch < 0x800) {
			putf[k++] = static_cast<char>(0xC0 | (uch >> 6));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		} else if ((uch >= SURROGATE_LEAD_FIRST) &&
			(uch <= SURROGATE_TRAIL_LAST)) {
			// Half a surrogate pair
			i++;
			unsigned int xch = 0x10000 + ((uch & 0x3ff) << 10) + (uptr[i] & 0x3ff);
			putf[k++] = static_cast<char>(0xF0 | (xch >> 18));
			putf[k++] = static_cast<char>(0x80 | ((xch >> 12) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | ((xch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (xch & 0x3f));
		} else {
			putf[k++] = static_cast<char>(0xE0 | (uch >> 12));
			putf[k++] = static_cast<char>(0x80 | ((uch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		}
		i++;
	}
	putf[len] = '\0';
}

static size_t UTF16Length(const char *s, unsigned int len) {
	size_t ulen = 0;
	size_t charLen;
	for (size_t i=0; i<len;) {
		unsigned char ch = static_cast<unsigned char>(s[i]);
		if (ch < 0x80) {
			charLen = 1;
		} else if (ch < 0x80 + 0x40 + 0x20) {
			charLen = 2;
		} else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
			charLen = 3;
		} else {
			charLen = 4;
			ulen++;
		}
		i += charLen;
		ulen++;
	}
	return ulen;
}

static size_t UTF16FromUTF8(const char *s, size_t len, wchar_t *tbuf, size_t tlen) {
	size_t ui=0;
	const unsigned char *us = reinterpret_cast<const unsigned char *>(s);
	size_t i=0;
	while ((i<len) && (ui<tlen)) {
		unsigned char ch = us[i++];
		if (ch < 0x80) {
			tbuf[ui] = ch;
		} else if (ch < 0x80 + 0x40 + 0x20) {
			tbuf[ui] = static_cast<wchar_t>((ch & 0x1F) << 6);
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + (ch & 0x7F));
		} else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
			tbuf[ui] = static_cast<wchar_t>((ch & 0xF) << 12);
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + ((ch & 0x7F) << 6));
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + (ch & 0x7F));
		} else {
			// Outside the BMP so need two surrogates
			int val = (ch & 0x7) << 18;
			ch = us[i++];
			val += (ch & 0x3F) << 12;
			ch = us[i++];
			val += (ch & 0x3F) << 6;
			ch = us[i++];
			val += (ch & 0x3F);
			tbuf[ui] = static_cast<wchar_t>(((val - 0x10000) >> 10) + SURROGATE_LEAD_FIRST);
			ui++;
			tbuf[ui] = static_cast<wchar_t>((val & 0x3ff) + SURROGATE_TRAIL_FIRST);
		}
		ui++;
	}
	return ui;
}

wchar_t* StringFromUTF8(const char *s) {
	size_t sLen = s ? strlen(s) : 0;
	size_t wideLen = UTF16Length(s, sLen);
	wchar_t* vgc = new wchar_t[wideLen + 1];
	size_t outLen = UTF16FromUTF8(s, sLen, vgc, wideLen);
	vgc[outLen] = 0;
	return vgc;
}

char* UTF8FromString(const wchar_t *s) {
	size_t sLen = wcslen(s);
	size_t narrowLen = UTF8Length(s, sLen);
	char* vc = new char[narrowLen + 1];
	UTF8FromUTF16(s, sLen, vc, narrowLen);
	return vc;
}
// -------------------------------------------------------------------

TWin* get_parent()
{
	return s_parent;
}

TWin* get_last_parent()
{
	return s_last_parent;
}

TWin *get_desktop_window()
{
	static TWin *desk = NULL;
	if (desk == NULL) {
		desk = new TWin(GetDesktopWindow());
	}
	return desk;
}


class PromptDlg: public TModalDlg {
public:
	wchar_t m_val[256];
    const wchar_t *m_field_name;

	PromptDlg(TEventWindow *parent, pchar field_name, const wchar_t* val)
		: TModalDlg(parent,L"Enter:"), m_field_name(field_name)
	{
		wcscpy(m_val,val);
	}

	void layout(Layout& lo)
    {
		lo << Field(m_field_name,m_val);
    }
};

// show a message on the SciTE output window
void OutputMessage(lua_State *L)
{
	if (lua_isstring(L,-1)) {
		size_t len;
		const char *msg = lua_tolstring(L,-1,&len);
		char *buff = new char[len+2];
		strncpy(buff,msg,len);
		buff[len] = '\n';
		buff[len+1] = '\0';
		lua_pop(L,1);
		if (lua_checkstack(L,3)) {
			lua_getglobal(L,"output");
			lua_getfield(L,-1,"AddText");
			lua_insert(L,-2);
			lua_pushstring(L,buff);
			lua_pcall(L,2,0,0);
		}
		delete[] buff;
	}
}

void dispatch_ref(lua_State* L,int idx, int ival)
{
	if (idx != 0) {
		lua_rawgeti(L,LUA_REGISTRYINDEX,idx);
		lua_pushnumber(L,ival);
		
		if (lua_pcall(L,1,0,0)) {
			OutputMessage(L);
		}
	}
}

void dispatch_ref_bool(lua_State* L,int idx, bool ival)
{
	if (idx != 0) {
		lua_rawgeti(L,LUA_REGISTRYINDEX,idx);
		lua_pushboolean(L,ival);
		
		if (lua_pcall(L,1,0,0)) {
			OutputMessage(L);
		}
	}
}

int dispatch_rref(lua_State* L,int idx, int ival)
{
	if (idx != 0) {
		lua_rawgeti(L,LUA_REGISTRYINDEX,idx);
		lua_pushnumber(L,ival);
		
		if (lua_pcall(L,1,1,0)) {
			OutputMessage(L);
		} else {
			return lua_toboolean(L, -1);
		}
	}
	return 0;
}

bool function_ref(lua_State* L, int idx, int* pr)
{
	if (*pr != 0) {
		luaL_unref(L,LUA_REGISTRYINDEX,*pr);
	}
	lua_pushvalue(L,idx);
	*pr = luaL_ref(L,LUA_REGISTRYINDEX);
	return true;
}

class LuaWindow: public TEventWindow
{
protected:
	lua_State* L;
	int onclose_idx;
	int onfocus_idx;
	int onshow_idx;
public:
	LuaWindow(const wchar_t* caption,lua_State* l, TWin *parent, int stylex = 0, bool is_child = false, int style = -1)
		: TEventWindow(caption,parent,stylex,is_child,style),L(l),onclose_idx(0),onfocus_idx(0),onshow_idx(0)
	{}

	void handler(Item* item)
	{
		wchar_t* name = (wchar_t*)item->data;
		if (wcsncmp(name,L"IDM_",4) == 0) {
			wchar_t buff[128];
			swprintf(buff,L"scite.MenuCommand(%s)",name); //
			luaL_dostring(L,UTF8FromString(buff));
		} else {
			lua_getglobal(L,UTF8FromString(name));
			if (lua_pcall(L,0,0,0)) {
				OutputMessage(L);
			}
		}
	}

	void set_on_close(int iarg)
	{
		function_ref(L,iarg,&onclose_idx);
	}

	void on_close()
	{
		dispatch_ref(L,onclose_idx,0);
	}

	void set_on_show(int iarg)
	{
		function_ref(L,iarg,&onshow_idx);
	}

	void on_showhide(bool show)
	{
		dispatch_ref_bool(L,onshow_idx,show);
	}
};

class PanelWindow: public LuaWindow
{
public:
	PanelWindow(lua_State* l)
		: LuaWindow(L"",l,get_parent(),0,true)
	{}
};

class PaletteWindow;
static PaletteWindow* instances[50];
static int n_instances = 0;

class PaletteWindow: public LuaWindow
{
protected:
	bool m_shown;
public:
	PaletteWindow(const wchar_t* caption, lua_State* l, int stylex = 0, int style = -1)
		: LuaWindow(caption,l,NULL,stylex,false,style)
	{
		instances[n_instances++] = this;
		instances[n_instances] = NULL;
	}

	void show(int how = SW_SHOW)
	{
		TEventWindow::show(how);
		m_shown = true;
	}

	void hide()
	{
		TEventWindow::hide();
		m_shown = false;
	}

	virtual bool query_close()
	{
		hide();
		return false;
	}

	void really_show()
	{
		TEventWindow::show();
	}

	void really_hide()
	{
		TEventWindow::hide();
	}

	static void set_visibility(bool yesno)
	{
		for (int i = 0; instances[i]; i++) {
			PaletteWindow *w = instances[i];
			if (w->m_shown) {
				if (yesno) w->really_show();
				else w->really_hide();
			}
		}
	}

};

class ToolbarWindow: public PaletteWindow
{
public:
	ToolbarWindow(const wchar_t* caption, wchar_t** item, int sz, const wchar_t* path, lua_State* l)
		: PaletteWindow(caption,l,WS_EX_PALETTEWINDOW, WS_OVERLAPPED)
	{
		TToolbar tbar(this,sz,sz);
		tbar.set_path(path);
		for (;*item; item++) {
			wchar_t* img_text = wcstok(*item,L"|");
			wchar_t* fun = wcstok(NULL,L"");
			tbar << Item(img_text,(EH)&LuaWindow::handler,fun);
		}
		tbar.release();
		SIZE sze = tbar.get_size();
		client_resize(sze.cx, sze.cy + 10);
	}

};

////// This acts as the top-level frame window containing these controls; it supports
////// adding extra buttons and actions.
class ContainerWindow: public PaletteWindow
{
public:
	TListView* listv;
	const wchar_t* select_name;
	const wchar_t* double_name;

	ContainerWindow(const wchar_t* caption, lua_State* l)
		: PaletteWindow(caption,l),select_name(NULL),double_name(NULL)
	{
		set_icon_from_window(s_parent);
	}

	void dispatch(const wchar_t* name, int ival)
	{
		if (name != NULL) {
			lua_getglobal(L,UTF8FromString(name));
			lua_pushnumber(L,ival);
			if (lua_pcall(L,1,0,0)) {
				OutputMessage(L);
			}
		}
	}

	void on_button(Item* item)
	{
		dispatch((wchar_t*)item->data,0);  //listv->selected_id()
	}

	void add_buttons(lua_State* L)
	{
		int nargs = lua_gettop(L);
		int i = 2;
		TEW* panel = new TEW(NULL,this,0,true);
		panel->align(alBottom,50);
		Layout layout(panel,this);
		while (i < nargs) {
			layout << Button(StringFromUTF8(luaL_checkstring(L,i)),(EH)&ContainerWindow::on_button,(void*)StringFromUTF8(luaL_checkstring(L,i+1)));
			i += 2;
		}
		layout.release();
		add(panel);
		size();
	}
};

class LuaControl
{
protected:
	lua_State *L;
	int select_idx;
	int double_idx;
	int onkey_idx;
	int rclick_idx;
	int onclose_idx;
	int onfocus_idx;
    Handle m_hpopup_menu;

public:
	LuaControl(lua_State *l)
		: L(l), select_idx(0), double_idx(0), onkey_idx(0), rclick_idx(0)
		, m_hpopup_menu(0), onclose_idx(0), onfocus_idx(0)
	{}

	void set_popup_menu(Handle menu)
	{
		m_hpopup_menu = menu;
	}

	virtual void set_select(int iarg)
	{
		function_ref(L,iarg,&select_idx);
	}

	virtual void set_double_click(int iarg)
	{
		function_ref(L,2,&double_idx);
	}

	virtual void set_onkey(int iarg)
	{
		function_ref(L,iarg,&onkey_idx);
	}

	virtual void set_rclick(int iarg)
	{
		function_ref(L,iarg,&rclick_idx);
	}

	virtual void set_on_close(int iarg)
	{
		function_ref(L,iarg,&onclose_idx);
	}

	virtual void set_on_focus(int iarg)
	{
		function_ref(L,iarg,&onfocus_idx);
	}

};

class TMemoLua: public TMemo, public LuaControl
{
public:
	TMemoLua(TWin *parent, lua_State *l, int id, bool do_scroll=false, bool plain=false)
		:TMemo(parent, id, do_scroll, plain), LuaControl(l)
	{}

	virtual int handle_onkey(int id)
	{
		return dispatch_rref(L,onkey_idx,id);
	}
};

class TListViewLua: public TListViewB, public LuaControl
{
public:
	TListViewLua(TWin *parent, lua_State *l,bool multiple_columns = false, bool single_select = true)
		: TListViewB(parent, false, multiple_columns, single_select),
		LuaControl(l)
	{
		if (! multiple_columns) {
			add_column(L"*",200);
		}
	}

	// implement
	virtual void handle_select(int id)
	{
		dispatch_ref(L,select_idx,id);
	}

	virtual void handle_double_click(int id)
	{
		dispatch_ref(L,double_idx,id);
	}

	virtual void handle_onkey(int id)
	{
		dispatch_ref(L,onkey_idx,id);
	}

	virtual int handle_rclick(int id)
	{
		if(m_hpopup_menu) {
			POINT p;
			GetCursorPos(&p);
			
			HWND hwnd = (HWND)(get_parent_win()->handle());
			TrackPopupMenu((HMENU)m_hpopup_menu, TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, hwnd, NULL);
			return 1;
		}

		return 0;
	}

	virtual void handle_onfocus(bool yes)
	{
		dispatch_ref_bool(L,onfocus_idx,yes);
	}
};

class TTreeViewLua: public TTreeView, public LuaControl
{
public:
	TTreeViewLua(TEventWindow *parent, lua_State *l)
		: TTreeView(parent),
		LuaControl(l)
	{
	}

	// implement
	virtual void handle_select(int id)
	{
		dispatch_ref(L,select_idx,id);
	}

	virtual void handle_double_click(int id)
	{
		dispatch_ref(L,double_idx,id);
	}

	virtual void handle_onkey(int id)
	{
		dispatch_ref(L,onkey_idx,id);
	}

	virtual int handle_rclick(int id)
	{
		if(m_hpopup_menu) {
			POINT p;
			GetCursorPos(&p);
			
			HWND hwnd = (HWND)(get_parent_win()->handle());
			TrackPopupMenu((HMENU)m_hpopup_menu, TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, hwnd, NULL);
			return 1;
		}

		return 0;
	}

	virtual void handle_onfocus(bool yes)
	{
		dispatch_ref_bool(L,onfocus_idx,yes);
	}
};

class TTabControlLua: public TTabControlB, public LuaControl
{
	int selection_changing_idx;
	TEventWindow *form;
public:

	TTabControlLua (TEventWindow *parent,lua_State* l)
		: TTabControlB(parent),LuaControl(l),selection_changing_idx(0),form(parent)
	{
	}

	void set_selection_changing(int iarg)
	{
		function_ref(L,iarg,&selection_changing_idx);
	}

  // implement
	virtual void handle_select(int id)
	{
	  TWin *page = (TWin*)get_data(id);
	  form->set_client(page);
	  form->size();
	  dispatch_ref(L,select_idx,id);
	}

	virtual int handle_selection_changing(int id)
	{
	  if (selection_changing_idx) {
		  dispatch_ref(L,selection_changing_idx,id);
		  return 1;
	  } else {
		  return 0;
	  }
	}

};

struct WinWrap {
	TWin *window;
	void *data;

};

static int wrap_window(lua_State* L, TWin* win)
{
	WinWrap *wrp = (WinWrap*)lua_newuserdata(L,sizeof(WinWrap));
	wrp->window = win;
	wrp->data = NULL;
	luaL_getmetatable(L,WINDOW_CLASS);
	lua_setmetatable(L,-2);
	return 1;
}

static void throw_error(lua_State* L, const wchar_t *msg)
{
	lua_pushstring(L,UTF8FromString(msg));
	lua_error(L);
}

static TWin* window_arg(lua_State* L, int idx = 1)
{
	WinWrap *wrp = (WinWrap*)lua_touserdata(L,idx);
	if (! wrp) throw_error(L,L"not a window");
	return (PaletteWindow*)wrp->window;
}

static void *& window_data(lua_State* L, int idx = 1)
{
	WinWrap *wrp = (WinWrap*)lua_touserdata(L,idx);
	if (! wrp) throw_error(L,L"not a window");
	return wrp->data;
}


static wchar_t** table_to_str_array(lua_State *L, int idx, int* psz = NULL)
{
	if (! lua_istable(L,idx)) {
		throw_error(L,L"table argument expected");
	}
    wchar_t** p = new wchar_t*[100];
    int i = 0;
    lua_pushnil(L); // first key
    while (lua_next(L, idx) != 0) {
         /* `key' is at index -2 and `value' at index -1 */
		 p[i++] = _wcsdup(StringFromUTF8(lua_tostring(L,-1)));
         lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
    }
    p[i] = NULL;  // conventional way of indicating end of string array
    if (psz) *psz = i;
    return p;
}

static int conv(const char* s, int i1)
{
	char args[] = {s[i1],s[i1+1],'\0'};
	int val;
	sscanf(args,"%X",&val);
	return val;
}

static unsigned int convert_colour_spec(const char* clrs)
{
	return RGB(conv(clrs,1),conv(clrs,3),conv(clrs,5));
}


static bool optboolean(lua_State* L, int idx, bool res)
{
	if (lua_isnoneornil(L,idx)) {
		return res;
	} else {
		return lua_toboolean(L,idx);
	}
}


///// Exported Lua functions ///////

/** gui.message(message_string, is_warning)
	@param message_string
	@param is_warning (0 for plain message, 1 for warning box)
	MSG_ERROR=2,MSG_WARNING=1, MSG_QUERY=3;
*/

int do_message(lua_State* L)
{
	const wchar_t* msg = StringFromUTF8(luaL_checkstring(L,1));
	const wchar_t* kind = StringFromUTF8(luaL_optstring(L,2,"message"));
	int type = 0;
	if (EQ(kind,L"message")) type = 0; else
	if (EQ(kind,L"warning")) type = 1; else
	if (EQ(kind,L"error")) type = 2; else
	if (EQ(kind,L"query")) type = 3;
	lua_pushboolean(L,get_parent()->message(msg,type));
	return 1;
}


/** gui.prompt_value(prompt_string, default_value)
	@param prompt_string
	@param default_value (must be a string)
	@return string value if OK, nil if Cancel.
*/
int do_prompt_value(lua_State* L)
{
	const wchar_t* varname = StringFromUTF8(luaL_checkstring(L,1));
	const wchar_t* value = StringFromUTF8(luaL_optstring(L,2,""));
	PromptDlg dlg((TEventWindow*)get_desktop_window(),varname, value);
	if (dlg.show_modal()) {
		lua_pushstring(L,UTF8FromString(dlg.m_val));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int do_run(lua_State* L)
{
	const wchar_t* lpFile = StringFromUTF8(luaL_checkstring(L,1));
	const wchar_t* lpParameters = StringFromUTF8(lua_tostring(L,2));
	const wchar_t* lpDirectory = StringFromUTF8(lua_tostring(L,3));
	int res = (int)ShellExecute (
		NULL,
		L"open",
		lpFile,
		lpParameters,
		lpDirectory,
		SW_SHOWDEFAULT
	);
	if (res <= 32) {
		lua_pushboolean(L,0);
		lua_pushinteger(L,res);
		return 2;
	} else {
		lua_pushinteger(L,res);
		return 1;
	}
}

/** gui.colour_dlg(default_colour)
	@param default_colour  colour either in form '#RRGGBB" or as a 32-bit integer
	@return chosen colour, in same form as default_colour
*/


int do_colour_dlg(lua_State* L)
{
	bool in_rgb = lua_isstring(L,1);
	unsigned int cval;
	if (in_rgb) {
		cval = convert_colour_spec(lua_tostring(L,1));		
	} else {
		cval = luaL_optinteger(L,1,0);
	}
	TColourDialog dlg(get_parent(),cval);
	if (dlg.go()) {
		cval = dlg.result();
		if (in_rgb) {
			char buff[12];
			sprintf(buff,"#%02X%02X%02X",GetRValue(cval),GetGValue(cval),GetBValue(cval));
			lua_pushstring(L,buff);
		} else {
			lua_pushnumber(L,cval);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/** gui.open_dlg(caption,filter)
	@param caption (defaults to "Open File")
	@param filter (defaults to "All (*.*)|*.*")
	@return chosen filename
*/
int do_open_dlg(lua_State* L)
{
	const wchar_t* caption = StringFromUTF8(luaL_optstring(L,1,"Open File"));
	const wchar_t* filter = StringFromUTF8(luaL_optstring(L,2,"All (*.*)|*.*"));
	TOpenFile tof (get_parent(),caption,filter);
	if (tof.go()) {
		lua_pushstring(L,UTF8FromString(tof.file_name()));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int do_save_dlg(lua_State* L)
{
	const wchar_t* caption = StringFromUTF8(luaL_optstring(L,1,"Save File"));
	const wchar_t* filter = StringFromUTF8(luaL_optstring(L,2,"All (*.*)|*.*"));
	TSaveFile tof (get_parent(),caption,filter);
	if (tof.go()) {
		lua_pushstring(L,UTF8FromString(tof.file_name()));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/** gui.select_dir_dlg(description,initialdir)
	@param description (defaults to empty string)
	@return chosen directory
*/
int do_select_dir_dlg(lua_State* L)
{
	const wchar_t* descr = StringFromUTF8(luaL_optstring(L,1,""));
	const wchar_t* initdir = StringFromUTF8(luaL_optstring(L,2,""));
	TSelectDir dir(get_parent(), descr, initdir);
	if (dir.go()) {
		lua_pushstring(L, UTF8FromString(dir.path()));
	} else {
		lua_pushnil(L);
	}
	return 1;
}


int new_toolbar(lua_State* L)
{
	const wchar_t* caption = StringFromUTF8(luaL_checkstring(L,1));
	wchar_t** items = table_to_str_array(L,2);
	int sz = luaL_optinteger(L,3,16);
	const wchar_t* path = StringFromUTF8(lua_tostring(L,4));
	ToolbarWindow* ew = new ToolbarWindow(caption,items,sz,path,L);
	ew->show();
	delete[] items;
	return wrap_window(L,ew);
}

int new_window(lua_State* L)
{
	const wchar_t* caption = StringFromUTF8(luaL_checkstring(L,1));
	ContainerWindow* cw = new ContainerWindow(caption,L);
	s_last_parent = cw;
	return wrap_window(L,cw);
}

int new_panel(lua_State* L)
{
	PanelWindow* pw = new PanelWindow(L);
	pw->align(alLeft,luaL_checkinteger(L,1));
	s_last_parent = pw;
	return wrap_window(L,pw);
}

int window_client(lua_State* L)
{
	TEventWindow *cw = (TEventWindow*)window_arg(L,1);
	TWin* child = window_arg(L,2);
	if (! child) throw_error(L,L"must provide a child window");
	child->set_parent(cw);
	cw->set_client(child);
	return 0;
}

int window_add(lua_State* L)
{
	TEventWindow *cw = (TEventWindow*)window_arg(L,1);
	TWin* child = window_arg(L,2);
	const wchar_t *align = StringFromUTF8(luaL_optstring(L,3,"client"));
	bool splitter = optboolean(L,5,true);
	int sz = luaL_optinteger(L,4,100);
	child->set_parent(cw);
	if (EQ(align,L"top")) {
		child->align(alTop,sz);
    } else
	if (EQ(align,L"bottom")) {
		child->align(alBottom,sz);
	} else
	if (EQ(align,L"left")) {
		child->align(alLeft,sz);
	} else
	if (EQ(align,L"right")) {
		child->align(alRight,sz);
	} else {
		child->align(alClient,sz);
	}
	cw->add(child);
	if (splitter && child->align() != alClient) {
		TSplitter *split = new TSplitter(cw,child);
		cw->add(split);
		window_data(L,2) = split;
	}
	return 0;
}

int window_remove(lua_State* L)
{
	TEventWindow* form = (TEventWindow*)window_arg(L,1);
	form->remove(window_arg(L,2));
	TWin *split = (TWin*)window_data(L,2);
	if (split) {
		form->remove(split);
	}
	return 0;
}

int window_context_menu(lua_State* L)
{
	TWin* w = window_arg(L,1);
	if(LuaWindow *cw = dynamic_cast<LuaWindow*> (w)) {
		wchar_t** items = table_to_str_array(L,2);
		ContextMenu mnu(cw);
		for (;*items; items++) {
			wchar_t* text = wcstok(*items,L"|");
			wchar_t* fun = wcstok(NULL,L"");
			if ( ( text == 0 || *text == 0 )
				 && ( fun == 0 || *fun == 0 ) )
			{
				mnu.add_separator();
			}
			else
			{
				mnu << Item(text,(EH)&LuaWindow::handler,fun);
			}
		}
	} else if(TListViewLua* lv = dynamic_cast<TListViewLua*>(w)) {
		TEventWindow* parent = dynamic_cast<TEventWindow*>(lv->get_parent_win());
		wchar_t** items = table_to_str_array(L,2);
		HMENU hm = CreatePopupMenu();
		Popup mnu(hm);
		for (;*items; items++) {
			wchar_t* text = wcstok(*items,L"|");
			wchar_t* fun = wcstok(NULL,L"");
			if ( ( text == 0 || *text == 0 )
				 && ( fun == 0 || *fun == 0 ) )
			{
				mnu.add_separator();
			}
			else
			{
				mnu << Item(text,(EH)&LuaWindow::handler,fun);
			}
		}
		lv->set_popup_menu(hm);
		mnu.get_menu_handler()->set_form(parent);
		parent->add_handler(mnu.get_menu_handler());
	}

	return 0;
}

int new_tabbar(lua_State* L)
{
	TEventWindow* form = (TEventWindow*)window_arg(L,1);
	TTabControlLua *tab = new TTabControlLua(form,L);
	s_last_parent = form;
	tab->align(alTop);
	form->add(tab);
	return wrap_window(L,tab);
}

int tabbar_add(lua_State* L)
{
	TTabControl *tab = (TTabControl*)window_arg(L,1);
	tab->add(StringFromUTF8(luaL_checkstring(L,2)),window_arg(L,3));
	return 0;
}


int new_memo(lua_State* L)
{
	TWin* p = get_last_parent();
	if(p) {
		TMemoLua *m = new TMemoLua(p,L,1);
		return wrap_window(L,m);
	} else {
		luaL_error(L, "There is no parent panel to create 'gui.memo'");
		return 0;
	}
}

int memo_set(lua_State* L)
{
	TMemoLua *m = (TMemoLua*)window_arg(L,1);
	m->set_text(StringFromUTF8(luaL_checkstring(L,2)));
	return 0;
}

int memo_get(lua_State* L)
{
	wchar_t str[BUFSIZE];
	TMemoLua *m = (TMemoLua*)window_arg(L,1);
	m->get_text(str, BUFSIZE);
	lua_pushstring(L, UTF8FromString(str));
	return 1;
}

int memo_set_colour(lua_State* L)
{
	TMemoLua *m = (TMemoLua*)window_arg(L,1);
	m->set_text_colour(convert_colour_spec(luaL_checkstring(L,2))); // Must be only ASCII
	m->set_background_colour(convert_colour_spec(luaL_checkstring(L,3)));

	return 0;
}

int new_tree(lua_State* L)
{
	TWin* p = get_last_parent();
	if(p) {
		TTreeViewLua *lv = new TTreeViewLua((TEventWindow*)p,L);
		return wrap_window(L,lv);
	} else {
		luaL_error(L, "There is no parent panel to create 'gui.tree'");
		return 0;
	}
}


/** lw:list(multiple_columns,multiple_selection)
	@param multiple_columns (default false)
	@param single_selection (default true)
	@return new ListWindow instance.
*/
int new_list_window(lua_State* L)
{
	TWin* p = get_last_parent();
	if(p) {
		bool multiple_columns = optboolean(L,1,false);
		bool single_select = optboolean(L,2,true);
		TListViewLua *lv = new TListViewLua(p,L,multiple_columns,single_select);
		return wrap_window(L,lv);
	} else {
		luaL_error(L, "There is no parent panel to create 'gui.list'");
		return 0;
	}
}


/** w:show()
	@param self
*/
int window_show(lua_State* L)
{
	window_arg(L)->show();
	return 0;
}

/** w:hide()
	@param self
*/
int window_hide(lua_State* L)
{
	window_arg(L)->hide();
	return 0;
}

/** w:size()
	@param self
	@param width
	@param height
*/
int window_size(lua_State* L)
{
	int w = luaL_checkinteger(L,2);
	int h = luaL_checkinteger(L,3);
	window_arg(L)->resize(w,h);
	return 0;
}

/** w:size()
	@param self
	@param x
	@param y
*/
int window_position(lua_State* L)
{
	int x = luaL_checkinteger(L,2);
	int y = luaL_checkinteger(L,3);
	window_arg(L)->move(x,y);
	return 0;
}

int window_get_bounds(lua_State* L)
{
	TWin *win = (TWin*)window_arg(L);
	Rect rt;
	win->get_rect(rt);
	lua_pushboolean(L,win->visible());
	lua_pushinteger(L,rt.left);
	lua_pushinteger(L,rt.right);
	lua_pushinteger(L,rt.width());
	lua_pushinteger(L,rt.height());
	return 5;
}


TListViewLua* list_window_arg(lua_State* L)
{
//	ListWindow* lw = dynamic_cast<ListWindow*>(window_arg(L));
//	if (! lw) lua_error(L);
//	return lw->listv;
	return (TListViewLua*)(window_arg(L));
}

/** w:add_column()
	@param self
	@param column_title
	@param column_size
*/
int window_add_column(lua_State* L)
{
	list_window_arg(L)->add_column(StringFromUTF8(luaL_checkstring(L,2)),luaL_checkinteger(L,3));
	return 0;
}

void window_aux_item(lua_State* L, bool at_index)
{
	TWin* w = window_arg(L);
	if(TListViewLua* lv = dynamic_cast<TListViewLua*>(w)) {
		int next_arg,ipos;
		void *data = NULL;
		if (at_index) {
			next_arg = 3;
			ipos = luaL_checkinteger(L,2);
		} else {
			next_arg = 2;
			ipos = lv->count();
		}
		if (! lua_isnoneornil(L,next_arg+1)) {
			lua_pushvalue(L,next_arg+1);
			data = (void*)luaL_ref(L,LUA_REGISTRYINDEX);
		}
		if (lua_isstring(L,next_arg)) {
			lv->add_item_at(ipos,StringFromUTF8(luaL_checkstring(L,next_arg)),0,data);
		} else {
			wchar_t** items = table_to_str_array(L,next_arg);
			int i = 0, ncol = lv->columns();
			int idx = lv->add_item_at(ipos,*items,0,data);
			++items;
			++i;
			for(; *items && i < ncol; ++items) {
				lv->add_subitem(idx,*items,i);
				++i;
			}
		}
	} else if (TTreeViewLua* tv = dynamic_cast<TTreeViewLua*>(w)) {
		tv->add_item(StringFromUTF8(luaL_checkstring(L, 2)), 0, NULL);
	}
}

/** w:add_item()
	@param self
	@param item_string
*/
int window_add_item(lua_State* L)
{
	window_aux_item(L,false);
	return 0;
}

/** w:insert_item()
	@param self
	@param item_string
	@param index
*/
int window_insert_item(lua_State* L)
{
	window_aux_item(L,true);
	return 0;
}

/** w:delete_item()
	@param self
	@param index
*/
int window_delete_item(lua_State* L)
{
	list_window_arg(L)->delete_item(luaL_checkinteger(L,2));
	return 0;
}

int window_count (lua_State* L)
{
	int sz = list_window_arg(L)->count();
	lua_pushinteger(L,sz);
	return 1;
}

int window_set_colour(lua_State* L)
{
	TListViewLua* ls = list_window_arg(L);
	ls->set_foreground(convert_colour_spec(luaL_checkstring(L,2)));
	ls->set_background(convert_colour_spec(luaL_checkstring(L,3)));
	return 0;
}


/** w:get_item_text()
	@param self
	@param index
	@return item string
*/
int window_get_item_text(lua_State* L)
{
	wchar_t* buff = new wchar_t[BUFFER_SIZE];
	list_window_arg(L)->get_item_text(luaL_checkinteger(L,2),buff,BUFFER_SIZE);
	lua_pushstring(L,UTF8FromString(buff));
	delete[] buff;
	return 1;
}

int window_get_item_data(lua_State* L)
{
	void *data = list_window_arg(L)->get_item_data(luaL_checkinteger(L,2));
	lua_rawgeti(L,LUA_REGISTRYINDEX,(int)data);
	return 1;
}

int window_selected_item(lua_State* L)
{
	int idx =list_window_arg(L)->selected_id();
	lua_pushinteger(L,idx);
	return 1;
}

int window_selected_items(lua_State* L)
{
	TListViewLua* lv = list_window_arg(L);
	int i = -1;
	int idx = 0;
	lua_newtable(L);
	while(true) {
		i = lv->next_selected_id(i);
		if (i < 0) break;
		lua_pushinteger(L, ++idx);
		lua_pushinteger(L,i);
		lua_settable(L, -3);
	}

	return 1;
}

int window_selected_count(lua_State* L)
{
	int count =list_window_arg(L)->selected_count();
	lua_pushinteger(L,count);
	return 1;
}

int window_select_item(lua_State* L)
{
	list_window_arg(L)->select_item(luaL_checkinteger(L,2));
	return 0;
}

/** w:clear()
	@param self
*/
int window_clear(lua_State* L)
{
	list_window_arg(L)->clear();
	return 0;
}

/** w:autosize()
	@param self
	@param column_index
	@param by_contents (default false)
*/
int window_autosize(lua_State* L)
{
	list_window_arg(L)->autosize_column(luaL_checkinteger(L,2),optboolean(L,3,false));
	return 0;
}


/** w:on_select()
	@param self
	@param select_callback (string representing global function)
*/
int window_on_select(lua_State* L)
{
	LuaControl* lc = dynamic_cast<LuaControl*>(window_arg(L,1));
	lc->set_select(2);
	return 0;
}

/** w:on_double_click()
	@param self
	@param select_callback (string representing global function)
*/
int window_on_double_click(lua_State* L)
{
	list_window_arg(L)->set_double_click(2);
	return 0;
}

int window_on_close(lua_State* L)
{
	if(LuaWindow* cw = dynamic_cast<LuaWindow*>(window_arg(L)))
		cw->set_on_close(2);
	return 0;
}

int window_on_show(lua_State* L)
{
	if(LuaWindow* cw = dynamic_cast<LuaWindow*>(window_arg(L)))
		cw->set_on_show(2);
	return 0;
}

int window_on_focus(lua_State* L)
{
	LuaControl* lc = dynamic_cast<LuaControl*>(window_arg(L,1));
	lc->set_on_focus(2);
	return 0;
}

int window_on_key(lua_State* L)
{
	TWin* w = window_arg(L);
	if(TListViewLua* lv = dynamic_cast<TListViewLua*>(w)) {
		lv->set_onkey(2);
	} else if (TMemoLua* m = dynamic_cast<TMemoLua*>(w)) {
		m->set_onkey(2);
	}
	return 0;
}

int window_add_buttons(lua_State* L)
{
	ContainerWindow* lw = dynamic_cast<ContainerWindow*>(window_arg(L));
	lw->add_buttons(L);
	return 0;
}


static HWND hSciTE = NULL, hContent = NULL, hCode;
static WNDPROC old_scite_proc, old_scintilla_proc, old_content_proc;
static lua_State *sL;
static TWin *code_window = NULL;
static TWin *extra_window = NULL;
static TWin *content_window = NULL;
static TWin *extra_window_splitter = NULL;
static bool forced_resize = false;
static Rect m, cwb, extra;

static bool eq(const Rect& r1, const Rect& r2)
{
	return r1.left == r2.left && r1.top == r2.top && r1.right == r2.right && r1.bottom == r2.bottom;
}


static void force_contents_resize()
{
	// get the code pane's extents, and don't try to resize it again!
	code_window->get_rect(m,true);
	if (eq(cwb,m)) return;
	int oldw = m.width();
	int w = extra_window->width();
	int h = m.height();
	int sw = extra_window_splitter->width();
	extra = m;
	cwb = m;
	if (extra_window->align() == alLeft) {
		// on the left goes the extra pane, followed by the splitter
		extra.right = extra.left + w;
		extra_window->resize(m.left,m.top,w,h);
		extra_window_splitter->resize(m.left + w,m.top,sw,h);
		cwb.left += w + sw;
	} else {
		int margin = m.right - w;
		extra.left = margin;
		extra_window->resize(margin,m.top, w,h);
		extra_window_splitter->resize(margin-sw,m.top,sw,h);
		cwb.right -= w + sw;
	}
	// and then the code pane; note the hack necessary to prevent a nasty recursion here.
	forced_resize = true;
	code_window->resize(cwb);
	forced_resize = false;
}

static int call_named_function(lua_State* L, const char *name, int arg)
{
	int ret = 0;
	lua_getglobal(L,name);
	if (lua_isfunction(L,-1)) {
		lua_pushinteger(L,arg);
		if (lua_pcall(L,1,1,0)) {
			OutputMessage(L);
		} else {
			ret = lua_toboolean(L,-1);
			lua_pop(L,1);
		}
	}
	lua_pop(L,1);
	return ret;
}

// we subclass the main SciTE window proc mostly because we need to track whether
// SciTE is the active app or not, so that toolwindows can be hidden.
static LRESULT SciTEWndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if (iMessage == WM_ACTIVATEAPP) {
		PaletteWindow::set_visibility(wParam);
		call_named_function(sL,"OnActivate",wParam);
		if (wParam) { // floating toolbars may grab the focus, so restore it.
			code_window->set_focus();
		}
	} else
	if (iMessage == WM_CLOSE) {
		call_named_function(sL,"OnClosing",0);
	} else
	if (iMessage == WM_COMMAND) {
		if (call_named_function(sL,"OnCommand",LOWORD(wParam))) return TRUE;
	}
	return CallWindowProc(old_scite_proc,hwnd,iMessage,wParam,lParam);
}

// we are interested in any attempts to resize the main code pane, because we
// may wish to place our own pane on the left.
static LRESULT ScintillaWndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if (iMessage == WM_SIZE) {
		if (extra_window) {
			if (! forced_resize) {
				force_contents_resize();
			}
		}
	}
	if (IsWindowUnicode(hwnd)) {
		return CallWindowProcW(old_scintilla_proc,hwnd,iMessage,wParam,lParam);
	} else {
		return CallWindowProcA(old_scintilla_proc,hwnd,iMessage,wParam,lParam);
	}
}

// the content pane contains the two Scintilla windows (editor and output).
// This subclass prevents SciTE from forcing its dragger cursor onto our left pane.
static LRESULT ContentWndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if (iMessage == WM_SETCURSOR) {
		Point ptCursor;
		GetCursorPos(&ptCursor);
		Point ptClient = ptCursor;
		ScreenToClient(hContent, &ptClient);
		if (extra.is_inside(ptClient)) {
			return DefWindowProc(hSciTE,iMessage,wParam,lParam);
		}
	}
	return CallWindowProc(old_content_proc,hwnd,iMessage,wParam,lParam);
}

#ifndef GetWindowLongPtrW
#define GetWindowLongPtrW(hwnd,offs) (void*)GetWindowLongW(hwnd,offs)
#define GetWindowLongPtrA(hwnd,offs) (void*)GetWindowLongA(hwnd,offs)
#define SetWindowLongPtrW(hwnd,offs,newv) (void*)SetWindowLongW(hwnd,offs,newv)
#define SetWindowLongPtrA(hwnd,offs,newv) (void*)SetWindowLongA(hwnd,offs,newv)
#endif


static WNDPROC subclass(HWND hwnd, LONG_PTR newproc)
{
	WNDPROC old;
	if (::IsWindowUnicode(hwnd)) {
		old = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
		SetWindowLongPtrW(hwnd, GWLP_WNDPROC, newproc);
	} else {
		old = reinterpret_cast<WNDPROC>(GetWindowLongPtrA(hwnd, GWLP_WNDPROC));
		SetWindowLongPtrA(hwnd, GWLP_WNDPROC, newproc);
	}
	return old;
}

static void subclass_scite_window ()
{
	static bool subclassed = false;
	if (!subclassed) {  // to prevent a recursion
		old_scite_proc     = subclass(hSciTE,   (long)SciTEWndProc);
		old_content_proc   = subclass(hContent, (long)ContentWndProc);
		old_scintilla_proc = subclass(hCode,    (long)ScintillaWndProc);
		subclassed = true;
	}
}

// this terrible hack gets a SciTE resize by 'shaking the frame' by a little bit
static void shake_scite_window()
{
	static int delta = 1;
	Rect frt;
	s_parent->get_rect(frt,false);
	frt.right += delta;
	delta = - delta;
	s_parent->resize(frt);
}

// Unfortunately, shake_scite_window() does not work in the full screen mode.
// So here is yet another hack, which seems even more terrible than the first one,
// since its workability completely depends on WM_SIZE handler implementation
// in SciTEWin.
static void shake_scite_descendants()
{
	Rect frt;
	s_parent->get_rect(frt,false);
	s_parent->send_msg(WM_SIZE, SIZE_RESTORED, MAKELONG(frt.width(), frt.height()));
}

class SideSplitter: public TSplitterB
{
public:
	SideSplitter(TEventWindow *form, TWin *control)
		: TSplitterB(form,control,5)
	{}

	void paint(TDC& dc)
	{
		Rect rt(this);
		dc.rectangle(rt);
	}


	void on_resize(const Rect& rt)
	{
		TSplitterB::on_resize(rt);
		shake_scite_descendants();
	}
};

static int do_set_panel(lua_State *L)
{
	if (content_window == NULL) {
		lua_pushstring(L,"Window subclassing was not successful");
		lua_error(L);
	}
	if (! lua_isuserdata(L,1) && extra_window != NULL) {
		extra_window->hide();
		extra_window = NULL;
		extra_window_splitter->close();
		delete extra_window_splitter;
		extra_window_splitter = NULL;
		shake_scite_descendants();
	} else {
		extra_window = window_arg(L);
		const wchar_t *align = StringFromUTF8(luaL_optstring(L,2,"left"));
		if (EQ(align,L"left")) {
			extra_window->align(alLeft);
		} else
		if (EQ(align,L"right")) {
			extra_window->align(alRight);
		}
		extra_window->set_parent(content_window);
		extra_window->show();
		extra_window_splitter = new SideSplitter((TEventWindow*)content_window, extra_window);
		extra_window_splitter->show();
		force_contents_resize();
	}
	
	return 0;
}

static int do_chdir(lua_State *L)
{
	const char *dirname = luaL_checkstring(L,1);
	int res = _chdir(dirname);
	lua_pushboolean(L,res == 0);
	return 1;
}

static int append_file(lua_State *L, int idx,int attrib,bool look_for_dir, const wchar_t *value)
{
	if (((attrib & _A_SUBDIR) != 0) == look_for_dir) {
		if (look_for_dir && (EQ(value,L".") || EQ(value,L".."))) return idx;
		lua_pushinteger(L,idx);
		lua_pushstring(L,UTF8FromString(value));
		lua_settable(L,-3);
		return idx + 1;
	}
	return idx;
}

static int do_files(lua_State *L)
{
	struct _wfinddata_t c_file;
	const wchar_t *mask = StringFromUTF8(luaL_checkstring(L,1));
	bool look_for_dir = optboolean(L,2,false);
	long hFile = _wfindfirst(mask,&c_file);
	int i = 1;
	if (hFile == -1L) { lua_pushboolean(L,0); return 1; }
	lua_newtable(L);
	i = append_file(L,i,c_file.attrib,look_for_dir,c_file.name);
	while( _wfindnext( hFile, &c_file ) == 0) {
		i = append_file(L,i,c_file.attrib,look_for_dir,c_file.name);
	}
	return 1;
}

//  this will allow us to hand keyboard focus over to editor
static int do_pass_focus(lua_State *L)
{
	lua_getglobal(L,"editor");
	lua_pushboolean(L,1);
	lua_setfield(L,-2,"Focus");
	lua_pop(L,1);
	if (code_window) {
		code_window->set_focus();
	}
	return 0;
}

static const struct luaL_reg gui[] = {
	{"message",do_message},
	{"colour_dlg",do_colour_dlg},
	{"open_dlg",do_open_dlg},
	{"save_dlg",do_save_dlg},
	{"select_dir_dlg",do_select_dir_dlg},
	{"toolbar",new_toolbar},
	{"window",new_window},
	{"panel",new_panel},
	{"tabbar",new_tabbar},
	{"list",new_list_window},
	{"tree",new_tree},
	{"memo",new_memo},
	{"prompt_value",do_prompt_value},
	{"run",do_run},
	{"chdir",do_chdir},
	{"files",do_files},
	{"set_panel",do_set_panel},
	{"pass_focus",do_pass_focus},
	{NULL, NULL},
};

static const struct luaL_reg window_methods[] = {
	{"show",window_show},
	{"hide",window_hide},
	{"size",window_size},
	{"position",window_position},
	{"bounds",window_get_bounds},
	{"client",window_client},
	{"add",window_add},
	{"remove",window_remove},
	{"context_menu",window_context_menu},
	{"add_column",window_add_column},
	{"add_item",window_add_item},
	{"insert_item",window_insert_item},
	{"delete_item",window_delete_item},
	{"count",window_count},
	{"get_item_text",window_get_item_text},
	{"get_item_data",window_get_item_data},
	{"get_selected_item",window_selected_item},
	{"get_selected_items",window_selected_items},
	{"selected_count",window_selected_count},
	{"set_selected_item",window_select_item},
	{"on_select",window_on_select},
	{"on_double_click",window_on_double_click},
	{"on_close",window_on_close},
	{"on_show",window_on_show},
	{"on_focus",window_on_focus},
	{"on_key",window_on_key},
	{"clear",window_clear},
	{"autosize",window_autosize},
	{"add_buttons",window_add_buttons},
	{"add_tab",tabbar_add},
	{"set_text",memo_set},
	{"get_text",memo_get},
	{"set_memo_colour",memo_set_colour},
	{"set_list_colour",window_set_colour},
	{NULL, NULL},
};

BOOL CALLBACK CheckSciteWindow(HWND  hwnd, LPARAM  lParam)
{
	wchar_t buff[120];
    GetClassName(hwnd,buff,sizeof(buff));	
    if (wcscmp(buff,L"SciTEWindow") == 0) {
		*(HWND *)lParam = hwnd;
		return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK CheckContainerWindow(HWND  hwnd, LPARAM  lParam)
{
	wchar_t buff[120];
    GetClassName(hwnd,buff,sizeof(buff));	
    if (wcscmp(buff,L"SciTEWindowContent") == 0) {
		*(HWND *)lParam = hwnd;
		return FALSE;
    }
    return TRUE;
}

void destroy_windows()
{
	if (extra_window) {
		extra_window->hide();
		extra_window->set_parent(0);
		extra_window->close();
		delete extra_window;
		extra_window = 0;
	}
	if (extra_window_splitter) {
		extra_window_splitter->hide();
		extra_window_splitter->set_parent(0);
		extra_window_splitter->close();
		delete extra_window_splitter;
		extra_window_splitter = 0;
	}
	extra.bottom = extra.top = extra.left = extra.right = 0;
	shake_scite_descendants();
}

extern "C" __declspec(dllexport)
int luaopen_gui(lua_State *L)
{
	// at this point, the SciTE window is available. Can't always assume
	// that it is the foreground window, so we hunt through all windows
	// associated with this thread (the main GUI thread) to find a window
	// matching the appropriate class name
	EnumThreadWindows(GetCurrentThreadId(),CheckSciteWindow,(long)&hSciTE);
	s_parent = new TWin(hSciTE);
	sL = L;

	// Destroy window hierarchy created before.
	// And there are still memory and handle leaks.
	destroy_windows();
	
	// Its first child shold be the content pane (editor+output), 
	// but we check this anyway....	
	EnumChildWindows(hSciTE,CheckContainerWindow,(long)&hContent);
	// the first child of the content pane is the editor pane.
	bool subclassed = false;
	if (hContent != NULL) {
		content_window = new TWin(hContent);
		hCode = GetWindow(hContent,GW_CHILD);
		if (hCode != NULL) {
			code_window = new TWin(hCode);		
			subclass_scite_window();
			subclassed = true;
		}
	}
	if (! subclassed) {
		get_parent()->message(L"Cannot subclass SciTE Window",2);
	}
	luaL_openlib (L, "gui", gui, 0);
	luaL_newmetatable(L, WINDOW_CLASS);  // create metatable for window objects
	lua_pushvalue(L, -1);  // push metatable
	lua_setfield(L, -2, "__index");  // metatable.__index = metatable
	luaL_register(L, NULL, window_methods);
	return 1;
}
