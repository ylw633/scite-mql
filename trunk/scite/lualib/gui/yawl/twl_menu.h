/* TWL_MENU.H
 */
#ifndef __TWL_MENU_H
#define __TWL_MENU_H

#include "twl.h"

struct Item;

typedef void (TEventWindow::*EventHandler)();
typedef EventHandler EH;
typedef void (TEventWindow::*SelectionHandler)(int);
typedef SelectionHandler SH;
typedef void (TEventWindow::*DataEventHandler)(Item*);
typedef EventHandler DEH;

struct EXPORT Item {
 const wchar_t *caption;
 EventHandler handler;
 int id;
 void *data;
 bool inactive_data;

 Item(wchar_t *_caption=NULL, EventHandler _handler=NULL, void* data = NULL, int id = -1, bool inactive = false);
};

#ifdef IN_MENU_CPP
//#include <list>
#include "list"
typedef std::list<Item> ItemList;
typedef std::list<ACCEL> AccelList;
#else
typedef int ItemList;
typedef int AccelList;
#endif

typedef ItemList *PItemList;
typedef AccelList *PAccelList;

class EXPORT MessageHandler: public AbstractMessageHandler {
 protected:
   PItemList m_list;
 public:
   MessageHandler(TEventWindow *form);
   ~MessageHandler();
   void add(Item& item);
   void remove(int id);
   bool dispatch(int id, int notify, Handle ctrl);
   void read();
   void write();
   void add_handler(AbstractMessageHandler *);
};


class EXPORT Menu {
protected:
  TEventWindow *m_form;
  Handle m_handle;
  MessageHandler *m_menu_handler;
  PAccelList m_accel_list;
public:
  Menu(TEventWindow *form);
  void add_menu(Item& item);
  void add_menu(Menu& menu);
  void add_separator();
  MessageHandler * get_menu_handler();
  void insert_menu(int id_before, Item& item);
  void delete_menu(int id);
  operator Handle () { return m_handle; }
  virtual void create();
  virtual void release();
  virtual ~Menu() { release(); }
};

class EXPORT Popup: public Menu {
protected:
  wchar_t *m_name;
public:
  Popup(wchar_t *name);
  Popup(Handle h);
  wchar_t *name() { return m_name; }
  void create();
  void release();
  Handle get_handle();
};

class EXPORT PopupMenu: public Menu {
protected:
  int m_index;
public:
	PopupMenu(TEventWindow *form, int index);
	void create();
	void release();
};

class EXPORT ContextMenu: public Menu {
protected:
    TEventWindow *m_main_form;
public:
    ContextMenu(TEventWindow *form);
    void create();
    void release();
    ~ContextMenu() { release(); }
};

struct Sep { 
	int id;
};

// these were all passed by reference...
inline 
Menu& operator << (Menu& mnu, Item item) 
{
  mnu.add_menu(item);
  return mnu;
}

inline 
Menu& operator << (Menu& mnu, Menu sub_mnu) 
{
  mnu.add_menu(sub_mnu);
  return mnu;
}

inline
Menu& operator << (Menu& mnu, Sep sep)
{ 
   mnu.add_separator();
   return mnu;
}

#endif
