// SciTE - Scintilla based Text Editor
/** @file scite_lua_win.h
 ** SciTE Lua scripting interface.
 **/
// Copyright 2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

/* Modifications for Windows to allow UTF-8 file names and command lines */
/*
Imported into Lua build with -DLUA_USER_H=\"scite_lua_win.h\"
Redirect fopen and _popen to functions that treat their arguments as UTF-8.
If UTF-8 does not work then retry with the original strings as may be in locale characters.
*/
#if defined(LUA_WIN)
#include <stdio.h>
FILE *scite_lua_fopen(const char *filename, const char *mode);
#define fopen scite_lua_fopen
FILE *scite_lua_popen(const char *filename, const char *mode);
#define _popen scite_lua_popen
//!-start-[EncodingToLua]
int scite_lua_remove(const char *filename);
#define remove scite_lua_remove
int scite_lua_rename(const char *oldfilename, const char *newfilename);
#define rename scite_lua_rename
int scite_lua_system(const char *command);
#define system scite_lua_system
//!-end-[EncodingToLua]
#endif
