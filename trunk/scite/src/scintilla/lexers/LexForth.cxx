// Scintilla source code edit control
/** @file LexForth.cxx
 ** Lexer for FORTH
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
//!-start-[ForthImprovement]

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

static bool is_whitespace(int ch){
    return ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ';
}

bool is_blank(int ch){
    return ch == '\t' || ch == ' ';
}
//#define FORTH_DEBUG
#ifdef FORTH_DEBUG
static FILE *f_debug;
#define log(x)  fputs(f_debug,x);
#else
#define log(x)
#endif

#define STATE_LOCALE
#define BL ' '
#define FORTH_DEFINITION_FLAG 0x40
#define FORTH_INTERP_FLAG 0x60
#define FORTH_FLAG_MASK 0xE0
#define FORTH_STYLE_MASK 0x1F

static Accessor *st;
static int cur_pos,pos1,pos2,pos0,lengthDoc;
char *buffer;

char getChar(bool is_bl){
    char ch=st->SafeGetCharAt(cur_pos);
    if(is_bl) if(is_whitespace(ch)) ch=BL;
    return ch;
}

char getCharBL(){
    char ch=st->SafeGetCharAt(cur_pos);
    return ch;
}
bool is_eol(char ch){
    return ch=='\n' || ch=='\r';
}

int parse(char ch, bool skip_eol){
// pos1 - start pos of word
// pos2 - pos after of word
// pos0 - start pos
    char c=0;
    int len;
    bool is_bl=ch==BL;
    pos0=pos1=pos2=cur_pos;
    for(;cur_pos<lengthDoc && (c=getChar(is_bl))==ch; cur_pos++){
        if(is_eol(c) && !skip_eol){
            pos2=pos1;
            return 0;
        }
    }
    pos1=cur_pos;
    pos2=pos1;
    if(cur_pos==lengthDoc) return 0;
    for(len=0;cur_pos<lengthDoc && (c=getChar(is_bl))!=ch; cur_pos++){
        if(is_eol(c) && !skip_eol) break;
        pos2++;
        buffer[len++]=c;
    }
    if(c==ch) pos2--;
    buffer[len]='\0';
#ifdef FORTH_DEBUG
    fprintf(f_debug,"parse: %c %s\n",ch,buffer);
#endif
    return len;
}

bool _is_number(char *s,int base){
    for(;*s;s++){
        int digit=((int)*s)-(int)'0';
#ifdef FORTH_DEBUG
    fprintf(f_debug,"digit: %c %d\n",*s,digit);
#endif
        if(digit>9 && base>10) digit-=7;
        if(digit<0) return false;
        if(digit>=base) return false;
    }
    return true;
}

bool is_number(char *s){
    if(strncmp(s,"0x",2)==0) return _is_number(s+2,16);
    return _is_number(s,10);
}

static void ColouriseForthDoc(unsigned int startPos, int length, int initStyle, WordList *keywordLists[], Accessor &styler)
{
    bool stylingNoInterp = styler.GetPropertyInt("lexer.forth.no.interpretation") != 0;

    if(startPos > 0){
        int prevStyle = styler.StyleAt(startPos - 1);
        if((initStyle&FORTH_FLAG_MASK)==FORTH_INTERP_FLAG ||
            ((initStyle&FORTH_STYLE_MASK)==SCE_FORTH_KEYWORD && (prevStyle&FORTH_FLAG_MASK)==FORTH_INTERP_FLAG)){
            //rollback to start of interpretation block
            startPos--;
            length++;
            while(startPos>0 && (styler.StyleAt(startPos)&FORTH_FLAG_MASK)==FORTH_INTERP_FLAG){
                startPos--;
                length++;
            }
            while(startPos>0 && (styler.StyleAt(startPos)&FORTH_STYLE_MASK)==SCE_FORTH_KEYWORD){
                startPos--;
                length++;
            }
            initStyle = styler.StyleAt(startPos);
        }
    }
    int interp_pos1=0, interp_pos2=0;
    int stateFlag = initStyle&FORTH_FLAG_MASK; // flag for state
    bool isInDefinition = stateFlag==FORTH_DEFINITION_FLAG; // flag for inside definition tags state
    bool isPossibleRollback = startPos>0; // flag for possible undefined state by start pos
    bool isEq=false, isBegin=false;
    const char *string_end;
    st=&styler;
    cur_pos=startPos;
    lengthDoc = startPos + length;
    buffer = new char[length];

#ifdef FORTH_DEBUG
    f_debug=fopen("c:\\sci.log","at");
#endif

    WordList &control = *keywordLists[0];
    WordList &keyword = *keywordLists[1];
    WordList &defword = *keywordLists[2];
    WordList &preword1 = *keywordLists[3];
    WordList &preword2 = *keywordLists[4];
    WordList &strings = *keywordLists[5];
    WordList &startdefword = *keywordLists[6];
    WordList &enddefword = *keywordLists[7];
    WordList &gui = *keywordLists[10];
    WordList &oop = *keywordLists[11];
    WordList &word1 = *keywordLists[12];
    WordList &word2 = *keywordLists[13];
    WordList &word3 = *keywordLists[14];
    WordList &word4 = *keywordLists[15];
    WordList &control_in = *keywordLists[16];
    WordList &keyword_in = *keywordLists[17];
    WordList &defword_in = *keywordLists[18];
    WordList &preword_in = *keywordLists[19];
#ifdef FORTH_DEBUG
    fprintf(f_debug,"\nColouriseForthDoc: %d %d %d %d %d\n",startPos,length,initStyle,stateFlag,isInDefinition);
#endif

    // go through all provided text segment
    // using the hand-written state machine shown below
	styler.StartAt(startPos);// , static_cast<char>(STYLE_MAX));
    styler.StartSegment(startPos);
    while(parse(BL,true)!=0)
    if(stateFlag==FORTH_INTERP_FLAG){
        if(pos0!=pos1){
            styler.ColourTo(pos0,SCE_FORTH_DEFAULT|stateFlag);
            styler.ColourTo(pos1-1,SCE_FORTH_DEFAULT|stateFlag);
        }
        if(strcmp("\\",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_COMMENT|stateFlag);
            parse(1,false);
            styler.ColourTo(pos2,SCE_FORTH_COMMENT|stateFlag);
        }else if(strcmp("(",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_COMMENT|stateFlag);
            parse(')',true);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_COMMENT|stateFlag);
        }else if(strcmp("]",buffer)==0){
            stateFlag = isInDefinition?FORTH_DEFINITION_FLAG:0;
            styler.ColourTo(pos2,SCE_FORTH_KEYWORD|stateFlag);
        }else if(enddefword.InList(buffer)) {
            // rollback and clear interpretation state
            styler.Flush();
			styler.StartAt(interp_pos1);// , static_cast<char>(STYLE_MAX));
            styler.StartSegment(interp_pos1);
            cur_pos = interp_pos2;
            stateFlag = isInDefinition?FORTH_DEFINITION_FLAG:0;
            styler.ColourTo(cur_pos,SCE_FORTH_DEFAULT|stateFlag);
        }else if(strcmp("{",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_LOCALE|stateFlag);
            parse('}',false);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_LOCALE|stateFlag);
		}
		else if (strings.InMultiWordsList(buffer, '~', isEq, isBegin, string_end) && isBegin) {
            styler.ColourTo(pos1,SCE_FORTH_STRING|stateFlag);
            parse(*string_end,false);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_STRING|stateFlag);
        }else if(control_in.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_CONTROL|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_CONTROL|stateFlag);
        }else if(keyword_in.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_KEYWORD|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_KEYWORD|stateFlag);
        }else if(defword_in.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_KEYWORD|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_KEYWORD|stateFlag);
            parse(BL,false);
            styler.ColourTo(pos1-1,SCE_FORTH_DEFAULT|stateFlag);
            styler.ColourTo(pos1,SCE_FORTH_DEFWORD|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_DEFWORD|stateFlag);
        }else if(preword_in.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_PREWORD1|stateFlag);
            parse(BL,false);
            styler.ColourTo(pos2,SCE_FORTH_PREWORD1|stateFlag);
        }else if(gui.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_GUI|stateFlag);
        }else if(oop.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_OOP|stateFlag);
        }else if(word1.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD1|stateFlag);
        }else if(word2.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD2|stateFlag);
        }else if(word3.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD3|stateFlag);
        }else if(word4.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD4|stateFlag);
        }else if(is_number(buffer)){
            styler.ColourTo(pos1,SCE_FORTH_NUMBER|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_NUMBER|stateFlag);
        }
    }else if(stateFlag==FORTH_DEFINITION_FLAG){
        if(pos0!=pos1){
            styler.ColourTo(pos0,SCE_FORTH_DEFAULT|stateFlag);
            styler.ColourTo(pos1-1,SCE_FORTH_DEFAULT|stateFlag);
        }
        if(strcmp("\\",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_COMMENT|stateFlag);
            parse(1,false);
            styler.ColourTo(pos2,SCE_FORTH_COMMENT|stateFlag);
        }else if(strcmp("(",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_COMMENT|stateFlag);
            parse(')',true);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_COMMENT|stateFlag);
        }else if(strcmp("[",buffer)==0){
            if (stylingNoInterp) {
                styler.ColourTo(pos1,SCE_FORTH_KEYWORD|FORTH_INTERP_FLAG);
                styler.ColourTo(pos2,SCE_FORTH_KEYWORD|FORTH_INTERP_FLAG);
            } else {
                styler.ColourTo(pos1,SCE_FORTH_KEYWORD|stateFlag);
                styler.ColourTo(pos2,SCE_FORTH_KEYWORD|stateFlag);
                stateFlag = FORTH_INTERP_FLAG;
                interp_pos1 = pos1;
                interp_pos2 = cur_pos;
                isPossibleRollback = false;
            }
        }else if(strcmp("]",buffer)==0){
            if (stylingNoInterp) {
                styler.ColourTo(pos1,SCE_FORTH_KEYWORD|FORTH_INTERP_FLAG);
                styler.ColourTo(pos2,SCE_FORTH_KEYWORD|FORTH_INTERP_FLAG);
            } else
            if(isPossibleRollback){
                // rollback to start of definition because find ] before [
                cur_pos = startPos;
                while(cur_pos>0 && styler.StyleAt(cur_pos)!=SCE_FORTH_DEFAULT){
                    cur_pos--;
                }
                styler.Flush();
				styler.StartAt(cur_pos);// , static_cast<char>(STYLE_MAX));
                styler.StartSegment(cur_pos);
                delete []buffer;
                buffer = new char[lengthDoc - cur_pos];
                isInDefinition = false;
                isPossibleRollback = false;
                stateFlag = 0;
            }
        }else if(enddefword.InList(buffer)) {
            isInDefinition = false;
            isPossibleRollback = false;
            stateFlag = 0;
            styler.ColourTo(pos2,SCE_FORTH_KEYWORD);
        }else if(strcmp("{",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_LOCALE|stateFlag);
            parse('}',false);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_LOCALE|stateFlag);
		}
		else if (strings.InMultiWordsList(buffer, '~', isEq, isBegin, string_end) && isBegin) {
            styler.ColourTo(pos1,SCE_FORTH_STRING|stateFlag);
            parse(*string_end,false);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_STRING|stateFlag);
        }else if(control.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_CONTROL|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_CONTROL|stateFlag);
        }else if(keyword.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_KEYWORD|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_KEYWORD|stateFlag);
        }else if(preword1.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_PREWORD1|stateFlag);
            parse(BL,false);
            styler.ColourTo(pos2,SCE_FORTH_PREWORD1|stateFlag);
        }else if(preword2.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_PREWORD2|stateFlag);
            parse(BL,false);
            styler.ColourTo(pos2,SCE_FORTH_PREWORD2|stateFlag);
            parse(BL,false);
            styler.ColourTo(pos1,SCE_FORTH_STRING|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_STRING|stateFlag);
        }else if(gui.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_GUI|stateFlag);
        }else if(oop.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_OOP|stateFlag);
        }else if(word1.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD1|stateFlag);
        }else if(word2.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD2|stateFlag);
        }else if(word3.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD3|stateFlag);
        }else if(word4.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD4|stateFlag);
        }else if(is_number(buffer)){
            styler.ColourTo(pos1,SCE_FORTH_NUMBER|stateFlag);
            styler.ColourTo(pos2,SCE_FORTH_NUMBER|stateFlag);
        }
    }else{
        if(pos0!=pos1){
            styler.ColourTo(pos0,SCE_FORTH_DEFAULT);
            styler.ColourTo(pos1-1,SCE_FORTH_DEFAULT);
        }
        if(strcmp("\\",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_COMMENT);
            parse(1,false);
            styler.ColourTo(pos2,SCE_FORTH_COMMENT);
        }else if(strcmp("(",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_COMMENT);
            parse(')',true);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_COMMENT);
        }else if(strcmp("[",buffer)==0){
            if (stylingNoInterp) {
                styler.ColourTo(pos1,SCE_FORTH_KEYWORD|FORTH_INTERP_FLAG);
                styler.ColourTo(pos2,SCE_FORTH_KEYWORD|FORTH_INTERP_FLAG);
            } else {
                styler.ColourTo(pos1,SCE_FORTH_KEYWORD);
                styler.ColourTo(pos2,SCE_FORTH_KEYWORD);
                stateFlag = FORTH_INTERP_FLAG;
                interp_pos1 = pos1;
                interp_pos2 = cur_pos;
            }
        }else if(strcmp("]",buffer)==0){
            if (stylingNoInterp) {
                styler.ColourTo(pos1,SCE_FORTH_KEYWORD|FORTH_INTERP_FLAG);
                styler.ColourTo(pos2,SCE_FORTH_KEYWORD|FORTH_INTERP_FLAG);
            } else
            if (isPossibleRollback) {
                // rollback to [ or start
                cur_pos = startPos;
                while(cur_pos>0 && styler.SafeGetCharAt(cur_pos)!='['){
                    cur_pos--;
                }
                if(cur_pos>1) cur_pos-=2;
                styler.Flush();
				styler.StartAt(cur_pos);// , static_cast<char>(STYLE_MAX));
                styler.StartSegment(cur_pos);
                delete []buffer;
                buffer = new char[lengthDoc - cur_pos];
                isInDefinition = false;
                isPossibleRollback = false;
                stateFlag = 0;
            }
        }else if(strcmp("{",buffer)==0){
            styler.ColourTo(pos1,SCE_FORTH_LOCALE);
            parse('}',false);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_LOCALE);
		}
		else if (strings.InMultiWordsList(buffer, '~', isEq, isBegin, string_end) && isBegin) {
            styler.ColourTo(pos1,SCE_FORTH_STRING);
            parse(*string_end,false);
            if(cur_pos<lengthDoc) cur_pos++;
            styler.ColourTo(cur_pos,SCE_FORTH_STRING);
        }else if(startdefword.InList(buffer)) {
            isInDefinition = true;
            stateFlag = FORTH_DEFINITION_FLAG;
            styler.ColourTo(pos1,SCE_FORTH_KEYWORD);
            styler.ColourTo(pos2,SCE_FORTH_KEYWORD);
            if(defword.InList(buffer)) {
                parse(BL,false);
                styler.ColourTo(pos1-1,SCE_FORTH_DEFAULT|stateFlag);
                styler.ColourTo(pos1,SCE_FORTH_DEFWORD|stateFlag);
                styler.ColourTo(pos2,SCE_FORTH_DEFWORD|stateFlag);
            }
        }else if(control.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_CONTROL);
            styler.ColourTo(pos2,SCE_FORTH_CONTROL);
        }else if(keyword.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_KEYWORD);
            styler.ColourTo(pos2,SCE_FORTH_KEYWORD);
        }else if(defword.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_KEYWORD);
            styler.ColourTo(pos2,SCE_FORTH_KEYWORD);
            parse(BL,false);
            styler.ColourTo(pos1-1,SCE_FORTH_DEFAULT);
            styler.ColourTo(pos1,SCE_FORTH_DEFWORD);
            styler.ColourTo(pos2,SCE_FORTH_DEFWORD);
        }else if(preword1.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_PREWORD1);
            parse(BL,false);
            styler.ColourTo(pos2,SCE_FORTH_PREWORD1);
        }else if(preword2.InList(buffer)) {
            styler.ColourTo(pos1,SCE_FORTH_PREWORD2);
            parse(BL,false);
            styler.ColourTo(pos2,SCE_FORTH_PREWORD2);
            parse(BL,false);
            styler.ColourTo(pos1,SCE_FORTH_STRING);
            styler.ColourTo(pos2,SCE_FORTH_STRING);
        }else if(gui.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_GUI);
        }else if(oop.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_OOP);
        }else if(word1.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD1);
        }else if(word2.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD2);
        }else if(word3.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD3);
        }else if(word4.InList(buffer)) {
            styler.ColourTo(pos2,SCE_FORTH_WORD4);
        }else if(is_number(buffer)){
            styler.ColourTo(pos1,SCE_FORTH_NUMBER);
            styler.ColourTo(pos2,SCE_FORTH_NUMBER);
        }
    }
#ifdef FORTH_DEBUG
    fclose(f_debug);
#endif
    delete []buffer;
    return;
}

static void FoldForthDoc(unsigned int startPos, int length, int initStyle,
    WordList *keywordlists[], Accessor &styler)
{
    WordList &startdefword = *keywordlists[6];
    WordList &enddefword = *keywordlists[7];
    WordList &fold_begin = *keywordlists[8];
    WordList &fold_end = *keywordlists[9];

    int line = styler.GetLine(startPos);
    int level = styler.LevelAt(line);
    int levelIndent = 0;
    unsigned int endPos = startPos + length;
    char word[256];
    int wordlen = 0;
    int style = initStyle & FORTH_STYLE_MASK;
    int wordstyle = style;
    // Scan for tokens
    for (unsigned int i = startPos; i < endPos; i++) {
        int c = styler.SafeGetCharAt(i, '\n');
        style = styler.StyleAt(i) & FORTH_STYLE_MASK;
        if (is_whitespace(c)) {
            if (wordlen) { // done with token
                word[wordlen] = '\0';
                // CheckFoldPoint
                if (wordstyle == SCE_FORTH_KEYWORD && startdefword.InList(word)) {
                    levelIndent += 1;
                } else
                if (wordstyle == SCE_FORTH_KEYWORD && enddefword.InList(word)) {
                    levelIndent -= 1;
                } else
                if (fold_begin.InList(word)) {
                    levelIndent += 1;
                } else
                if (fold_end.InList(word)) {
                    levelIndent -= 1;
                }
                wordlen = 0;
            }
        }
        else if (!(style == SCE_FORTH_COMMENT || style == SCE_FORTH_COMMENT_ML
            || style == SCE_FORTH_LOCALE || style == SCE_FORTH_STRING
            || style == SCE_FORTH_DEFWORD || style == SCE_FORTH_PREWORD1
            || style == SCE_FORTH_PREWORD2)) {
            if (wordlen) {
                if (wordlen < 255) {
                    word[wordlen] = static_cast<char>(c);
                    wordlen++;
                }
            } else { // start scanning at first word character
                word[0] = static_cast<char>(c);
                wordlen = 1;
                wordstyle = style;
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

static const char * const forthWordLists[] = {
            "control keywords",
            "keywords",
            "definition words",
            "prewords with one argument",
            "prewords with two arguments",
            "string definition keywords",
            "definition start words",
            "definition end words",
            "folding start words",
            "folding end words",
            "GUI",
            "OOP",
            "user defined words 1",
            "user defined words 2",
            "user defined words 3",
            "user defined words 4",
            "control keywords in interpretation",
            "keywords in interpretation",
            "definition words in interpretation",
            "prewords in interpretation",
            0,
        };

LexerModule lmForth(SCLEX_FORTH, ColouriseForthDoc, "forth", FoldForthDoc, forthWordLists);
//!-end-[ForthImprovement]
