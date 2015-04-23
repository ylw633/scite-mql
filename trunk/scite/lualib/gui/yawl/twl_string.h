
#ifndef __TWL_STRING_H
#define __TWL_STRING_H

#include "twl_layout.h"
typedef std::string* PString;

class StdStringData: public Data {
public:
  StdStringData(void *ptr)
	 : Data(ptr)
  { }

 void write()
 {
   *PString(m_ptr) = m_co->get_str();
 }

 void read()
 { 
  m_co->set_str((char*)PString(m_ptr)->c_str());
 }

};

struct SField:  Field {
	SField(char *caption, std::string& str)
   { 
     field(caption,new StdStringData(&str),NULL);
   }
};

#endif
