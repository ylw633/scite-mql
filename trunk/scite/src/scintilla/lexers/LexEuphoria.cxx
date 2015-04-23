// SciTE - Scintilla based Text Editor
/** @file LexEuphoria.cxx
 ** Lexer for Euphoria.
 **
 ** adapted for euphoria by M Duffy
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "PropSetSimple.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

static inline bool IsAWordChar(int ch) {
	// '.' prevents filenames like get.e from being evaluated as a keyword
	return (isalnum(ch) || ch == '_');
}

static inline bool IsAHexChar(int ch) {
	return (isdigit(ch) ||
		ch == 'A' || ch == 'B' || ch == 'C' ||
		ch == 'D' || ch == 'E' || ch == 'F' );
}

static inline bool IsEscapedChar(int ch) {
	return (ch == '\\' || ch == '\'' || ch == '\"' || ch == 'r' || ch == 'n' || ch == 't');
}

static inline bool IsEuphoriaOperator(int ch) {
	return (ch == '*' || ch == '/' || ch == '-' || ch == '+' || ch == '(' || ch == ')' ||
		ch == '=' || ch == '!' || ch == '{' || ch == '}' || ch == ',' || ch == '&' || ch == '$' ||
		ch == '[' || ch == ']' || ch == '<' || ch == '>'  || ch == '?' || ch == '.');
}

static void ColouriseEuphoriaDoc(unsigned int startPos,  int length, int initStyle, WordList *keywordlists[], Accessor &styler) {
	WordList &keywords = *keywordlists[0];
	WordList &keywords2 = *keywordlists[1];
	WordList &keywords3 = *keywordlists[2];
	WordList &keywords4 = *keywordlists[3];
	WordList &keywords5 = *keywordlists[4];
	WordList &keywords6 = *keywordlists[5];
	WordList &keywords7 = *keywordlists[6];
	WordList &keywords8 = *keywordlists[7];

	// Do not leak onto next line
	if (initStyle == SCE_EUPHORIA_STRINGEOL) {
		initStyle = SCE_EUPHORIA_DEFAULT;
	}
	int charStart = 0;
	StyleContext sc(startPos, length, initStyle, styler);

	for (; sc.More(); sc.Forward()) {
		if (sc.atLineEnd) {
			// Update the line state, so it can be seen by next line
			int currentLine = styler.GetLine(sc.currentPos);
			styler.SetLineState(currentLine, 0);
		}
		if (sc.atLineStart) {
			// Prevent SCE_EUPHORIA_STRINGEOL from leaking back to previous line
			if (sc.state == SCE_EUPHORIA_STRING) {
				sc.SetState(SCE_EUPHORIA_STRING);
			} else if (sc.state == SCE_EUPHORIA_CHARACTER) {
				sc.SetState(SCE_EUPHORIA_CHARACTER);
			}
		}
		// Determine if the current state should terminate.
		if (sc.state == SCE_EUPHORIA_OPERATOR) {
			sc.SetState(SCE_EUPHORIA_DEFAULT);
		} else if (sc.state == SCE_EUPHORIA_NUMBER) {
			if ((sc.ch == '+' || sc.ch == '-') && sc.chPrev != 'e' && !isdigit(sc.chNext)) {
				sc.SetState(SCE_EUPHORIA_DEFAULT);
			} else if (sc.ch == '.' && (!isdigit(sc.chNext))) {
				sc.SetState(SCE_EUPHORIA_DEFAULT);
			} else if (sc.ch == 'e' && !isdigit(sc.chPrev)) {
				sc.SetState(SCE_EUPHORIA_DEFAULT);
			} else if (!isdigit(sc.ch) && sc.ch != 'e') {
				sc.SetState(SCE_EUPHORIA_DEFAULT);
			}
		} else if (sc.state == SCE_EUPHORIA_HEXNUMBER) {
			if (!IsAHexChar(sc.ch)) {
				sc.SetState(SCE_EUPHORIA_DEFAULT);
			}
		} else if (sc.state == SCE_EUPHORIA_IDENTIFIER) {
			if (!IsAWordChar(sc.ch)) {
				char s[100];
				sc.GetCurrent(s, sizeof(s));
				if (keywords.InList(s)) {
					sc.ChangeState(SCE_EUPHORIA_WORD);
				} else if (keywords2.InList(s)) {
					sc.ChangeState(SCE_EUPHORIA_WORD2);
				} else if (keywords3.InList(s)) {
					sc.ChangeState(SCE_EUPHORIA_WORD3);
				} else if (keywords4.InList(s)) {
					sc.ChangeState(SCE_EUPHORIA_WORD4);
				} else if (keywords5.InList(s)) {
					sc.ChangeState(SCE_EUPHORIA_WORD5);
				} else if (keywords6.InList(s)) {
					sc.ChangeState(SCE_EUPHORIA_WORD6);
				} else if (keywords7.InList(s)) {
					sc.ChangeState(SCE_EUPHORIA_WORD7);
				} else if (keywords8.InList(s)) {
					sc.ChangeState(SCE_EUPHORIA_WORD8);
				}
				sc.SetState(SCE_EUPHORIA_DEFAULT);
			}
		} else if (sc.state == SCE_EUPHORIA_COMMENTLINE) {
			if (sc.atLineEnd) {
				sc.SetState(SCE_EUPHORIA_DEFAULT);
			}
		} else if (sc.state == SCE_EUPHORIA_COMMENT) {
			if (sc.Match('*', '/')) {
				sc.Forward();
				sc.ForwardSetState(SCE_EUPHORIA_DEFAULT);
			}
		} else if (sc.state == SCE_EUPHORIA_STRING) {
			if (sc.ch == '\\' ) {
				if (IsEscapedChar(sc.chNext )) {
					sc.Forward();
				} else {
					while (!sc.atLineEnd) {
						sc.Forward();
					}
					sc.ChangeState(SCE_EUPHORIA_STRINGEOL);
					sc.ForwardSetState(SCE_EUPHORIA_DEFAULT);
				}
			} else if (sc.ch == '\"') {
				sc.ForwardSetState(SCE_EUPHORIA_DEFAULT);
			} else if (sc.atLineEnd) {
				sc.ChangeState(SCE_EUPHORIA_STRINGEOL);
				sc.ForwardSetState(SCE_EUPHORIA_DEFAULT);
			}
		} else if (sc.state == SCE_EUPHORIA_CHARACTER) {
			int charLength = sc.currentPos - charStart;
			if ( (charLength > 2) && !IsEscapedChar(sc.chPrev) ) {
				while (!sc.atLineEnd) {
					sc.Forward();
				}
				sc.ChangeState(SCE_EUPHORIA_STRINGEOL);
				sc.ForwardSetState(SCE_EUPHORIA_DEFAULT);
			}
			if (sc.ch == '\\' ) {
				if (IsEscapedChar(sc.chNext )) {
					sc.Forward();
				} else {
					while (!sc.atLineEnd) {
						sc.Forward();
					}
					sc.ChangeState(SCE_EUPHORIA_STRINGEOL);
					sc.ForwardSetState(SCE_EUPHORIA_DEFAULT);
				}
			} else if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_EUPHORIA_DEFAULT);
			} else if (sc.atLineEnd) {
				sc.ChangeState(SCE_EUPHORIA_STRINGEOL);
				sc.ForwardSetState(SCE_EUPHORIA_DEFAULT);
			}
		}
		// Determine if a new state should be entered.
		if (sc.state == SCE_EUPHORIA_DEFAULT) {
			if (isdigit(sc.ch) ) { //|| (sc.ch == '.' && isdigit(sc.chNext))) {
				sc.SetState(SCE_EUPHORIA_NUMBER);
			} else if (sc.ch == '.' && !isalpha(sc.chPrev) && sc.chPrev != '.' && isdigit(sc.chNext)) {
				sc.SetState(SCE_EUPHORIA_NUMBER);
			} else if ((sc.ch == '+' || sc.ch == '-') && sc.chPrev == 'e' && isdigit(sc.chNext)) {
				sc.SetState(SCE_EUPHORIA_NUMBER);
			} else if (isalpha(sc.ch)) {
				sc.SetState(SCE_EUPHORIA_IDENTIFIER);
			} else if (sc.ch == '\"') {
				sc.SetState(SCE_EUPHORIA_STRING);
			} else if (sc.ch == '\'') {
				sc.SetState(SCE_EUPHORIA_CHARACTER);
				charStart = sc.currentPos;
			} else if (sc.Match('-', '-')) {
				sc.SetState(SCE_EUPHORIA_COMMENTLINE);
				sc.Forward();
			} else if (sc.Match('/', '*')) {
				sc.SetState(SCE_EUPHORIA_COMMENT);
				sc.Forward();
			} else if (sc.ch == '#') {
				sc.SetState(SCE_EUPHORIA_HEXNUMBER);
			} else if (IsEuphoriaOperator(sc.ch)) {
				sc.SetState(SCE_EUPHORIA_OPERATOR);
			}
		}
	}
	sc.Complete();
}

// Folding based on state and keyword
static bool WordStartsFold(const char *s) {
	if (strcmp(s, "if") == 0)
		;
	else if (strcmp(s, "for") == 0)
		;
	else if (strcmp(s, "while") == 0)
		;
	else if (strcmp(s, "function") == 0)
		;
	else if (strcmp(s, "procedure") == 0)
		;
	else if (strcmp(s, "type") == 0)
		;
	else
		return false;
	 return true;
}

static void FoldEuphoriaDocKeyWord(unsigned int startPos, int length, int iInitStyle, WordList *[], Accessor &styler) {
	unsigned int lengthDoc = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent-1) >> 16;
	int levelNext = levelCurrent;
	char ch = styler.SafeGetCharAt(startPos-1);
	char chNext = styler.SafeGetCharAt(startPos);
	int style = iInitStyle;
	int styleNext = styler.StyleAt(startPos);

	bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;
	bool foldAtElse = styler.GetPropertyInt("fold.at.else", 1) != 0;
	bool foldAtBraces = styler.GetPropertyInt("fold.symbols", 1) != 0;
	bool foldComment = styler.GetPropertyInt("fold.comment", 1) != 0;

	for (unsigned int i = startPos; i < lengthDoc; i++) {
		char chPrev = ch;
		ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		int stylePrev = style;
		style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || ch == '\n' ;
		if (style == SCE_EUPHORIA_WORD) {
			char s[100];
			unsigned int j = 0;
			while (j < (sizeof(s) - 1) && iswordchar(styler[i + j])) {
				s[j] = styler[i + j];
				j++;
			}
			s[j] = '\0';
			if (WordStartsFold(s)) {
				// looking for whole words
				if (!iswordchar(chPrev)) {
					levelNext++;
				}
			} else if (foldAtElse) {
				if (strcmp(s, "else") == 0 || strcmp(s, "elsif") == 0) {
					if (levelCurrent > 0){
						levelCurrent--;
					}
				}
				// may need to clear previous line header flag
				int lvl = styler.LevelAt(lineCurrent - 1);
				int hdrflg = lvl & SC_FOLDLEVELHEADERFLAG;
				int lvlprev = lvl & SC_FOLDLEVELNUMBERMASK;
				if (lvlprev == levelCurrent && hdrflg != 0) {
					// fold header flag is the ONLY thing we want to change
					styler.SetLevel(lineCurrent - 1, lvl & ~SC_FOLDLEVELHEADERFLAG);
				}
			}
			if (strcmp(s, "end") == 0) {
				// Euphorias uses a compound end statement  if .. end if  etc.
				if (levelNext > 1) {
					levelNext -= 2;
				}
			}
		} else if (style == SCE_EUPHORIA_OPERATOR && foldAtBraces) {
			if (ch == '{' || ch == '(') {
				levelNext++;
			} else if (ch == '}' || ch == ')') {
				if (levelNext > 0) {
					levelNext--;
				}
			}
		} else if (style == SCE_EUPHORIA_COMMENTLINE && foldComment) {
			// Euphoria does not have a stream comment, these character pairs
			// can  be embeded in a comment line to serve the purpose for folding
			if (ch == '<' && chNext == '<') {
				levelNext++;
			} else if (ch == '>' && chNext == '>') {
				if (levelNext > 0) {
					levelNext--;
				}
			}
		} else if (style == SCE_EUPHORIA_COMMENT && foldComment) {
			if (stylePrev != SCE_EUPHORIA_COMMENT) {
				levelNext++;
			} else if ((styleNext != SCE_EUPHORIA_COMMENT) && !atEOL) {
				levelNext--;
			}
		}
		if (atEOL) {
			int lev = levelCurrent | levelNext << 16;
			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			if (levelCurrent < levelNext)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelCurrent = levelNext;
			visibleChars = 0;
		}
		if (!isspacechar(ch)) {
			visibleChars++;
		}
	}
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later

	int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	styler.SetLevel(lineCurrent, levelCurrent | flagsNext);
}

// Folding based on indentation
static inline bool IsEuphoriaComment(Accessor &styler, int pos, int len) {
	return len>1 && styler[pos]=='-' && styler[pos+1]=='-';
}
static void FoldEuphoriaDocIndent(unsigned int startPos, int length, int , WordList *[], Accessor &styler) {
	int lengthDoc = startPos + length;
	// Backtrack to previous line in case need to fix its fold status
	int lineCurrent = styler.GetLine(startPos);
	if (startPos > 0) {
		if (lineCurrent > 0) {
			lineCurrent--;
			startPos = styler.LineStart(lineCurrent);
		}
	}
	int spaceFlags = 0;
	int indentCurrent = styler.IndentAmount(lineCurrent, &spaceFlags, IsEuphoriaComment);
	char chNext = styler[startPos];
	for (int i = startPos; i < lengthDoc; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		if ((ch == '\r' && chNext != '\n') || (ch == '\n') || (i == lengthDoc)) {
			int lev = indentCurrent;
			int indentNext = styler.IndentAmount(lineCurrent + 1, &spaceFlags, IsEuphoriaComment);
			if (!(indentCurrent & SC_FOLDLEVELWHITEFLAG)) {
				// Only non whitespace lines can be headers
				if ((indentCurrent & SC_FOLDLEVELNUMBERMASK) < (indentNext & SC_FOLDLEVELNUMBERMASK)) {
					lev |= SC_FOLDLEVELHEADERFLAG;
				} else if (indentNext & SC_FOLDLEVELWHITEFLAG) {
					// Line after is blank so check the next - maybe should continue further?
					int spaceFlags2 = 0;
					int indentNext2 = styler.IndentAmount(lineCurrent + 2, &spaceFlags2, IsEuphoriaComment);
					if ((indentCurrent & SC_FOLDLEVELNUMBERMASK) < (indentNext2 & SC_FOLDLEVELNUMBERMASK)) {
						lev |= SC_FOLDLEVELHEADERFLAG;
					}
				}
			}
			indentCurrent = indentNext;
			styler.SetLevel(lineCurrent, lev);
			lineCurrent++;
		}
	}
}

static const char * const euphoriaWordLists[] = {
	"Keywords",
	"Operators",
	"Builtin Routines",
	"Library Routines",
	"User Library 1",
	"User Library 2",
	"User Library 3",
	"User Library 4",
	0
};

// Fold document
static void FoldEuphoriaDoc(unsigned int startPos, int length, int iInitStyle, WordList *keywords[], Accessor &styler) {
	if (0 != styler.GetPropertyInt("fold", 0)) {
		int foldingStyle = 3 & styler.GetPropertyInt("fold", 0);
		switch (foldingStyle) {
			case 0:
				break;
			case 1:
				FoldEuphoriaDocKeyWord( startPos, length, iInitStyle, keywords, styler);
				break;
			case 2:
				FoldEuphoriaDocIndent(startPos,  length,  iInitStyle, keywords, styler);
				break;
			default:
				FoldEuphoriaDocKeyWord( startPos, length, iInitStyle, keywords, styler);
		}
	}
}

LexerModule lmEuphoria(SCLEX_EUPHORIA, ColouriseEuphoriaDoc, "euphoria", FoldEuphoriaDoc, euphoriaWordLists);

