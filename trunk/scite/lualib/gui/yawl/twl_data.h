// TWL_DATA.H

#ifndef __TWL_DATA_H
#define __TWL_DATA_H
#include "twl.h"
#include "twl_cntrls.h"

class  EXPORT EConversionError {
 int m_why;
};

class  EXPORT EBadNumber: public EConversionError
{
};

class EXPORT Converter {
protected:
	int m_digits;
	TControl *m_ctrl;
public:
  Converter() : m_digits(4) {}
  void set_ctrl(TControl *ctrl) { m_ctrl = ctrl; }
  virtual wchar_t* get_str() { return L""; }
  virtual void set_str(pchar) { };	 
  virtual double get_double();
  virtual void set_double(double);
  virtual int get_long();
  virtual void set_long(int);
};

class EXPORT EditConverter: public Converter {
public:
  // overrides:
  wchar_t* get_str();
  void set_str(pchar);	 

   
  TEdit *edit()  { return (TEdit *)m_ctrl; }
};

class EXPORT CheckboxConverter: public Converter {
public:
   // overrides:
   int get_long();
   void set_long(int); 

  TCheckBox *check_box()  { return (TCheckBox *)m_ctrl; }
};

class EXPORT ListboxConverter: public Converter {
public:
	// overrides:
    wchar_t *get_str();
    void set_str(pchar);	 

	TListBox *list_box() { return (TListBox *)m_ctrl; }
};


class EXPORT Data {
protected:
	Converter *m_co;
	void *m_ptr;
public:
	Data(void *ptr) : m_ptr(ptr) { }
	void converter(Converter *co) { m_co = co; }
	virtual void write()=0;
	virtual void read()=0;
};

typedef Data *PData;

class EXPORT StringData: public Data {
  public:
	  StringData(void *ptr) : Data(ptr) { }
	  void write();
	  void read();
  };

class EXPORT FloatData: public Data {
  public:
	  FloatData(void *ptr) : Data(ptr) { }
	  void write();
	  void read();
  };

  class EXPORT DoubleData: public Data {
  public:
	  DoubleData(void *ptr) : Data(ptr) { }
	  void write();
	  void read();
  };

  class EXPORT IntData: public Data {
  public:
	  IntData(void *ptr) : Data(ptr) { }
	  void write();
	  void read();
  };

  class EXPORT BoolData: public Data {
  public:
	  BoolData(void *ptr) : Data(ptr) { }
	  void write();
	  void read();
  };


  class EXPORT ShortData: public Data {
  public:
	  ShortData(void *ptr) : Data(ptr) { }
	  void write();
	  void read();
  };

  class EXPORT UIntData: public Data {
  public:
	  UIntData(void *ptr) : Data(ptr) { }
	  void write();
	  void read();
  };

#endif
