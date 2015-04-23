// TWL_INI.CPP
/*
 * Steve Donovan, 2003
 * This is GPL'd software, and the usual disclaimers apply.
 * See LICENCE
*/
#include <windows.h>
#include <direct.h>
#include "twl_ini.h"

#define WIN32_INI

EXPORT void output_debug_str(const wchar_t *buff)
{
  OutputDebugString(buff);
}

wchar_t IniBase::_tmpbuff_[BUFSZ];

void IniBase::write_int(pcchar key, int val)
{
  _itow(val,_tmpbuff_,10);
  write_string(key,_tmpbuff_);
}

int  IniBase::read_int(pcchar key, int def)
{
  wchar_t defstr[20];
  _itow(def,defstr,10);
  wchar_t* istr = read_string(key,_tmpbuff_,BUFSZ,defstr);
  return _wtoi(istr);
}

#ifndef WIN32_INI
struct Pair {
	char* key;
	char* value;
};
#define MAP(ptr,section,key,val) (*(IniMap)m_data)[section][key])
#endif

static void trim_end(char* buff)
{
	buff[strlen(buff)-1] = '\0';
}

IniFile::IniFile(pcchar file, bool in_cwd)
{
    if (! in_cwd) m_file = _wcsdup(file);
    else {
        get_app_path(_tmpbuff_,BUFSZ);
        wcscat(_tmpbuff_,L"/");
        wcscat(_tmpbuff_,file);
        m_file = _wcsdup(_tmpbuff_);
    }
#ifndef WIN32_INI
	FILE *in = fopen(file,"r");
	while (! feof(in)) {
		fgets(_tmpbuff_,BUFSZ,in);
		trim_end(_tmpbuff_);
		if (*_tmpbuff_ == ';' || *_tmpbuff_ == '\0')
			continue;
		if (*_tmpbuff_ == '[') {
			char* section = _tmpbuff_ + 1;
			trim_end(section);
		}
	}
	fclose(in);

#endif
}

void IniFile::set_section(pcchar section)
{
	 m_section = section;
}

void IniFile::write_string(pcchar key, pcchar value)
{
    WritePrivateProfileString(m_section,key,value,m_file);
}

wchar_t *IniFile::read_string(pcchar key, wchar_t *value, int sz, pcchar def)
{
    GetPrivateProfileString(m_section,key,def,value,sz,m_file);
    return value;
}
