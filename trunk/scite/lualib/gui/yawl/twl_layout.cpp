// twl_layout.cpp
/*
 * Steve Donovan, 2003
 * This is GPL'd software, and the usual disclaimers apply.
 * See LICENCE
 */
#include <stdlib.h>
#include <memory.h> 
#include <string.h>
#include "twl_layout.h"

const int MAX_ROWS = 30, MAX_COLS = 3;


static TControl *s_first_ctrl;
static TControl *_controls_[MAX_ROWS][MAX_COLS];
static int s_left = 20,s_top = 10;
static int s_field_width = 0;

Layout::Layout(TEventWindow *form, TEventWindow *mform)
{
    m_left = s_left; m_top = s_top;
	m_xpos = m_left;  m_ypos = m_top;
    m_xspacing = 10;
	m_yspacing = 5;
    m_last_id = 300;
	m_width = 0;
    m_released = false;
	m_row = m_col = m_max_col = 0;
	memset(_controls_,0,sizeof(_controls_));
    if(form != NULL)
		set_form(form,mform);
	else m_form = NULL; 
}

void Layout::set_margins(int tm, int lm)
{
    s_top = tm;
    s_left = lm;
}

Layout::~Layout()
{
	release();
}

void Layout::set_form(TEventWindow *form, TEventWindow *mform)
{
   m_form = form;
   m_parent = form;
   m_ctrl_handler = new MessageHandler(mform ? mform : form);
}

#ifndef __max
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

TControl *
Layout::add(TControl *ctrl, bool new_line)
{
	if (ctrl->height() < 10) 
      ctrl->resize(m_xpos,m_ypos,90,20);
	else 
	  ctrl->move(m_xpos,m_ypos);
	m_last_ctrl = ctrl;
	m_xpos += ctrl->width() + m_xspacing;	
	m_width = __max(m_xpos,m_width);
	_controls_[m_row][m_col++] = ctrl;
	if (new_line) next_row();
	return ctrl;
}

void
Layout::next_row()
{
	m_ypos += m_last_ctrl->height() + m_yspacing;
	m_xpos = m_left;
	m_max_col = __max(m_max_col,m_col);
	m_row++;
	m_col = 0;
}

void
Layout::space(int yy, int xx)
{
    m_ypos += yy;
    if (xx != -1) m_xpos = m_left + xx;
}

void 
Layout::center_row()
{
  // assume we are still on the _current row_ and that everyone has the same _width_
  TControl **row = _controls_[m_row];
  int cw = row[0]->width();
  int i = 0;
  while (row[i] != NULL) i++;
  int n = i, spacing = (m_width - n*cw)/(n+1);
  for(i = 0; i < n; i++) {
	int pos = i*cw + (i+1)*spacing;  
    row[i]->move(pos,m_ypos);  
  }
}


void
Layout::left_align(int row)
{
 int sr, er;
 if (row==-1) { sr = 0; er = m_row+1; }
 else { sr = row; er = row+1; }
 TControl *ctrl;
 for(int col = 1; col < m_max_col; col++) {  // first col will already be aligned by default
	 // find the maximum x pos for this particular column...
	 int maxx = 0;
	 Rect rt;
	 for(row = sr; row < er; row++) {
		 ctrl = _controls_[row][col];
		 if (ctrl == NULL) break;
		 ctrl->get_rect(rt,true);
		 maxx = __max(maxx,rt.left);
     }
	 for(row = sr; row < er; row++) {
		 ctrl = _controls_[row][col];
		 if (ctrl == NULL || ! ctrl->is_type(L"TEdit")) break; //*new
		 ctrl->get_rect(rt,true);
         ctrl->move(maxx, rt.top);
         m_width = __max(m_width,rt.right); //*new
     }
 }
}

void
Layout::add_label(pchar caption, bool new_line)
{
	if (caption && *caption) {
		TLabel *lbl = new TLabel(m_parent,caption);
		lbl->calc_size(); // not called by default...
		add(lbl,new_line);
    }
}

void
Layout::add_ctrl(TControl *ctrl, bool new_line)
{
    if (s_field_width > 0) {
        int h = ctrl->height();
        if (h < 10) h = 20;
        ctrl->resize(s_field_width,h);
        s_field_width = 0;
    }
	add(ctrl,new_line);
    subclass_control(ctrl);
	ctrl->parent(m_form);
 // add to handler list!!
  
}

void 
Layout::field_width(int w)
{
   s_field_width = w;
}

void check_first_focus(TControl *ctrl)
{ 
  // keep tabs on the _first_ data control to be added!	 
  if(s_first_ctrl==NULL) s_first_ctrl = ctrl;
}

void
Layout::add_data_control(TControl *ctrl, Data *obj, Converter *co, bool new_line)
{
  check_first_focus(ctrl);
  ctrl->data(obj);
  obj->converter(co);
  co->set_ctrl(ctrl);
  add_ctrl(ctrl,new_line);
}

void
Layout::add_edit(Data *obj, int id, bool new_line)
{
   add_data_control(new TEdit(m_parent,L"",id),
	   obj,
	   new EditConverter,new_line
	   );
}

void
Layout::add_button(pchar capt, int id, bool new_line)
{
	TButton *btn = new TButton(m_parent,capt,id);
	add_ctrl(btn,new_line);
}

void 
Layout:: add_listbox(int id, int height, bool new_line, Data *obj)
{
	TListBox *lb = new TListBox(m_parent,id);
	lb->resize(90,height);
	if (obj) add_data_control(lb,obj,new ListboxConverter,new_line);
	else add_ctrl(lb,new_line);
	check_first_focus(lb); // focus, even _if_ no data object! *ISSUE*
}

void
Layout::add_checkbox(pchar capt, Data *obj, int id, bool new_line)
{
   TCheckBox* box = new TCheckBox(m_parent,capt,id,true);
   box->calc_size();
   add_data_control(
	   box, obj, new CheckboxConverter,new_line
	  );
}

void
Layout::add_action(Item& item)
{
 if (item.id == -1) item.id = m_last_id++;
 m_ctrl_handler->add(item);
}

void 
Layout::finish_off()
{
// if we did not finish the last row, add it
 if (m_xpos != m_left) next_row();
 left_align(-1); //*new
 m_height = m_ypos + m_top;  // + m_yspacing makes buttons _too close_ to edge!
 m_width += m_left;    // add a right margin
}

#define EQ(s1,s2) (strcmp(s1,s2)==0)

void
Layout::release()
{
 if (m_released) return; 
 m_released = true;

 finish_off();  
 
 m_form->client_resize(m_width,m_height);
 m_form->add_handler(m_ctrl_handler);
 m_form->get_handler()->read();

 if (s_first_ctrl) { 
    s_first_ctrl->set_focus();
    if (s_first_ctrl->is_type(L"TEdit"))
        ((TEdit*)s_first_ctrl)->set_selection(0,-1);
    s_first_ctrl = NULL;
 } 
}

// The GroupBox is a Layout object
GroupBox::GroupBox(pchar caption, TEventWindow *form)
: Layout(form)
{
 m_parent = new TGroupBox(form,caption);
 m_ypos += m_top;
}

void GroupBox::release()
{
 if (m_released) return; 
 m_released = true;
 finish_off();
 //m_height -= m_top;
 m_parent->resize(0,0,m_width,m_height); 
 m_form->add_handler(m_ctrl_handler);
}


// the actual insertion of controls is done through overloading <<

Layout& operator << (Layout& l, Field f)
{
    l.add_label(f.caption,false);
	l.add_action(f);
    l.add_edit((Data *)f.data,f.id,true);

    return l;
}

Layout& operator << (Layout& l, pchar caption)
{
	l.add_label(caption,false);
	return l;
}

Layout& operator << (Layout& l, Control c)
{
	l.add((TControl *)c.data,false);
	l.add_action(c);
	return l;
}

Layout& operator << (Layout& l, Button b)
{
	l.add_action(b);
    l.add_button(b.caption,b.id,false);
	return l;
}

Layout& operator << (Layout& l, Listbox lb)
{
	lb.id = -1;
	l.add_action(lb);
	l.add_listbox(lb.id, lb.height,false,(Data *)lb.data);
    return l;
}

Layout& operator << (Layout& l, Checkbox f)
{
	l.add_action(f);
	l.add_checkbox(f.caption,(Data *)f.data,f.id,true);
	return l;
}

Layout& operator << (Layout& lp, Layout lc)
{
	lc.release();
	lp.add((TControl *)lc.win(),false);
    return lp;
}


LayoutManipulator Next(1),End(2),Center(3),LeftAlign(4),Margin(5);

Layout& operator << (Layout& l, LayoutManipulator lm)
{
	switch(lm.type) {
	case 1: l.next_row(); break;
	case 2: l.release();  break;
    case 3: l.center_row(); break;
	case 4: l.left_align(-1); break;
    case 5: l.space(s_top,-1); break;
	}
	return l;
}

Layout& operator << (Layout& l, NamedAs nm)
{
    *(nm.ctrl_ref) = l.last_ctrl();
    return l;
}

Layout& operator << (Layout& l, Space nm)
{
   l.space(nm.y,nm.x);
   return l;
}

Layout& operator << (Layout& l, Width w)
{
   l.field_width(w.width);
   return l;
}
