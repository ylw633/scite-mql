// TWL_DATA.CPP
/*
 * Steve Donovan, 2003
 * This is GPL'd software, and the usual disclaimers apply.
 * See LICENCE
 */
#include "twl_data.h"
#include <stdlib.h>
#include <string.h>

 // Converter defines all its operations assuming that
 // set & get string will be defined sensibly in derived 
 // classes.

static char temp_buff[255];
static wchar_t wtemp_buff[255];
  
double Converter::get_double()
{
  wchar_t *endptr = L"t"; // must not be NULL
  double f = wcstod(get_str(),&endptr);
  if (*endptr) throw EBadNumber();
  return f; 
}

void Converter::set_double(double f)
{
  const char * p = gcvt(f,m_digits,temp_buff);
  wchar_t * w = wtemp_buff;
  do
  {
    *w++ = (unsigned char)*p;
  } while(*p++ != 0);
  set_str(w);
}

int Converter::get_long()
{
  return _wtoi(get_str());
}

void Converter::set_long(int val)
{
   _itow(val,wtemp_buff,10);
   set_str(wtemp_buff);
}

wchar_t *EditConverter::get_str()
{
 return (wchar_t*)edit()->get_text();  // *hack
}

void EditConverter::set_str(pchar str)
{
 edit()->set_text(str);
 edit()->update();
}

// Hooking into Check boxes
int CheckboxConverter::get_long()
{
	return check_box()->check() ? 1 : 0;
}

void CheckboxConverter::set_long(int val)
{
	check_box()->check(val ? true : false);
}

// And into List boxes
wchar_t *
ListboxConverter::get_str()
{
	int idx = list_box()->selected();
	if (idx != -1) {
		list_box()->get_text(idx,wtemp_buff);
		return wtemp_buff;
    }
	else return L"";
}

void
ListboxConverter::set_str(pchar str)
{
 // *SJD* There is no corresponding simple operation on list boxes;
 // we could of course fiddle this, but I don't know useful it really is. 
}


// The various descendants of Data define their
// conversion behaviour using the conversion object

void StringData::write()
{ wcscpy((wchar_t *)m_ptr,m_co->get_str()); }

void StringData::read()
{ m_co->set_str((wchar_t *)m_ptr); }


void FloatData::write()
{ *(float *)m_ptr = (float)m_co->get_double(); }

void FloatData::read()
{ m_co->set_double(*(float *)m_ptr); }

void DoubleData::write()
{ *(double *)m_ptr = m_co->get_double(); }

void DoubleData::read()
{ m_co->set_double(*(double *)m_ptr); }

void IntData::write()
{ *(int *)m_ptr = m_co->get_long(); }

void IntData::read()
{ m_co->set_long(*(int *)m_ptr); }

void ShortData::write()
{ *(short *)m_ptr = (short)m_co->get_long(); }

void ShortData::read()
{ m_co->set_long(*(short *)m_ptr); }

void UIntData::write()
{ *(unsigned int *)m_ptr = m_co->get_long(); }

void UIntData::read()
{ m_co->set_long(*(unsigned int *)m_ptr); }


void BoolData::write()
{
	*(bool *)m_ptr = m_co->get_long() ? true : false;
}

void BoolData::read()
{
	m_co->set_long( *(bool *)m_ptr ? 1 : 0);
}

  
