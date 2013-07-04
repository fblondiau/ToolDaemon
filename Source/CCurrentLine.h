// ===========================================================================
//	CCurrentLine.h			   ©1996-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CCurrentLine
#define _H_CCurrentLine
#pragma once

#include <LView.h>
#include <LPeriodical.h>

typedef UInt32 CharIndexT;


// ===========================================================================
//		¥ CCurrentLine
// ===========================================================================
//	This class provides a rudimentary line buffer.

class CCurrentLine {

public:
							~CCurrentLine();

						// terminal primitives

	virtual char			DoWriteChar(
									char				inChar);
	virtual void			DoWriteStr(
									const char*			inString);
	virtual void			DoWriteBfr(
									const char*			inBuffer,
									SInt32				inByteCount);
	virtual UInt8 *			GetLine();
		
protected:
						// direct access to the screen buffer
	
	virtual void			PutCharAt(
									const CharIndexT&	inCharPosition,
									char				inChar);
	
	enum {
		maxLen = 512										// size of line buffer
	};

			UInt8			mLineBfr[maxLen];

			CharIndexT		mCurrentCursor;				// position of character cursor
			CharIndexT		mCurrentSize;				// position of last character

			UInt8			mNextToSkip;				// number of next chars to ignore 

private:
			void			InitLine();
	
							CCurrentLine();
							CCurrentLine(const CCurrentLine& inOriginal);
			CCurrentLine&	operator=(const CCurrentLine& inRhs);

};


#endif // _H_CCurrentLine
