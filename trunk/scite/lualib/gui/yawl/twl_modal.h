// TWL_MODAL.H

#ifndef _TWL_MODAL_H
#define _TWL_MODAL_H

#include "twl.h"
#include "twl_layout.h"

const int ML_CANCEL = 2, ML_OKCANCEL = 3, ML_OK = 1, ML_LEFT_ALIGN = 4,
          ML_STANDARD=ML_OKCANCEL | ML_LEFT_ALIGN;

class EXPORT TModalDlg: public TEventWindow {
protected:
	bool m_modal_result;
	int  m_type;
	TWin *m_parent;
public:
    void on_ok();
	void on_cancel();
    virtual void layout(Layout& lo);
	virtual void do_buttons(Layout& lo);
	virtual void do_layout();
	bool show_modal();
    TWin* parent()     { return m_parent; }
	// refuse to be deactivated!!
	bool activate(bool yes);
	void finish(bool ok_cancel);
	TModalDlg(TWin *parent,pchar caption, int type = ML_STANDARD);
	~TModalDlg();
};	

template <class T>
class TNumberDlg: public TModalDlg {
protected:
	T& m_num;
    const char *m_field_name;
public:
	
	TNumberDlg(TEventWindow *parent, pchar field_name, T& num)
		: TModalDlg(parent,"Enter:"), m_num(num), m_field_name(field_name)
	{
	
	}

	void layout(Layout& lo)
    {
		lo << Field(m_field_name,m_num);
    }

};

class EXPORT LayoutDlg: public TModalDlg
{
	Layout& m_layout;
	LayoutDlg& operator =(const LayoutDlg&);  // or else msvc grumbles about impossibility to generate an assignment operator
public:	
	LayoutDlg(TEventWindow *ew, Layout& lo, pchar caption, int type = ML_STANDARD);
	virtual void do_layout();
};

class EXPORT TModalLayout: public Layout
{
	LayoutDlg m_dlg;
	bool m_released;
public:
	TModalLayout(TEventWindow *ew, pchar caption, int type = ML_STANDARD);
	int show_modal();
	virtual ~TModalLayout();
	static bool success();
};



#endif
