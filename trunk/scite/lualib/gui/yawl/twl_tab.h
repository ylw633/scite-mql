#ifndef __TWL_TAB_H
#define __TWL_TAB_H
typedef int (TEventWindow::*NotifyEventHandler)(int id);
typedef NotifyEventHandler NEH;

class EXPORT TTabControlB: public TNotifyWin {
	int m_index;
public:
  TTabControlB(TWin* form, bool multiline = false);
  void add(const wchar_t* caption, void* data, int image_idx = -1);
  void remove(int idx = -1);
  void* get_data(int idx = -1);
  void selected(int idx);
  int selected();

  virtual void handle_select(int id) = 0;
  virtual int handle_selection_changing(int i) = 0;

  // override
  int handle_notify(void *p);
};

class EXPORT TTabControl: public TTabControlB {
	SelectionHandler       m_on_select;
	NotifyEventHandler m_on_selection_changing;
	TEventWindow* m_form;
public:
  void on_select(SelectionHandler handler)
  { m_on_select = handler; }

  void on_selection_changing(NEH handler)
  { m_on_selection_changing = handler; }

  TTabControl(TEventWindow* form, bool multiline = false);

  // implement
  virtual void handle_select(int id);
  virtual int handle_selection_changing(int i);

};
#endif




