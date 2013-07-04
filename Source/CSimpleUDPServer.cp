// ===========================================================================
//	CSimpleUDPServer.cp			©1995-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
//	Servers are endpoints that watch and wait for connection requests from
//	remote clients. The actual connection to and interaction with remote clients
//	is handled by the "responders."

#include "CSimpleUDPServer.h"

#include <UNetworkFactory.h>
#include <LWindow.h>
#include <LApplication.h>
#include <UMemoryMgr.h>
#include <LStaticText.h>

#include "AppConstants.h"


// ---------------------------------------------------------------------------
//	¥ CSimpleUDPServer									[public]
// ---------------------------------------------------------------------------

CSimpleUDPServer::CSimpleUDPServer(
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
//	¥ ~CSimpleUDPServer									[protected, virtual]
// ---------------------------------------------------------------------------

CSimpleUDPServer::~CSimpleUDPServer()
{
	delete mEndpoint;
	mEndpoint = nil;

	if (mServerThread) {
		mServerThread->DeleteThread();
	}
}


// ---------------------------------------------------------------------------
//	¥ ListenToMessage									[public, virtual]
// ---------------------------------------------------------------------------
// Async messages such as ICMP are reported via the Listen mechanism
// implement/override this method as necessary.

void
CSimpleUDPServer::ListenToMessage(
	MessageT	inMessage,
	void*		ioParam)
{
#pragma unused(inMessage, ioParam)
	// This is just a placeholder
}


// ---------------------------------------------------------------------------
//	¥ WaitForUDPData									[public, virtual]
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
CSimpleUDPServer::WaitForUDPData(
	UInt32		inPort)
{
	CreateServerWindow(inPort);
	
	mEndpoint = UNetworkFactory::CreateUDPEndpoint();

	mServerThread = new CUDPServerThread(
							inPort,
							mEndpoint,
							this,
							mTerminalPane);

	mServerThread->Resume();
}


// ---------------------------------------------------------------------------
//	¥ ServerThreadDied									[public, virtual]
// ---------------------------------------------------------------------------

void
CSimpleUDPServer::ServerThreadDied()
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
//	¥ GetEndpoint										[public, virtual]
// ---------------------------------------------------------------------------

LUDPEndpoint* 
CSimpleUDPServer::GetEndpoint() const
{
	return mEndpoint;
}


// ---------------------------------------------------------------------------
//	¥ CreateServerWindow								[public, virtual]
// ---------------------------------------------------------------------------

void
CSimpleUDPServer::CreateServerWindow(
	UInt32		inPort)
{
	mWindow = LWindow::CreateWindow(PPob_UDPTerminalWindow, this);
	ThrowIfNil_(mWindow);

	mTerminalPane = dynamic_cast<CTerminalPane*>(mWindow->FindPaneByID(pTerminal));
	ThrowIfNil_(mTerminalPane);
	
	LStaticText* theField = dynamic_cast<LStaticText*>(mWindow->FindPaneByID(pPortItemID));
	ThrowIfNil_(theField);
	theField->SetValue(inPort);
	
	theField = dynamic_cast<LStaticText*>(mWindow->FindPaneByID(pAddress));
	ThrowIfNil_(theField);
	
	StDeleter<LInternetMapper> theMapper(UNetworkFactory::CreateInternetMapper());

	StDeleter<LInternetAddress> tempAddress(theMapper->GetLocalAddress());

	Str255	tempDescriptor;
	theField->SetDescriptor(tempAddress->GetIPDescriptor(tempDescriptor));
}


// ---------------------------------------------------------------------------
//	¥ BindCompleted										[public, virtual]
// ---------------------------------------------------------------------------
//	We are now waiting for T_Listens... show the connection window

void
CSimpleUDPServer::BindCompleted()
{
	mWindow->Show();
}


// ---------------------------------------------------------------------------
//	¥ BindFailed										[public, virtual]
// ---------------------------------------------------------------------------
//	Could not bind to the requested port.

void
CSimpleUDPServer::BindFailed()
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
CSimpleUDPServer::AllowSubRemoval(
	LCommander*		inSub)
{
	if (inSub == mWindow) {
		mServerThread->Complete();
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
CSimpleUDPServer::AttemptQuitSelf(
	SInt32	inSaveOption)
{
	if (mQuitWhenDone) {
		return LCommander::AttemptQuitSelf(inSaveOption);
	} else {
		mQuitWhenDone = true;
		mSaveOption = inSaveOption;

		mServerThread->Complete();
		return false;
	}
}


// ---------------------------------------------------------------------------
//	¥ÊObeyCommand										[public, virtual]
// ---------------------------------------------------------------------------
//	Handle Commands for this object

Boolean
CSimpleUDPServer::ObeyCommand(
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
CSimpleUDPServer::FindCommandStatus(
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
