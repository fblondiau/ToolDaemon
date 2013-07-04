// ===========================================================================
//	CCurrentLine.cp		   й1996-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================

#include "CCurrentLine.h"

#include <Sound.h>
#include <LowMem.h>
#include <Fonts.h>

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

#pragma mark еее CCurrentLine еее


// ---------------------------------------------------------------------------
//	е CCurrentLine								[public, virtual]
// ---------------------------------------------------------------------------
//	Constructor

CCurrentLine::CCurrentLine()
{
	InitLine ();
}


// ---------------------------------------------------------------------------
//	е ~CCurrentLine								[public, virtual]
// ---------------------------------------------------------------------------
//	Destructor

CCurrentLine::~CCurrentLine()
{
	// nothing
}


// ===========================================================================

#pragma mark -
#pragma mark ее line primitives

// ---------------------------------------------------------------------------
//	е DoWriteChar									[public, virtual]
// ---------------------------------------------------------------------------
//	Write a single character to the line.

char
CCurrentLine::DoWriteChar(
	char 	inChar)
{
	char outChar;
	// Parse a few control characters.

	outChar = inChar;

	switch (inChar)
	{

		case charNUL:
		case charBEL:
			break;

		case charBS:
			if (mCurrentCursor > 0)
				mCurrentCursor--;
			else
				outChar = charBEL;
			break;

		case charLF:
			mLineBfr[mCurrentSize] = '\0';
			mCurrentSize = 0;
			break;
			
		case charCR:
			mCurrentCursor = 0;
			break;

		case charDEL:
			if (mCurrentSize > 0)
			{
				mCurrentSize--;
				if (mCurrentCursor > 0)
					mCurrentCursor--;
				else
					outChar = charBEL;
			}
			else
				outChar = charBEL;
			break;

		default:
			PutCharAt(mCurrentCursor, inChar);
			if (mCurrentCursor < maxLen -1)
				mCurrentCursor++;
			if (mCurrentSize < mCurrentCursor)
				mCurrentSize = mCurrentCursor;
	}
	
	return outChar;
}


// ---------------------------------------------------------------------------
//	е DoWriteStr									[public, virtual]
// ---------------------------------------------------------------------------
//	Print a C string to the line.

void
CCurrentLine::DoWriteStr(
	const char* 	inString)		// the string to be printed
{
	while (*inString) {
		DoWriteChar(*(inString++));
	}
}


// ---------------------------------------------------------------------------
//	е DoWriteBfr									[public, virtual]
// ---------------------------------------------------------------------------
//	Write a string of arbitrary length to the buffer.

void
CCurrentLine::DoWriteBfr(
	const char*	inBuffer,
	SInt32		inByteCount)
{
	while (inByteCount--) {
		DoWriteChar(*(inBuffer++));
	}
}


// ---------------------------------------------------------------------------
//	е GetLine										[public, virtual]
// ---------------------------------------------------------------------------
//	Fetch the line.

UInt8 *
CCurrentLine::GetLine(void)
{
	return mLineBfr;
}


// ===========================================================================

#pragma mark -
#pragma mark ее direct access to line buffer

// ---------------------------------------------------------------------------
//	е PutCharAt										[protected, virtual]
// ---------------------------------------------------------------------------
//	Place the character in the screen buffer.

void
CCurrentLine::PutCharAt(
	const CharIndexT&	inCharPosition,
	char				inChar)
{
	mLineBfr[inCharPosition] = inChar;
}


// ===========================================================================

#pragma mark -
#pragma mark ее initialization

// ---------------------------------------------------------------------------
//	е InitTerm										[private]
// ---------------------------------------------------------------------------
//	Set parameters for line display.
//	Should only be called by constructors for CCurrentLine.

void
CCurrentLine::InitLine()
{
	mCurrentCursor = 0;
	mCurrentSize = 0;
}
