// twl_splitter.cpp
#define NO_STRICT
#include <windows.h>
#include "twl_splitter.h"

TSplitterB::TSplitterB(TEventWindow* parent, TWin* control, int thick /* = 3*/)
:  TEventWindow(L"",parent,0,true),
   m_form(parent),m_control(control),m_width(thick),m_min_size(30),m_line_visible(false),m_down(false)
{
	// we take our alignment from the control which we're abutting...
    align(control->align());
    if (align() == alLeft || align() == alRight) {
        m_cursor = SIZE_HORZ;
		m_vertical = false;
        resize(m_width,0);
    } else {
        m_cursor = SIZE_VERT;
		m_vertical = true;
        resize(0,m_width);
    }    
}

TSplitter::TSplitter(TEventWindow* parent, TWin* control)
:  TSplitterB(parent,control)
{
}

static Point m_start;

void TSplitterB::mouse_down(Point& pt)
{
  Alignment a = align();
  m_start = pt;
  cursor(m_cursor);

  // allocate ourselves a DC to draw the line, and draw it. 
  update_size(pt.x,pt.y);
  m_line_dc = ::GetDCEx(m_form->handle(),0,DCX_CACHE | DCX_CLIPSIBLINGS | DCX_LOCKWINDOWUPDATE);
  draw_line();
  // capture the mouse
  m_down = true;
  mouse_capture(true);
}

void TSplitterB::mouse_move(Point& pt)
{
    cursor(m_cursor); 
    if (m_down) {
        draw_line();
        update_size(pt.x,pt.y);
        draw_line();
    }
}

void TSplitterB::mouse_up(Point& pt)
{
   Rect rt;
   // release the mouse
   m_down = false;
   mouse_capture(false);

   m_control->get_rect(rt,true);
   int w = rt.right - rt.left, h = rt.bottom - rt.top;
   switch(align()) {
   case alLeft:  w = m_new_size;  break;
   case alTop:   h = m_new_size;  break;
   case alRight:
       rt.left += (w - m_new_size);  
       w = m_new_size;
       break;
   case alBottom:
       rt.top += (h - m_new_size);
       h = m_new_size;
       break;
   }
   if (m_line_visible) draw_line();
   ::ReleaseDC(m_form->handle(),m_line_dc);
   rt.right = rt.left + w;
   rt.bottom = rt.top + h;
   on_resize(rt);   
}

void TSplitterB::on_resize(const Rect& rt)
{
   m_control->resize(rt);
}


void TSplitter::on_resize(const Rect& rt)
{
	TSplitterB::on_resize(rt);
	m_form->size();  // to recalculate alignments...
	m_form->invalidate();
}

int clip(int val, int vmin, int vmax);

void TSplitterB::update_size(int xx, int yy)
{
// this hack comes from the fact that these guys are _really_ 16-bit values!
   short y = (short)yy;
   short x = (short)xx;
//   char buff[80];
//   sprintf(buff,"split %d %d\n",y,m_start.y);
//   OutputDebugString(buff);

   if (! m_vertical) {
       m_split = x - m_start.x;
   } else {
       m_split = y - m_start.y;
   }
   int size = 0, w = m_control->width(), h = m_control->height();
   Alignment aa = align();
   switch(aa) {
   case alLeft:  size = w + m_split; break;
   case alRight: size = w - m_split; break;
   case alTop:   size = h + m_split; break;
   case alBottom:size = h - m_split; break;
   }
   m_new_size = size;
  // m_new_size = clip(m_new_size,10,400);  //m_min_size,m_max_size);
   if (m_new_size != size) { // we were clipped...
      if (aa == alRight || aa == alBottom)
          size -= m_new_size;
      else
          size = m_new_size - size;
      m_split += size;
   }
}


void TSplitterB::draw_line()
{
   Rect rt;
   get_rect(rt,true);
   int W = rt.width(), H = rt.height();
   m_line_visible = ! m_line_visible;
   if (! m_vertical) 
       rt.left += m_split;
   else
       rt.top  += m_split;
   ::PatBlt(m_line_dc,rt.left,rt.top,W,H,PATINVERT);
}

/*
void paint(TDC& dc)
{
 // only for bevelled case

}
*/

int clip(int val, int vmin, int vmax)
{
 if (val < vmin) return vmin;
 if (val > vmax) return vmax;
 return val;
}




