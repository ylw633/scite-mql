// TWL_MODAL.CPP
/*
 * Steve Donovan, 2003
 * This is GPL'd software, and the usual disclaimers apply.
 * See LICENCE
*/
#include "twl_modal.h"

void TModalDlg::on_ok()
{
    update_data();
    finish(true);
}

void TModalDlg::on_cancel()
{ 
    finish(false);
} 

void TModalDlg::layout(Layout& lo)
{
		lo << L"base class" << Next;
}

void TModalDlg::do_buttons(Layout& lo)
{ 
        lo << Margin;   // *add 1.0.0 space before the buttons
		if (m_type & ML_LEFT_ALIGN) lo << LeftAlign; 
		if (m_type & ML_OK)         lo << OKButton((EH)&TModalDlg::on_ok);
		if (m_type & ML_CANCEL)     lo << CancelButton((EH)&TModalDlg::on_cancel);
		lo << Center;
}

void TModalDlg::do_layout()
{
    Layout lo(this);
	layout(lo);
    do_buttons(lo);
}

bool TModalDlg::show_modal()
{
      do_layout();
	  //mouse_capture(true);

    // center the dialog in the middle of its parent
      Rect rt;
      parent()->get_rect(rt,false);
      int x0 = rt.left, y0 = rt.top,
          W = (rt.right - rt.left), H = (rt.bottom - rt.top);
      get_client_rect(rt);
      move(x0 + (W - rt.right)/2, y0 + (H - rt.bottom)/2);

// show it, and go into the idle loop
      show(SW_SHOW);	  
	  run();
	  // close();
	  TWin::show(0); // hide the window!!
	  m_parent->set_focus();
      return m_modal_result;
}

// refuse to be deactivated!!
bool TModalDlg::activate(bool yes)
{
    return yes;
}   

void TModalDlg::finish(bool ok_cancel)
{
	m_modal_result = ok_cancel;
    //mouse_capture(false);
    quit(0);
}

TModalDlg::TModalDlg(TWin *parent,pchar caption, int type /*= ML_STANDARD*/)
		: TEventWindow(caption,parent),m_parent(parent),m_type(type)
{
}

TModalDlg::~TModalDlg()
{ 
    close();
}


LayoutDlg::LayoutDlg(TEventWindow *ew, Layout& lo, pchar caption, int type /*= ML_STANDARD*/)
		: TModalDlg(ew,caption,type), m_layout(lo)
{}

void LayoutDlg::do_layout()
 {  } 

static int mModalResult;

#ifdef _MSC_VER
// Suppress an annoying MSVC warning about use 'this' pointer.
#pragma warning(push)
#pragma warning(disable:4355)
#endif

TModalLayout::TModalLayout(TEventWindow *ew, pchar caption, int type /*= ML_STANDARD*/)
: m_dlg(ew,*this,caption,type), m_released(false)
{
		set_form(&m_dlg);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

int TModalLayout::show_modal()
{   
      m_dlg.do_buttons(*this);
      release();
	  m_released = true;
      return m_dlg.show_modal();
}

TModalLayout::~TModalLayout()
{
     if(!m_released) mModalResult = show_modal();
}

bool TModalLayout::success()
{
    return mModalResult;
} 


