// ===========================================================================
//	CSimpleTCPServer.h			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CSimpleTCPServer
#define _H_CSimpleTCPServer
#pragma once

#include <LSingleDoc.h>
#include <LListener.h>
#include <LTCPEndpoint.h>

/* FB ( */
#include <LCaption.h>
#include <LStaticText.h>
#include <LProgressBar.h>
/* ) FB */

#include "CTCPServerThread.h"

class CTCPServerThread;
class LStaticText;


// ===========================================================================
//	¥ CSimpleTCPServer
// ===========================================================================

class CSimpleTCPServer :	public LSingleDoc,
							public LListener  {
public:

								CSimpleTCPServer(
									LCommander*		inSuper);
			
	void						ListenToMessage(
									MessageT		inMessage,
									void*			ioParam);

	virtual LTCPEndpoint*		GetEndpoint() const;

	virtual void				WaitForConnections(
									UInt32			inListenQueueSize,
									UInt32			inPort);
	
	virtual void				CreateServerWindow(
									UInt32			inListenQueueSize,
									UInt32			inPort);
	
	virtual void				AddToConnectionCount(
									UInt16			theCount);
	
	virtual void				IncRejectionCount();

	virtual void				BindCompleted();
	virtual void				BindFailed();
	
	virtual Boolean				IsIdle();
	
	virtual void				ServerThreadDied();
	
	virtual void				Disconnect();

	virtual Boolean				AllowSubRemoval(
									LCommander*		inSub);
								
	virtual Boolean				AttemptQuitSelf(
									SInt32			inSaveOption);

	virtual	Boolean				ObeyCommand(
									CommandT			inCommand,
									void*				ioParam = nil);
	virtual	void				FindCommandStatus(
									CommandT			inCommand,
									Boolean&			outEnabled,
									Boolean&			outUsesMark,
									UInt16&				outMark,
									Str255				outName);
								
protected:
	virtual						~CSimpleTCPServer();

			LTCPEndpoint*		mEndpoint;				// our network endpoint object
			LStaticText*		mConnectionCountField;	// field used to display current # of connections
			LStaticText*		mRejectionCountField;	// field used to display current # of rejected connections
			UInt16				mConnectionCount;
			SInt32				mRejectionCount;

/* FB ( */
			LProgressBar*		mConnectionCountBar;
/* ) FB */

			CTCPServerThread*	mServerThread;			// the thread that actually handles the connection
			
			bool				mQuitWhenDone;			// re-issue quit request when disconnect complete
			SInt32				mSaveOption;

	friend class CTCPServerThread;

private:
								CSimpleTCPServer();
								CSimpleTCPServer(const CSimpleTCPServer& inOriginal);
			CSimpleTCPServer&	operator=(const CSimpleTCPServer& inRhs);
};


#endif // _H_CSimpleTCPServer
