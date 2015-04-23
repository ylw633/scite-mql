//---------------------------------------------------------------------------
#ifndef memmapH
#define memmapH
#define NULL 0
typedef void *Handle;

class MemBlock {
  char *m_ptr, *m_P;
  unsigned long m_size;
 public:
   MemBlock(char *p, int size);
   void  set_ptr(char *p, int size);
   char *begin() { return m_ptr; }
   char *end()   { return m_ptr + m_size - 1; }
   int   size()  { return m_size; }
   int   pos()   { return (int)m_P - (int)m_ptr; }
   char *text();
};

class MemMapFile: public MemBlock {
protected:
  Handle m_hFile,m_hFileMap;
  int m_size_override;
  bool m_do_create;
 public:
   MemMapFile(char *fname=NULL, int size=0,bool do_create=false);
   ~MemMapFile();
   bool open(char *fname, char *mname=NULL, void *base=NULL);
   void close();
   //operator int() { return (int)begin(); }
};

class ShareMemMap: public MemMapFile {
   public:
      ShareMemMap(char *mname, int size, void *base=NULL);
};

typedef MemMapFile MMF;
typedef ShareMemMap SMM;
//---------------------------------------------------------------------------
#endif
