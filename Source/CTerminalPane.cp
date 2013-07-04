// ===========================================================================
//	CTerminalPane.cp		   й1996-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#include "CTerminalPane.h"

#include <Sound.h>
#include <LowMem.h>
#include <Fonts.h>

const SInt16	resetInvalsChar				= 3;

// ===========================================================================
//		е Display parameters
// ===========================================================================
//	Most of the appearance of this pane is hard-wired. It works for the
//	simple demonstration we're doing here, but isn't intended for industrial-
//	strength work. :-)

enum {
	pixelsX = 6,					// default font char width (Monaco 9)
	pixelsY = 11,					// default font char height
	
	sizeX = 488,					// 80 columns * 6 pixels + 8 margin
	sizeY = 272						// 24 rows * 11 pixels + 8 margin
};

// ===========================================================================
//		е Standard ASCII keycodes
// ===========================================================================

enum {
	charNUL = 0,
	charBEL = 7,
	charBS,
	charHT,
	charLF,
	charVT,
	charFF,
	charCR,
	charDEL = 127
};


// ===========================================================================

#pragma mark еее CTerminalPane еее


// ---------------------------------------------------------------------------
//	е CTerminalPane									[public]
// ---------------------------------------------------------------------------
//	Stream constructor

CTerminalPane::CTerminalPane(
	LStream* 	inStream)

	: LView(inStream)
{
	InitTerm();
}


// ---------------------------------------------------------------------------
//	е ~CTerminalPane								[public, virtual]
// ---------------------------------------------------------------------------
//	Destructor

CTerminalPane::~CTerminalPane()
{
	// nothing
}


// ===========================================================================

#pragma mark -
#pragma mark ее terminal primitives

// ---------------------------------------------------------------------------
//	е DoWriteChar									[public, virtual]
// ---------------------------------------------------------------------------
//	Write a single character to the terminal.

void
CTerminalPane::DoWriteChar(
	char 	inChar)
{
		// Parse a few control characters.

	switch (inChar) {

		case charNUL: {
			break;
		}
		
		case charBEL: {
			::SysBeep(0);
			break;
		}

		case charBS: {
			if (mCurrentCursor.col > 0) {
				mCurrentCursor.col--;
			}
			CursorMoved();
			break;
		}

		case charHT: {
			mCurrentCursor.col = (static_cast<SInt16>(((mCurrentCursor.col + 7) / 8))) * 8;
			if (mCurrentCursor.col >= maxX) {
				mCurrentCursor.col = maxX-1;
			}
			CursorMoved();
			break;
		}
			
		case charLF: {
			if (mCurrentCursor.row < maxY-1) {
				mCurrentCursor.row++;
			} else {
				ScrollTerm();
			}
			CursorMoved();
			break;
		}
			
		case charFF: {
			DoClearScreen();
			break;
		}
			
		case charCR: {
			mCurrentCursor.col = 0;
			CursorMoved();
			break;
		}
		
		default: {
			PutCharAt(mCurrentCursor, inChar);
			InvalChar(mCurrentCursor);
			if (mCurrentCursor.col < maxX-1) {
				mCurrentCursor.col++;
			}
			CursorMoved();
		}
	}
}


// ---------------------------------------------------------------------------
//	е DoWriteStr									[public, virtual]
// ---------------------------------------------------------------------------
//	Print a C string to the terminal.

void
CTerminalPane::DoWriteStr(
	const char* 	inString)		// the string to be printed
{
		// Optimize for text characters.
	while (*inString) {
		if (*inString >= char_Space) {
			TermCharT originalChar = mCurrentCursor;
			while (*inString >= char_Space) {
				PutCharAt(mCurrentCursor, *(inString++));
				if (mCurrentCursor.col < maxX-1) {
					mCurrentCursor.col++;
				}
			}
			InvalChar(originalChar, mCurrentCursor.col - originalChar.col);
			CursorMoved();
		}
		if (*inString) {
			DoWriteChar(*(inString++));
		}
	}
}


// ---------------------------------------------------------------------------
//	е DoWriteBfr									[public, virtual]
// ---------------------------------------------------------------------------
//	Write a string of arbitrary length to the buffer.

void
CTerminalPane::DoWriteBfr(
	const char*	inBuffer,
	SInt32		inByteCount)
{
	while (inByteCount--) {
		DoWriteChar(*(inBuffer++));
	}
}


// ---------------------------------------------------------------------------
//	е DoWriteCharNum								[public, virtual]
// ---------------------------------------------------------------------------
//	A debugging routine. Writes a character number in decimal with
//	optional brackets (i.e. [240]).

void
CTerminalPane::DoWriteCharNum(
	char	inChar,
	char	inLeftBracket,
	char	inRightBracket)
{
	Str255 cNumber;

	if (inLeftBracket) {
		DoWriteChar(inLeftBracket);
	}

	::NumToString(static_cast<SInt32>(inChar), cNumber);
	cNumber[cNumber[0]+1] = '\0';
	DoWriteStr((char*) (&cNumber)+1);

	if (inRightBracket)	 {
		DoWriteChar(inRightBracket);
	}
}


// ---------------------------------------------------------------------------
//	е DoClearScreen									[public, virtual]
// ---------------------------------------------------------------------------
//	Clear the entire screen display and reset the cursor to the upper left corner

void
CTerminalPane::DoClearScreen()
{
	mCurrentCursor.row = mCurrentCursor.col = 0;
	ClearToEOS(mCurrentCursor);
	CursorMoved();
}


// ===========================================================================

#pragma mark -
#pragma mark ее configuration

// ---------------------------------------------------------------------------
//	е SetBlinking									[public, virtual]
// ---------------------------------------------------------------------------

void
CTerminalPane::SetBlinking(
	bool	inBlinkMode)
{
	mBlinkCursor = inBlinkMode;
	if ((inBlinkMode) && (IsActive())) {
		mPreviousCursor = mCurrentCursor;
		CursorMoved();
	} else {
		mCursorBlinkVisible = true;
		InvalChar(mCurrentCursor);
	}
}


// ===========================================================================

#pragma mark -
#pragma mark ее drawing

// ---------------------------------------------------------------------------
//	е DrawSelf										[protected, virtual]
// ---------------------------------------------------------------------------
//	Drawing code adapted from LTable::DrawSelf.

void
CTerminalPane::DrawSelf()
{
		// Reset the invalidate optimization.
	mCharsToInvalLine = resetInvalsChar;

		// Determine cells that need updating. Rather than checking
		// on a character by character basis, we just see which cells intersect
		// the bounding box of the update region. This is relatively
		// fast, but may result in unnecessary cell updates for
		// non-rectangular update regions.
	RgnHandle localUpdateRgnH = GetLocalUpdateRgn();
	Rect updateRect = (**localUpdateRgnH).rgnBBox;
	::DisposeRgn(localUpdateRgnH);
	
		// Find character at top left of update rect.
	SPoint32 topLeftUpdate;
	TermCharT topLeftChar;
	LocalToImagePoint(topLeft(updateRect), topLeftUpdate);
	FetchCharHitBy(topLeftUpdate, topLeftChar);

	if (topLeftChar.row < 0) {
		topLeftChar.row = 0;
	}
	if (topLeftChar.col < 0) {
		topLeftChar.col = 0;
	}

		// Find cell at bottom right of update rect.
	SPoint32 botRightUpdate;
	TermCharT botRightChar;
	LocalToImagePoint(botRight(updateRect), botRightUpdate);
	FetchCharHitBy(botRightUpdate, botRightChar);

	if (botRightChar.row >= mRows) {
		botRightChar.row = mRows - 1;
	}
	if (botRightChar.col >= mCols) {
		botRightChar.col = mCols - 1;
	}
	
		// Hard-wire for Monaco 9 (at least for now...).
	::TextFont(kFontIDMonaco);
	::TextFace(normal);
	::TextSize(9);

		// Draw each line within the update rect.
	TermCharT theChar;
	theChar.col = topLeftChar.col;
	
	for (theChar.row = topLeftChar.row; theChar.row <= botRightChar.row; theChar.row++) {
		Rect charFrame;
		if (FetchLocalCharFrame(theChar, charFrame)) {
			::MoveTo(charFrame.left, charFrame.bottom - 2);		//! TEMPORARY: hard-code the baseline
			::DrawText(&mScreenBfr[theChar.row][theChar.col], 0, botRightChar.col - topLeftChar.col + 1);
		}
	}
	
		// If we overwrote the cursor, re-highlight it.
	if ((topLeftChar.col <= mCurrentCursor.col)
		&& (botRightChar.col >= mCurrentCursor.col)
		&& (topLeftChar.row <= mCurrentCursor.row)
		&& (botRightChar.row >= mCurrentCursor.row)
		&& mCursorBlinkVisible) {
		
		InvertChar(mCurrentCursor);
	}
}


// ===========================================================================

#pragma mark -
#pragma mark ее blinking cursor support

// ---------------------------------------------------------------------------
//	е ActivateSelf									[protected, virtual]
// ---------------------------------------------------------------------------
//	Start the cursor blinking.

void
CTerminalPane::ActivateSelf()
{
	mCursorBlinkVisible = true;
	InvalChar(mCurrentCursor);
	StartIdling();
}


// ---------------------------------------------------------------------------
//	е DeactivateSelf								[protected, virtual]
// ---------------------------------------------------------------------------
//	Stop cursor blinking.

void
CTerminalPane::DeactivateSelf()
{
	StopIdling();
	mCursorBlinkVisible = true;
	InvalChar(mCurrentCursor);
}


// ---------------------------------------------------------------------------
//	е SpendTime										[protected, virtual]
// ---------------------------------------------------------------------------
//	This is where we cause the cursor to blink.

void
CTerminalPane::SpendTime(
	const EventRecord& /* inMacEvent */)
{
	if (!IsActive()) {
		return;
	}
	
	if (mBlinkCursor) {
		if (LMGetTicks() >= mCursorBlinkTick + GetCaretTime()) {
			mCursorBlinkVisible = !mCursorBlinkVisible;
			mCursorBlinkTick = LMGetTicks();
			InvalChar(mCurrentCursor);
		}
	}
}


// ===========================================================================

#pragma mark -
#pragma mark ее internal screen primitives

// ---------------------------------------------------------------------------
//	е ClearToEOL									[protected, virtual]
// ---------------------------------------------------------------------------
//	Clear screen from the specified position to the end of that line.

void
CTerminalPane::ClearToEOL(
	const TermCharT& 	inStartChar)		// where to start clearing
{
	TermCharT theChar = inStartChar;
	
	InvalChar(theChar, mCols - theChar.col);
	while (theChar.col < maxX) {				//! TEMPORARY: bypass PutCharAt
		mScreenBfr[theChar.row][theChar.col++] = char_Space;
	}
}


// ---------------------------------------------------------------------------
//	е ClearToEOS									[protected, virtual]
// ---------------------------------------------------------------------------
//	Clear screen from the specified position to the end of the screen.

void
CTerminalPane::ClearToEOS(
	const TermCharT& 	inStartChar)
{
	TermCharT theChar = inStartChar;
	while (theChar.row < mRows) {
		ClearToEOL(theChar);
		theChar.row++;
		theChar.col = 0;
	}
}


// ---------------------------------------------------------------------------
//	е ScrollTerm									[protected, virtual]
// ---------------------------------------------------------------------------

void
CTerminalPane::ScrollTerm()
{
	::BlockMoveData(&mScreenBfr[1][0], &mScreenBfr[0][0], (maxY-1) * maxX);
	TermCharT theChar = { mRows-1, 0 };
	ClearToEOL(theChar);
	Refresh();
}


// ---------------------------------------------------------------------------
//	е CursorMoved									[protected, virtual]
// ---------------------------------------------------------------------------
//	Update the cursor display. If several characters are being displayed at
//	once on the same line, we invalidate the entire line. (This is assuming
//	that several more characters will arrive before we can process the next
//	update event; thus we can reduce the number of calls to InvalChar.)

void
CTerminalPane::CursorMoved()
{
	mCursorBlinkVisible = true;
	mCursorBlinkTick = LMGetTicks();

	if (mPreviousCursor.row == mCurrentCursor.row) {
		switch (mCharsToInvalLine) {
			case 0:							// optimization to reduce calls to InvalRect()
				break;
			case 1:
				InvalChar(mCurrentCursor, mCols - mCurrentCursor.col);
				mCharsToInvalLine = 0;
				break;
			default:
				InvalChar(mPreviousCursor);
				InvalChar(mCurrentCursor);
				mCharsToInvalLine--;
		}
	} else {
		InvalChar(mPreviousCursor);
		InvalChar(mCurrentCursor);
		mCharsToInvalLine = resetInvalsChar;
	}
	
	mPreviousCursor = mCurrentCursor;
}


// ---------------------------------------------------------------------------
//	е InvalChar										[protected, virtual]
// ---------------------------------------------------------------------------

void
CTerminalPane::InvalChar(
	const TermCharT&	inChar,				// cursor position to invalidate
	CharIndexT			inNumChars)			// (optional, default=1) number of chars to invalidate
{
	FocusDraw();

	Rect charRect;
	if (FetchLocalCharFrame(inChar, charRect, inNumChars, 1)) {
		LocalToPortPoint(topLeft(charRect));
		LocalToPortPoint(botRight(charRect));
		InvalPortRect(&charRect);
	}
}


// ---------------------------------------------------------------------------
//	е InvertChar									[protected, virtual]
// ---------------------------------------------------------------------------
//	Invert the rectangle for a single character (usually the cursor position).

void
CTerminalPane::InvertChar(
	const TermCharT&	inChar)
{
	Rect charFrame;
	if (FetchLocalCharFrame(inChar, charFrame)) {
		::InvertRect(&charFrame);
		if (!IsActive()) {
			::InsetRect(&charFrame, 1, 1);
			::InvertRect(&charFrame);
		}
	}
}


// ===========================================================================

#pragma mark -
#pragma mark ее character-to-pixel conversions

// ---------------------------------------------------------------------------
//	е FetchCharHitBy								[protected, virtual]
// ---------------------------------------------------------------------------

void
CTerminalPane::FetchCharHitBy(
	const SPoint32&		inImagePt,
	TermCharT&			outChar)
{
	outChar.row = (inImagePt.v) / mRowHeight;
	outChar.col = (inImagePt.h) / mColWidth;
}


// ---------------------------------------------------------------------------
//	е FetchLocalCharFrame							[protected, virtual]
// ---------------------------------------------------------------------------

bool
CTerminalPane::FetchLocalCharFrame(
	const TermCharT&	inChar,			// character coordinates (top-left if fetching multiple chars)
	Rect&				outCharFrame,	// returns local frame of character
	CharIndexT			inNumCols,		// (optional, default=1) number of columns to fetch
	CharIndexT			inNumRows)		// (optional, defualt=1) number of rows to fetch
{
	
		// Get top-left in image coordinates.
	SPoint32 charImage;
	charImage.h = (inChar.col) * mColWidth;
	charImage.v = (inChar.row) * mRowHeight;
	
		// If character intersects the frame, convert to local coordinates.
	bool intersectsFrame = ImageRectIntersectsFrame(charImage.h, charImage.v,
							charImage.h + (mColWidth * inNumCols),
							charImage.v + (mRowHeight * inNumRows));
	if (intersectsFrame) {
		ImageToLocalPoint(charImage, topLeft(outCharFrame));
		outCharFrame.right = outCharFrame.left + (mColWidth * inNumCols);
		outCharFrame.bottom = outCharFrame.top + (mRowHeight * inNumRows);
	}
	
	return intersectsFrame;
}


// ===========================================================================

#pragma mark -
#pragma mark ее direct access to screen buffer

// ---------------------------------------------------------------------------
//	е PutCharAt										[protected, virtual]
// ---------------------------------------------------------------------------
//	Place the character in the screen buffer.

void
CTerminalPane::PutCharAt(
	const TermCharT&	inCharPosition,
	char				inChar)
{
	mScreenBfr[inCharPosition.row][inCharPosition.col] = inChar;
}


// ---------------------------------------------------------------------------
//	е GetCharFrom									[protected, virtual]
// ---------------------------------------------------------------------------
//	Fetch the character at a particular position.

char
CTerminalPane::GetCharFrom(
	const TermCharT&	inCharPosition) const
{
	return mScreenBfr[inCharPosition.row][inCharPosition.col];
}


// ===========================================================================

#pragma mark -
#pragma mark ее initialization

// ---------------------------------------------------------------------------
//	е InitTerm										[private]
// ---------------------------------------------------------------------------
//	Set parameters for terminal display.
//	Should only be called by constructors for CTerminalPane.

void
CTerminalPane::InitTerm()
{
		//! TEMPORARY: feed values from hard-wired parameters
	
	mRows				= maxY;
	mCols				= maxX;

	mRowHeight			= pixelsY;
	mColWidth			= pixelsX;

	mPreviousCursor.row = 0;
	mPreviousCursor.col = 0;
	mCharsToInvalLine	= resetInvalsChar;

	mBlinkCursor		= false;
	mCursorBlinkVisible	= true;
	mCursorBlinkTick	= 0L;

	DoClearScreen();
}
