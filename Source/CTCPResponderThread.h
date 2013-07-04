// ===========================================================================
//	CTCPResponderThread.h		©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CTCPResponderThread
#define _H_CTCPResponderThread
#pragma once

#include <LThread.h>
#include <LListener.h>

#include <LTCPEndpoint.h>
#include "CTerminalPane.h"
/* FB ( */
#include "CCurrentLine.h"
/* ) FB */
#include "CSimpleTCPServer.h"

class CTCPResponder;

// ===========================================================================
//		¥ CTCPResponderThread
// ===========================================================================
//	The thread that actually does the work of the Server interpreter.
//	Note that typed characters (i.e. data sent to the remote host
//	are NOT handled by this object.) Only incoming data are processed
//	here.

class CTCPResponderThread :	public LThread,
							public LListener {

public:
									CTCPResponderThread(
										LTCPEndpoint*		inNetworkEndpoint,
/* FB ( */
											long				inQueueID,
/* ) FB */
										CTerminalPane*		inTerminalPane,
										CTCPResponder*		inResponderMaster,
										CSimpleTCPServer*	inMasterServer);
	virtual							~CTCPResponderThread();

			void					StartDisconnect();

	virtual void					ListenToMessage(
											MessageT		inMessage,
											void*			ioParam);

protected:
	virtual void*					Run();
/* FB ( */
			Boolean					DoTelnetCommand(Boolean toolServerRunning);
/* ) FB */

			LTCPEndpoint*			mEndpoint;			// network endpoint
			CTerminalPane*			mTerminalPane;		// terminal display for this connection

/* FB ( */
			CCurrentLine			mCurrentLine;
			long					mQueueID;
/* ) FB */

			CTCPResponder*			mResponderMaster;	// Responder object that spawned this thread
			CSimpleTCPServer*		mMasterServer;		// Master EP that issued the Listen();
			bool					mContinue;
			bool					mInDisconnect;

/* FB ( */
			enum					{ kStartingPhase, kStartingUserNamePhase,
										kWaitingUserNameLoginPhase,
										kCheckingDelay, kDelayingPasswordPhase,
										kStartingPasswordPhase, kWaitingPasswordLoginPhase,
										kValidatingLoginPhase, kLoginFailedLoginPhase,
										kCheckingLoginPhase, kLoggingLoginPhase,
										kLaunchingToolServerLoginPhase,
										kSuccessLoginPhase, kRunningLoginPhase }
									mLoginPhase;
			long					mLoginFailed;
			short					mLoginTry;

			enum					{ kUserNameMaxLen = 32,	kPasswordMaxLen = 8	};
			unsigned char			mEnteredUserName [kUserNameMaxLen + 1];
			unsigned char			mEnteredPassword [kPasswordMaxLen + 1];
			unsigned char			mFoundUserName [kUserNameMaxLen + 1];
			OSErr					mLoginErr;

			Str255					mOutDescriptor;
			
			FSSpec					mToolServerFSSpec;
			ProcessSerialNumber		mProcessSerialNumber;
/* ) FB */

private:
									CTCPResponderThread();
									CTCPResponderThread(const CTCPResponderThread& inOriginal);
			CTCPResponderThread&	operator=(const CTCPResponderThread& inRhs);
};

#endif // _H_CTCPResponderThread
