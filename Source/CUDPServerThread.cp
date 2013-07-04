// ===========================================================================
//	CUDPServerThread.cp			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#include "CUDPServerThread.h"

#include <PP_KeyCodes.h>
 

// ---------------------------------------------------------------------------
//	¥ CUDPServerThread									[public]
// ---------------------------------------------------------------------------

CUDPServerThread::CUDPServerThread(
	UInt16				inLocalPort,
	LUDPEndpoint*		inNetworkEndpoint,
	CSimpleUDPServer*	inServerMaster,
	CTerminalPane*		inTerminalPane)

	: LThread(	false,
				thread_DefaultStack,
				LThread::threadOption_Default,
				nil),
	  mPort(inLocalPort),
	  mEndpoint(inNetworkEndpoint),
	  mServerMaster(inServerMaster),
	  mTerminalPane(inTerminalPane)
{
	mContinue = true;
}


// ---------------------------------------------------------------------------
//	¥ ~CUDPServerThread									[public, virtual]
// ---------------------------------------------------------------------------

CUDPServerThread::~CUDPServerThread()
{
	// Nothing
}


// ---------------------------------------------------------------------------
//	¥ Complete											[public]
// ---------------------------------------------------------------------------

void
CUDPServerThread::Complete()
{
	if (mContinue) {
		mContinue = false;
		mEndpoint->AbortThreadOperation(this);
	}
}


// ---------------------------------------------------------------------------
//	¥ Run												[protected, virtual]
// ---------------------------------------------------------------------------

void*
CUDPServerThread::Run()
{
	try {

			// Initialization: Bind to the port specified
		LInternetAddress address(0, mPort);
		mEndpoint->Bind(address, 1);
		mServerMaster->BindCompleted();
		
			// Endless loop: watch and wait for incomming UDP data.
		while (mContinue) {
			try {
				LInternetAddress	remoteAddress;
				char 				dataBuffer[80];
				UInt32				ioDataSize = 80;
				
				mEndpoint->ReceiveFrom(remoteAddress, dataBuffer, ioDataSize);
				
					// Get the IP Address/Port of the sender
				Str255 remoteAddressString;
				remoteAddress.GetIPDescriptor(remoteAddressString, true);
				LStr255 remoteAddressDisplay = remoteAddressString;
					
					// Add CRLF
				remoteAddressDisplay += char_Return;
				remoteAddressDisplay += char_LineFeed;
	
					// Display the remote address info
				mTerminalPane->DoWriteBfr(remoteAddressDisplay.TextPtr(), remoteAddressDisplay.LongLength());
				
					// Display the UDP data
				mTerminalPane->DoWriteBfr(dataBuffer, ioDataSize);
				
					// Turn the data back arround to the sender
				mEndpoint->SendPacketData(remoteAddress, dataBuffer, ioDataSize);
			}
			catch (LException& inErr) {
					// Examples of things to watch for
				if (inErr.GetErrorCode() == Timeout_Error) {
					continue;					// you might want to break here instead
				}
				else if (inErr.GetErrorCode() == Abort_Error) {
					break;
				}
				else {
					break;
				}
			}
		}
	}
	catch(...) {
			// We're done with the connectionÉ bail out.
		mEndpoint->Unbind();
	}

	mServerMaster->ServerThreadDied();
	return nil;
}
