// TWL_LAYOUT.H
#ifndef __TWL_LAYOUT_H
#define __TWL_LAYOUT_H

#include "twl.h"
#include "twl_menu.h"
#include "twl_cntrls.h"
#include "twl_data.h"

// Field inserts a TLabel followed by a TEdit
 struct EXPORT Field: public Item {
	 
  Field(pchar caption, float& val, EventHandler eh=NULL) {
     field(caption,new FloatData(&val),eh);
  }

  Field(pchar caption, double& val, EventHandler eh=NULL) {
     field(caption,new DoubleData(&val),eh);
  }

  Field(pchar caption, int& val, EventHandler eh=NULL) {
     field(caption,new IntData(&val),eh);
  }

  Field(pchar caption, short& val, EventHandler eh=NULL) {
     field(caption,new ShortData(&val),eh);
  }

  Field(pchar caption, unsigned int& val, EventHandler eh=NULL) {
     field(caption,new UIntData(&val),eh);
  }

  Field(pchar caption, wchar_t* val, EventHandler eh=NULL) {
     field(caption,new StringData(val),eh);
  }


  Field() {}

  void field(pchar _caption, Data *_data, EventHandler _eh) {
	  caption = _caption;  
	  data = _data;
	  handler = _eh;
	  id = -1;  // i.e. let these be allocated automatically...
  }

  Field(const Field& f)
  {
    caption = f.caption;
    data = f.data;
    handler = f.handler;
    id = f.id;
  }
 };

 struct EXPORT Checkbox: public Field {
	 Checkbox(pchar caption, int& val,EventHandler eh=NULL) {
		 field(caption, new IntData(&val),eh);
     };
 	 Checkbox(pchar caption, short& val,EventHandler eh=NULL) {
		 field(caption, new ShortData(&val),eh);
     };
  	 Checkbox(pchar caption, bool& val,EventHandler eh=NULL) {
		 field(caption, new BoolData(&val),eh);
     };

 };


 struct EXPORT Control: public Item {
	 Control(TControl *ctrl,EventHandler eh=NULL) {
		 data = ctrl;  handler = eh;  id = ctrl->get_id();
     }
 };

 struct EXPORT Button: public Item {
	 Button(pchar _caption, EventHandler _eh, void* _data = NULL) {
       caption = _caption;
	   handler = _eh;
	   data = _data;
	   id = -1;
	   inactive_data = data != NULL;
	 }
 };

 struct EXPORT OKButton: public Button {
	 OKButton(EventHandler _eh) 
		 : Button(L"&OK",_eh) { id = ID_OK; }
 };

 struct EXPORT CancelButton: public Button {
	 CancelButton(EventHandler _eh)
		 : Button(L"&Cancel",_eh) { id = ID_CANCEL; }
 };

 struct EXPORT Listbox: public Item {
	 int width,height;
	 Listbox(int _height, EventHandler _eh=NULL) {
		 height = _height;  // a fiddle!!
		 handler = _eh;
		 data = NULL;
     };
	 Listbox(int _height, char* val, EventHandler _eh=NULL) {
	 height = _height;  
		 handler = _eh;
		 data = new StringData(val);
     };
 	 Listbox(int _height, int& val, EventHandler _eh=NULL) {
	     height = _height;  // a fiddle!!
		 handler = _eh;
		 data = new IntData(&val);
     };

 };

 // Manipulators are used to control the layout 
 struct EXPORT LayoutManipulator {
	 int type;
	 LayoutManipulator(int _type) : type(_type) { }
 };

 struct EXPORT NamedAs {
	 TControl **ctrl_ref;
	 NamedAs(void * ptr) { ctrl_ref = (TControl **)ptr; }
 };

 struct EXPORT Space {
     int x, y;
     Space(int _y, int _x = -1) { x = _x; y = _y; }
 };

 struct EXPORT Width {
     int width;

     Width(int _w) { width = _w; }
 };
         


#ifndef __UNDERC__
 extern LayoutManipulator Next,End,Center,LeftAlign,Margin;
#else
 LayoutManipulator Next(1),End(2),Center(3),LeftAlign(4),Margin(5);
#endif

 class EXPORT Layout {
 protected:
  int m_left,m_top, m_xpos, m_ypos,m_xspacing, m_yspacing;
  int m_width, m_height, m_last_id;
  int m_col, m_row, m_max_col;
  TControl *m_last_ctrl;
  TEventWindow *m_form;
  TWin *m_parent;
  MessageHandler *m_ctrl_handler;
  bool m_released;

 public:
  Layout(TEventWindow *form=NULL,TEventWindow *mform=NULL);
  virtual ~Layout();
  void set_form(TEventWindow *form,TEventWindow *mform=NULL);
  void set_yspacing(int s)          { m_yspacing = s; }
  void set_xspacing(int s)          { m_xspacing = s; }
  void set_position(int x, int y)   { m_left = x; m_ypos = y; }
  static void set_margins(int tm, int lm);
  TControl *add(TControl *ctrl, bool new_line=true);
  void next_row();
  void center_row();
  void left_align(int row);
  void field_width(int w);
  void space(int yy, int xx=-1);
  void add_label(pchar caption, bool new_line=true);
  void add_ctrl(TControl *ctrl, bool new_line=true);
  void add_data_control(TControl *ctrl, Data *obj, Converter *co, bool new_line);
  void add_edit(Data *obj, int id, bool new_line=true);
  void add_button(pchar capt, int id, bool new_line=false);
  void add_checkbox(pchar capt, Data *obj, int id, bool new_line=true);
  void add_listbox(int id,int height, bool new_line=false, Data *obj=NULL);  
  void add_action(Item& item);
  TControl*  last_ctrl()             { return m_last_ctrl; }
  TWin*      win()                   { return m_parent;    }
  void finish_off();
  virtual void release(); 
 };

 class EXPORT GroupBox: public Layout {
 public:
	 GroupBox(pchar caption, TEventWindow *form);
  	 void release();
 };


 EXPORT Layout& operator << (Layout& l, Field f);
 EXPORT Layout& operator << (Layout& l, Button f);
 EXPORT Layout& operator << (Layout& l, Listbox f);
 EXPORT Layout& operator << (Layout& l, Checkbox f);
 EXPORT Layout& operator << (Layout& l, Control c);
 EXPORT Layout& operator << (Layout& l, pchar caption);
 EXPORT Layout& operator << (Layout& lp, Layout lc);
 EXPORT Layout& operator << (Layout& l, LayoutManipulator lm);
 EXPORT Layout& operator << (Layout& l, NamedAs nm);
 EXPORT Layout& operator << (Layout& l, Space nm);
 EXPORT Layout& operator << (Layout& l, Width w);


#endif

