// Scintilla source code edit control
/** @file WordList.h
 ** Hold a list of words.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef WORDLIST_H
#define WORDLIST_H

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

/**
 */
class WordList {
	// Each word contains at least one character - a empty word acts as sentinel at the end.
	char **words;
	char *list;
	int len;
	bool onlyLineEnds;	///< Delimited by any white space or only line ends
	int starts[256];
public:
	explicit WordList(bool onlyLineEnds_ = false);
	~WordList();
	operator bool() const;
	bool operator!=(const WordList &other) const;
	int Length() const;
	void Clear();
	void Set(const char *s);
	bool InList(const char *s) const;
	bool InListAbbreviated(const char *s, const char marker) const;
	const char *WordAt(int n) const;
	bool InMultiWordsList(const char *s, const char marker, bool &eq, bool &begin); //!-add-[InMultiWordsList]
	bool InMultiWordsList(const char *s, const char marker, bool &eq, bool &begin, const char* &keyword_end); //!-add-[InMultiWordsList]
	bool InListPartly(const char *s, const char marker, int &mainLen, int &finLen); //!-add-[InMultiWordsList]
};

#ifdef SCI_NAMESPACE
}
#endif

#endif
