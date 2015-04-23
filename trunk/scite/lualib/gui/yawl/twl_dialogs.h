// twl_dialogs.h

#ifndef __TWL_DIALOGS_H
#define __TWL_DIALOGS_H

class EXPORT TOpenFile {
protected:
  void *m_ofn;
  wchar_t *m_filename;
  bool m_prompt;
  wchar_t *m_file;
  wchar_t *m_file_out;
  wchar_t *m_path;
public:
  TOpenFile(TWin *parent,const wchar_t *caption,const wchar_t *filter,bool do_prompt=true);
  ~TOpenFile();
  virtual bool go();
  void initial_dir(const wchar_t *dir);
  bool next();
  const wchar_t *file_name();
  void file_name(const wchar_t *buff);
};

class TSelectDir {
	TWin *parent;
	wchar_t *descr;
	wchar_t *dirPath;
	wchar_t *lpszInitialDir;
	HWND m_hWndTreeView;
	TSelectDir();
public:
	TSelectDir(TWin *_parent, const wchar_t *_description=L"",const wchar_t *_initialdir=L"");
	virtual ~TSelectDir();
	virtual bool go();
	const wchar_t *path() const
		{ return dirPath; }
private:
	static int WINAPI SHBrowseCallBack( HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData );
};

class EXPORT TSaveFile: public TOpenFile {
	public:
	TSaveFile(TWin *parent, const wchar_t *caption, const wchar_t *filter,bool do_prompt=true)
		: TOpenFile(parent,caption,filter,false)
	{}
	bool go();
};

class EXPORT TColourDialog {
protected:
	void *m_choose_color;
public:
	TColourDialog(TWin *parent, unsigned int clr);
	virtual bool go();
	int result();
};
#endif
