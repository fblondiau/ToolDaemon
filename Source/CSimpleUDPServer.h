// ===========================================================================
//	CSimpleUDPServer.h			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CSimpleUDPServer
#define _H_CSimpleUDPServer
#pragma once

#include <LUDPEndpoint.h>

#include <LSingleDoc.h>
#include <LCaption.h>

#include "CUDPServerThread.h"
#include "CTerminalPane.h"

// ===========================================================================
//		¥ CSimpleUDPServer
// ===========================================================================

class CUDPServerThread;

class CSimpleUDPServer :	public LSingleDoc{

public:
								CSimpleUDPServer(
										LCommander*		inSuper);
			
	virtual	void				ListenToMessage(
										MessageT		inMessage,
										void*			ioParam);

	virtual LUDPEndpoint*		GetEndpoint() const;

	virtual void				WaitForUDPData(
										UInt32			inPort);
	
	virtual void				CreateServerWindow(
										UInt32			inPort);

	virtual void				BindCompleted();
	virtual void				BindFailed();
	
	virtual void				ServerThreadDied();
	
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
	virtual						~CSimpleUDPServer();

			LUDPEndpoint*		mEndpoint;		// our network endpoint object

			CTerminalPane*		mTerminalPane;
			CUDPServerThread*	mServerThread;	// the thread that actually handles the connection
			
			bool				mQuitWhenDone;	// re-issue quit request when disconnect complete
			SInt32				mSaveOption;

	friend class CUDPServerThread;
	
private:
								CSimpleUDPServer();
								CSimpleUDPServer(const CSimpleUDPServer& inOriginal);
			CSimpleUDPServer&	operator=(const CSimpleUDPServer& inRhs);
};

#endif // _H_CSimpleUDPServer
