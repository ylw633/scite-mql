// Scintilla source code edit control
/** @file LexCrontab.cxx
 ** Lexer to use with extended crontab files used by a powerful
 ** Windows scheduler/event monitor/automation manager nnCron.
 ** (http://nemtsev.eserv.ru/)
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

static void ColouriseNncrontabDoc(unsigned int startPos, int length, int, WordList
*keywordLists[], Accessor &styler)
{
	int state = SCE_NNCRONTAB_DEFAULT;
	char chNext = styler[startPos];
	int lengthDoc = startPos + length;
	// create a buffer large enough to take the largest chunk...
	char *buffer = new char[length+1];
	int bufferCount = 0;
	// used when highliting environment variables inside quoted string:
	bool insideString = false;

	// this assumes that we have 3 keyword list in conf.properties
	WordList &section = *keywordLists[0];
	WordList &keyword = *keywordLists[1];
	WordList &modifier = *keywordLists[2];
//!-start-[ForthImprovement]
	WordList &word1 = *keywordLists[5];
	WordList &word2 = *keywordLists[6];
	WordList &word3 = *keywordLists[7];
	WordList &word4 = *keywordLists[8];
	WordList &word5 = *keywordLists[9];
	WordList &word6 = *keywordLists[10];
//!-end-[ForthImprovement]

	// go through all provided text segment
	// using the hand-written state machine shown below
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	for (int i = startPos; i < lengthDoc; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		if (styler.IsLeadByte(ch)) {
			chNext = styler.SafeGetCharAt(i + 2);
			i++;
			continue;
		}
		switch(state) {
			case SCE_NNCRONTAB_DEFAULT:
				if( ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
					// whitespace is simply ignored here...
					styler.ColourTo(i,SCE_NNCRONTAB_DEFAULT);
					break;
				} else if( ch == '#' && styler.SafeGetCharAt(i+1) == '(') {
					// signals the start of a task...
					state = SCE_NNCRONTAB_TASK;
					styler.ColourTo(i,SCE_NNCRONTAB_TASK);
				}
				  else if( ch == '\\' && (styler.SafeGetCharAt(i+1) == ' ' ||
										 styler.SafeGetCharAt(i+1) == '\t')) {
					// signals the start of an extended comment...
					state = SCE_NNCRONTAB_COMMENT;
					styler.ColourTo(i,SCE_NNCRONTAB_COMMENT);
				} else if( ch == '#' ) {
					// signals the start of a plain comment...
					state = SCE_NNCRONTAB_COMMENT;
					styler.ColourTo(i,SCE_NNCRONTAB_COMMENT);
				} else if( ch == ')' && styler.SafeGetCharAt(i+1) == '#') {
					// signals the end of a task...
					state = SCE_NNCRONTAB_TASK;
					styler.ColourTo(i,SCE_NNCRONTAB_TASK);
				} else if( ch == '"') {
					state = SCE_NNCRONTAB_STRING;
					styler.ColourTo(i,SCE_NNCRONTAB_STRING);
				} else if( ch == '%') {
					// signals environment variables
					state = SCE_NNCRONTAB_ENVIRONMENT;
					styler.ColourTo(i,SCE_NNCRONTAB_ENVIRONMENT);
				} else if( ch == '<' && styler.SafeGetCharAt(i+1) == '%') {
					// signals environment variables
					state = SCE_NNCRONTAB_ENVIRONMENT;
					styler.ColourTo(i,SCE_NNCRONTAB_ENVIRONMENT);
				} else if( ch == '*' ) {
					// signals an asterisk
					// no state jump necessary for this simple case...
					styler.ColourTo(i,SCE_NNCRONTAB_ASTERISK);
				} else if( (IsASCII(ch) && isalpha(ch)) || ch == '<' ) {
					// signals the start of an identifier
					bufferCount = 0;
					buffer[bufferCount++] = ch;
					state = SCE_NNCRONTAB_IDENTIFIER;
				} else if( IsASCII(ch) && isdigit(ch) ) {
					// signals the start of a number
					bufferCount = 0;
					buffer[bufferCount++] = ch;
					state = SCE_NNCRONTAB_NUMBER;
				} else {
					// style it the default style..
					styler.ColourTo(i,SCE_NNCRONTAB_DEFAULT);
				}
				break;

			case SCE_NNCRONTAB_COMMENT:
				// if we find a newline here,
				// we simply go to default state
				// else continue to work on it...
				if( ch == '\n' || ch == '\r' ) {
					state = SCE_NNCRONTAB_DEFAULT;
				} else {
					styler.ColourTo(i,SCE_NNCRONTAB_COMMENT);
				}
				break;

			case SCE_NNCRONTAB_TASK:
				// if we find a newline here,
				// we simply go to default state
				// else continue to work on it...
				if( ch == '\n' || ch == '\r' ) {
					state = SCE_NNCRONTAB_DEFAULT;
				} else {
					styler.ColourTo(i,SCE_NNCRONTAB_TASK);
				}
				break;

			case SCE_NNCRONTAB_STRING:
				if( ch == '%' ) {
					state = SCE_NNCRONTAB_ENVIRONMENT;
					insideString = true;
					styler.ColourTo(i-1,SCE_NNCRONTAB_STRING);
					break;
				}
				// if we find the end of a string char, we simply go to default state
				// else we're still dealing with an string...
				if( (ch == '"' && styler.SafeGetCharAt(i-1)!='\\') ||
					(ch == '\n') || (ch == '\r') ) {
					state = SCE_NNCRONTAB_DEFAULT;
				}
				styler.ColourTo(i,SCE_NNCRONTAB_STRING);
				break;

			case SCE_NNCRONTAB_ENVIRONMENT:
				// if we find the end of a string char, we simply go to default state
				// else we're still dealing with an string...
				if( ch == '%' && insideString ) {
					state = SCE_NNCRONTAB_STRING;
					insideString = false;
					break;
				}
				if( (ch == '%' && styler.SafeGetCharAt(i-1)!='\\')
					|| (ch == '\n') || (ch == '\r') || (ch == '>') ) {
					state = SCE_NNCRONTAB_DEFAULT;
					styler.ColourTo(i,SCE_NNCRONTAB_ENVIRONMENT);
					break;
				}
				styler.ColourTo(i+1,SCE_NNCRONTAB_ENVIRONMENT);
				break;

			case SCE_NNCRONTAB_IDENTIFIER:
				// stay  in CONF_IDENTIFIER state until we find a non-alphanumeric
				if( (IsASCII(ch) && isalnum(ch)) || (ch == '_') || (ch == '-') || (ch == '/') ||
					(ch == '$') || (ch == '.') || (ch == '<') || (ch == '>') ||
					(ch == '@') ) {
					buffer[bufferCount++] = ch;
				} else {
					state = SCE_NNCRONTAB_DEFAULT;
					buffer[bufferCount] = '\0';

					// check if the buffer contains a keyword,
					// and highlight it if it is a keyword...
					if(section.InList(buffer)) {
						styler.ColourTo(i,SCE_NNCRONTAB_SECTION );
					} else if(keyword.InList(buffer)) {
						styler.ColourTo(i-1,SCE_NNCRONTAB_KEYWORD );
					} // else if(strchr(buffer,'/') || strchr(buffer,'.')) {
					//	styler.ColourTo(i-1,SCE_NNCRONTAB_EXTENSION);
					// }
					  else if(modifier.InList(buffer)) {
						styler.ColourTo(i-1,SCE_NNCRONTAB_MODIFIER );
//!-start-[ForthImprovement]
					} else if(word1.InList(buffer)) {
						styler.ColourTo(i-1,SCE_NNCRONTAB_WORD1);
					} else if(word2.InList(buffer)) {
						styler.ColourTo(i-1,SCE_NNCRONTAB_WORD2);
					} else if(word3.InList(buffer)) {
						styler.ColourTo(i-1,SCE_NNCRONTAB_WORD3);
					} else if(word4.InList(buffer)) {
						styler.ColourTo(i-1,SCE_NNCRONTAB_WORD4);
					} else if(word5.InList(buffer)) {
						styler.ColourTo(i-1,SCE_NNCRONTAB_WORD5);
					} else if(word6.InList(buffer)) {
						styler.ColourTo(i-1,SCE_NNCRONTAB_WORD6);
//!-end-[ForthImprovement]
					  }	else {
						styler.ColourTo(i-1,SCE_NNCRONTAB_DEFAULT);
					}
					// push back the faulty character
					chNext = styler[i--];
				}
				break;

			case SCE_NNCRONTAB_NUMBER:
				// stay  in CONF_NUMBER state until we find a non-numeric
				if( IsASCII(ch) && isdigit(ch) /* || ch == '.' */ ) {
					buffer[bufferCount++] = ch;
				} else {
					state = SCE_NNCRONTAB_DEFAULT;
					buffer[bufferCount] = '\0';
					// Colourize here... (normal number)
					styler.ColourTo(i-1,SCE_NNCRONTAB_NUMBER);
					// push back a character
					chNext = styler[i--];
				}
				break;
		}
	}
	delete []buffer;
}

//!-start-[ForthImprovement]
static void FoldNncrontabDoc(unsigned int startPos, int length, int initStyle,
	WordList *keywordlists[], Accessor &styler)
{
	WordList &fold_begin = *keywordlists[3];
	WordList &fold_end = *keywordlists[4];

	int line = styler.GetLine(startPos);
	int level = styler.LevelAt(line);
	int levelIndent = 0;
	unsigned int endPos = startPos + length;
	char word[256];
	int wordlen = 0;
	int style = initStyle;
	// Scan for tokens
	for (unsigned int i = startPos; i < endPos; i++) {
		int c = styler.SafeGetCharAt(i, '\n');
		style = styler.StyleAt(i);
		if (c == '\n' || c == '\r' || c == '\t' || c == ' ') {
			if (wordlen) { // done with token
				word[wordlen] = '\0';
				// CheckFoldPoint
				if (fold_begin.InList(word)) {
					levelIndent += 1;
				} else
				if (fold_end.InList(word)) {
					levelIndent -= 1;
				}
				wordlen = 0;
			}
		}
		else if (!(style == SCE_NNCRONTAB_COMMENT || style == SCE_NNCRONTAB_STRING)) {
			if (wordlen) {
				if (wordlen < 255) {
					word[wordlen] = static_cast<char>(c);
					wordlen++;
				}
			} else { // start scanning at first word character
				word[0] = static_cast<char>(c);
				wordlen = 1;
			}
		}
		if (c == '\n') { // line end
			if (levelIndent > 0) {
				level |= SC_FOLDLEVELHEADERFLAG;
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
//!-end-[ForthImprovement]
static const char * const cronWordListDesc[] = {
	"Section keywords and Forth words",
	"nnCrontab keywords",
	"Modifiers",
//!-start-[ForthImprovement]
	"folding start words",
	"folding end words",
	"user defined words 1",
	"user defined words 2",
	"user defined words 3",
	"user defined words 4",
	"user defined words 5",
	"user defined words 6",
//!-end-[ForthImprovement]
	0
};

//!LexerModule lmNncrontab(SCLEX_NNCRONTAB, ColouriseNncrontabDoc, "nncrontab", 0, cronWordListDesc);
LexerModule lmNncrontab(SCLEX_NNCRONTAB, ColouriseNncrontabDoc, "nncrontab", FoldNncrontabDoc, cronWordListDesc); //!-change-[ForthImprovement]
