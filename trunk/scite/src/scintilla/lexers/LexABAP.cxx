// Scintilla source code edit control
/** @file LexABAP.cxx
 ** Lexer for ABAP/4.
 **/
// Copyright 2007 by Vladislav V. Vorobyev <vladvro(a)gmail.com>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
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

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

static inline bool IsAWordChar(int ch) {
	return (ch < 0x80) && (isalnum(ch) || ch == '_' || ch == '-');
}

static inline bool IsAWordStart(int ch) {
	return (ch < 0x80) && (isalpha(ch) || ch == '_');
}

static inline bool IsANumberChar(int ch) {
	// Not exactly following number definition (several dots are seen as OK, etc.)
	// but probably enough in most cases.
	return (ch < 0x80) &&
	        (isdigit(ch) || toupper(ch) == 'E' || ch == '-' || ch == '+');
}

static inline bool IsSpace(int ch) {
	// Any space beetwen two identifiers
	return (ch < 0x80) && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
}

static int LowerCase(int c)
{
	if (c >= 'A' && c <= 'Z')
		return 'a' + c - 'A';
	return c;
}

static void ColouriseABAPDoc(unsigned int startPos, int length, int initStyle,
		WordList *keywordlists[], Accessor &styler)
{

	WordList &keywords = *keywordlists[0];
	WordList &types = *keywordlists[1];
	WordList &kw_user1 = *keywordlists[4];
	WordList &kw_user2 = *keywordlists[5];
	WordList &kw_user3 = *keywordlists[6];
	WordList &kw_user4 = *keywordlists[7];

	// go back to start of keyword
	if (initStyle >= SCE_ABAP_WORD && initStyle <= SCE_ABAP_USER4) {
		while (styler.StyleAt(startPos-1) >= SCE_ABAP_WORD && startPos > 0)
			startPos--;
		initStyle = styler.StyleAt(startPos-1);
	}

	class StyleContextEx : public StyleContext
	{
	public:
		StyleContextEx( unsigned int startPos,
						unsigned int length,
						int initStyle,
						LexAccessor &styler_ )
		: StyleContext( startPos, length, initStyle, styler_)
		, endPosEx( startPos + length )
		, stylerEx( styler_ )
		{
		}
		void MoveTo(unsigned int pos) {
			if (pos < endPosEx) {
				pos--;
				currentPos = pos;
				chPrev = 0;
				ch = static_cast<unsigned char>(stylerEx.SafeGetCharAt(pos));
				if (stylerEx.IsLeadByte(static_cast<char>(ch))) {
					pos++;
					ch = ch << 8;
					ch |= static_cast<unsigned char>(stylerEx.SafeGetCharAt(pos));
				}
				chNext = static_cast<unsigned char>(stylerEx.SafeGetCharAt(pos+1));
				if (stylerEx.IsLeadByte(static_cast<char>(chNext))) {
					chNext = chNext << 8;
					chNext |= static_cast<unsigned char>(stylerEx.SafeGetCharAt(pos+2));
				}
				// End of line?
				// Trigger on CR only (Mac style) or either on LF from CR+LF (Dos/Win)
				// or on LF alone (Unix). Avoid triggering two times on Dos/Win.
				atLineEnd = (ch == '\r' && chNext != '\n') ||
							(ch == '\n') ||
							(currentPos >= endPosEx);
				Forward();
			} else {
				currentPos = endPosEx;
				atLineStart = false;
				chPrev = ' ';
				ch = ' ';
				chNext = ' ';
				atLineEnd = true;
			}
		}
	private:
		unsigned int endPosEx;
		LexAccessor &stylerEx;
	};
	StyleContextEx sc(startPos, length, initStyle, styler);
	unsigned int firstWordEndPos = 0;
	unsigned int currLen = 0;
	char s[1000];
	s[currLen] = '\0';
	bool isTypeCheck = false;
	
	for (bool doing = sc.More(); doing; doing = sc.More(), sc.Forward()) {
		// Determine if the current state should terminate.
		switch (sc.state) {
		case SCE_ABAP_OPERATOR:
			sc.SetState(SCE_ABAP_DEFAULT);
			break;
		case SCE_ABAP_NUMBER:
			// We stop the number definition on non-number char
			if (!IsANumberChar(sc.ch)) {
				sc.SetState(SCE_ABAP_DEFAULT);
			}
			break;
		case SCE_ABAP_IDENTIFIER:
		case SCE_ABAP_WORD:
		case SCE_ABAP_TYPE:
		case SCE_ABAP_USER1:
		case SCE_ABAP_USER2:
		case SCE_ABAP_USER3:
		case SCE_ABAP_USER4:
			if (!IsAWordChar(sc.ch)) {
				int nextState = SCE_ABAP_IDENTIFIER;
				if (s[currLen-1] != ' ') {
					bool isEq = false, isBegin = false;
					bool eq, begin;
					if (keywords.InMultiWordsList(s, '~', eq, begin)) {
						isEq |= eq;
						isBegin |= begin;
						if (eq)
							sc.ChangeState(SCE_ABAP_WORD);
					}
					if (isTypeCheck && types.InList(s)) {
						isEq = true;
						sc.ChangeState(SCE_ABAP_TYPE);
					}
					if (kw_user1.InMultiWordsList(s, '~', eq, begin)) {
						isEq |= eq;
						isBegin |= begin;
						if (eq)
							sc.ChangeState(SCE_ABAP_USER1);
					}
					if (kw_user2.InMultiWordsList(s, '~', eq, begin)) {
						isEq |= eq;
						isBegin |= begin;
						if (eq)
							sc.ChangeState(SCE_ABAP_USER2);
					}
					if (kw_user3.InMultiWordsList(s, '~', eq, begin)) {
						isEq |= eq;
						isBegin |= begin;
						if (eq)
							sc.ChangeState(SCE_ABAP_USER3);
					}
					if (kw_user4.InMultiWordsList(s, '~', eq, begin)) {
						isEq |= eq;
						isBegin |= begin;
						if (eq)
							sc.ChangeState(SCE_ABAP_USER4);
					}
					if (isBegin && IsSpace(sc.ch)) {
						// continue identifier with next word
						if (!firstWordEndPos)
							firstWordEndPos = sc.currentPos;
						if (currLen + 1 < 1000) {
							s[currLen++] = ' ';
							s[currLen] = '\0';
						} else {
							// requesting for commit identifier (abnormal final)
							nextState = SCE_ABAP_DEFAULT;
						}
					} else {
						if (isEq)
							firstWordEndPos = 0;
						// requesting for commit identifier
						nextState = SCE_ABAP_DEFAULT;
					}
				} else if (!IsSpace(sc.ch)) {
					// requesting for commit identifier
					nextState = SCE_ABAP_DEFAULT;
				}
				if (nextState != SCE_ABAP_IDENTIFIER) {
					// commiting identifier
					if (firstWordEndPos) {
						sc.MoveTo(firstWordEndPos);
						firstWordEndPos = 0;
					}
					sc.SetState(nextState);
					// type check
					if (!strcmp(s, "type") || !strncmp(s, "type ", 5) || !strncmp(s, "type\t", 5))
						isTypeCheck = true;
					else
						isTypeCheck = false;
				}
			} else {
				if (currLen + 1 < 1000) {
					s[currLen++] = static_cast<char>(LowerCase(sc.ch));
					s[currLen] = '\0';
				}
			}
			break;
		case SCE_ABAP_COMMENT:
		case SCE_ABAP_COMMENTLINE:
			// We stop the comment at end of line
			if (sc.atLineStart) {
				sc.SetState(SCE_ABAP_DEFAULT);
			}
			break;
		case SCE_ABAP_STRING:
			if (sc.ch == '\'') {
				if (sc.chNext == '\'') {
					sc.Forward();
				} else {
					sc.ForwardSetState(SCE_ABAP_DEFAULT);
				}
			}
			break;
		}

		// Determine if a new state should be entered.
		if (sc.state == SCE_ABAP_DEFAULT) {
			if (IsADigit(sc.ch)) {
				sc.SetState(SCE_ABAP_NUMBER);
			} else if (sc.Match('\"')) {
				sc.SetState(SCE_ABAP_COMMENT);
			} else if (sc.Match('*') && sc.atLineStart) {
				sc.SetState(SCE_ABAP_COMMENTLINE);
			} else if (sc.Match('\'')) {
				sc.SetState(SCE_ABAP_STRING);
			} else if (isoperator(static_cast<char>(sc.ch))) {
				sc.SetState(SCE_ABAP_OPERATOR);
			} else if (IsAWordStart(sc.ch)) {
				sc.SetState(SCE_ABAP_IDENTIFIER);
				currLen = 0;
				s[currLen++] = static_cast<char>(LowerCase(sc.ch));
				s[currLen] = '\0';
			}
		}
	}
	sc.Complete();
}

static void FoldABAPDoc(unsigned int startPos, int length, int initStyle,
	WordList *keywordlists[], Accessor &styler)
{
	WordList &fold_begin = *keywordlists[2];
	WordList &fold_end = *keywordlists[3];

	// go back to start of keyword
	if (initStyle >= SCE_ABAP_WORD && initStyle <= SCE_ABAP_USER4) {
		while (startPos > 0 && styler.StyleAt(startPos-1) >= SCE_ABAP_WORD)
			startPos--;
		initStyle = styler.StyleAt(startPos-1);
	}

	int line = styler.GetLine(startPos);
	int level = styler.LevelAt(line);
	int done = 0, levelIndent = 0;
	unsigned int endPos = startPos + length;
	char word[256];
	int wordlen = 0;
	int style = initStyle;
	bool inPerform = false;
	int levelNum = level & SC_FOLDLEVELNUMBERMASK;
	if (levelNum > SC_FOLDLEVELBASE) {
		// Find folding start if it is top
		int lineLook = line - 1;
		while ((lineLook > 0) && (
				(!(styler.LevelAt(lineLook) & SC_FOLDLEVELHEADERFLAG)) ||
				((styler.LevelAt(lineLook) & SC_FOLDLEVELNUMBERMASK) == levelNum)))
			lineLook--;
		if ((styler.LevelAt(lineLook) & SC_FOLDLEVELHEADERFLAG) &&
				((styler.LevelAt(lineLook) & SC_FOLDLEVELNUMBERMASK) < levelNum))
		{
			// Validate folding start
			unsigned int sPos = styler.LineStart(lineLook);
			if (styler.StyleAt(sPos) == SCE_ABAP_WORD) {
				while (sPos > 0 && styler.StyleAt(sPos-1) == SCE_ABAP_WORD)
					sPos--;
			}
			unsigned int ePos = styler.LineStart(lineLook + 1);
			while (ePos < endPos && styler.StyleAt(ePos) == SCE_ABAP_WORD)
				ePos++;
			for (unsigned int i = sPos; i < ePos; i++) {
				int c = styler.SafeGetCharAt(i, '\n');
				style = styler.StyleAt(i);
				if (style == SCE_ABAP_WORD) {
					if (IsSpace(c)) {
						if (!IsSpace(word[wordlen - 1])) {
							word[wordlen] = ' ';
							if (wordlen < 255)
								wordlen++;
						}
					} else {
						word[wordlen] = static_cast<char>(LowerCase(c));
						if (wordlen < 255)
							wordlen++;
					}
				} else {
					if (wordlen) {
						word[wordlen] = '\0';
						if (strcmp(word,"call function")==0 || strcmp(word,"perform")==0) {
							// Folding for function call or perform construction
							inPerform = true;
						}
						wordlen = 0;
					}
				}
			}
			wordlen = 0;
		}
	}
	// Scan for tokens at the start of the line (they may include
	// whitespace, for tokens like "End Function"
	for (unsigned int i = startPos; i < endPos; i++) {
		int c = styler.SafeGetCharAt(i, '\n');
		int prevStyle = style;
		style = styler.StyleAt(i);
		if(inPerform) {
			if (style == SCE_ABAP_OPERATOR && c == '.') {
				inPerform = false;
				levelIndent -= 1;
			}
		} else {
			if (style == SCE_ABAP_COMMENTLINE) {
				if (style != prevStyle)
					levelIndent = 1;
			} else
			if (!done && style != SCE_ABAP_COMMENT) {
				if (wordlen) { // are we scanning a token already?
					word[wordlen] = static_cast<char>(LowerCase(c));
					if (!IsAWordChar(c)) { // done with token
						word[wordlen] = '\0';
						// CheckFoldPoint
						int go = 0;
						if (strcmp(word,"call function")==0 || strcmp(word,"perform")==0) {
							// Folding for function call or perform construction
							go = 1;
							inPerform = true;
						} else {
							// Folding by settings
							bool isEq, isBegin;
							if (fold_begin.InMultiWordsList(word, '~', isEq, isBegin) && isEq) {
								go = 1;
							} else
								if (fold_end.InMultiWordsList(word, '~', isEq, isBegin) && isEq) {
								go = -1;
							}
						}
						if (!go) {
							// Treat any whitespace as single blank, for
							// things like "end of".
							if (IsSpace(c)) {
								if (IsAWordChar(word[wordlen - 1])) {
									word[wordlen] = ' ';
									if (wordlen < 255)
										wordlen++;
								}
							}
							else // done with this operator
								done = 1;
						} else {
							levelIndent += go;
							done = 1;
						}
					} else if (wordlen < 255) {
						wordlen++;
					}
				} else { // start scanning at first word character
					if (IsAWordChar(c)) {
						word[0] = static_cast<char>(LowerCase(c));
						wordlen = 1;
					}
				}
			}
		}
		if (style == SCE_ABAP_OPERATOR && (c == '.' || c == ',' || c == ':')) { // operator end
			// reset state
			wordlen = 0;
			done = 0;
		} else
		if (c == '\n') { // line end
			if (style == SCE_ABAP_COMMENTLINE && !inPerform) {
				if (style != styler.StyleAt(i + 1)) levelIndent--;
			}
			if (levelIndent > 0) {
				level |= SC_FOLDLEVELHEADERFLAG;
				levelIndent = 1;
			} else if (levelIndent < 0) {
				levelIndent = -1;
			}
			if (level != styler.LevelAt(line))
				styler.SetLevel(line, level);
			level += levelIndent;
			if ((level & SC_FOLDLEVELNUMBERMASK) < SC_FOLDLEVELBASE)
				level = SC_FOLDLEVELBASE;
			line++;
			// reset state
			levelIndent = 0;
			level &= ~SC_FOLDLEVELHEADERFLAG;
			level &= ~SC_FOLDLEVELWHITEFLAG;
		}
	}
}

static const char * const abapWordListDesc[] = {
	"Keywords",
	"Types",
	"Fold Begin",
	"Fold End",
	"User Keywords 1",
	"User Keywords 2",
	"User Keywords 3",
	"User Keywords 4",
	0
};

LexerModule lmABAP(SCLEX_ABAP, ColouriseABAPDoc, "abap", FoldABAPDoc, abapWordListDesc);
