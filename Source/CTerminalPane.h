// ===========================================================================
//	CTerminalPane.h			   ©1996-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CTerminalPane
#define _H_CTerminalPane
#pragma once

#include <LView.h>
#include <LPeriodical.h>

typedef UInt32 CharIndexT;

struct TermCharT {
	CharIndexT row;
	CharIndexT col;
};


// ===========================================================================
//		• CTerminalPane
// ===========================================================================
//	This class provides a rudimentary terminal emulator. It’s nothing fancy
//	or snazzy. It doesn’t do VT100; it doesn’t do scrollback; it doesn’t
//	handle any kind of formatting.

class CTerminalPane :	public LView,
						public LPeriodical {

public:
			enum { class_ID = FOUR_CHAR_CODE('TERM') };
	
							CTerminalPane(
									LStream*			inStream);
							~CTerminalPane();

						// terminal primitives

	virtual void			DoWriteChar(
									char				inChar);
	virtual void			DoWriteStr(
									const char*			inString);
	virtual void			DoWriteBfr(
									const char*			inBuffer,
									SInt32				inByteCount);
	virtual void			DoWriteCharNum(
									char				inChar,
									char				inLeftBracket = 0,
									char				inRightBracket = 0);
	virtual void			DoClearScreen();

	virtual void			SetBlinking(
									bool				inBlinkMode);
		
protected:
	virtual void			DrawSelf();	
		
						// blinking cursor support
	
	virtual void			ActivateSelf();
	virtual void			DeactivateSelf();
	virtual void			SpendTime(
									const EventRecord&	inMacEvent);
	
						// internal screen primitives
	
	virtual void			ClearToEOL(
									const TermCharT&	inStartChar);
	virtual void			ClearToEOS(
									const TermCharT&	inStartChar);
	virtual void			ScrollTerm();
	virtual void			CursorMoved();

	virtual void			InvalChar(
									const TermCharT&	inChar,
									CharIndexT			inNumChars = 1);
	virtual void			InvertChar(
									const TermCharT&	inChar);

						// character-to-pixel conversions
	
	virtual void			FetchCharHitBy(
									const SPoint32&		inImagePt,
									TermCharT&			outChar);
	virtual bool			FetchLocalCharFrame(
									const TermCharT&	inChar,
									Rect&				outCharFrame,
									CharIndexT			inNumCols = 1,
									CharIndexT			inNumRows = 1);
	
						// direct access to the screen buffer
	
	virtual void			PutCharAt(
									const TermCharT&	inCharPosition,
									char				inChar);
	virtual char			GetCharFrom(
									const TermCharT&	inCharPosition) const;
	

	enum {
		maxX = 80,										// width of screen
		maxY = 24										// height of screen
	};

			UInt8			mScreenBfr[maxY][maxX];		//! TEMPORARY: contents of the screen

			CharIndexT		mRows;						// size of table
			CharIndexT		mCols;
			SInt32			mRowHeight;					// size of character grid
			SInt32			mColWidth;

			TermCharT		mCurrentCursor;				// position of character cursor
			TermCharT		mPreviousCursor;			// previous position of cursor
			SInt16			mCharsToInvalLine;			// used for optimization to invalidate entire line at once
			
			bool			mBlinkCursor;				// cursor blinking is enabled
			bool			mCursorBlinkVisible;		// cursor is inverted
			SInt32			mCursorBlinkTick;			// time at last cursor flash


private:
			void			InitTerm();
	
							CTerminalPane();
							CTerminalPane(const CTerminalPane& inOriginal);
			CTerminalPane&	operator=(const CTerminalPane& inRhs);

};


#endif // _H_CTerminalPane
