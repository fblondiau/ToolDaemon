// ===========================================================================
//	CUDPServerThread.h			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CUDPServerThread
#define _H_CUDPServerThread
#pragma once

#include <LThread.h>
#include <LUDPEndpoint.h>

#include "CSimpleUDPServer.h"
#include "CTerminalPane.h"

class CSimpleUDPServer;

// ===========================================================================
//		¥ CUDPServerThread
// ===========================================================================
//	The thread that actually does the work of the server responder.
//	Note that typed characters (i.e. data sent to the remote host
//	are NOT handled by this object.) Only incoming data are processed
//	here.

class CUDPServerThread :	public LThread {

public:
								CUDPServerThread(
									UInt16				inLocalPort,
									LUDPEndpoint*		inNetworkEndpoint,
									CSimpleUDPServer*	inServerMaster,
									CTerminalPane*		inTerminalPane);
	virtual						~CUDPServerThread();
	
			void				Complete();
	
protected:
			bool				mContinue;
	
	virtual void*				Run();

			LUDPEndpoint*		mEndpoint;
			CSimpleUDPServer*	mServerMaster;
			UInt16				mMaxConnections;
			UInt16				mPort;
			CTerminalPane*		mTerminalPane;
	
	friend class CSimpleUDPServer;

private:
								CUDPServerThread();
								CUDPServerThread(const CUDPServerThread& inOriginal);
			CUDPServerThread&	operator=(const CUDPServerThread& inRhs);
};

#endif // _H_CUDPServerThread
