// Scintilla source code edit control
/** @file CallTip.cxx
 ** Code for displaying call tips.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <string>

#include "Platform.h"

#include "Scintilla.h"

#include "SplitVector.h" //!-add-[BetterCalltips]
#include "StringCopy.h"
#include "CallTip.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

CallTip::CallTip() {
	wCallTip = 0;
	inCallTipMode = false;
	posStartCallTip = 0;
	rectUp = PRectangle(0,0,0,0);
	rectDown = PRectangle(0,0,0,0);
	lineHeight = 1;
	offsetMain = 0;
/*!	startHighlight = 0;
	endHighlight = 0;*/
	highlightChanged = false; //!-change-[BetterCalltips]
	tabSize = 0;
	above = false;
	useStyleCallTip = false;    // for backwards compatibility

	insetX = 5;
	widthArrow = 14;
	borderHeight = 2; // Extra line for border and an empty line at top and bottom.
	verticalOffset = 1;

#ifdef __APPLE__
	// proper apple colours for the default
	colourBG = ColourDesired(0xff, 0xff, 0xc6);
	colourUnSel = ColourDesired(0, 0, 0);
#else
	colourBG = ColourDesired(0xff, 0xff, 0xff);
	colourUnSel = ColourDesired(0x80, 0x80, 0x80);
#endif
	colourSel = ColourDesired(0, 0, 0x80);
	colourShade = ColourDesired(0, 0, 0);
	colourLight = ColourDesired(0xc0, 0xc0, 0xc0);
	codePage = 0;
	clickPlace = 0;
}

CallTip::~CallTip() {
	font.Release();
	wCallTip.Destroy();
}

// Although this test includes 0, we should never see a \0 character.
static bool IsArrowCharacter(char ch) {
	return (ch == 0) || (ch == '\001') || (ch == '\002');
}

// We ignore tabs unless a tab width has been set.
bool CallTip::IsTabCharacter(char ch) const {
	return (tabSize > 0) && (ch == '\t');
}

int CallTip::NextTabPos(int x) const {
	if (tabSize > 0) {              // paranoia... not called unless this is true
		x -= insetX;                // position relative to text
		x = (x + tabSize) / tabSize;  // tab "number"
		return tabSize*x + insetX;  // position of next tab
	} else {
		return x + 1;                 // arbitrary
	}
}

// Draw a section of the call tip that does not include \n in one colour.
// The text may include up to numEnds tabs or arrow characters.
void CallTip::DrawChunk(Surface *surface, int &x, const char *s,
	int posStart, int posEnd, int ytext, PRectangle rcClient,
	bool highlight, bool draw) {
	s += posStart;
	int len = posEnd - posStart;

	// Divide the text into sections that are all text, or that are
	// single arrows or single tab characters (if tabSize > 0).
	int maxEnd = 0;
	const int numEnds = 10;
	int ends[numEnds + 2];
	for (int i=0; i<len; i++) {
		if ((maxEnd < numEnds) &&
		        (IsArrowCharacter(s[i]) || IsTabCharacter(s[i]))) {
			if (i > 0)
				ends[maxEnd++] = i;
			ends[maxEnd++] = i+1;
		}
	}
	ends[maxEnd++] = len;
	int startSeg = 0;
	int xEnd;
	for (int seg = 0; seg<maxEnd; seg++) {
		int endSeg = ends[seg];
		if (endSeg > startSeg) {
			if (IsArrowCharacter(s[startSeg])) {
				xEnd = x + widthArrow;
				bool upArrow = s[startSeg] == '\001';
				rcClient.left = static_cast<XYPOSITION>(x);
				rcClient.right = static_cast<XYPOSITION>(xEnd);
				if (draw) {
					const int halfWidth = widthArrow / 2 - 3;
					const int quarterWidth = halfWidth / 2;
					const int centreX = x + widthArrow / 2 - 1;
					const int centreY = static_cast<int>(rcClient.top + rcClient.bottom) / 2;
					surface->FillRectangle(rcClient, colourBG);
					PRectangle rcClientInner(rcClient.left + 1, rcClient.top + 1,
					                         rcClient.right - 2, rcClient.bottom - 1);
					surface->FillRectangle(rcClientInner, colourUnSel);

					if (upArrow) {      // Up arrow
						Point pts[] = {
    						Point::FromInts(centreX - halfWidth, centreY + quarterWidth),
    						Point::FromInts(centreX + halfWidth, centreY + quarterWidth),
    						Point::FromInts(centreX, centreY - halfWidth + quarterWidth),
						};
						surface->Polygon(pts, ELEMENTS(pts), colourBG, colourBG);
					} else {            // Down arrow
						Point pts[] = {
    						Point::FromInts(centreX - halfWidth, centreY - quarterWidth),
    						Point::FromInts(centreX + halfWidth, centreY - quarterWidth),
    						Point::FromInts(centreX, centreY + halfWidth - quarterWidth),
						};
						surface->Polygon(pts, ELEMENTS(pts), colourBG, colourBG);
					}
				}
				offsetMain = xEnd;
				if (upArrow) {
					rectUp = rcClient;
				} else {
					rectDown = rcClient;
				}
			} else if (IsTabCharacter(s[startSeg])) {
				xEnd = NextTabPos(x);
			} else {
				xEnd = x + RoundXYPosition(surface->WidthText(font, s + startSeg, endSeg - startSeg));
				if (draw) {
					rcClient.left = static_cast<XYPOSITION>(x);
					rcClient.right = static_cast<XYPOSITION>(xEnd);
					surface->DrawTextTransparent(rcClient, font, static_cast<XYPOSITION>(ytext),
										s+startSeg, endSeg - startSeg,
					                             highlight ? colourSel : colourUnSel);
				}
			}
			x = xEnd;
			startSeg = endSeg;
		}
	}
}

/*!
int CallTip::PaintContents(Surface *surfaceWindow, bool draw) {
	PRectangle rcClientPos = wCallTip.GetClientPosition();
	PRectangle rcClientSize(0.0f, 0.0f, rcClientPos.right - rcClientPos.left,
	                        rcClientPos.bottom - rcClientPos.top);
	PRectangle rcClient(1.0f, 1.0f, rcClientSize.right - 1, rcClientSize.bottom - 1);

	// To make a nice small call tip window, it is only sized to fit most normal characters without accents
	int ascent = RoundXYPosition(surfaceWindow->Ascent(font) - surfaceWindow->InternalLeading(font));

	// For each line...
	// Draw the definition in three parts: before highlight, highlighted, after highlight
	int ytext = static_cast<int>(rcClient.top) + ascent + 1;
	rcClient.bottom = ytext + surfaceWindow->Descent(font) + 1;
	const char *chunkVal = val.c_str();
	bool moreChunks = true;
	int maxWidth = 0;

	while (moreChunks) {
		const char *chunkEnd = strchr(chunkVal, '\n');
		if (chunkEnd == NULL) {
			chunkEnd = chunkVal + strlen(chunkVal);
			moreChunks = false;
		}
		int chunkOffset = static_cast<int>(chunkVal - val.c_str());
		int chunkLength = static_cast<int>(chunkEnd - chunkVal);
		int chunkEndOffset = chunkOffset + chunkLength;
		int thisStartHighlight = Platform::Maximum(startHighlight, chunkOffset);
		thisStartHighlight = Platform::Minimum(thisStartHighlight, chunkEndOffset);
		thisStartHighlight -= chunkOffset;
		int thisEndHighlight = Platform::Maximum(endHighlight, chunkOffset);
		thisEndHighlight = Platform::Minimum(thisEndHighlight, chunkEndOffset);
		thisEndHighlight -= chunkOffset;
		rcClient.top = static_cast<XYPOSITION>(ytext - ascent - 1);

		int x = insetX;     // start each line at this inset

		DrawChunk(surfaceWindow, x, chunkVal, 0, thisStartHighlight,
			ytext, rcClient, false, draw);
		DrawChunk(surfaceWindow, x, chunkVal, thisStartHighlight, thisEndHighlight,
			ytext, rcClient, true, draw);
		DrawChunk(surfaceWindow, x, chunkVal, thisEndHighlight, chunkLength,
			ytext, rcClient, false, draw);

		chunkVal = chunkEnd + 1;
		ytext += lineHeight;
		rcClient.bottom += lineHeight;
		maxWidth = Platform::Maximum(maxWidth, x);
	}
	return maxWidth;
}
*/
//!-start-[BetterCalltips]
#define IS_WS(ch) (((ch) == ' ') || ((ch) == '\t'))
void CallTip::WrapLine(const char *text, int offset, int length, SplitVector<int> &wrapPosList) {
	wrapPosList.DeleteAll();
	int lastWrapPos = -1;
	int nextWrapBound = offset + wrapBound;
	for (int i = offset; i < offset + length; i++) {
		if (IS_WS(text[i]) && (i > offset) && !IS_WS(text[i - 1])) {
			lastWrapPos = i;
		}
		if ((i >= nextWrapBound) && (lastWrapPos != -1)) {
			wrapPosList.Insert(wrapPosList.Length(), lastWrapPos);
			nextWrapBound = lastWrapPos + wrapBound;
			lastWrapPos = -1;
		}
	}
}

PRectangle CallTip::PaintContents(Surface *surfaceWindow, bool draw) {
	PRectangle rcClientPos = wCallTip.GetClientPosition();
	PRectangle rcClientSize(0, 0, rcClientPos.right - rcClientPos.left,
	                        rcClientPos.bottom - rcClientPos.top);
	PRectangle rcClient(1, 1, rcClientSize.right - 1, rcClientSize.bottom - 1);

	// To make a nice small call tip window, it is only sized to fit most normal characters without accents
	int ascent = static_cast<int>(surfaceWindow->Ascent(font) - surfaceWindow->InternalLeading(font));

	int ytext = static_cast<int>(rcClient.top) + ascent + 1;
	rcClient.bottom = ytext + surfaceWindow->Descent(font) + 1;
	const char *chunkVal = val.c_str();
	bool moreChunks = true;
	int maxWidth = 0;
	int numLines = 0;
	SplitVector<int> wrapPosList;

	while (moreChunks) {
		const char *chunkEnd = strchr(chunkVal, '\n');
		if (chunkEnd == NULL) {
			chunkEnd = chunkVal + strlen(chunkVal);
			moreChunks = false;
		}
		int chunkOffset = static_cast<int>(chunkVal - val.c_str());
		int chunkLength = static_cast<int>(chunkEnd - chunkVal);
		int chunkEndOffset = chunkOffset + chunkLength;

		rcClient.top = ytext - ascent - 1;

		int x = insetX;     // start each line at this inset

		if (wrapBound)
			WrapLine(val.c_str(), chunkOffset, chunkLength, wrapPosList);

		int off = chunkOffset;
		do {
			int hlStart = chunkEndOffset;
			int hlEnd = chunkEndOffset;
			// find next highlighted range within the rest of the current line
			int i;
			for (i = 0; i < startHighlight.Length(); i++) {
				if ((startHighlight[i] >= off) && (startHighlight[i] < chunkEndOffset)) {
					if (hlStart > startHighlight[i]) {
						hlStart = startHighlight[i];
						hlEnd = endHighlight[i];
					}
				}
			}
			if (hlEnd > chunkEndOffset) hlEnd = chunkEndOffset;

			// draw definition part (not highlighted)
			int wrapPos = off;
			for (i = 0; i < wrapPosList.Length(); i++) {
				if ((wrapPosList[i] >= off) && (wrapPosList[i] <= hlStart) && (wrapPos != wrapPosList[i])) {
					// line wrap is needed here
					DrawChunk(surfaceWindow, x, chunkVal, wrapPos - chunkOffset, wrapPosList[i] - chunkOffset,
						ytext, rcClient, false, draw);
					wrapPos = wrapPosList[i];
					ytext += lineHeight;
					rcClient.bottom += lineHeight;
					maxWidth = Platform::Maximum(maxWidth, x);
					x = insetX;
					numLines++;
				}
			}
			DrawChunk(surfaceWindow, x, chunkVal, wrapPos - chunkOffset, hlStart - chunkOffset,
				ytext, rcClient, false, draw);
			// draw definition part (highlighted)
			wrapPos = hlStart;
			for (i = 0; i < wrapPosList.Length(); i++) {
				if ((wrapPosList[i] >= hlStart) && (wrapPosList[i] <= hlEnd) && (wrapPos != wrapPosList[i])) {
					DrawChunk(surfaceWindow, x, chunkVal, wrapPos - chunkOffset, wrapPosList[i] - chunkOffset,
						ytext, rcClient, true, draw);
					wrapPos = wrapPosList[i];
					ytext += lineHeight;
					rcClient.bottom += lineHeight;
					maxWidth = Platform::Maximum(maxWidth, x);
					x = insetX;
					numLines++;
				}
			}
			DrawChunk(surfaceWindow, x, chunkVal, wrapPos - chunkOffset, hlEnd - chunkOffset,
				ytext, rcClient, true, draw);

			off = hlEnd;
		} while (off < chunkEndOffset);
		chunkVal = chunkEnd + 1;
		ytext += lineHeight;
		rcClient.bottom += lineHeight;
		maxWidth = Platform::Maximum(maxWidth, x);
		numLines++;
	}
	return PRectangle(0, 0, maxWidth + insetX, lineHeight * numLines - surfaceWindow->InternalLeading(font) + 2 + 2);
}
//!-end-[BetterCalltips]

void CallTip::PaintCT(Surface *surfaceWindow) {
	if (val.empty())
		return;
	PRectangle rcClientPos = wCallTip.GetClientPosition();
	PRectangle rcClientSize(0.0f, 0.0f, rcClientPos.right - rcClientPos.left,
	                        rcClientPos.bottom - rcClientPos.top);
	PRectangle rcClient(1.0f, 1.0f, rcClientSize.right - 1, rcClientSize.bottom - 1);

	surfaceWindow->FillRectangle(rcClient, colourBG);

	offsetMain = insetX;    // initial alignment assuming no arrows
	PaintContents(surfaceWindow, true);

#ifndef __APPLE__
	// OSX doesn't put borders on "help tags"
	// Draw a raised border around the edges of the window
	surfaceWindow->MoveTo(0, static_cast<int>(rcClientSize.bottom) - 1);
	surfaceWindow->PenColour(colourShade);
	surfaceWindow->LineTo(static_cast<int>(rcClientSize.right) - 1, static_cast<int>(rcClientSize.bottom) - 1);
	surfaceWindow->LineTo(static_cast<int>(rcClientSize.right) - 1, 0);
	surfaceWindow->PenColour(colourLight);
	surfaceWindow->LineTo(0, 0);
	surfaceWindow->LineTo(0, static_cast<int>(rcClientSize.bottom) - 1);
#endif
}

void CallTip::MouseClick(Point pt) {
	clickPlace = 0;
	if (rectUp.Contains(pt))
		clickPlace = 1;
	if (rectDown.Contains(pt))
		clickPlace = 2;
}

PRectangle CallTip::CallTipStart(int pos, Point pt, int textHeight, const char *defn,
                                 const char *faceName, int size,
                                 int codePage_, int characterSet,
								 int technology, Window &wParent) {
/*!	clickPlace = 0;
	val = defn;
	codePage = codePage_;
	Surface *surfaceMeasure = Surface::Allocate(technology);
	if (!surfaceMeasure)
		return PRectangle();
	surfaceMeasure->Init(wParent.GetID());
	surfaceMeasure->SetUnicodeMode(SC_CP_UTF8 == codePage);
	surfaceMeasure->SetDBCSMode(codePage);
	startHighlight = 0;
	endHighlight = 0;
	inCallTipMode = true;
	posStartCallTip = pos;
	XYPOSITION deviceHeight = static_cast<XYPOSITION>(surfaceMeasure->DeviceHeightFont(size));
	FontParameters fp(faceName, deviceHeight / SC_FONT_SIZE_MULTIPLIER, SC_WEIGHT_NORMAL, false, 0, technology, characterSet);
	font.Create(fp);
	// Look for multiple lines in the text
	// Only support \n here - simply means container must avoid \r!
	int numLines = 1;
	const char *newline;
	const char *look = val.c_str();
	rectUp = PRectangle(0,0,0,0);
	rectDown = PRectangle(0,0,0,0);
	offsetMain = insetX;            // changed to right edge of any arrows
	int width = PaintContents(surfaceMeasure, false) + insetX;
	while ((newline = strchr(look, '\n')) != NULL) {
		look = newline + 1;
		numLines++;
	}
	lineHeight = RoundXYPosition(surfaceMeasure->Height(font));

	// The returned
	// rectangle is aligned to the right edge of the last arrow encountered in
	// the tip text, else to the tip text left edge.
	int height = lineHeight * numLines - static_cast<int>(surfaceMeasure->InternalLeading(font)) + borderHeight * 2;
	delete surfaceMeasure;
	if (above) {
		return PRectangle(pt.x - offsetMain, pt.y - verticalOffset - height, pt.x + width - offsetMain, pt.y - verticalOffset);
	} else {
		return PRectangle(pt.x - offsetMain, pt.y + verticalOffset + textHeight, pt.x + width - offsetMain, pt.y + verticalOffset + textHeight + height);
	}
}
*/
//!-start-[BetterCalltips]
	clickPlace = 0;
/*	if (val)
		delete []val;
	val = new char[strlen(defn) + 1];
	if (!val)
		return PRectangle();*/
	val = defn;
	codePage = codePage_;
	Surface *surfaceMeasure = Surface::Allocate(technology);
	if (!surfaceMeasure)
		return PRectangle();
	surfaceMeasure->Init(wParent.GetID());
	surfaceMeasure->SetUnicodeMode(SC_CP_UTF8 == codePage);
	surfaceMeasure->SetDBCSMode(codePage);
	ClearHighlight();
	inCallTipMode = true;
	posStartCallTip = pos;
	int deviceHeight = surfaceMeasure->DeviceHeightFont(size);
	FontParameters fp(faceName, deviceHeight / SC_FONT_SIZE_MULTIPLIER, SC_WEIGHT_NORMAL, false, 0, technology, characterSet);
	font.Create(fp);
	// Look for multiple lines in the text
	// Only support \n here - simply means container must avoid \r!
	rectUp = PRectangle(0,0,0,0);
	rectDown = PRectangle(0,0,0,0);
	offsetMain = insetX;            // changed to right edge of any arrows
	lineHeight = surfaceMeasure->Height(font);
	PRectangle rcSize = PaintContents(surfaceMeasure, false);
	delete surfaceMeasure;

	// Extra line for border and an empty line at top and bottom. The returned
	// rectangle is aligned to the right edge of the last arrow encountered in
	// the tip text, else to the tip text left edge.
	if (above) {
		return PRectangle(pt.x - offsetMain, pt.y - 1 - rcSize.Height(), pt.x + rcSize.Width() - offsetMain, pt.y - 1);
	} else {
		return PRectangle(pt.x - offsetMain, pt.y + 1 + textHeight, pt.x - offsetMain + rcSize.Width(), pt.y + 1 + rcSize.Height() + textHeight);
	}
}
//!-end-[BetterCalltips]

void CallTip::CallTipCancel() {
	inCallTipMode = false;
	if (wCallTip.Created()) {
		wCallTip.Destroy();
	}
}

/*!
void CallTip::SetHighlight(int start, int end) {
	// Avoid flashing by checking something has really changed
	if ((start != startHighlight) || (end != endHighlight)) {
		startHighlight = start;
		endHighlight = (end > start) ? end : start;
		if (wCallTip.Created()) {
			wCallTip.InvalidateAll();
		}
	}
}
*/
//!-start-[BetterCalltips]
void CallTip::SetHighlight(int start, int end) {
	ClearHighlight();
	AddHighlight(start, end);
	UpdateHighlight();
}

void CallTip::AddHighlight(int start, int end) {
	int i = 0;
	if (start == end) return;
	while (i < startHighlight.Length()) {
		if ((start == startHighlight[i]) && (end == endHighlight[i])) return;
		// check if ranges overlap
		if ((start <= endHighlight[i]) && (end >= startHighlight[i])) {
			// combine ranges
			start = Platform::Minimum(start, startHighlight[i]);
			end = Platform::Maximum(end, endHighlight[i]);
			// delete old range
			startHighlight.Delete(i);
			endHighlight.Delete(i);
		}
		else {
			i++;
		}
	}
	startHighlight.Insert(startHighlight.Length(), start);
	endHighlight.Insert(endHighlight.Length(), end);
	highlightChanged = true;
}

void CallTip::ClearHighlight() {
	if (startHighlight.Length() != 0) highlightChanged = true;
	startHighlight.DeleteAll();
	endHighlight.DeleteAll();
}

void CallTip::UpdateHighlight() {
	// Avoid flashing by checking something has really changed
	if (highlightChanged && wCallTip.Created()) {
		highlightChanged = false;
		wCallTip.InvalidateAll();
	}
}
//!-end-[BetterCalltips]

// Set the tab size (sizes > 0 enable the use of tabs). This also enables the
// use of the STYLE_CALLTIP.
void CallTip::SetTabSize(int tabSz) {
	tabSize = tabSz;
	useStyleCallTip = true;
}

// Set the calltip position, below the text by default or if above is false
// else above the text.
void CallTip::SetPosition(bool aboveText) {
	above = aboveText;
}

// It might be better to have two access functions for this and to use
// them for all settings of colours.
void CallTip::SetForeBack(const ColourDesired &fore, const ColourDesired &back) {
	colourBG = back;
	colourUnSel = fore;
}
//!-start-[BetterCalltips]
void CallTip::SetWrapBound(int wrapBnd) {
	wrapBound = wrapBnd;
}
//!-end-[BetterCalltips]
