// ===========================================================================
//	CTCPResponder.h			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CTCPResponder
#define _H_CTCPResponder
#pragma once

#include <LSingleDoc.h>
#include <LInternetAddress.h>

#include <LTCPEndpoint.h>

/* FB ( */
#include <LSharedQueue.h>

#include "CAEThread.h"
/* ) FB */

#include "CSimpleTCPServer.h"
#include "CTerminalPane.h"

class CTCPResponderThread;

// ===========================================================================
//		¥ CTCPResponder
// ===========================================================================

class CTCPResponder :	public LSingleDoc {

public:
									CTCPResponder(
										LCommander*				inSuper);
								
	virtual void					Accept(
										CSimpleTCPServer*		inServer);
	virtual void					Disconnect();
	virtual Boolean					IsIdle();

	virtual Boolean					AllowSubRemoval(
											LCommander*				inSub);
	virtual Boolean					HandleKeyPress(
										const EventRecord&		inKeyEvent);

	virtual void					BindCompleted();
	virtual void					BindFailed();
	
	virtual void					ServerThreadDied();

	virtual Boolean					AttemptQuitSelf(
										SInt32	inSaveOption);

	virtual	Boolean					ObeyCommand(
										CommandT			inCommand,
										void*				ioParam = nil);
	virtual	void					FindCommandStatus(
										CommandT			inCommand,
										Boolean&			outEnabled,
										Boolean&			outUsesMark,
										UInt16&				outMark,
										Str255				outName);
								
protected:

	virtual							~CTCPResponder();

			void					BuildServerWindow();
			void					SetWindowTitle(
										LInternetAddress&		inAddress);
								
			LTCPEndpoint* 			mEndpoint;

			char					mLineBuffer[256];	// linemode send buffer
			UInt16					mLineBufferIndex;	// size of mLineBuffer
			bool					mLineMode;			// true if buffering for entire line
			bool					mEchoCharacters;	// true to echo keyboard input to screen

			CTerminalPane*			mTerminalPane;		// terminal display for this connection

/* FB ( */
			CAEThread *				mAEThread;
			LSharedQueue *			mAEQueue;
			long					mQueueID;
/* ) FB */

			CTCPResponderThread*	mResponderThread;	// the thread that actually handles the connection

			bool					mQuitWhenDone;		// re-issue quit request when disconnect complete
			SInt32					mSaveOption;

private:
									CTCPResponder();
									CTCPResponder(const CTCPResponder& inOriginal);
			CTCPResponder&			operator=(const CTCPResponder& inRhs);
};

#endif // _H_CTCPResponder
