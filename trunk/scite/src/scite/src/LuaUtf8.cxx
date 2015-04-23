#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "Scintilla.h"
#include "GUI.h"

extern "C" {
#ifdef _WIN32
bool CFileExists(const char* filename)
{
    FILE* fp = NULL;
    fp = fopen(filename, "r");
    if(fp != NULL)
    {
        fclose(fp);
        return true;
    }
    return false;
}

FILE *scite_lua_fopen(const char *filename, const char *mode) {
	GUI::gui_string sFilename = GUI::StringFromUTF8(filename);
	GUI::gui_string sMode = GUI::StringFromUTF8(mode);
	FILE *f = _wfopen(sFilename.c_str(), sMode.c_str());
	if (f == NULL)
		// Fallback on narrow string in case already in CP_ACP
		f = fopen(filename, mode);
	return f;
}

FILE *scite_lua_popen(const char *filename, const char *mode) {
	GUI::gui_string sFilename = GUI::StringFromUTF8(filename);
	GUI::gui_string sMode = GUI::StringFromUTF8(mode);
	FILE *f = _wpopen(sFilename.c_str(), sMode.c_str());
	if (f == NULL)
		// Fallback on narrow string in case already in CP_ACP
		f = _popen(filename, mode);
	return f;
}

int scite_lua_remove(const char *filename) {
	if (CFileExists(filename))
		return remove(filename);
	else {
		GUI::gui_string sFilename = GUI::StringFromUTF8(filename);
		return _wremove(sFilename.c_str());
	}
}

int scite_lua_rename(const char *oldfilename, const char *newfilename) {
	GUI::gui_string sOldFilename = GUI::StringFromUTF8(oldfilename);
	GUI::gui_string sNewFilename = GUI::StringFromUTF8(newfilename);
	int r = _wrename(sOldFilename.c_str(), sNewFilename.c_str());
	if (r != 0) // TODO: Check errno...
		r = rename(oldfilename, newfilename);
	return r;
}

int scite_lua_system(const char *command) {
	GUI::gui_string sCommand = GUI::StringFromUTF8(command);
	return _wsystem(sCommand.c_str());
}
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
// ============ STRING ==================================
static int lua_string_from_utf8(lua_State *L) {
	if(lua_gettop(L) != 2) luaL_error(L, "Wrong arguments count for string.from_utf8");
	const char *s = luaL_checkstring(L, 1);
	int cp = 0;
	if(!lua_isnumber(L, 2))
		cp = GUI::CodePageFromName(lua_tostring(L, 2));
	else
		cp = lua_tointeger(L, 2);
	std::string ss = GUI::ConvertFromUTF8(s, cp);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int lua_string_to_utf8(lua_State *L) {
	if(lua_gettop(L) != 2) luaL_error(L, "Wrong arguments count for string.to_utf8");
	const char *s = luaL_checkstring(L, 1);
	int cp = 0;
	if(!lua_isnumber(L, 2))
		cp = GUI::CodePageFromName(lua_tostring(L, 2));
	else
		cp = lua_tointeger(L, 2);
	std::string ss = GUI::ConvertToUTF8(s, cp);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int lua_string_utf8_to_upper(lua_State *L) {
	const char *s = luaL_checkstring(L, 1);
	std::string ss = GUI::UTF8ToUpper(s);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int lua_string_utf8_to_lower(lua_State *L) {
	const char *s = luaL_checkstring(L, 1);
	std::string ss = GUI::UTF8ToLower(s);
	lua_pushstring(L, ss.c_str());
	return 1;
}

static int lua_string_utf8len(lua_State *L) {
	const char *str = luaL_checkstring(L, 1);
	GUI::gui_string wstr = GUI::StringFromUTF8(str);
	lua_pushinteger(L, wstr.length());
	return 1;
}

// --------------------------------------------------------------
#ifdef _WIN32
#define LUA_POF         "luaopen_"
#define LUA_OFSEP       "_"
#define POF             LUA_POF
#define LIB_FAIL        "open"
#define ERRLIB          1
#define ERRFUNC         2
#define LIBPREFIX       "LOADLIB: "
#include "windows.h"

static void pusherror (lua_State *L) {
  int error = GetLastError();
  char buffer[128];
  if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, error, 0, buffer, sizeof(buffer), NULL))
    lua_pushstring(L, buffer);
  else
    lua_pushfstring(L, "system error %d\n", error);
}

static lua_CFunction ll_sym (lua_State *L, void *lib, const char *sym) {
        lua_CFunction f = (lua_CFunction)GetProcAddress((HINSTANCE)lib, sym);
        if (f == NULL) pusherror(L);
        return f;
}

static void *ll_load (lua_State *L, const char *path) {
        HINSTANCE lib = LoadLibraryW(GUI::StringFromUTF8(path).c_str());
        if (lib == NULL) pusherror(L);
        return lib;
}

static void **ll_register (lua_State *L, const char *path) {
  void **plib;
  lua_pushfstring(L, "%s%s", LIBPREFIX, path);
  lua_gettable(L, LUA_REGISTRYINDEX);  /* check library in registry? */
  if (!lua_isnil(L, -1))  /* is there an entry? */
    plib = (void **)lua_touserdata(L, -1);
  else {  /* no entry yet; create one */
    lua_pop(L, 1);
    plib = (void **)lua_newuserdata(L, sizeof(const void *));
    *plib = NULL;
    luaL_getmetatable(L, "_LOADLIB");
    lua_setmetatable(L, -2);
    lua_pushfstring(L, "%s%s", LIBPREFIX, path);
    lua_pushvalue(L, -2);
    lua_settable(L, LUA_REGISTRYINDEX);
  }
  return plib;
}

static int ll_loadfunc (lua_State *L, const char *path, const char *sym) {
  void **reg = ll_register(L, path);
  if (*reg == NULL) *reg = ll_load(L, path);
  if (*reg == NULL)
    return ERRLIB;  /* unable to load library */
  else {
    lua_CFunction f = ll_sym(L, *reg, sym);
    if (f == NULL)
      return ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);
    return 0;  /* return function */
  }
}

static int readable (const char *filename) {
        FILE *f = _wfopen(GUI::StringFromUTF8(filename).c_str(), GUI_TEXT("r"));  /* try to open file */
        if (f == NULL) return 0;  /* open failed */
        fclose(f);
        return 1;
}

static const char *pushnexttemplate (lua_State *L, const char *path) {
  const char *l;
  while (*path == *LUA_PATHSEP) path++;  /* skip separators */
  if (*path == '\0') return NULL;  /* no more templates */
  l = strchr(path, *LUA_PATHSEP);  /* find next separator */
  if (l == NULL) l = path + strlen(path);
  lua_pushlstring(L, path, l - path);  /* template */
  return l;
}

static const char *findfile (lua_State *L, const char *name,
                                           const char *pname) {
  const char *path;
  name = luaL_gsub(L, name, ".", LUA_DIRSEP);
  lua_getfield(L, LUA_GLOBALSINDEX, "package");
  lua_getfield(L, -1, pname);
  lua_remove(L, -2);
  //lua_getfield(L, LUA_ENVIRONINDEX, pname);
  path = lua_tostring(L, -1);
  if (path == NULL)
    luaL_error(L, LUA_QL("package.%s") " must be a string", pname);
  lua_pushliteral(L, "");  /* error accumulator */
  while ((path = pushnexttemplate(L, path)) != NULL) {
    const char *filename;
    filename = luaL_gsub(L, lua_tostring(L, -1), LUA_PATH_MARK, name);
    lua_remove(L, -2);  /* remove path template */
    if (readable(filename))  /* does file exist and is readable? */
      return filename;  /* return that file name */
    lua_pushfstring(L, "\n\tno file " LUA_QS, filename);
    lua_remove(L, -2);  /* remove file name */
    lua_concat(L, 2);  /* add entry to possible error message */
  }
  return NULL;  /* not found */
}

static void loaderror (lua_State *L, const char *filename) {
  luaL_error(L, "error loading module " LUA_QS " from file " LUA_QS ":\n\t%s",
                lua_tostring(L, 1), filename, lua_tostring(L, -1));
}

static const char *mkfuncname (lua_State *L, const char *modname) {
  const char *funcname;
  const char *mark = strchr(modname, *LUA_IGMARK);
  if (mark) modname = mark + 1;
  funcname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  funcname = lua_pushfstring(L, POF"%s", funcname);
  lua_remove(L, -2);  /* remove 'gsub' result */
  return funcname;
}

static int utf8_loader_Lua (lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path");
  if (filename == NULL) return 1;  /* library not found in this path */
  if (luaL_loadfile(L, filename) != 0)
    loaderror(L, filename);
  return 1;  /* library loaded successfully */
}

static int utf8_loader_C (lua_State *L) {
  const char *funcname;
  const char *name = luaL_checkstring(L, 1);
  const char *filename = findfile(L, name, "cpath");
  if (filename == NULL) return 1;  /* library not found in this path */
  funcname = mkfuncname(L, name);
  if (ll_loadfunc(L, filename, funcname) != 0)
    loaderror(L, filename);
  return 1;  /* library loaded successfully */
}

static int utf8_loader_Croot (lua_State *L) {
  const char *funcname;
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  const char *p = strchr(name, '.');
  int stat;
  if (p == NULL) return 0;  /* is root */
  lua_pushlstring(L, name, p - name);
  filename = findfile(L, lua_tostring(L, -1), "cpath");
  if (filename == NULL) return 1;  /* root not found */
  funcname = mkfuncname(L, name);
  if ((stat = ll_loadfunc(L, filename, funcname)) != 0) {
    if (stat != ERRFUNC) loaderror(L, filename);  /* real error */
    lua_pushfstring(L, "\n\tno module " LUA_QS " in file " LUA_QS,
                       name, filename);
    return 1;  /* function not found */
  }
  return 1;
}

// ==============================================================
static const lua_CFunction utf8_loaders[] =
  {utf8_loader_Lua, utf8_loader_C, utf8_loader_Croot, NULL};
#endif

static const luaL_Reg utf8_string_funcs[] = {
	{"to_utf8", lua_string_to_utf8},
	{"from_utf8", lua_string_from_utf8},
	{"utf8upper", lua_string_utf8_to_upper},
	{"utf8lower", lua_string_utf8_to_lower},
	{"utf8len", lua_string_utf8len},
	{NULL, NULL}
};

void lua_utf8_register_libs (lua_State *L) {
	luaL_openlibs(L); // register standard libs
	luaL_register(L, LUA_STRLIBNAME, utf8_string_funcs);
	lua_pop(L, 1);
#ifdef _WIN32
	// ************ REGISTER OUR UTF-8 LOADERS ****************************
    lua_getfield(L, LUA_GLOBALSINDEX, "package");
    lua_getfield(L, -1, "loaders");
    lua_remove(L, -2);
    int i = lua_objlen(L, -1);
    for (int k=0; utf8_loaders[k] != NULL; k++) {
			lua_rawgeti(L, -1, k+2);
			lua_rawseti(L, -2, i + k + 1);
            lua_pushcfunction(L, utf8_loaders[k]);
            lua_rawseti(L, -2, k+2);
    }
    lua_pop(L, 1);
    // ====================================================================
#endif
}