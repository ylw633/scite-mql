/* TWL.H
 * Steve Donovan, 2003
 * This is GPL'd software, and the usual disclaimers apply.
 * See LICENCE
*/
#ifndef __TWL_H
#define __TWL_H

typedef const wchar_t *pchar;

#if !defined(_WINDOWS_) && !defined(_WINDOWS_H)
// some constants are fairly essential...
#define WM_COMMAND	0x0111
#define MB_OK                       0x0000
#define MB_OKCANCEL                 0x0001
#define MB_ABORTRETRYIGNORE         0x0002
#define MB_YESNOCANCEL              0x0003
#define MB_YESNO                    0x0004
#define MB_RETRYCANCEL              0x0005
#define MB_ICONHAND                 0x0010
#define MB_ICONQUESTION             0x0020
#define MB_ICONEXCLAMATION          0x0030
#define MB_ICONASTERISK             0x0040
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#define SW_SHOW 5
#define SW_HIDE 0
#define WS_EX_TOOLWINDOW        0x00000080L
#define WS_EX_TOPMOST           0x00000008L
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_PALETTEWINDOW     (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)
#ifndef NULL
#  define NULL 0
#endif
  struct tagPOINT { int x,y; };
  typedef tagPOINT POINT;
  struct tagSIZE { int cx, cy; };
  typedef tagSIZE SIZE;
  struct RECT {
	long left,top,right,bottom;
  };
  typedef int ChildList;
#else
//#include <list>
  #include "list"
  class TWin;
  typedef std::list<TWin*> ChildList;
#endif

#define WM_USER_PLUS 0x9999

#ifndef __UNDERC__
// #define EXPORT __declspec(dllexport)	--[http://forum.ru-board.com/topic.cgi?forum=2&topic=3339&start=400#20]
#define EXPORT
#define DLGFN EXPORT BOOL __stdcall
#define WNDFN EXPORT long __stdcall
#else
#define EXPORT
#define DLGFN __stdcall int
#define WNDFN __stdcall long
#endif
typedef void *Handle;

struct EXPORT Point : POINT{
	Point() { x = 0; y = 0; }
  Point(int xp, int yp) { x = xp; y = yp; }
  void set(int xp, int yp) { x = xp; y = yp; }
};
class TEventWindow;
struct EXPORT Rect : RECT{
	enum Corner { TOP_LEFT,TOP_RIGHT,BOTTOM_RIGHT, BOTTOM_LEFT };
	Rect() { }
	Rect(const TEventWindow* pwin);
	Rect(int x0, int y0, int x1, int y1)
	{ left = x0; top = y0; right = x1;  bottom = y1; }
	bool is_inside(const Point& p) const;
	Point corner(int idx) const;
	long width() const;
	long height() const;
	void offset_by(int dx, int dy);
};
struct RectD {
	double left,top,right,bottom;
};
typedef unsigned long dword;

// warnings about 'int' to 'bool' conversions can be suppressed
#pragma warning (disable: 4800)

#define BUFSIZE 120
#define ID_ABOUT	103

extern wchar_t obuff[BUFSIZE];

const int TOPLEFT = 1, MSG_ERROR=2,MSG_WARNING=1, MSG_QUERY=3;
const long TOPMOST = 0x8, TOOLWINDOW = 0x80;
#define PARENT_WND ((TWin*)0x1)

enum Alignment { alNone, alTop, alBottom, alLeft, alRight, alClient };

class EXPORT TWin {
// a basic wrapper for a HWND
public:
  TWin ()
    {set(NULL);}
  TWin (Handle hwnd)
    {set(hwnd);}
  void set(Handle hwnd) 
    { m_hwnd = hwnd; m_align = alNone; }
  Handle handle() 
   { return m_hwnd; }

  TWin(TWin *parent, pchar winclss, pchar text, int id, dword styleEx=0);
  TWin *create_child(pchar winclss, pchar text, int id, dword styleEx=0);

  virtual void update();
  void  invalidate(Rect *lprt=NULL);
  void  get_client_rect(Rect &rt) const;
  void  get_rect(Rect &rt, bool use_client=false);
  int   width();
  int   height();
  void  set_text(pchar str);
  pchar get_text(wchar_t* str=obuff,int sz=BUFSIZE);
  void  set_text(int id, pchar str);
  pchar get_text(int id, wchar_t* str=obuff, int sz=BUFSIZE);
  void  set_int(int id, int val);
  int   get_int(int id);
  TWin *get_twin(int id);
  TWin *get_active_window();
  int   get_id();
  void  set_focus();
  void  mouse_capture(bool do_grab);
  void  resize(int x0, int y0, int w, int h);
  void  resize(const Rect& rt);
  void  resize(int w, int h);
  void  move(int x0, int y0);
  void  map_points(Point* pt, int n, TWin* target_wnd = PARENT_WND);
  void  on_top();  
  void  to_foreground();
  virtual void  show(int how = SW_SHOW);
  //*new
  virtual void  hide();
  bool  visible();
  void  set_parent(TWin* w);
  void  set_style(dword s);
  int   send_msg(int msg, int wparam=0, int lparam=0) const;
  int   send_cmd(int id,  int lparam=0)
	{ return send_msg(WM_COMMAND,id,lparam); }
  void  close();
  int message(pchar msg, int type=0);
  Alignment align() { return m_align; }
  void align(Alignment a, int size = 0);
  static TWin* get_foreground_window();
protected:
  Handle m_hwnd;
  Alignment m_align;
};

//int message_box(pchar txt, pchar title, int style=MB_OK);
//EXPORT int Message(char *format);

class EXPORT TGDIObj {
  public:
	 TGDIObj() { m_hand = NULL; };
	 //~TGDIObj();
	 void  set_handle(Handle hand)
	  { m_hand = hand; }
	 Handle handle()
	  { return m_hand; }
    void destroy();
  protected:
	Handle m_hand;
};

enum { NORMAL, BOLD=2, ITALIC=4 };

class EXPORT TFont: public TGDIObj {
  public:
   TFont();
   ~TFont();
   void create();
  // TFont& operator = (pchar spec);
   TFont& operator = (const TFont& f);
    void  set(pchar name, int sz, int ftype=NORMAL);
    void  set_name(pchar name);
    void  set_size(int pts);
    void  set_bold();
    void  set_italic();
protected:
    void *m_pfont;
};


///// Wrapping up the Windows Device Context
class EXPORT TDC {
 public:
  TDC();
  ~TDC();
  void   set_hdc(Handle hdc)
  { m_hdc = hdc; }

  Handle get_hdc()
  { return m_hdc; }

  void set_twin(TWin* w)
  { m_twin = w; }

  void get(TWin *pw=NULL);
  void release(TWin *pw=NULL);
  void kill();
  Handle select(Handle obj);
  Handle select(TGDIObj& obj)
    { return select(obj.handle()); }   
  void select_stock(int val);

  void xor_pen(bool on_off);
  // this changes both the _pen_ and the _text_ colour
  void set_colour(float r, float g, float b);
  // wrappers around common graphics calls
  void set_text_align(int flags);
  void get_text_extent(pchar text, int& w, int& h,TFont *font=NULL);
  void draw_text(pchar msg);
  void move_to(int x, int y);
  void line_to(int x, int y);
  void rectangle(const Rect& rt);
  void polyline(Point* pts, int npoints);
  void draw_focus_rect(const Rect& rt);
  void draw_line(const Point& p1, const Point& p2);

 protected:
	Handle m_hdc, m_pen, m_font, m_brush;
	int m_flags;
	TWin *m_twin;
 };

class TClientDC: public TDC {
public:
   TClientDC(TWin* w)
    {  
      set_twin(w);
      get();
    }

    ~TClientDC()
    {
       release();
    }
};


class AbstractMessageHandler;

class EXPORT TNotifyWin: public TWin {
public:
    virtual int handle_notify(void*p) = 0;
};

enum CursorType { RESTORE, ARROW, HOURGLASS, SIZE_VERT, SIZE_HORZ, CROSS, HAND, UPARROW };

class EXPORT TEventWindow: public TWin
{
    friend WNDFN WndProc (Handle, unsigned int, unsigned int, long);
    bool m_do_resize;
    POINT m_fixed_size;
    AbstractMessageHandler *m_dispatcher;
    Handle m_accel;
    Handle m_hpopup_menu;
    TDC* m_dc;
    bool m_child_messages;
	long m_style_extra;
    Handle m_old_cursor;
    ChildList* m_children;
    TWin* m_tool_bar;
 public:

	 enum { TM_CHAR_HEIGHT=1, TM_CHAR_WIDTH,
		  TM_CAPTION_HEIGHT, TM_MENU_HEIGHT,TM_CLIENT_EXTRA,
		  TM_SCREEN_WIDTH, TM_SCREEN_HEIGHT,
		  TM_END
	 };     

     POINT fixed_size();
     void enable_resize(bool do_resize, int w=0, int h=0);
     bool cant_resize();
     TDC* get_dc()                                { return m_dc; }
     bool child_messages()                        { return m_child_messages; }
     void view_child_messages(bool yesno)         { m_child_messages = yesno; }
     virtual void client_resize(int cwidth, int cheight);
     TEventWindow()                               { set_defaults(); }
     TEventWindow(pchar caption, TWin *parent=0, int style_extra = 0, int is_child = false, int style_override = -1);
     virtual ~TEventWindow( );

     void add(TWin* win);
	 void remove(TWin* win);
     TWin *get_client()   	{ return m_client; }
	 void set_client(TWin *cli, bool do_subclass=false);

     void set_defaults();
     void set_window();
     void add_accelerator(Handle accel);
     void set_icon(pchar file);
	 void set_icon_from_window(TWin *win);
     void start_child();
     bool next_child(TWin*& win);
     bool check_notify(long lParam, int& ret);
     void create_window(pchar caption, TWin *parent, bool is_child);
     void create_timer(int msec);
     void kill_timer();
     void set_menu(pchar res);
	 void set_menu(Handle menu);
     void set_popup_menu(Handle menu);
     void last_mouse_pos(int& x, int& y);
     Handle get_menu()                                  { return m_hmenu;      }
     void check_menu(int id, bool check);
     void set_toolbar(TWin* tb, TNotifyWin* tth);
     TWin* toolbar()                                { return m_tool_bar;   }
     void add_handler(AbstractMessageHandler *m_hand);
     AbstractMessageHandler *get_handler()              { return m_dispatcher; }
     void update_data();
     void update_controls();
     void quit(int retcode=0);
     int metrics(int ntype);
     void cursor(CursorType curs);
     int run();

//-----------Event Handling-------------------
  //virtual void show() {}
  virtual void size();
  virtual void paint(TDC&);
  virtual bool query_close() { return true; }
  virtual void on_close() { }
  virtual void on_showhide(bool show) { }
  virtual bool activate(bool yes) { return true; }
  virtual void on_select(Rect& rt) {}

 // input
  virtual void keydown(int vkey);

 // mouse messages
  virtual void mouse_down(Point& pt);
  virtual void mouse_move(Point& pt);
  virtual void mouse_up  (Point& pt);
  virtual void right_mouse_down(Point& pt);

 // scrolling
  //virtual void hscroll(int code, int posn);
  //virtual void vscroll(int code, int posn);

 // command
  virtual bool command(int id, int code);
  virtual bool sys_command(int id);
  virtual int  notify(int id, void* ph);
  virtual int  handle_user(long lparam, long wparam);

 // other
  virtual void timer();
  virtual void focus();
  virtual void destroy();
  void    show(int cmd_show=0);

  void set_background(float r, float g, float b); // how to spec. colour??

 protected:
	 Handle m_bkgnd_brush, m_hmenu;
	 long m_style, m_timer;
	 long m_bk_color;
     TWin *m_client;
 };

 class EXPORT AbstractMessageHandler {
 protected:
   TEventWindow *m_form;
 public:
   AbstractMessageHandler(TEventWindow *form)
    : m_form(form) { }
   virtual ~AbstractMessageHandler() { }
   void set_form(TEventWindow *form) { m_form = form; }
   virtual bool dispatch(int id, int notify, Handle ctrl) = 0;
   virtual void add_handler(AbstractMessageHandler *) = 0;
   virtual void write()=0;
   virtual void read()=0;
 };

 class EXPORT TFrameWindow: public TEventWindow {
  public:
	 TFrameWindow(pchar caption=L"twl", bool has_status=false, TWin *cli=0);
	 ~TFrameWindow();

     // overrides
    void destroy();
    void client_resize(int cwidth, int cheight);

    void set_status_fields(int* parts = NULL, int n = 0);
    void set_status_text(int id, pchar txt);

protected:
	 TWin* m_status;
 };

 typedef TEventWindow TEW;  //...common alias for EventWindow!

 //////// Wrappers around Windows Controls //////

 class EXPORT TControl: public TWin {
 public:
 	 void *m_wnd_proc;  //*TEMP*
	 TControl(TWin *parent,pchar classname, pchar text,int id=-1, long style=0);
	 ~TControl();
	 virtual pchar type_name()                   { return L"TControl"; }
	 virtual void calc_size();
	 bool is_type(pchar tname);
	 void set_font(TFont *fnt);
	 void set_colour(float r, float g, float b);
	 long get_colour() const                     { return m_colour; }
     TEventWindow *parent() const 	             { return m_parent; }
     void parent(TEventWindow *ew)       	     { m_parent = ew; }
	 void data(void *data)                       { m_data = data; }
	 void *data()                                { return m_data; } 

	  static TControl *user_data(Handle handle);	  
 protected:
	 long m_colour;
	 TFont *m_font;
	 TEventWindow *m_parent;
	 void *m_data;
 };

 class EXPORT TLabel: public TControl {
 public:
	 TLabel(TWin *parent,pchar text, int id=-1);
	 pchar type_name() { return L"TLabel"; }
 };

 class EXPORT TEdit: public TControl {
 public:
     TEdit(TWin *parent, pchar text, int id, long style=0);
	 pchar type_name() { return L"TEdit"; }
     void set_selection(int start, int finish);
 };


 ////////// Dialog Box Class///////////////////////////////
class EXPORT TDialog: public TWin
{
 public: 
  TDialog (pchar dlg_name,TWin *owner_win=NULL)
  {
    m_name = dlg_name;
    m_owner = owner_win;
    is_modeless(false);
  }
  ~TDialog ();

  void go (); 
  bool was_successful(); 
  bool modeless()
   { return (bool)m_modeless; }
  void is_modeless(bool which=true)
   { m_modeless = which; }

  TWin *get_owner()
   { return m_owner; }

//...virtual event handlers......
  virtual bool init()       {return true;}
  virtual bool command(int) {return true;}
  virtual bool finis()      {return true;}

  TWin *field(int id);

 protected:
   void* m_lpfnBox;
   int m_modeless, m_ret;
   const wchar_t* m_name;
   TWin *m_owner;
};

 EXPORT int exec(pchar s, int mode=SW_SHOW);
 EXPORT void get_app_path(wchar_t* buff, int sz);
 EXPORT long make_rgb(int r, int g, int b);
 EXPORT void debug_output(char* str);
 
#ifndef __UNDERC__
 int main(int argc, char **argv);
 int get_argc();
 char **get_argv();
#endif


#endif
