#include <windows.h>
#include "../utils/luaargs.h"
#include "resource.h"
#include "../utf.h"

extern "C" {
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

//------------------------------------------------------------------------------
struct Rect : public RECT {
	Rect() {
		left = top = right = bottom = 0;
	}
	Rect(int Left, int Top, int Right, int Bottom) {
		left   = Left;
		top    = Top;
		right  = Right;
		bottom = Bottom;
	}
	Rect(const Rect &rc) {
		left   = rc.left;
		top    = rc.top;
		right  = rc.right;
		bottom = rc.bottom;
	}
	Rect& operator = (const Rect &rc) {
		if (this != &rc) {
			left   = rc.left;
			top    = rc.top;
			right  = rc.right;
			bottom = rc.bottom;
		}
		return *this;
	}
	Rect& operator = (const Rect *rc) {
		if (this != rc) {
			left   = rc->left;
			top    = rc->top;
			right  = rc->right;
			bottom = rc->bottom;
		}
		return *this;
	}
	int width() const
		{ return right - left; }
	int height() const
		{ return bottom - top; }
	void GetWindowRect(HWND hwnd) {
			::GetWindowRect(hwnd, this);
		}
	void GetWindowRect(int ctrlID, HWND parent) {
			::GetWindowRect(::GetDlgItem(parent, ctrlID), this);
		}
	void GetClientRect(HWND hwnd) {
			::GetClientRect(hwnd, this);
		}
	void MapPoints(HWND hwnd) {
			::MapWindowPoints(0, hwnd, static_cast<POINT*>(static_cast<void*>(this)), 2);
		}
	void MoveWindow(HWND hwnd, int left, int top, bool repaint=false) {
			::MoveWindow(hwnd, left, top, width(), height(), repaint? 1:0);
		}
	void MoveWindow(int ctrlID, HWND parent, int left, int top, bool repaint=false) {
			::MoveWindow(GetDlgItem(parent, ctrlID), left, top, width(), height(), repaint? 1:0);
		}
	void AdjustWindowRect(HWND hwnd, bool hasMenu=false) {
			::AdjustWindowRect(this, GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_OVERLAPPED, hasMenu? 1:0);
		}
};

//------------------------------------------------------------------------------


int GetClientWidth(HWND hwnd)
{
	Rect rc;
	::GetClientRect(hwnd, &rc);
	return rc.width();
}

int GetClientWidth(int ctrlID, HWND parent)
{
	Rect rc;
	::GetClientRect(::GetDlgItem(parent, ctrlID), &rc);
	return rc.width();
}

int GetWindowWidth(HWND hwnd)
{
	Rect rc;
	::GetWindowRect(hwnd, &rc);
	return rc.width();
}

int GetWindowWidth(int ctrlID, HWND parent)
{
	Rect rc;
	::GetWindowRect(::GetDlgItem(parent, ctrlID), &rc);
	return rc.width();
}

int GetWindowHeight(int ctrlID, HWND parent)
{
	Rect rc;
	::GetWindowRect(::GetDlgItem(parent, ctrlID), &rc);
	return rc.height();
}

//------------------------------------------------------------------------------
class InputBox {
	typedef struct {
		InputBox *self;
		bool isFirst;
		Rect rc;
	} DlgData;

	enum { MAX_SHORT_STRING=128, MAX_MIDDLE_STRING=512, MAX_LONG_STRING=1024 };

public:
	InputBox(const wchar_t *Caption, const wchar_t *Prompt, const wchar_t *Value,
		int CharMinCount, int OnChar, lua_State* L);
	~InputBox();
	int  ShowModal();
	const wchar_t *Text() const;

private:

	InputBox();

	int  PrepareTextOut(HWND hdlg);
	BOOL OutText(HDC hdc);
	void PrepareEdit(HWND hdlg);
	void Layout(HWND hdlg);
	void AdjustWidth(int ctrlID, HWND hdlg, int width);
	void AdjustWidth(int ctrlID, HWND hdlg);
	void AdjustDlg(HWND hdlg);
	void MoveY(int ctrlID, HWND hdlg, int dy);
	void CenterButtons(HWND hdlg);
	void CalcDlgMinWidth(HWND hdlg);
	static BOOL CALLBACK EditHandler(HWND, UINT, WPARAM, LPARAM);
	static BOOL CALLBACK DlgHandler(HWND, UINT, WPARAM, LPARAM);

private:

	DlgData data;

	wchar_t editText[MAX_MIDDLE_STRING];  // введённый пользователем текст
	wchar_t caption[MAX_SHORT_STRING];    // заголовок окна диалога
	wchar_t prompt[MAX_LONG_STRING];      // многострочная надпись над полем ввода

	int marginX;       // гор. отступ от края окна
	int marginY;       // верт. отступ от края окна
	int spacing;       // верт. интервал между полем ввода и кнопками
	int btnSpacing;    // гориз. интервал между кнопками
	int stcDy;
	int charMinCount;  // ширина поля ввода в усреднённых символах
	int minWidth;      // мин. ширина окна (в пикселах)
	HICON smallIcon;
	HICON bigIcon;
	lua_State *luaState;
	int onChar;
};

//------------------------------------------------------------------------------
// Вычисляет прибл. ширину строки символов в пикселах по кол-ву этих символов
//------------------------------------------------------------------------------
int CalcAverWidth(HWND hwnd, int charCount)
{
	// Ширина = (усреднённая ширина символа) * (мин. кол-во символов)
	TEXTMETRIC mtr;
	HDC hdc = GetDC(hwnd);
	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	GetTextMetrics(hdc, &mtr);
	ReleaseDC(hwnd, hdc);
	return mtr.tmAveCharWidth*charCount;
}

//------------------------------------------------------------------------------
// Вычисляет ограничения на мин. ширину окна
//------------------------------------------------------------------------------
void InputBox::CalcDlgMinWidth(HWND hdlg)
{
	int stc = GetWindowWidth(IDC_PROMPTTEXT, hdlg);
	int edc = GetWindowWidth(IDC_EDITTEXT, hdlg);
	int btn = GetWindowWidth(IDOK, hdlg);

	minWidth = CalcAverWidth(GetDlgItem(hdlg, IDC_EDITTEXT), charMinCount);
	minWidth = max(minWidth, max(stc, edc));
	minWidth = max(minWidth, 2*btn + btnSpacing);

	Rect rc;
	rc.GetClientRect(hdlg);
	rc.right = minWidth + 2*marginX;
	rc.AdjustWindowRect(hdlg);
	minWidth = rc.width();
}

//------------------------------------------------------------------------------
// Изменяет ширину контрола на заданную
//------------------------------------------------------------------------------
void InputBox::AdjustWidth(int ctrlID, HWND hdlg, int width)
{
	HWND hctrl = GetDlgItem(hdlg, ctrlID);
	Rect rc;
	rc.GetWindowRect(hctrl);
	rc.MapPoints(hdlg);
	SetWindowPos(hctrl, 0, 0, 0, width, rc.height(), SWP_NOMOVE|SWP_NOZORDER);
}

//------------------------------------------------------------------------------
// Подгоняет ширину контрола под ширину окна диалога
//------------------------------------------------------------------------------
void InputBox::AdjustWidth(int ctrlID, HWND hdlg)
{
	Rect rc, rcCtrl;
	rc.GetClientRect(hdlg);
	HWND hctrl = GetDlgItem(hdlg, ctrlID);
	rcCtrl.GetWindowRect(hctrl);
	rcCtrl.MapPoints(hdlg);
	SetWindowPos(hctrl, 0, marginX, rcCtrl.top,
		rc.width() - 2*marginX, rcCtrl.height(), SWP_NOZORDER);
}

//------------------------------------------------------------------------------
// Подгоняет размеры окна диалога и его элементов друг к другу
//------------------------------------------------------------------------------
void InputBox::AdjustDlg(HWND hdlg)
{
	Rect rcStc, rcEdc, rcOk;
	rcStc.GetWindowRect(IDC_PROMPTTEXT, hdlg);
	rcEdc.GetWindowRect(IDC_EDITTEXT, hdlg);
	rcOk.GetWindowRect(IDOK, hdlg);
	rcOk.MapPoints(hdlg);

	// Фиксируем мин. ширину окна диалога
	CalcDlgMinWidth(hdlg);

	// Настраиваем размеры окна диалога
	Rect rc(0, 0, minWidth, rcOk.bottom + marginY);
	rc.AdjustWindowRect(hdlg);
	SetWindowPos(hdlg, 0, 0, 0, rc.width(), rc.height(), SWP_FRAMECHANGED|SWP_NOMOVE);
	minWidth = GetWindowWidth(hdlg);

	// Подгоняем ширину контролов под ширину окна
	AdjustWidth(IDC_PROMPTTEXT, hdlg);
	AdjustWidth(IDC_EDITTEXT, hdlg);
}

//------------------------------------------------------------------------------
// Смещает контролы по высоте
//------------------------------------------------------------------------------
void InputBox::MoveY(int ctrlID, HWND hdlg, int y)
{
	Rect rc;
	rc.GetWindowRect(ctrlID, hdlg);
	rc.MapPoints(hdlg);
	rc.MoveWindow(ctrlID, hdlg, rc.left, y, true);
}

//------------------------------------------------------------------------------
// Центрирует кнопки по горизонтали, делает их равного размера
//------------------------------------------------------------------------------
void InputBox::CenterButtons(HWND hdlg)
{
	// Кнопка Ok для упрощения всегда помещается слева.
	// Размеры и положение по вертикали берутся из таковых для кнопки Ok

	HWND hOk = GetDlgItem(hdlg, IDOK);
	HWND hCancel = GetDlgItem(hdlg, IDCANCEL);
	Rect rcOk;
	rcOk.GetWindowRect(hOk);
	rcOk.MapPoints(hdlg);

	// вычисляем требуемые смещения
	int okX = (GetClientWidth(hdlg) - 2*rcOk.width() - btnSpacing)/2;
	int cancelX = okX + rcOk.width() + btnSpacing;

	// центрируем ряд с кнопками
	rcOk.MoveWindow(hOk, okX, rcOk.top, true);
	rcOk.MoveWindow(hCancel, cancelX, rcOk.top, true);
}

//------------------------------------------------------------------------------
// Готовит static для вывода многострочного текста.
//------------------------------------------------------------------------------
int InputBox::PrepareTextOut(HWND hdlg)
{
	if (!prompt || ! *prompt)
		return 0;

	HWND hctrl = GetDlgItem(hdlg, IDC_PROMPTTEXT);

	SIZE maxSize = { 0 };
	HDC hdc = GetDC(hctrl);
	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	GetTextExtentPoint32 (hdc, prompt, lstrlen(prompt), &maxSize);
	maxSize.cx = 0;

	SIZE size = { 0 };
	int lnCount = 1;
	const wchar_t *prev = prompt;
	const wchar_t *p = prompt;

	for (; *p; ++p) {
		if (*p == '\r' || *p == '\n') {
			GetTextExtentPoint32 (hdc, prev, p-prev, &size);
			maxSize.cx = max(size.cx, maxSize.cx);
			prev = p + 1;
			lnCount++;
		}
	}

	if (prev != p) {
		GetTextExtentPoint32 (hdc, prev, p-prev, &size);
		maxSize.cx = max(size.cx, maxSize.cx);
	}

	ReleaseDC(hdlg, hdc);

	// Вычислим высоту надписи с интерлиньяжем в 1.1.
	int ext = maxSize.cy/10;
	stcDy = maxSize.cy + ext;
	maxSize.cy = maxSize.cy*lnCount + (lnCount-1)*ext;

	// Устанавливаем размеры надписи
	SetWindowPos(hctrl, 0, 0, 0, maxSize.cx, maxSize.cy, SWP_NOMOVE);

	// Вернём высоту контрола плюс дополнительный интервал
	return maxSize.cy + 4*ext;
}

//------------------------------------------------------------------------------
// Выводит текст надписи
//------------------------------------------------------------------------------
BOOL InputBox::OutText(HDC hdc)
{
	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	SelectObject(hdc, reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1));
	SetBkMode(hdc, TRANSPARENT);
	const wchar_t *prev = prompt;
	const wchar_t *p = prompt;
	int y = 0;

	for (; *p; ++p) {
		if (*p == '\r' || *p == '\n') {
			TextOut(hdc, 0, y, prev, p-prev);
			prev = p + 1;
			y += stcDy;
		}
	}
	if (prev != p)
		TextOut(hdc, 0, y, prev, p-prev);

	return reinterpret_cast<BOOL>(GetStockObject(NULL_BRUSH));
}

//------------------------------------------------------------------------------
// Готовит edit для ввода текста.
//------------------------------------------------------------------------------
void InputBox::PrepareEdit(HWND hdlg)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_EDITTEXT);

	HDC hdc = GetDC(hctrl);
	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

	SIZE size = { 0 };
	Rect rc;
	rc.GetClientRect(hctrl);
	rc.right = 0;

	int len = lstrlen(editText);
	if (len) {
		GetTextExtentPoint32 (hdc, editText, len, &size);
		if (rc.right < size.cx)
			rc.right = size.cx;
	}

	TEXTMETRIC tm = { 0 };
	GetTextMetrics(hdc, &tm);
	if (rc.height() < tm.tmHeight)
		rc.bottom = rc.top + tm.tmHeight;

	rc.AdjustWindowRect(hctrl);
	SetWindowPos(hctrl, 0, 0, 0, rc.width(), rc.height(), SWP_NOZORDER|SWP_NOMOVE);

	ReleaseDC(hdlg, hdc);
}

//------------------------------------------------------------------------------
// Размещает элементы диалога
//------------------------------------------------------------------------------
void InputBox::Layout(HWND hdlg)
{
	// Выведем заголовок диалога
	SetWindowText(hdlg, caption);

	// Подготовим надпись
	int editY =  marginY + PrepareTextOut(hdlg);

	// Подготовим поле ввода
	PrepareEdit(hdlg);

	// Позиционируем контролы по вертикали
	MoveY(IDC_PROMPTTEXT, hdlg, marginY);
	MoveY(IDC_EDITTEXT, hdlg, editY);
	editY += GetWindowHeight(IDC_EDITTEXT, hdlg) + spacing;
	MoveY(IDOK, hdlg, editY);
	MoveY(IDCANCEL, hdlg, editY);

	// Окончательная подгонка размеров
	AdjustDlg(hdlg);

	// Центрируем кнопки
	CenterButtons(hdlg);
}

//------------------------------------------------------------------------------
// Выводит текст на консоль SciTE
//------------------------------------------------------------------------------
void OutputMessage(lua_State *L)
{
	if (lua_isstring(L, -1)) {
		size_t len;
		const wchar_t *msg = StringFromUTF8(lua_tolstring(L, -1, &len));
		wchar_t *buff = new wchar_t[len+2];
		wcsncpy(buff, msg, len);
		buff[len] = '\n';
		buff[len+1] = '\0';
		lua_pop(L, 1);
		if (lua_checkstack(L, 3)) {
			lua_getglobal(L, "output");
			lua_getfield(L, -1, "AddText");
			lua_insert(L, -2);
			lua_pushstring(L, UTF8FromString(buff));
			lua_pcall(L, 2, 0, 0);
		}
		delete[] buff;
	}
}

//------------------------------------------------------------------------------
// Передаёт пользовательской функции on_char вводимый текст для проверки
//------------------------------------------------------------------------------
bool IsInputValid(lua_State *L, const wchar_t *str, wchar_t ch, int checker)
{
	if (checker) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, checker);
		lua_pushstring(L, UTF8FromString(str));
		if (ch)
			lua_pushlstring(L, UTF8FromString(&ch), 1);
		else
			lua_pushnil(L);
		if (lua_pcall(L, 2, 1, 0)) {
			OutputMessage(L);
			lua_pushboolean(L, 1);
		}
		bool ret = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);
		return ret;
	}
	return true;
}

//------------------------------------------------------------------------------
WNDPROC originalEditHandler = 0;

//------------------------------------------------------------------------------
// Обработчик сообщений поля ввода
//------------------------------------------------------------------------------
BOOL CALLBACK InputBox::EditHandler(HWND hctrl, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InputBox *self = reinterpret_cast<InputBox*>(GetWindowLongPtr(hctrl, GWLP_USERDATA));

	switch (msg) {
		case WM_CHAR:
		{
			wchar_t ch = static_cast<wchar_t>(wParam);
			if (ch != VK_BACK) {
				int bSel = 0;
				int eSel = 0;
				wchar_t str[sizeof(self->editText)];
				wchar_t tail[sizeof(self->editText)];
				tail[0] = '\0';
				int len = GetWindowText(hctrl, str, sizeof(self->editText));

				if (len) {
					SendMessage(hctrl, EM_GETSEL, reinterpret_cast<WPARAM>(&bSel),
						reinterpret_cast<LPARAM>(&eSel));
					lstrcpy(tail, str+eSel);
				}

				str[bSel] = ch;
				str[bSel+1] = '\0';
				lstrcat(str, tail);

				if (!IsInputValid(self->luaState, str, ch, self->onChar)) {
					return 0;
				}
			}
		}
	}
	return CallWindowProc(originalEditHandler, hctrl, msg, wParam, lParam);
}

//------------------------------------------------------------------------------
// Обработчик сообщений окна диалога
//------------------------------------------------------------------------------
BOOL CALLBACK InputBox::DlgHandler(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG) {

		// Свяжем этот экземляр окна с объектом типа InputBox, создавшим его
		InputBox *self = reinterpret_cast<InputBox*>(lParam);
		self->data.self = self;
		self->data.isFirst = true;
		SetWindowLongPtr(hdlg, DWLP_USER, reinterpret_cast<LONG_PTR>(&self->data));

		// Делаем подкласс поля ввода с целью обработки вводимых символов
		HWND hedit = GetDlgItem(hdlg, IDC_EDITTEXT);
		originalEditHandler = reinterpret_cast<WNDPROC>(
			SetWindowLongPtr(hedit, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(EditHandler))
		);
		SetWindowLongPtr(hedit, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));

		WPARAM font = reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT));
		SendMessage(GetDlgItem(hdlg, IDC_PROMPTTEXT), WM_SETFONT, font, FALSE);
		SendMessage(hedit, WM_SETFONT, font, FALSE);

		// Разместим в окне элементы диалога
		self->Layout(hdlg);

		// Помещаем текст в поле ввода
		SetDlgItemText(hdlg, IDC_EDITTEXT, self->editText);

		// Ограничиваем допустимый размер вводимого текста
		SendMessage(GetDlgItem(hdlg, IDC_EDITTEXT), EM_SETLIMITTEXT, sizeof(self->editText)-1, 0);

		// Центрируем диалог на экране
		Rect rcDt, rcDlg;
		rcDt.GetWindowRect(GetDesktopWindow());
		rcDlg.GetWindowRect(hdlg);
		rcDlg.MoveWindow(hdlg, rcDt.left + (rcDt.width() - rcDlg.width())/2,
			rcDt.top + (rcDt.height() - rcDlg.height())/2);

		return TRUE;

	} else {

		DlgData  *data = reinterpret_cast<DlgData*>(GetWindowLongPtr(hdlg, DWLP_USER));
		InputBox *self = 0;
		Rect     *strc = 0;

		if (data) {
			self = data->self;
			strc = &data->rc;
		}

		switch (msg) {
			case WM_COMMAND:
			{
				switch (LOWORD(wParam)) {
					case IDOK:
						if (!GetDlgItemText(hdlg, IDC_EDITTEXT, self->editText, MAX_MIDDLE_STRING)) {
							self->editText[0] = '\0';
						}

						if (!IsInputValid(self->luaState, self->editText, '\0', self->onChar)) {
							return TRUE;
						}
					case IDCANCEL:
						EndDialog(hdlg, wParam);
						return TRUE;
				}
				break;
			}

			case WM_GETICON:
				// Возвращаем загруженные ранее иконки взамен стандартных
				switch (wParam) {
					case ICON_BIG:
						SetWindowLongPtr(hdlg, DWL_MSGRESULT, reinterpret_cast<LONG_PTR>(self->bigIcon));
						break;
					case ICON_SMALL:
					case 2 /*ICON_SMALL2*/:
						SetWindowLongPtr(hdlg, DWL_MSGRESULT, reinterpret_cast<LONG_PTR>(self->smallIcon));
						break;
					default:
						return FALSE;
				}
				return TRUE;

			case WM_CTLCOLORSTATIC:
			{
				// Выводим надпись
				HWND hctrl = reinterpret_cast<HWND>(lParam);
				if (hctrl == GetDlgItem(hdlg, IDC_PROMPTTEXT)) {
					// Всегда возвращаем NULL_BRUSH, чтобы надпись не стиралась
					return self->OutText(reinterpret_cast<HDC>(wParam));
				}
				break;
			}

			case WM_SIZING:
			{
				// Позволим пользователю в установленных пределах изменять ширину окна

				Rect *rc = reinterpret_cast<Rect*>(lParam);

				if (!data->isFirst) {
					switch (wParam) {
						case WMSZ_BOTTOM:
						case WMSZ_BOTTOMLEFT:
						case WMSZ_BOTTOMRIGHT:
							if (rc->bottom != strc->bottom)
								rc->bottom = strc->bottom;
							break;
						case WMSZ_TOP:
						case WMSZ_TOPLEFT:
						case WMSZ_TOPRIGHT:
							if (rc->top != strc->top)
								rc->top = strc->top;
							break;
					}

					int newWidth = rc->width();

					switch (wParam) {
						case WMSZ_RIGHT:
						case WMSZ_TOPRIGHT:
						case WMSZ_BOTTOMRIGHT:
						{
							if (newWidth < self->minWidth) {
								newWidth = self->minWidth;
								rc->right = rc->left + self->minWidth;
							}
						}
						case WMSZ_LEFT:
						case WMSZ_TOPLEFT:
						case WMSZ_BOTTOMLEFT:
						{
							if (newWidth < self->minWidth) {
								newWidth = self->minWidth;
								rc->left = rc->right - self->minWidth;
							}
						}
					}

					*strc = *rc;
					return TRUE;
				}
				break;
			}

			case WM_SIZE:
			{
				// Подгоняем размеры и позицию контролов под изменения

				int dx = strc->width();
				GetWindowRect(hdlg, strc);
				dx = strc->width() - dx;

				if (data->isFirst) {
					data->isFirst = false;
				} else if (wParam == SIZE_RESTORED) {
					self->AdjustWidth(IDC_PROMPTTEXT, hdlg);
					self->AdjustWidth(IDC_EDITTEXT, hdlg);
					self->CenterButtons(hdlg);
				}
				return TRUE;
			}

			case WM_MOVE:
			{
				// Запомним новую позицию окна
				strc->GetWindowRect(hdlg);
				return TRUE;
			}
		}
	}
	return FALSE;
}

//------------------------------------------------------------------------------
InputBox::InputBox(const wchar_t *Caption, const wchar_t *Prompt, const wchar_t *Value,
	int CharMinCount, int OnChar, lua_State* L)
	:
	marginX      (10),
	marginY      (10),
	spacing      (15),
	btnSpacing   (10),
	stcDy        (0),
	charMinCount (CharMinCount),
	luaState     (L),
	minWidth     (0),
	onChar       (OnChar)
{
	lstrcpyn(editText, Value, sizeof(editText));
	lstrcpyn(caption, Caption, sizeof(caption));
	lstrcpyn(prompt, Prompt, sizeof(prompt));

	smallIcon = static_cast<HICON>(
		LoadImage(GetModuleHandle(0), L"SCITE", IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR));
	bigIcon = static_cast<HICON>(
		LoadImage(GetModuleHandle(0), L"SCITE", IMAGE_ICON,
			48, 48, LR_DEFAULTCOLOR));
}

//------------------------------------------------------------------------------
InputBox::~InputBox()
{
	DestroyIcon(smallIcon);
	DestroyIcon(bigIcon);
}

//------------------------------------------------------------------------------
const wchar_t *InputBox::Text() const
{
	return editText;
}

//------------------------------------------------------------------------------
// The next functions were borrowed from Steve Donovan's gui library
//------------------------------------------------------------------------------
BOOL CALLBACK CheckSciteWindow(HWND hwnd, LPARAM lParam)
{
	wchar_t buff[120];
    GetClassName(hwnd, buff, sizeof(buff));	
    if (lstrcmp(buff, L"SciTEWindow") == 0) {
		*reinterpret_cast<HWND*>(lParam) = hwnd;
		return FALSE;
    }
    return TRUE;
}

HWND FindScite()
{
	HWND SciTE = 0;
	EnumThreadWindows(GetCurrentThreadId(), CheckSciteWindow,
		reinterpret_cast<LPARAM>(&SciTE));
	return SciTE;
}

//------------------------------------------------------------------------------
int InputBox::ShowModal()
{
	int result = DialogBoxParam(GetModuleHandle(L"shell.dll"), L"IBOX_DLG",
		FindScite(), DlgHandler, reinterpret_cast<LPARAM>(this));
	if (result == -1) {
		// Вообще-то, это означает, что произошла какая-то ошибка,
		// но мы сделаем вид, что всё Ок: будто бы нажата Cancel
		return IDCANCEL;
	}
	return result;
}

//------------------------------------------------------------------------------
// shell.showinputbox(caption, prompt, default, check, modality, width)
//------------------------------------------------------------------------------
extern int showinputbox(lua_State* L)
{
	LuaArgs lua(L);
	const wchar_t *caption = StringFromUTF8(lua.gets(1, "InputBox"));
	const wchar_t *prompt  = StringFromUTF8(lua.gets(2, "Enter:"));
	const wchar_t *value   = StringFromUTF8(lua.gets(3, ""));
	int         onchar  = lua.getf(4);
	int         width   = lua.geti(5, 20);

	InputBox dlg(caption, prompt, value, width, onchar, L);
	bool res = dlg.ShowModal() == IDOK;

	if (res)
		lua_pushstring(L, UTF8FromString(dlg.Text()));
	else
		lua_pushnil(L);

	return 1;
}

#ifdef RESTOREUNICODENESS
#  if RESTOREUNICODENESS == 3
#    define  UNICODE 1
#    define _UNICODE 1
#  elseif RESTOREUNICODENESS == 2
#    define UNICODE  1
#  else
#    define _UNICODE 1
#  endif
#  undef RESTOREUNICODENESS
#endif
