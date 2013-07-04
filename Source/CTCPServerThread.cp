// ===========================================================================
//	CTCPServerThread.cp			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#include "CTCPServerThread.h"
#include "CTCPResponder.h"


// ---------------------------------------------------------------------------
//	¥ ~CTCPServerThread									[public]
// ---------------------------------------------------------------------------

CTCPServerThread::CTCPServerThread(
	UInt16				inMaxConnections,
	UInt16				inLocalPort,
	LTCPEndpoint*		inNetworkEndpoint,
	CSimpleTCPServer*	inServerMaster)

	: LThread(	false,
				thread_DefaultStack, 
				LThread::threadOption_Default,
				nil),
	  mEndpoint(inNetworkEndpoint),
	  mMaxConnections(inMaxConnections),
	  mPort(inLocalPort),
	  mServerMaster(inServerMaster)
{
	mDisconnectReceived = mStartDisconnect = false;
}


// ---------------------------------------------------------------------------
//	¥ ~CTCPServerThread									[public, virtual]
// ---------------------------------------------------------------------------

CTCPServerThread::~CTCPServerThread()
{
	// Nothing
}

// ---------------------------------------------------------------------------
//	¥ StartDisconnect									[public]
// ---------------------------------------------------------------------------

void
CTCPServerThread::StartDisconnect()
{
	if (!mStartDisconnect) {
		mStartDisconnect = true;
		
		if (mState == LThread::threadState_Waiting) {
			ResumeServerThread();
		} else {
			mEndpoint->AbortThreadOperation(this);
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ SuspendThread										[protected, virtual]
// ---------------------------------------------------------------------------

void
CTCPServerThread::SuspendServerThread()
{
	mConnectionSignal.Wait();
}


// ---------------------------------------------------------------------------
//	¥ ResumeThread										[protected, virtual]
// ---------------------------------------------------------------------------

void
CTCPServerThread::ResumeServerThread()
{
	mConnectionSignal.Signal();
}


// ---------------------------------------------------------------------------
//	¥ Run												[protected, virtual]
// ---------------------------------------------------------------------------

void*
CTCPServerThread::Run()
{
	try {
		LInternetAddress address(0, mPort);
		mEndpoint->Bind(address, mMaxConnections);
		mServerMaster->BindCompleted();

			// Endless loop: watch and wait for incomming connections.
		while (true) {
			try {
				SuspendServerThread();	// Nothing to do but wait for T_LISTEN

				if (mStartDisconnect) {
					break;
				}
				
				mEndpoint->Listen();
				
				if (mServerMaster->mConnectionCount < mMaxConnections) {
					CTCPResponder* theConnection = new CTCPResponder(mServerMaster->GetSuperCommander());
					theConnection->Accept(mServerMaster);
				} else {
					mEndpoint->RejectIncoming();
					mServerMaster->IncRejectionCount();
				}
			}
			catch(...) { }
		}
	}
	catch(...) { }

		// We're done with the connectionÉ bail out.
	try {
		mEndpoint->Unbind();
	}
	catch(...) { }

	mServerMaster->ServerThreadDied();
	return nil;
}
