// ===========================================================================
//	CSimpleTCPServer.cp			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================
//	Servers are endpoints that watch and wait for connection requests from
//	remote clients. The actual connection to and interaction with remote clients
//	is handled by the "responders."

#include "CSimpleTCPServer.h"

#include <UNetworkFactory.h>
#include <LWindow.h>
#include <LApplication.h>
#include <UMemoryMgr.h>
#include <LStaticText.h>

#include <Sound.h>

#include "AppConstants.h"
#include "CPrefs.h"


// ---------------------------------------------------------------------------
//	¥ CSimpleTCPServer									[public]
// ---------------------------------------------------------------------------

CSimpleTCPServer::CSimpleTCPServer(
	LCommander*		inSuper)

	: LSingleDoc(inSuper)
{
	mWindow			= nil;
	mEndpoint		= nil;
	mServerThread	= nil;
	mQuitWhenDone	= false;
	mSaveOption		= 0;
}


// ---------------------------------------------------------------------------
//	¥ ~CSimpleTCPServer									[protected, virtual]
// ---------------------------------------------------------------------------

CSimpleTCPServer::~CSimpleTCPServer()
{
	if (mEndpoint != nil) {
		mEndpoint->RemoveListener(this);
		delete mEndpoint;
		mEndpoint = nil;
	}

	if (mServerThread != nil) {
		mServerThread->DeleteThread();
	}
}

// ---------------------------------------------------------------------------
//	¥ ListenToMessage									[public, virtual]
// ---------------------------------------------------------------------------

void
CSimpleTCPServer::ListenToMessage(
	MessageT		inMessage,
	void*			/* ioParam */)
{
	switch (inMessage) {
		case T_LISTEN:
			if (mServerThread) {
				mServerThread->ResumeServerThread();
			}
			break;
	}
}


// ---------------------------------------------------------------------------
//	¥ WaitForConnections								[public, virtual]
// ---------------------------------------------------------------------------
//	Wait for connection(s) from remote machine(s) using the parameters that
//	have been passed to us. The connection information window is also opened
//	at this time.
//
//	This routine creates an endpoint and a thread for the server process.
//	When the thread runs, it will bind the endpoint and wait for T_Listen
//	events. Responses to T_Listens will be handled on NEW endpoints. This
//	endpoint ONLY listens.

void
CSimpleTCPServer::WaitForConnections(
	UInt32		inListenQueueSize,
	UInt32		inPort)
{
	mConnectionCount = mRejectionCount = 0;

	CreateServerWindow(inListenQueueSize, inPort);
	
/* FB ( */
	try
	{
/* FB ) */
		mEndpoint = UNetworkFactory::CreateTCPEndpoint();
/* FB ( */
	}
	catch (...)
	{
		::StopAlert(ALRT_NoTCPIP, nil);
		ExitToShell ();
	}
/* FB ) */

	mEndpoint->AddListener(this);

	mServerThread = new CTCPServerThread(
							inListenQueueSize,
							inPort,
							mEndpoint,
							this);

	mServerThread->Resume();
}


// ---------------------------------------------------------------------------
//	¥ IsIdle											[public, virtual]
// ---------------------------------------------------------------------------

Boolean
CSimpleTCPServer::IsIdle()
{
	if (mEndpoint == nil) {
		return true;
	}
	
	EEndpointState netState = mEndpoint->GetState();
	return ((netState == endpoint_Idle) || (netState == endpoint_Unbound));
}


// ---------------------------------------------------------------------------
//	¥ ServerThreadDied									[public, virtual]
// ---------------------------------------------------------------------------

void
CSimpleTCPServer::ServerThreadDied()
{
	mServerThread = nil;

		//Re-issue the quit command when we are ready.
	if (mQuitWhenDone) {
		LApplication* theApp = dynamic_cast<LApplication*>(GetSuperCommander());
		if (theApp != nil) {
			theApp->DoQuit(mSaveOption);
		}
	}

	delete this;
}


// ---------------------------------------------------------------------------
//	¥ Disconnect										[public, virtual]
// ---------------------------------------------------------------------------

void
CSimpleTCPServer::Disconnect()
{
	if (mServerThread) {
		mServerThread->StartDisconnect();
	}
}


// ---------------------------------------------------------------------------
//	¥ GetEndpoint										[public, virtual]
// ---------------------------------------------------------------------------

LTCPEndpoint*
CSimpleTCPServer::GetEndpoint() const
{
	return mEndpoint;
}


// ---------------------------------------------------------------------------
//	¥ CreateServerWindow								[public, virtual]
// ---------------------------------------------------------------------------

void
CSimpleTCPServer::CreateServerWindow(
	UInt32		inListenQueueSize,
	UInt32		inPort)
{
	mWindow = LWindow::CreateWindow(PPob_WaitIngWindow, this);
	ThrowIfNil_(mWindow);
	
	LStaticText* theField = dynamic_cast<LStaticText*>(mWindow->FindPaneByID(pPortItemID));
	ThrowIfNil_ (theField);
	theField->SetValue(inPort);
	
	theField = dynamic_cast<LStaticText*>(mWindow->FindPaneByID(pAddress));
	ThrowIfNil_ (theField);
	
	{
		StDeleter<LInternetMapper>	theMapper(UNetworkFactory::CreateInternetMapper());

		StDeleter<LInternetAddress>	tempAddress(theMapper->GetLocalAddress());

		Str255	tempDescriptor;
		theField->SetDescriptor(tempAddress->GetIPDescriptor(tempDescriptor));
	}
	
	theField = dynamic_cast<LStaticText*>(mWindow->FindPaneByID(pMaxConnection));
	ThrowIfNil_(theField);
	theField->SetValue(inListenQueueSize);

	mConnectionCountField = dynamic_cast<LStaticText*>(mWindow->FindPaneByID(pCurrConnection));
	ThrowIfNil_(mConnectionCountField);
	mConnectionCountField->SetValue(mConnectionCount);

/* FB ( */
	mConnectionCountBar = dynamic_cast<LProgressBar*>(mWindow->FindPaneByID(pCurrConnectionBar));
	ThrowIfNil_ (mConnectionCountBar);
	mConnectionCountBar->SetValue(mConnectionCount);
	mConnectionCountBar->SetMaxValue(inListenQueueSize);
/* ) FB */

	mRejectionCountField = dynamic_cast<LStaticText*>(mWindow->FindPaneByID(pRejectedConnection));
	ThrowIfNil_(mRejectionCountField);
	mRejectionCountField->SetValue(mRejectionCount);
}


// ---------------------------------------------------------------------------
//	¥ AddToConnectionCount								[public, virtual]
// ---------------------------------------------------------------------------

void
CSimpleTCPServer::AddToConnectionCount(
	UInt16		theCount)
{
	mConnectionCount += theCount;
	mConnectionCountField->SetValue(mConnectionCount);
	
/* FB ( */
	mConnectionCountBar->SetValue(mConnectionCount);
/* ) FB */

		//Start disconnect if we are quitting and the connection count is 0
	if ((mConnectionCount == 0) && (mQuitWhenDone)) {
		Disconnect();
	}
}


// ---------------------------------------------------------------------------
//	¥ IncRejectionCount									[public, virtual]
// ---------------------------------------------------------------------------
void
CSimpleTCPServer::IncRejectionCount()
{
	mRejectionCount++;
	mRejectionCountField->SetValue(mRejectionCount);
}


// ---------------------------------------------------------------------------
//	¥ BindCompleted										[public, virtual[
// ---------------------------------------------------------------------------
//	We are now waiting for T_Listens... show the connection window

void
CSimpleTCPServer::BindCompleted()
{
	mWindow->Show();
}


// ---------------------------------------------------------------------------
//	¥ BindFailed										[public, virtual]
// ---------------------------------------------------------------------------
//	Could not bind to the requested port.

void
CSimpleTCPServer::BindFailed()
{
	SignalStringLiteral_("Bind Failed");
	delete this;
}


// ---------------------------------------------------------------------------
//	¥ AllowSubRemoval									[public, virtual]
// ---------------------------------------------------------------------------
//	If user tries to close the main window, and we have ongoing connections,
//	then block the action. If we don't have any connections then
//	interpret this as a request to close the server.

Boolean
CSimpleTCPServer::AllowSubRemoval(
	LCommander*		inSub)
{
	if ((inSub == mWindow) && (mConnectionCount > 0)) {
		::SysBeep(30);
		return false;
	} else {
		return LSingleDoc::AllowSubRemoval(inSub);
	}
}


// ---------------------------------------------------------------------------
//	¥ AttemptQuitSelf									[public, virtual]
// ---------------------------------------------------------------------------
//	Close our connection before quitting. We re-issue the quit command via
//	the ServerThreadDied method once we are done.

Boolean
CSimpleTCPServer::AttemptQuitSelf(
	SInt32	inSaveOption)
{
		//This may seem odd, but...
		//	subcommanders are called in order of creation so
		//	this AttemptQuitSelf is called before any of the
		//	responers it may have spawned.
		//	Thus we return true here if mConnectionCount > 0
		//	to give those a chance to get called too.
	if (mConnectionCount > 0) {
		return true;
	}
	
	if (mQuitWhenDone) {
		return LCommander::AttemptQuitSelf(inSaveOption);
	} else {
		mQuitWhenDone = true;
		mSaveOption = inSaveOption;
		if (mConnectionCount == 0) {
			Disconnect();
		}
		return false;
	}
}


// ---------------------------------------------------------------------------
//	¥ÊObeyCommand										[public, virtual]
// ---------------------------------------------------------------------------
//	Handle Commands for this object

Boolean
CSimpleTCPServer::ObeyCommand(
	CommandT		inCommand,
	void*			ioParam)
{
		// This override is provided for convenience as you implement
		// your project.
	
	Boolean	cmdHandled = true;
	
	cmdHandled = LSingleDoc::ObeyCommand(inCommand, ioParam);

	return cmdHandled;
}


// ---------------------------------------------------------------------------
//	¥ FindCommandStatus								[public, virtual]
// ---------------------------------------------------------------------------
//	Determine the status of a command for the purpose of menu updating.

void
CSimpleTCPServer::FindCommandStatus(
	CommandT		inCommand,
	Boolean&		outEnabled,
	Boolean&		outUsesMark,
	UInt16&			outMark,
	Str255			outName)
{
		// This override is provided for the stationery's default to
		// disable the Save and Save As commands. You may wish to
		// modify this default behavior, especially if you implement
		// saving.
	
	switch (inCommand) {
	
		case cmd_Save:
		case cmd_SaveAs: {
			outEnabled = false;
			break;
		}
		
		default: {
			LSingleDoc::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
		}
	}
}
