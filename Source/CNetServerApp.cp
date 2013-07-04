// ===========================================================================
//	CNetServerApp.cp			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================
//	This project is a simple shell to set up server connections.  You can
//  test this program with the NetClient application.

#include "CNetServerApp.h"

#include <LGrowZone.h>
#include <UDrawingState.h>
#include <UMemoryMgr.h>
#include <UModalDialogs.h>
#include <URegistrar.h>
#include <UDebugging.h>
#include <UEnvironment.h>

#include <LWindow.h>
#include <LTabGroupView.h>
#include <LEditText.h>
#include <LCheckBox.h>
#include <UAttachments.h>

#include <UControlRegistry.h>

#include <LThread.h>
#include <UThread.h>
#include <LCleanupTask.h>

/* FB ( */
#include <LGroupBox.h>

#include "CAEThread.h"
#include "CSetWithID.h"
/* ) FB */

#include "CSimpleTCPServer.h"
#include "CSimpleUDPServer.h"
#include "CTerminalPane.h"

/* FB ( */
#include "AppConstants.h"

#include "CPrefs.h"

#include "CTCPResponderThread.h"

const ResIDT	ALRT_Expired					= 1002;
/* ) FB */

// ---------------------------------------------------------------------------
//	¥ main
// ---------------------------------------------------------------------------

int main()
{
		// Set Debugging options
	SetDebugThrow_(debugAction_Alert);
	SetDebugSignal_(debugAction_Alert);

		// Initialize Memory Manager. Parameter is the number of
		// master pointer blocks to allocate
	InitializeHeap(15);
	
		// Initialize standard Toolbox managers
	UQDGlobals::InitializeToolbox(&qd);

		// Check for Thread Manager.
	if (UEnvironment::HasFeature(env_HasThreadManager) == false) {
		::StopAlert(ALRT_NoThreadManager, nil);
		::ExitToShell();
	}

		// Install a GrowZone to catch low-memory situations	
	new LGrowZone(20000);

		// Run the application.
	new UMainThread;

	CNetServerApp 	theApp;
	theApp.AddAttachment(new LYieldAttachment);
	theApp.Run();

		// Make sure async tasks get cleaned up. This call is VERY IMPORTANT.
		// (Note: LCleanupTask patches ExitToShell, so things get cleaned up
		// appropriately if you kill the application.)
	LCleanupTask::CleanUpAtExit();
	
	return 0;
}


// ---------------------------------------------------------------------------
//	¥ CNetServerApp									[public]
// ---------------------------------------------------------------------------
//	Applicaton object constructor

CNetServerApp::CNetServerApp()
{
		// Register ourselves with the Appearance Manager
	if (UEnvironment::HasFeature(env_HasAppearance)) {
		::RegisterAppearanceClient();
	}

	RegisterClasses();

/* FB ( */
	InstallCallBacks ();
/* ) FB */

		// Increase responsiveness for Networking
	SetSleepTime(1);
}


// ---------------------------------------------------------------------------
//	¥ ~CNetServerApp								[public, virtual]
// ---------------------------------------------------------------------------
//	Application object destructor

CNetServerApp::~CNetServerApp()
{
	// Nothing
}


/* FB ( */

// ---------------------------------------------------------------------------
//	¥ Initialize									[public, virtual]
// ---------------------------------------------------------------------------
//	Application initialization

void CNetServerApp::Initialize()
{
	if (gPrefs.notRegistered ())
		DoPreferences (true);
	MakeNewDocument();
}

/* ) FB */

// ---------------------------------------------------------------------------
//	¥ StartUp										[protected, virtual]
// ---------------------------------------------------------------------------
//	Perform an action in response to the Open Application AppleEvent.
//	Here, issue the New command to open a window.

void
CNetServerApp::StartUp()
{
	ObeyCommand(cmd_New, nil);
}


// ---------------------------------------------------------------------------
//	¥ ObeyCommand									[public, virtual]
// ---------------------------------------------------------------------------
//	Respond to Commands. Returns true if the Command was handled, false if not.

Boolean
CNetServerApp::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;	// Assume we'll handle the command

	switch (inCommand) {

/* FB ( */
		case cmd_Preferences:
			DoPreferences(false);
			break;

		case 'rsld':
			gPrefs.resetDelays();
			break;
/* ) FB */

		default: {
			cmdHandled = LDocApplication::ObeyCommand(inCommand, ioParam);
			break;
		}
	}
	
	return cmdHandled;
}


// ---------------------------------------------------------------------------
//	¥ FindCommandStatus								[public, virtual]
// ---------------------------------------------------------------------------
//	Determine the status of a Command for the purposes of menu updating.

void
CNetServerApp::FindCommandStatus(
	CommandT	inCommand,
	Boolean&	outEnabled,
	Boolean&	outUsesMark,
	UInt16&		outMark,
	Str255		outName)
{
	switch (inCommand) {

/* FB ( */
		case cmd_Preferences:
			outEnabled = true;
			break;

		case 'rsld':
			outEnabled = true;
			break;
/*
		case cmd_Open: {
			// The stationery default does nothing for these
			// commands, so they are disabled to avoid any confusion
			// in the menubar. If you implement the appropriate
			// handler functions (e.g. ChooseDocument for cmd_Open)
			// be certain to either remove the case from here
			// (and let LDocApplication or another Commander handle it)
			// or add your own handler here.
			
			outEnabled = false;
			break;
		}
*/
/* ) FB */
	
		default: {
			LDocApplication::FindCommandStatus(inCommand, outEnabled,
											outUsesMark, outMark, outName);
			break;
		}
	}
}

/* FB ( */

// ---------------------------------------------------------------------------
//		¥ DoPreferences
// ---------------------------------------------------------------------------
void
CNetServerApp::DoPreferences(Boolean cantCancel)
{
	gPrefs.DoPreferences (cantCancel, this);
}

// ---------------------------------------------------------------------------
//	¥ SendAECreateDocument											  [public]
// ---------------------------------------------------------------------------
//	Can't create docs!

void
CNetServerApp::SendAECreateDocument()
{
}

/* ) FB */

// ---------------------------------------------------------------------------
//	¥ MakeNewDocument								[public, virtual]
// ---------------------------------------------------------------------------
//	What happens to File->New Session from the menu. Creates a settings dialog
//	then waits modally for a configuration. If completed, then opens a new session.

LModelObject*
CNetServerApp::MakeNewDocument()
{
		// Show the configuration dialog.

/* FB (
	StDialogHandler theHandler(PPob_ServerSettings, this);
	LWindow*		theDialog = theHandler.GetDialog();
	Assert_(theDialog != nil);
	theDialog->Show();
	
	while (true) {
		MessageT hitMessage = theHandler.DoDialog();
		if (hitMessage == msg_Cancel) {
			return nil;
		} else if (hitMessage == msg_OK) {
			break;
		}
	}
	
		// Get the user values from the theHandler fields
	LEditText* countField = dynamic_cast<LEditText*>
								(theDialog->FindPaneByID(ServerSettings_EditCount));
	ThrowIfNil_ (countField);
	SInt32 count = countField->GetValue();

	LEditText* portField = dynamic_cast<LEditText*>
								(theDialog->FindPaneByID(ServerSettings_EditPort));
	ThrowIfNil_ (portField);
	SInt32 port = portField->GetValue();
	
	LCheckBox* udpOptionBox = dynamic_cast<LCheckBox*>
								(theDialog->FindPaneByID(ServerSettings_CheckUDP));
	ThrowIfNil_ (udpOptionBox);
	bool useUDP = udpOptionBox->GetValue();
	
	if (useUDP) {
		CSimpleUDPServer* theConnection = new CSimpleUDPServer(this);
		theConnection->WaitForUDPData(port);
		return theConnection;
	} else {
		CSimpleTCPServer* theConnection = new CSimpleTCPServer(this);
		theConnection->WaitForConnections(count, port);
		return theConnection;
	}
*/
	CSimpleTCPServer* theConnection = new CSimpleTCPServer(this);
	theConnection -> WaitForConnections (gPrefs.GetCount (), gPrefs.GetPort ());

	return theConnection;
/* ) FB */
}

/* FB (
// ---------------------------------------------------------------------------
//	¥ OpenDocument										[public, virtual]
// ---------------------------------------------------------------------------
//	Open a Document specified by an FSSpec

void
CNetServerApp::OpenDocument(
	FSSpec*		inMacFSSpec)
{
#pragma unused(inMacFSSpec)

	// You must implement.
}


// ---------------------------------------------------------------------------
//	¥ PrintDocument										[public, virtual]
// ---------------------------------------------------------------------------
//	Print a Document specified by an FSSpec

void
CNetServerApp::PrintDocument(
	FSSpec*		inMacFSSpec)
{
#pragma unused(inMacFSSpec)

	// You must implement
}


// ---------------------------------------------------------------------------
//	¥ ChooseDocument									[public, virtual]
// ---------------------------------------------------------------------------
//	Allow the user to pick a Document (usually for opening)

void
CNetServerApp::ChooseDocument()
{
	// You must implement
}

) FB */

// ---------------------------------------------------------------------------
//	¥ RegisterClasses								[protected, virtual]
// ---------------------------------------------------------------------------
//	To reduce clutter within the Application object's constructor, class
//	registrations appear here in this seperate function for ease of use.

void
CNetServerApp::RegisterClasses()
{
/* FB ( */
//	RegisterClass_(LCaption);
//	RegisterClass_(LDialogBox);
//	RegisterClass_(LEditField);
	RegisterClass_(LScroller);
//	RegisterClass_(LStdButton);			
	RegisterClass_(LCheckBox);
	RegisterClass_(LTabGroup);
//	RegisterClass_(LGroupBox);
/* ) FB */

		// Register core PowerPlant classes.
	RegisterClass_(LWindow);
	RegisterClass_(LTabGroupView);
	RegisterClass_(LColorEraseAttachment);
	
	RegisterClass_(CTerminalPane);
	
		// Register the Appearance Manager/GA classes. You may want
		// to remove this use of UControlRegistry and instead perform
		// a "manual" registration of the classes. This cuts down on
		// extra code being linked in and streamlines your app and
		// project. However, use UControlRegistry as a reference/index
		// for your work, and ensure to check UControlRegistry against
		// your registrations each PowerPlant release in case
		// any mappings might have changed.
		
	UControlRegistry::RegisterClasses();
}

/* FB ( */
void
CNetServerApp::InstallCallBacks()
{
	OSErr theOSErr = ::AEInstallEventHandler ('MPS ', 'outp',
		NewAEEventHandlerProc (CNetServerApp::diagAndOutputEventHandler),
			0L, false);
		
	theOSErr = ::AEInstallEventHandler ('MPS ', 'diag',
		NewAEEventHandlerProc (CNetServerApp::diagAndOutputEventHandler),
			1L, false);
		
	theOSErr = ::AEInstallEventHandler (kCoreEventClass, kAEAnswer,
		NewAEEventHandlerProc (CNetServerApp::answerEventHandler),
			0L, false);
}

long
CNetServerApp::getTransactionID (const AppleEvent*	inAppleEvent)
{
    DescType typeBack;
    Size sizeBack;
    long transactionID;

    OSErr theOSErr = AEGetAttributePtr (inAppleEvent, keyTransactionIDAttr, typeLongInteger,
    	& typeBack, & transactionID, sizeof (long), & sizeBack);
	if (theOSErr)
		transactionID = 0;
	
	return transactionID;
}

pascal OSErr
CNetServerApp::diagAndOutputEventHandler (
	const AppleEvent*	inAppleEvent,
	AppleEvent*			/* outAEReply */,
	SInt32				inRefCon)
{
    long queueID = getTransactionID (inAppleEvent);
	LSharedQueue * theQueue = CSetWithID::GetData (queueID);

    inRefCon;

	if (theQueue)
	{
	    DescType typeBack;
	    Size sizeBack;

/* extra 512 bytes for possible CR/LF transition */
		OSErr theOSErr = AESizeOfParam (inAppleEvent, keyDirectObject, & typeBack, & sizeBack);
	    if (theOSErr == noErr)
	    {
			Size handleSize = sizeBack + 1024;
			Handle anAEReplyBufferHandle = NewHandle (handleSize);
			
			if (anAEReplyBufferHandle)
			{
				HLock (anAEReplyBufferHandle);
				theOSErr = AEGetParamPtr (inAppleEvent, keyDirectObject, typeChar,
					& typeBack, * anAEReplyBufferHandle, sizeBack, & sizeBack);
			    if (theOSErr == noErr)
				{
					for (Size i = 0; i < sizeBack; i++)
					{
						if (((* anAEReplyBufferHandle) [i] == 13) &&
								(sizeBack + 1 < handleSize))
						{
							unsigned long j;
							
							for (j = sizeBack; j > i; j --)
								(* anAEReplyBufferHandle) [j] =
									(* anAEReplyBufferHandle) [j - 1];
							sizeBack ++;
								
							(* anAEReplyBufferHandle) [i ++] = 10;
						}
					}
				}
				HUnlock (anAEReplyBufferHandle);
				
				SetHandleSize (anAEReplyBufferHandle, sizeBack);

				LLink * aCAELLinkPtr = new CAELLink (anAEReplyBufferHandle);
				theQueue -> NextPut (aCAELLinkPtr);
				LThread::Yield ();
			}
		}
	}
//	else
//		SysBeep (1);

	return noErr;
}

pascal OSErr
CNetServerApp::answerEventHandler (
	const AppleEvent*	inAppleEvent,
	AppleEvent*			/* outAEReply */,
	SInt32				/* inRefCon */)
{
    long queueID = getTransactionID (inAppleEvent);
	LSharedQueue * theQueue = CSetWithID::GetData (queueID);

	if (theQueue)
	{
	    DescType typeBack;
	    Size sizeBack;
	    OSErr longErr;
	    
	    Handle theHandle = nil;

	    OSErr theOSErr = AEGetParamPtr (inAppleEvent, keyErrorNumber, typeLongInteger,
	    	& typeBack, (Ptr) & longErr, sizeof (OSErr), & sizeBack);
	    if (theOSErr != noErr)
	    	longErr = noErr;

		if ((longErr == noErr) || (longErr == -1))
		{
			theOSErr = AESizeOfParam (inAppleEvent, 'who ', & typeBack, & sizeBack);
			if (theOSErr == noErr)
			{
				theHandle = NewHandle (sizeBack + 1);
				if (theHandle)
				{
					AEGetParamPtr (inAppleEvent, 'who ', typeChar, & typeBack,
						* theHandle, sizeBack, & sizeBack);
					(* theHandle) [sizeBack] = '>';
		    	}
			}
			else
			{
				theHandle = NewHandle (11);
				if (theHandle)
					BlockMoveData ("ToolDaemon>", * theHandle, 11);
			}
		}

		if (theHandle)
		{
			LLink * aCAELLinkPtr = new CAELLink (theHandle);
			theQueue -> NextPut (aCAELLinkPtr);
			LThread::Yield ();
		}
	}
//	else
//		SysBeep (1);

	return noErr;
}
/* ) FB */
