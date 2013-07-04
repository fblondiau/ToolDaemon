// ===========================================================================
//	CTCPResponder.cp			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================
//	Responders represent the endpoints that handle the actual connection to and
//	interaction with remote clients.

#include "CTCPResponder.h"
#include "CTCPResponderThread.h"

#include <UNetworkFactory.h>

#include <LWindow.h>
#include <LApplication.h>

#include <Sound.h>

/* FB ( */
#include "CSetWithID.h"
/* ) FB */

#include "AppConstants.h"

// ===========================================================================

#pragma mark ¥¥¥ CTCPResponder ¥¥¥


// ---------------------------------------------------------------------------
//	¥ CTCPResponder										[public]
// ---------------------------------------------------------------------------

CTCPResponder::CTCPResponder(
	LCommander*		inSuper)
	
	: LSingleDoc(inSuper)
{
	mTerminalPane		= nil;
	mEndpoint			= nil;
	mLineBufferIndex	= 0;
	mLineMode			= true;
	mEchoCharacters		= true;
	mResponderThread	= nil;
	mQuitWhenDone		= false;
	mSaveOption			= 0;
}


// ---------------------------------------------------------------------------
//	¥ ~CTCPResponder									[protected, virtual]
// ---------------------------------------------------------------------------

CTCPResponder::~CTCPResponder()
{
	delete mEndpoint;
	mEndpoint = nil;		// typically not necessary, but needed here.
}


// ---------------------------------------------------------------------------
//	¥ Accept											[public, virtual]
// ---------------------------------------------------------------------------
//	Accept a connection from a remote machine on this endpoint. This endpoint
//	will do all the dirty work. 
//	The display window is also opened at this time.
//
//	This routine creates an endpoint, but does NOT bind it!
//	The endpoint is automagically bound to the port on which incomming connection
//	was received via the 'AcceptIncoming' routine inside the thread.

void
CTCPResponder::Accept(
	CSimpleTCPServer*	inServer)
{
	BuildServerWindow();

	mEndpoint = UNetworkFactory::CreateTCPEndpoint();

/* FB ( */
	CSetWithID * theSet = new CSetWithID;
	mAEQueue = new LSharedQueue;
	mQueueID = theSet -> AddItem (mAEQueue);

	mAEThread = new CAEThread(mEndpoint, mAEQueue);
	mAEThread->Resume();

	mResponderThread = new CTCPResponderThread(mEndpoint, mQueueID,
		mTerminalPane, this, inServer);
// FB	mResponderThread = new CTCPResponderThread(mEndpoint, mTerminalPane, this, inServer);
/* ) FB */

	mResponderThread->Resume();
}


// ---------------------------------------------------------------------------
//	¥ IsIdle											[public, virtual]
// ---------------------------------------------------------------------------

Boolean
CTCPResponder::IsIdle()
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
CTCPResponder::ServerThreadDied()
{
	mResponderThread = nil;

/* FB ( */
	mAEThread -> DeleteThread ();

	CSetWithID::GetAndRemoveItem (mQueueID);
	delete mAEQueue;
/* ) FB */

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
CTCPResponder::Disconnect()
{
	if (mResponderThread) {
		mResponderThread->StartDisconnect();
	}
}


// ---------------------------------------------------------------------------
//	¥ BindCompleted										[public, virtual]
// ---------------------------------------------------------------------------

void
CTCPResponder::BindCompleted()
{
	LInternetAddress* theAddress = mEndpoint->GetRemoteHostAddress();
	SetWindowTitle(*theAddress);
	delete theAddress;
}


// ---------------------------------------------------------------------------
//	¥ BindFailed										[public, virtual]
// ---------------------------------------------------------------------------

void
CTCPResponder::BindFailed()
{
	// nothing
}


// ---------------------------------------------------------------------------
//	¥ HandleKeyPress									[public, virtual]
// ---------------------------------------------------------------------------
//	Server side does not allow user interaction with the server window...
//	We simply beep and swallow the event.

Boolean
CTCPResponder::HandleKeyPress(
	const EventRecord& /* inKeyEvent */)
{
	::SysBeep(30);
	return true;
}


// ---------------------------------------------------------------------------
//	¥ BuildServerWindow									[protected]
// ---------------------------------------------------------------------------
//	Build the window for this connection session.

void
CTCPResponder::BuildServerWindow()
{
	mWindow = LWindow::CreateWindow(PPob_TCPTerminalWindow, this);
	mTerminalPane = dynamic_cast <CTerminalPane*>(mWindow->FindPaneByID(pTerminal));
	ThrowIfNil_(mTerminalPane);
}


// ---------------------------------------------------------------------------
//	¥ SetWindowTitle									[protected]
// ---------------------------------------------------------------------------

void
CTCPResponder::SetWindowTitle(
	LInternetAddress&	inAddress)
{
	if (mWindow != nil) {
		Str255 titleStr;
		inAddress.GetIPDescriptor(titleStr);
		mWindow->SetDescriptor(titleStr);
	}
}


// ---------------------------------------------------------------------------
//	¥ AllowSubRemoval									[public, virtual]
// ---------------------------------------------------------------------------
//	If user tries to close the connection window, interpret this as a request
//	to close the connection. The window is not immediately closed; rather
//	it is closed when the connection actually goes away.

Boolean
CTCPResponder::AllowSubRemoval(
	LCommander*		inSub)
{
	if ((inSub == mWindow) && (IsIdle() == false)) {
		Disconnect();
		return false;
	}
	else {
		return LSingleDoc::AllowSubRemoval(inSub);
	}
}


// ---------------------------------------------------------------------------
//	¥ AttemptQuitSelf									[public, virtual]
// ---------------------------------------------------------------------------
//	Close our connection before quitting. We re-issue the quit command via
//	the ServerThreadDied method once we are done.

Boolean
CTCPResponder::AttemptQuitSelf(
	SInt32	inSaveOption)
{
	if (mQuitWhenDone) {
		return LCommander::AttemptQuitSelf(inSaveOption);
	} else {
		mQuitWhenDone = true;
		mSaveOption = inSaveOption;

		Disconnect();
		return false;
	}
}


// ---------------------------------------------------------------------------
//	¥ÊObeyCommand										[public, virtual]
// ---------------------------------------------------------------------------
//	Handle Commands for this object

Boolean
CTCPResponder::ObeyCommand(
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
CTCPResponder::FindCommandStatus(
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
