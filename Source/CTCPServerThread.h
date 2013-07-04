// ===========================================================================
//	CTCPServerThread.h			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CTCPServerThread
#define _H_CTCPServerThread
#pragma once

#include <LTCPEndpoint.h>
#include <LThread.h>
#include <LSemaphore.h>

#include "CSimpleTCPServer.h"
#include "CTerminalPane.h"

class CSimpleTCPServer;

// ===========================================================================
//		¥ CTCPServerThread
// ===========================================================================
//	The thread that actually does the work of the server responder.
//	Note that typed characters (i.e. data sent to the remote host
//	are NOT handled by this object.) Only incoming data are processed
//	here.

class CTCPServerThread :	public LThread {

public:
								CTCPServerThread(
									UInt16				inMaxConnections,
									UInt16				inLocalPort,
									LTCPEndpoint*		inNetworkEndpoint,
									CSimpleTCPServer*	inServerMaster);
	virtual						~CTCPServerThread();

			void				StartDisconnect();
	
protected:
	virtual void*				Run();
	virtual void				SuspendServerThread();
	virtual void				ResumeServerThread();

			LTCPEndpoint*		mEndpoint;
			CSimpleTCPServer*	mServerMaster;
			UInt16				mMaxConnections;
			UInt16				mPort;
	
			bool				mDisconnectReceived;	// true if a T_DISCONNECT or T_ORDREL
														//   event has occurred
			bool				mStartDisconnect;		// true if StartDisconnect() was called
	
			LSemaphore 			mConnectionSignal;
	
	friend class CSimpleTCPServer;

private:
								CTCPServerThread();
								CTCPServerThread(const CTCPServerThread& inOriginal);
			CTCPServerThread&	operator=(const CTCPServerThread& inRhs);
};

#endif // _H_CTCPServerThread
