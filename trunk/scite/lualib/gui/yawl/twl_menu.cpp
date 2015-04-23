/* TWL_MENU.CPP
 * Steve Donovan, 2003
 * This is GPL'd software, and the usual disclaimers apply.
 * See LICENCE
 */
 #include <windows.h>
#define IN_MENU_CPP
 #include "twl_menu.h"
#include "twl_data.h"
#include <string.h>

namespace {
  int last_id = 100;
}

Menu::Menu(TEventWindow *form)
 : m_form(form)
{
 m_menu_handler = new MessageHandler(form);
 create();
}

void Menu::create()
{
  if (m_form && m_form->get_menu()) {
		m_handle = m_form->get_menu();
		m_form = NULL;
  } else {
	  m_handle = (Handle)CreateMenu();
      m_accel_list = new AccelList;
  }
}


#define EQ(s1,s2) (wcscmp(s1,s2)==0)

void Menu::add_menu(Item& item)
{
 // if (item.id == -1) item.id = last_id++;
  item.id = last_id++; //*new
  AppendMenu(HMENU(m_handle),MF_STRING,item.id,item.caption);
  m_menu_handler->add(item);
  wchar_t buff[120];
  wcscpy(buff,item.caption);
  wcstok(buff,L"\t");
  wchar_t *astr = wcstok(NULL,L"");
  if (astr) {
    ACCEL accl; 
    wchar_t *ckey, *vkey;
	ckey = wcstok(astr,L"-");
	vkey = wcstok(NULL,L" ");
	if (vkey==NULL) { vkey = ckey; ckey = NULL; }
	else _wcsupr(ckey);
	_wcsupr(vkey);
	int key;
	accl.fVirt = FVIRTKEY;
	if (*vkey=='F' && *(vkey+1) != '\0') {
      int fkey_id = _wtoi(vkey+1);
	  key = VK_F1 + fkey_id - 1;
    } else
	if (EQ(vkey,L"UP"))     key = VK_UP; else
	if (EQ(vkey,L"DOWN"))   key = VK_DOWN; else
	if (EQ(vkey,L"RIGHT"))  key = VK_RIGHT; else
	if (EQ(vkey,L"LEFT"))   key = VK_LEFT; else
	if (EQ(vkey,L"DELETE")) key = VK_DELETE;
	else {
      //accl.fVirt = 0;
	  key = (int)vkey[0];
    }
    accl.key = (WORD)key;
	if (ckey != NULL) {
      if (EQ(ckey,L"CTRL")) accl.fVirt |= FCONTROL; else
      if (EQ(ckey,L"ALT")) accl.fVirt |= FALT; else
      if (EQ(ckey,L"SHIFT")) accl.fVirt |= FSHIFT;
    }
    accl.cmd = (WORD)item.id;
	//if (accl.fVirt & FVIRTKEY)  // for now...
	m_accel_list->push_back(accl);
  }

}

void Menu::add_menu(Menu& menu)
{
 wchar_t *name = ((Popup&)menu).name();
 AppendMenu(HMENU(m_handle),MF_STRING | MF_POPUP,(int)menu.m_handle,name);
 m_menu_handler->add_handler(menu.m_menu_handler);
 m_accel_list->splice(m_accel_list->end(),*menu.m_accel_list); 
}

void Menu::add_separator()
{
 AppendMenu(HMENU(m_handle),MF_SEPARATOR,0,NULL);
}

MessageHandler *Menu::get_menu_handler()
{
	return m_menu_handler;
}

void Menu::insert_menu(int id_before, Item& item)
{
  if (item.id == -1) item.id = last_id++;
  InsertMenu(HMENU(m_handle),id_before, MF_STRING,item.id,item.caption);
  m_menu_handler->add(item);
}

void Menu::delete_menu(int id)
{
  DeleteMenu(HMENU(m_handle),id,MF_BYCOMMAND);
  m_menu_handler->remove(id);
}

void Menu::release()
{
 if (!m_form) return;
 m_form->set_menu(HMENU(m_handle));
 m_form->add_handler(m_menu_handler);
 if (m_accel_list->size()) {
	 ACCEL *accels = new ACCEL[m_accel_list->size()];
	 AccelList::iterator ali;
	 int i = 0;
	 for(ali = m_accel_list->begin(); ali != m_accel_list->end(); ++ali)
		 accels[i++] = *ali;
	 HACCEL hAccel = CreateAcceleratorTable(accels,i);
	 m_form->add_accelerator(hAccel);
 }
}

Popup::Popup(wchar_t *name)
 : Menu(NULL),m_name(name)
{
}

Popup::Popup(Handle h)
: Menu(NULL),m_name(L"")
{
  m_handle = h;
}

void Popup::create()
{
 m_handle = CreatePopupMenu();
}

void Popup::release()
{
 // does nothing!!
}

PopupMenu::PopupMenu(TEventWindow *form, int index)
: Menu(form), m_index(index) 
{
	m_handle = GetSubMenu((HMENU)m_handle,m_index);
}

void PopupMenu::create()
{

}

void PopupMenu::release() {}

ContextMenu::ContextMenu(TEventWindow* form)
: Menu(NULL), m_main_form(form)
{
}

void ContextMenu::create()
{
 m_handle = CreatePopupMenu();
}

void ContextMenu::release()
{
// save the popup handle
 Popup pop(m_handle);
// create a top-level menu and add the popup to it...
 Menu::create();
 add_menu(pop);
// restore the popup handle and add it to the form
 m_handle = (Handle)pop;
 m_main_form->set_popup_menu(HMENU(m_handle));
 m_menu_handler->set_form(m_main_form);
 m_main_form->add_handler(m_menu_handler);
}

Item::Item(wchar_t *_caption, EventHandler _handler, void* _data, int _id, bool inactive)
  : caption(_caption),handler(_handler),data(_data),id(_id),inactive_data(inactive) { }


MessageHandler::MessageHandler(TEventWindow *form)
 : AbstractMessageHandler(form)
{
 m_list = new ItemList;

}

MessageHandler::~MessageHandler()
{
 delete m_list;
}

void MessageHandler::add(Item& item)
{
 if (item.id == -1) item.id = last_id++;
 m_list->push_back(item);
}

void MessageHandler::remove(int id)
{
 ItemList::iterator ihl = m_list->begin(),
                    iend = m_list->end();
 for(;  ihl != iend; ++ihl) {
   Item& item = *ihl;
   if (item.id == id) {
     m_list->erase(ihl);  // cd use remove(item), if op==
	 return;
   }
 }
}

bool MessageHandler::dispatch(int id, int notify, Handle ctrl_handle)
{
 TControl *ctrl = TControl::user_data(ctrl_handle);
 ItemList::iterator ihl = m_list->begin(),
                    iend = m_list->end();
 for(;  ihl != iend; ++ihl) {
   Item& item = *ihl;
   if (item.id == id) {  // an id needs to be handled
	   if (item.handler) { // is there actually a handler?
		   // if a control sent this notification, send the notification
		   // code instead of the id.
		   //int res = ctrl ?  notify : id;
           //int res = ctrl ?  notify : (int)item.data; //*new
		   if (item.data != NULL) {
			 DataEventHandler handler = (DataEventHandler)item.handler;
			 (m_form->*handler)(&item);
		   } else {
			(m_form->*item.handler)();
		   }
		   return true;  // handled with an action

       } else return true; // found, but no action
   }
  }
  return false;

}

void MessageHandler::read()
{
	ItemList::iterator ihl;

	for(ihl = m_list->begin();  ihl != m_list->end(); ++ihl) {
		void* data = ihl->data;
		if (data && ! ihl->inactive_data)
			PData(data)->read();
	}

}

void MessageHandler::write()
{
	ItemList::iterator ihl;
	try {
	  for(ihl = m_list->begin();  ihl != m_list->end(); ++ihl)
		if (ihl->data) PData(ihl->data)->write();
    } catch(...) {
       m_form->message(L"Bad Number!",MSG_ERROR);
    }
}

int list_size(ItemList* list)
{
	return list->size();
}

//Item list_item(ItemList* list, int i)
//{
//	return list->
//}


void MessageHandler::add_handler(AbstractMessageHandler *_hndlr)
{
 MessageHandler *hndlr = (MessageHandler *)_hndlr;
 m_list->splice(m_list->end(),*hndlr->m_list);
}
