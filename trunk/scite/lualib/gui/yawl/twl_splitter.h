#ifndef __TWL_SPLITTER
#define __TWL_SPLITTER
#include "twl.h"

class EXPORT TSplitterB: public TEventWindow {
protected:
	int m_width, m_min_size, m_max_size,m_split,m_new_size;
	CursorType m_cursor;
	bool m_line_visible, m_down, m_vertical;
	Handle m_line_dc;
	TWin* m_control;
	TEventWindow* m_form;

public:
	TSplitterB(TEventWindow* parent, TWin* control, int thick = 3);
	void update_size(int x, int y);
	void draw_line();

// overrides
	void mouse_down(Point& pt);
	void mouse_move(Point& pt);
	void mouse_up(Point& pt);

	virtual void on_resize(const Rect& rt);

};

class EXPORT TSplitter: public TSplitterB {
public:
	TSplitter(TEventWindow* parent, TWin* control);

	void on_resize(const Rect& rt);
};
#endif
