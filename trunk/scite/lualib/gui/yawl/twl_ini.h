// TWL_INI.H
#ifndef _TWL_INI_H
#define _TWL_INI_H
#include "twl.h"
 const int BUFSZ = 256;

 EXPORT void output_debug_str(const char *buff);

 typedef const wchar_t* pcchar;

 class EXPORT IniBase {
 protected:
     static wchar_t _tmpbuff_[BUFSZ];
 public:
    virtual void set_section(pcchar section) = 0;
    virtual void write_string(pcchar key, pcchar value) = 0;
    virtual wchar_t *read_string(pcchar key, wchar_t* value=_tmpbuff_, int sz=BUFSZ,pcchar def=L"") = 0;
    virtual void write_int(pcchar key, int val);
    virtual int  read_int(pcchar key, int def=0);
 };

 class EXPORT IniFile: public IniBase {
 private:
     pcchar m_file, m_section;
	 void *m_data;
 public:
    IniFile(pcchar file, bool in_cwd=false);
	// overrides!
    void set_section(pcchar section);
    void write_string(pcchar key, pcchar value);
    wchar_t *read_string(pcchar key, wchar_t* value=_tmpbuff_, int sz=BUFSZ,pcchar def=L"");
 };
#endif

