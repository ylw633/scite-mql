/* TWL_CNTRLS.CPP
 * Steve Donovan, 2003
 * This is GPL'd software, and the usual disclaimers apply.
 * See LICENCE
 */
#include "memmap.h"
//#define NO_STRICT
#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include <richedit.h>
#define IN_CONTROLS_CPP
#include "twl_cntrls.h"

const int BUFFSIZE = 60000;

TButton::TButton(TWin *parent,pchar caption, int id, long style)
 : TControl(parent,L"button",caption,id,style)
 {
 }

void TButton::check(bool c)
{
 send_msg(BM_SETCHECK,(int)c);
}

bool TButton::check() const
{
 return send_msg(BM_GETCHECK);
}

void TButton::state(bool s)
{
 send_msg(BM_SETSTATE,(int)s);
}

bool TButton::state() const
{
 return send_msg(BM_GETSTATE);
}

TCheckBox::TCheckBox(TWin *parent,pchar caption, int id, bool is_auto)
: TButton(parent,caption,id, 
   is_auto ? TButton::AUTOCHECKBOX : TButton::CHECKBOX)
{
}

void TCheckBox::calc_size()
{
  int cx,cy;
  TDC *dc = m_parent->get_dc();
  // If the parent was a TComboBox, then it won't have a DC ready...
  if (dc != NULL) {
	  dc->get_text_extent(get_text(),cx,cy,m_font);
	  resize(int(1.05*cx)+30,int(1.05*cy));
  }
}


TRadioButton::TRadioButton(TWin *parent,pchar caption, int id, bool is_auto)
: TButton(parent,caption,id,
   is_auto ? TButton::AUTORADIOBUTTON : TButton::RADIOBUTTON)
{
}

TListBox::TListBox(TWin *parent, int id, bool is_sorted)
 : TControl(parent,L"listbox",L"",id,
    LBS_NOTIFY | WS_VSCROLL | WS_BORDER | (is_sorted? LBS_SORT : 0))
{
}

void TListBox::add(pchar str, void* ptr)
{ 
    send_msg(LB_ADDSTRING,0,(int)str);
    if (ptr) {
        set_data(count()-1,ptr);
    }
}

void TListBox::set_data(int i, void* ptr)
{ send_msg(LB_SETITEMDATA,i,(long)ptr); }

void* TListBox::get_data(int i)
{ return (void *)send_msg(LB_GETITEMDATA,i); }

void TListBox::insert(int i, pchar str)
{ send_msg(LB_INSERTSTRING,i,(int)str); }

void TListBox::remove(int i)
{ send_msg(LB_DELETESTRING,i); }

void TListBox::clear()
{ send_msg(LB_RESETCONTENT);   }

void TListBox::redraw(bool on)
{ send_msg(WM_SETREDRAW,on? TRUE : FALSE); }

int  TListBox::count()
{ return send_msg(LB_GETCOUNT);   }

void TListBox::selected(int idx)
{ send_msg(LB_SETCURSEL,idx);  }

int  TListBox::selected() const
{ return send_msg(LB_GETCURSEL); }

void TListBox::get_text(int idx, wchar_t *buff)
{ send_msg(LB_GETTEXT,idx,(int)buff); }

void *ApplicationInstance();

TMemo::TMemo(TWin *parent, int id, bool do_scroll, bool plain)
: TControl(parent,plain ? L"edit" : L"RichEdit20A",L"",id,
    (do_scroll ? WS_HSCROLL | WS_VSCROLL : 0) |
	WS_BORDER | 
	ES_MULTILINE 
	 | /*ES_AUTOHSCROLL  |*/ ES_AUTOVSCROLL | ES_LEFT 
   ), m_file_name(NULL)
{
	if(!plain) {
		m_pfmt = new CHARFORMAT;
		m_pfmt->cbSize = sizeof(CHARFORMAT);
		m_pfmt->dwMask = 0;
		m_pfmt->dwEffects = 0;
    }
	send_msg(EM_SETEVENTMASK, 0, ENM_KEYEVENTS);
}

void TMemo::set_font(pchar facename, int size, int flags, bool selection)
{
  m_pfmt->dwMask = CFM_FACE | CFM_BOLD | CFM_ITALIC;
  lstrcpy(m_pfmt->szFaceName,facename);
  m_pfmt->dwEffects = 0;
  if (flags & BOLD) m_pfmt->dwEffects = CFE_BOLD;
  if (flags & ITALIC) m_pfmt->dwEffects |= CFE_ITALIC;
  send_char_format();
  m_pfmt->dwMask = 0;
  m_pfmt->dwEffects = 0;
  send_msg(EM_SETMARGINS,EC_LEFTMARGIN | EC_USEFONTINFO,5);
}

pchar TMemo::file_name()
{ return m_file_name; }

void TMemo::cut()
{ send_msg(WM_CUT); }

void TMemo::copy()
{ send_msg(WM_COPY); }

void TMemo::clear()
{ send_msg(WM_CLEAR); }

void TMemo::paste()
{ send_msg(WM_PASTE); }

void TMemo::undo()
{ send_msg(EM_UNDO);  }

int TMemo::text_size()
{ return send_msg(WM_GETTEXTLENGTH); }

void TMemo::replace_selection(pchar str)
{ send_msg(EM_REPLACESEL,TRUE,(LPARAM)str); }

bool TMemo::modified()
{ return send_msg(EM_GETMODIFY);  }

void TMemo::modified(bool yesno)
{ send_msg(EM_SETMODIFY,yesno ? TRUE : FALSE); }

int TMemo::line_count()
{ return send_msg(EM_GETLINECOUNT); }

int TMemo::line_offset(int line)
{ return send_msg(EM_LINEINDEX,line); }

int TMemo::line_size(int line)
{ return send_msg(EM_LINELENGTH,line); }

int TMemo::get_line_text(int line, char* buff, int sz)
{
    *(short *)(void *)buff = (short)sz;
    int len = send_msg(EM_GETLINE,line,(LPARAM)buff);
    buff[len] = '\0';
    return len;
}

void TMemo::get_selection(int& start, int& finish)
{  send_msg(EM_GETSEL,(WPARAM)&start, (LPARAM)&finish); }

void TMemo::set_selection(int start, int finish)
{ send_msg(EM_SETSEL,start,finish); }

void TMemo::select_all()
{
   set_selection(0,text_size());
}

void TMemo::go_to_end()
{
  set_selection(text_size(),text_size());
}

void TMemo::scroll_line(int line)
{
  send_msg(EM_LINESCROLL,0,line);
}

int TMemo::line_from_pos(int pos)
{
  return send_msg(EM_LINEFROMCHAR,pos,0);
}

void TMemo::scroll_caret()
{
	send_msg(EM_SCROLLCARET);
}

void TMemo::auto_url_detect(bool yn)
{
  send_msg(EM_AUTOURLDETECT,(int)yn,0);
}

static wchar_t buff[BUFFSIZE]; //*TEMPORARY*

//*SJD* Must understand this shit!!
// In the meantime, it keeps a buffer!!
wchar_t *
TMemo::get_buffer()
{ 
	get_text(buff,BUFFSIZE);
	return buff;
 } 

void TMemo::release_buffer()
{
 //LocalUnlock((HANDLE)send_msg(EM_GETHANDLE));
}

bool TMemo::load_from_file(pchar file)
{
/*  DEBUG
   ifstream in(file);
   if (! in) return false;
   m_file_name = strdup(file);
   in.read(buff,BUFFSIZE);
   set_text(buff);
   return true;
*/
    return false;
}


void TMemo::save_to_file(pchar file)
{  
/*
   ofstream out(file);
   out.write(get_text(buff,BUFFSIZE),BUFFSIZE);
   modified(false);
*/
}

void TMemo::send_char_format()
{
 send_msg(EM_SETCHARFORMAT,SCF_SELECTION,(int)m_pfmt);
}

void TMemo::find_char_format()
{
 send_msg(EM_GETCHARFORMAT,SCF_SELECTION,(int)m_pfmt);
}

int  TMemo::get_text_colour()
{
  m_pfmt->dwMask = CFM_COLOR;
  find_char_format();
  m_pfmt->dwMask = 0;
  m_pfmt->dwEffects = 0;
  return (int)m_pfmt->crTextColor;
}

void TMemo::set_text_colour(int colour)
{
  m_pfmt->dwMask = CFM_COLOR;
  m_pfmt->crTextColor = (COLORREF)colour;
  send_char_format();
  m_pfmt->dwMask ^= CFM_COLOR;
}

void TMemo::set_background_colour(int colour)
{
	send_msg(EM_SETBKGNDCOLOR,0,(LPARAM)colour);
}

void TMemo::go_to(int idx1, int idx2, int nscroll)
{
	if (idx2==-1) idx2 = idx1;
	set_focus();
	set_selection(idx1,idx2);
    scroll_caret();
	scroll_line(nscroll); //*SJD* Should have an estimate of the page size!!
}

int TMemo::current_pos()
{
    int start, finish;
	get_selection(start,finish);
	return start;
}

int TMemo::current_line()
{ 
    return line_from_pos(current_pos())+1;
}
    
void TMemo::go_to_line(int line)
{
	int ofs = line_offset(line-1);
	go_to(ofs,-1,current_line() > line ? -10 : +10);
}

int TMemo::get_line_colour(int l)
{
    int offs = line_offset(l);
    set_selection(offs,offs);
    return get_text_colour();
}


void TMemo::set_line_colour(int line, int colour)
{
	int old = current_pos();
	int ofs1 = line_offset(line-1), ofs2 = line_offset(line);
	set_selection(ofs1,ofs2);
	set_text_colour(colour);
	set_selection(old,old);
}

int TMemo::handle_notify(void*p)
{
	LPNMHDR np  = (LPNMHDR)p;
	switch(np->code) {
		case EN_MSGFILTER:
		MSGFILTER* msf = (MSGFILTER*)p;
		switch(msf->msg) {
			case WM_RBUTTONDOWN: // For future use
				return 0;
			case WM_KEYDOWN:
				return (bool) handle_onkey(msf->wParam);
		}
	}
	return 0;
}


TTrackBar::TTrackBar(TWin *parent, int id)
: TControl(parent,TRACKBAR_CLASS,L"",id,TBS_AUTOTICKS)
{
}

void TTrackBar::selection(int lMin, int lMax)
{ send_msg(TBM_SETSEL, m_redraw, MAKELONG(lMin,lMax)); }

void  TTrackBar::sel_start(int lStart)
{ send_msg(TBM_SETSELSTART, m_redraw, lStart);   }

int   TTrackBar::sel_start() // returns starting pos of current selection
{ return send_msg(TBM_GETSELSTART); }

int   TTrackBar::sel_end() // returns end pos
{ return send_msg(TBM_GETSELEND); }

void   TTrackBar::sel_clear()
{ send_msg(TBM_CLEARSEL, m_redraw,0); }

int  TTrackBar::pos()
{ return send_msg(TBM_GETPOS);   }

void TTrackBar::pos(int lPos)
{ send_msg(TBM_SETPOS,TRUE,lPos); }

void TTrackBar::range(int lMin, int lMax) 
{ send_msg(TBM_SETRANGE, m_redraw, MAKELONG(lMin,lMax));  }


// Subclassing the controls!!
const int MIN_ID = 300;
WNDFN FormWndProc (HWND hwnd, UINT msg, UINT wParam,LONG lParam);

void subclass_control(TControl *ctrl)
{
 HWND hwnd = (HWND)ctrl->handle();
 ctrl->m_wnd_proc = (void *)GetWindowLong(hwnd,GWL_WNDPROC);
 SetWindowLong(hwnd,GWL_WNDPROC,(long)FormWndProc);
}

void remove_subclass_control(TControl *ctrl)
{
 if (ctrl->m_wnd_proc != NULL)
  SetWindowLong((HWND)ctrl->handle(),GWL_WNDPROC,(long)ctrl->m_wnd_proc);
}

WNDFN FormWndProc (HWND hwnd, UINT msg, UINT wParam,LONG lParam)
{
 TControl *ctrl = TControl::user_data(hwnd);
 HWND hNext, hDlg = GetParent(hwnd); 
 TEventWindow *ew = (TEventWindow *) GetWindowLong(hDlg,0);
 switch (msg) {
  case WM_KEYDOWN:
   if (wParam==VK_TAB) {
     bool shift_down = GetKeyState(VK_SHIFT) < 0;
	 int id = GetWindowLong(hwnd,GWL_ID);
	 if (shift_down) {
	    if (id > MIN_ID)id--;
		hNext = GetDlgItem(hDlg,id);
	 } else {
	   id++;
	   hNext = GetDlgItem(hDlg,id);
	   if (!hNext) hNext = GetDlgItem(hDlg,MIN_ID);
     }
     SetFocus(hNext);
   } else if (wParam==VK_RETURN) {
	   PostMessage(hDlg,WM_COMMAND,IDOK,0);
	   return 0;
   } else if (wParam==VK_ESCAPE) {
       PostMessage(hDlg,WM_COMMAND,IDCANCEL,0);
	   return 0;
   }
   break;
  case WM_SETFOCUS:
    if (ctrl->is_type(L"TEdit"))
         ctrl->send_msg(EM_SETSEL,0,-1);
    break;
 }
 long ret = CallWindowProc(WNDPROC(ctrl->m_wnd_proc),hwnd,msg,wParam,lParam);
 if (ew && ew->child_messages()) {
  if (msg == WM_KEYDOWN || msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK)
      ew->send_msg(msg,wParam,lParam);
 } 
 return ret;
}


class _Cntrls_init {
	HMODULE hLib;
public:
	_Cntrls_init() {
		hLib = LoadLibrary(L"RICHED20.DLL");
    }
	~_Cntrls_init() {
		FreeLibrary(hLib);
    }
};

_Cntrls_init _init_;
