// ===========================================================================
//	CTCPResponderThread.cp		©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================
//	This is the threaded version of the responder process.

#include "CTCPResponderThread.h"
#include "CTCPResponder.h"

/* FB ( */
#include "simpleAE.h"

#include <stdio.h>

#include "CPrefs.h"
#include "authenticateUser.h"
/* ) FB */

#include <cstring>

#define kServerClosingMessage "\r\nServer closing down... Goodbye"

/* FB ( */
#define kServerUserNameMessage "Username:"
#define kServerPasswordMessage "Password:"
#define kServerWelcomeMessage "ToolDaemon 2.5.1. (c)1999-2000, DouWere.  Program by Frederic Blondiau.\r\n"
#define kServerAuthenticationFailedMessageStart "Authentication failed (error "
#define kServerAuthenticationFailedMessageEnd ").\r\n"
/* ) FB */


// ---------------------------------------------------------------------------
//	¥ CTCPResponderThread								[public]
// ---------------------------------------------------------------------------
//	Constructor

CTCPResponderThread::CTCPResponderThread(
	LTCPEndpoint*		inNetworkEndpoint,
/* FB ( */
	long				inQueueID,
/* ) FB */
	CTerminalPane*		inTerminalPane,
	CTCPResponder*		inResponderMaster,
	CSimpleTCPServer*	inMasterServer)

	: LThread(	false,
				thread_DefaultStack,
				LThread::threadOption_Default,
				nil),
	  mEndpoint(inNetworkEndpoint),
	  mTerminalPane(inTerminalPane),
	  mResponderMaster(inResponderMaster),
	  mMasterServer(inMasterServer)
{
	mContinue = true;
	mInDisconnect = false;

/* FB ( */
	mLoginPhase = kStartingPhase;
	mLoginTry = 0;

	* mEnteredUserName = 0;
	* mOutDescriptor = 0;

	mQueueID = inQueueID;
/* ) FB */

	mEndpoint->AddListener(this);
}


// ---------------------------------------------------------------------------
//	¥ ~CTCPResponderThread								[public, virtual]
// ---------------------------------------------------------------------------
//	Destructor

CTCPResponderThread::~CTCPResponderThread()
{
	// nothing
}


// ---------------------------------------------------------------------------
//	¥ ListenToMessage									[public, virtual]
// ---------------------------------------------------------------------------
void
CTCPResponderThread::ListenToMessage(
	MessageT	inMessage,
	void*		/*ioParam*/)
{
	switch (inMessage) {
		case T_DISCONNECT:
			mContinue = false;
			break;
			
		case T_ORDREL:
			mContinue = false;
			if (mInDisconnect) {
				this->Resume();
			}
			mInDisconnect = true;	//if not in a disconnect sequence we are now
			break;
	}
}


// ---------------------------------------------------------------------------
//	¥ StartDisconnect									[public]
// ---------------------------------------------------------------------------

void
CTCPResponderThread::StartDisconnect()
{
	if (mContinue) {
		mContinue = false;
		mEndpoint->AbortThreadOperation(this);
	}
}

/* FB ( */

// ---------------------------------------------------------------------------
//	¥ DoTelnetCommand											[protected]
// ---------------------------------------------------------------------------

enum
{
	kTelnetWILL = 251,
	kTelnetWONT,
	kTelnetDO,
	kTelnetDONT,
	kTelnetIAC
};

enum
{
	kTelnetNoSuchCommand = 0,
	kTelnetEchoCommand = 1,
	kTelnetSuppressGoAheadCommand = 3
};

Boolean CTCPResponderThread::DoTelnetCommand(Boolean toolServerRunning)
{
	char theChar;
	Boolean toSkip;

	toSkip = true;
	mEndpoint->ReceiveChar(theChar);
	switch ((unsigned char) theChar)
	{
		case 240:
		case 241:
		case 242:
		case 243:
		case 245:
		case 247:
		case 248:
		case 249:
		case 250:
			break;

		case 244: /* interrupt process */
			if (toolServerRunning)
				sendDoScript (& mProcessSerialNumber, ".", 1, mQueueID);
			break;

		case 246: /* are you there */
			if (toolServerRunning)
				sendDoScript (& mProcessSerialNumber, "", 0, mQueueID);
			break;

		case kTelnetWILL:
		{
			char telnetCommand [] = { kTelnetIAC, kTelnetDONT, kTelnetNoSuchCommand };

			mEndpoint -> ReceiveChar (theChar);
			telnetCommand [2] = theChar;

			if (theChar == kTelnetSuppressGoAheadCommand)
			{
// OK... don't ack, as we already asked!
				telnetCommand [1] = kTelnetDO;
				mEndpoint -> Send ((void *) telnetCommand, 3);
			}
			else
				mEndpoint -> Send ((void *) telnetCommand, 3);

			break;
		}

		case kTelnetWONT:
			mEndpoint -> ReceiveChar (theChar);
			break;

		case kTelnetDO:
		{
			char telnetCommand [] = { kTelnetIAC, kTelnetWONT, kTelnetNoSuchCommand };

			mEndpoint -> ReceiveChar (theChar);
			telnetCommand [2] = theChar;
			if ((theChar == kTelnetEchoCommand) || (theChar == kTelnetSuppressGoAheadCommand))
			{
// already in echo ON... don't ack!
// already in suppressing GoAhead... don't ack!
//				telnetCommand [1] = kTelnetWILL;
//				mEndpoint -> Send ((void *) telnetCommand, 3);
			}
			else
				mEndpoint -> Send ((void *) telnetCommand, 3);
			break;
		}

		case kTelnetDONT:
			mEndpoint -> ReceiveChar (theChar);
			if (theChar == kTelnetEchoCommand)
			{
				char willCommand [] = { kTelnetIAC, kTelnetWILL, kTelnetEchoCommand };
				mEndpoint -> Send ((void *) willCommand, 3);
			}

			break;
		
		case kTelnetIAC:
			toSkip = false;
	}
	
	return toSkip;
}

/* ) FB */

// ---------------------------------------------------------------------------
//	¥ Run												[protected, virtual]
// ---------------------------------------------------------------------------

#define kAbortTimeOut 15

void*
CTCPResponderThread::Run()
{
	try {
			//	The endpoint is automagically bound to the port on which
			//	incomming connection was recieved via the 'AcceptIncoming' event.
		mMasterServer->GetEndpoint()->AcceptIncoming(mEndpoint);
		mMasterServer->AddToConnectionCount(1);

		mResponderMaster->BindCompleted();

		while (mContinue)
		{

/* FB ( */
			switch (mLoginPhase)
			{
				case kStartingPhase:
				{
					char willEchoAndSupressGoAheadCommand [] =
					{
						kTelnetIAC, kTelnetWILL, kTelnetEchoCommand, 
						kTelnetIAC, kTelnetWILL, kTelnetSuppressGoAheadCommand
					};
/* will send echo */
					mEndpoint->Send ((void *) willEchoAndSupressGoAheadCommand, 6);

					LInternetAddress* theAddress = mEndpoint->GetRemoteHostAddress();

					if (gPrefs.GetDontUseDNS ())
						theAddress->GetIPDescriptor (mOutDescriptor);
					else
						theAddress->GetDNSDescriptor (mOutDescriptor, false);

					gPrefs.WriteLog (false, "\p", mOutDescriptor, "session opened", 0);

					mLoginPhase = kStartingUserNamePhase;
					break;
				}

				case kStartingUserNamePhase:
				{
					const short kServerUserNameMessageLen = (short) strlen (kServerUserNameMessage);

					mTerminalPane->DoWriteBfr (kServerUserNameMessage, kServerUserNameMessageLen);
					mEndpoint->Send ((void *) kServerUserNameMessage, kServerUserNameMessageLen);

					mLoginPhase = kWaitingUserNameLoginPhase;
					break;
				}

				case kWaitingUserNameLoginPhase:
				{
					char theChar;
					Boolean toSkip;

					if (mEndpoint->ReceiveChar (theChar, kAbortTimeOut))
					{
						if (theChar == -1)
							toSkip = DoTelnetCommand(false);
						else
							toSkip = false;

						if (! toSkip)
						{
	/* be kind, tab is return! */
							if (theChar == 0x09)
								theChar = 0x0D;

							if ((theChar == 0x0A) || (theChar == 0x00))
							{
							}
							else
							{
								mTerminalPane->DoWriteChar(theChar);
								theChar = mCurrentLine.DoWriteChar(theChar);
								if (theChar == 127)
								{
									char delSeq [3] = { 8, ' ', 8 };
									mEndpoint->Send (delSeq, 3);
								}
								else
									mEndpoint->Send (&theChar, 1);
								
								if (theChar == 0x0D)
								{
									theChar = 0x0A;

									mTerminalPane->DoWriteChar(theChar);
									mCurrentLine.DoWriteChar(theChar);
									mEndpoint->Send(&theChar, 1);

									strncpy ((char *) mEnteredUserName, (char *) mCurrentLine.GetLine (),
										kUserNameMaxLen);
									mEnteredUserName [kUserNameMaxLen] = 0;
									c2pstr ((char *) mEnteredUserName);
									
									if (* mEnteredUserName)
										mLoginPhase = kCheckingDelay;
									else
										mLoginPhase = kStartingUserNamePhase;
								}
							}
						}
					}
					else
						mContinue = false;
					break;
				}

				case kCheckingDelay:
				{
					mLoginPhase = kDelayingPasswordPhase;
					
					gPrefs.getLoginInfoOf (mEnteredUserName, & mLoginFailed, nil, nil, nil);
					break;
				}

				case kDelayingPasswordPhase:
				{
					if (mLoginFailed)
					{
						short abortTimeOut;
						
						abortTimeOut = kAbortTimeOut;
//						mEndpoint -> Send("Please, wait", 12);
						while (mLoginFailed)
						{
							mEndpoint -> Send(".", 1);
							mLoginFailed --;
							Sleep (1000);
							
							abortTimeOut --;
							if (! abortTimeOut)
							{
								mContinue = false;
								break;
							}
						} ;
						mEndpoint -> Send("\r\n", 2);
					}

					mLoginPhase = kStartingPasswordPhase;
					break;
				}

				case kStartingPasswordPhase:
				{
					const short kServerPasswordMessageLen = (short) strlen (kServerPasswordMessage);

//					mTerminalPane->DoWriteBfr (kServerPasswordMessage, kServerPasswordMessageLen);
					mEndpoint->Send ((void *) kServerPasswordMessage, kServerPasswordMessageLen);

					mLoginPhase = kWaitingPasswordLoginPhase;
					break;
				}

				case kWaitingPasswordLoginPhase:
				{
					char theChar;
					Boolean toSkip;

					if (mEndpoint->ReceiveChar (theChar, kAbortTimeOut))
					{
						if (theChar == -1)
							toSkip = DoTelnetCommand(false);
						else
							toSkip = false;

						if (! toSkip)
						{
							if ((theChar == 0x0A) || (theChar == 0x00))
							{
							}
							else
							{
								theChar = mCurrentLine.DoWriteChar(theChar);
								if ((theChar != 0x0D) && (theChar != 0x0A))
								{
									if (theChar == 127)
									{
										char delSeq [3] = { 8, ' ', 8 };
										mEndpoint->Send (delSeq, 3);
									}
									else if (theChar >= 32)
										mEndpoint->Send("*", 1);
									else
										mEndpoint->Send(&theChar, 1);
								}
								else
									mEndpoint->Send(&theChar, 1);

								if (theChar == 0x0D)
								{
									theChar = 0x0A;

									mTerminalPane->DoWriteChar(theChar);
									mCurrentLine.DoWriteChar(theChar);
									mEndpoint->Send(&theChar, 1);

									strncpy ((char *) mEnteredPassword, (char *) mCurrentLine.GetLine (),
										kPasswordMaxLen);
									mEnteredPassword [kPasswordMaxLen] = 0;
									c2pstr ((char *) mEnteredPassword);

									mLoginPhase = kValidatingLoginPhase;
								}
							}
						}
					}
					else
						mContinue = false;
					break;
				}

				case kValidatingLoginPhase:
				{
					mLoginErr = authenticateUser ((StringPtr) mEnteredUserName,
						(StringPtr) mEnteredPassword, gPrefs.GetAdminPass (),
							(StringPtr) mFoundUserName);

					if (mLoginErr == noErr)
						mLoginPhase = kCheckingLoginPhase ;
					else
						mLoginPhase = kLoginFailedLoginPhase ;
					break;
				}

				case kLoginFailedLoginPhase:
				{
					gPrefs.WriteLog (false, mEnteredUserName, mOutDescriptor, "login FAILED",
						mLoginErr);

					const short kServerAuthenticationFailedMessageStartLen =
						(short) strlen (kServerAuthenticationFailedMessageStart);
					const short kServerAuthenticationFailedMessageEndLen =
						(short) strlen (kServerAuthenticationFailedMessageEnd);

					mTerminalPane->DoWriteBfr (kServerAuthenticationFailedMessageStart,
						kServerAuthenticationFailedMessageStartLen);
					mEndpoint->Send ((void *) kServerAuthenticationFailedMessageStart,
						kServerAuthenticationFailedMessageStartLen);
					LStr255 loginErrMessage = mLoginErr;
					mTerminalPane->DoWriteBfr (loginErrMessage.TextPtr (),
						loginErrMessage.Length ());
					mEndpoint->Send ((void *) loginErrMessage.TextPtr (),
						loginErrMessage.Length ());
					mTerminalPane->DoWriteBfr (kServerAuthenticationFailedMessageEnd,
						kServerAuthenticationFailedMessageEndLen);
					mEndpoint->Send ((void *) kServerAuthenticationFailedMessageEnd,
						kServerAuthenticationFailedMessageEndLen);

					gPrefs.setLoginInfoOf (mEnteredUserName, false, nil);

					mLoginTry ++;
					if (mLoginTry >= 3)
						mContinue = false;

					mLoginPhase = kStartingUserNamePhase;

					break;
				}

				case kCheckingLoginPhase:
				{
					LStr255 toolServerPath = "\p:Users:";
					toolServerPath += mFoundUserName;
					
					OSErr theOSErr = FSMakeFSSpec (0, 0, toolServerPath, & mToolServerFSSpec);
					if (theOSErr == noErr)
					{
						short tmpRefNum;
						short tmpResFile;
						
						tmpResFile = CurResFile ();

						SetResLoad (false);
						tmpRefNum = HOpenResFile (mToolServerFSSpec.vRefNum,
							mToolServerFSSpec.parID, mToolServerFSSpec.name, fsRdWrPerm);
						theOSErr = ResError ();
						SetResLoad (true);

						if (theOSErr == noErr)
						{
							FInfo finfo;

							if (HGetFInfo (mToolServerFSSpec.vRefNum, mToolServerFSSpec.parID,
									mToolServerFSSpec.name, & finfo) == noErr)
							{
								if (finfo.fdFlags & kIsAlias)
								{
									Handle tmpHandle;

									tmpHandle = Get1Resource ('alis', 0);
									if (tmpHandle)
									{
										Boolean wasChanged;

										theOSErr = ResolveAlias (nil, (AliasHandle) tmpHandle,
											& mToolServerFSSpec, & wasChanged);
									}
								}
							}
							CloseResFile (tmpRefNum);
						}

						UseResFile (tmpResFile);

						toolServerPath = "\p:";
						toolServerPath += mToolServerFSSpec.name;
						toolServerPath += "\p:ToolServer";

						theOSErr = FSMakeFSSpec (mToolServerFSSpec.vRefNum, 
							mToolServerFSSpec.parID, toolServerPath, & mToolServerFSSpec);

						if (theOSErr)
						{
							mLoginErr = theOSErr;
							mLoginPhase = kLoginFailedLoginPhase;
						}
						else
							mLoginPhase = kLoggingLoginPhase;
					}
					else
					{
						mLoginErr = theOSErr;
						mLoginPhase = kLoginFailedLoginPhase;
					}
					break;
				}

				case kLoggingLoginPhase:
				{
					short nbFailed;
					long dateLoginSuccess;
					Str255 clientHostName;

					gPrefs.WriteLog (false, mEnteredUserName, mOutDescriptor, "login successful",
						0);
					
					gPrefs.getLoginInfoOf (mEnteredUserName, nil, & nbFailed,
						& dateLoginSuccess, clientHostName);
					
					if (dateLoginSuccess)
					{
						Str255 dateBuffer;
						Str255 timeBuffer;
						char buffer [256];
						Handle tmpHandle;
						
						tmpHandle = GetResource ('itl1', 0);
						DateString (dateLoginSuccess, true, dateBuffer, tmpHandle);

						tmpHandle = GetResource ('itl0', 0);
						TimeString (dateLoginSuccess, false, timeBuffer, tmpHandle);

						sprintf (buffer, "Last login on %#s at %#s%s%#s.\r\n", dateBuffer,
							timeBuffer, (* clientHostName) ? " from " : "", clientHostName);
						mEndpoint -> Send(buffer, strlen (buffer));
					}

					if (nbFailed)
					{
						char buffer [256];

						sprintf (buffer, "%d login attempt%s failed.\r\n", nbFailed,
							(nbFailed > 1) ? "s" : "");
						mEndpoint -> Send(buffer, strlen (buffer));
					}

					if (gPrefs.GetLogDetailed ())
					{
						char buffer [256];

						sprintf (buffer, "Your activity is logged.\r\n");
						mEndpoint -> Send(buffer, strlen (buffer));
					}

					gPrefs.setLoginInfoOf (mEnteredUserName, true, mOutDescriptor);

					mLoginPhase = kLaunchingToolServerLoginPhase;
					break;
				}

				case kLaunchingToolServerLoginPhase:
				{
					LaunchParamBlockRec thePB;
					OSErr theOSErr;
					
					thePB.launchBlockID = extendedBlock;
					thePB.launchEPBLength = extendedBlockLen;
					thePB.launchFileFlags = launchNoFileFlags;
					thePB.launchControlFlags = launchContinue | launchDontSwitch;
					thePB.launchAppSpec = & mToolServerFSSpec;
					thePB.launchAppParameters = NULL;
					
					theOSErr = LaunchApplication (& thePB);
					
					if (theOSErr)
					{
						mLoginErr = theOSErr;
						mLoginPhase = kLoginFailedLoginPhase;
					}
					else
					{
						mProcessSerialNumber = thePB.launchProcessSN;
						mLoginPhase = kSuccessLoginPhase ;
					}
					break;
				}

				case kSuccessLoginPhase:
				{
					const short kServerWelcomeMessageLen = (short) strlen (kServerWelcomeMessage);
					char loginPrompt [256];

					mTerminalPane->DoWriteBfr (kServerWelcomeMessage, kServerWelcomeMessageLen);
					mEndpoint->Send ((void *) kServerWelcomeMessage, kServerWelcomeMessageLen);

					gPrefs.getLoginPrompt (loginPrompt);
					if (strlen (loginPrompt))
						mEndpoint->Send (loginPrompt, strlen (loginPrompt));

					mLoginPhase = kRunningLoginPhase;
					
					gPrefs.rememberLogin ();

					LStr255 startupScript = "Set Users `Evaluate {Users}+1`;Export Users;Execute \"ToolDaemon Logon\" \"";
					startupScript += mFoundUserName;
					startupScript += "\"";
					sendDoScript (& mProcessSerialNumber, startupScript.TextPtr (),
						startupScript.Length (), mQueueID);

					break;
				}

				default:
				{
					char theChar;
					Boolean toSkip;

					mEndpoint->ReceiveChar(theChar);
					if (theChar == -1)
						toSkip = DoTelnetCommand(true);
					else
						toSkip = false;

					if (! toSkip)
					{
						if ((theChar == 0x0A) || (theChar == 0x00))
						{
						}
						else
						{
							mTerminalPane->DoWriteChar(theChar);
							theChar = mCurrentLine.DoWriteChar(theChar);
							if (theChar == 127)
							{
								char delSeq [3] = { 8, ' ', 8 };
								mEndpoint->Send (delSeq, 3);
							}
							else
								mEndpoint->Send (&theChar, 1);
							
							if (theChar == 0x0D)
							{
								char * line;
								
								theChar = 0x0A;

								mTerminalPane->DoWriteChar(theChar);
								mCurrentLine.DoWriteChar(theChar);
								mEndpoint->Send(&theChar, 1);

								line = (char *) mCurrentLine.GetLine ();
								if (sendDoScript (& mProcessSerialNumber, line,
										strlen (line), mQueueID))
									mContinue = false;
								gPrefs.WriteLog (true, mEnteredUserName, mOutDescriptor,
									line, ! mContinue);
							}
						}
					}
					break;
				}
			}
		}
	}
/* ) FB */

/* FB (
		{

//Example of using the ReceiveLine method
//				char currLine[80];
//				UInt32 dataSize = 80;
//				mEndpoint->ReceiveLine(currLine, dataSize, (5 * 60));	//wait 5 minutes

//Example of using the ReceiveChar method
				char theChar;
				mEndpoint->ReceiveChar(theChar);

								//Write it to the terminal pane
				mTerminalPane->DoWriteChar(theChar);
//				mTerminalPane->DoWriteBfr(currLine, dataSize);

								//Turn it back around to the client.
				mEndpoint->Send(&theChar, 1);
//				mEndpoint->Send(currLine, dataSize);
		}
	}
) FB */

	catch(...) { }

	gPrefs.WriteLog (false, mEnteredUserName, mOutDescriptor, "session closed", 0);

		// We're done with this connectionÉ disconnect as appropriate.
	try {
		if (!mInDisconnect) {			//We must be initiating disconnect
				//Tell remote machine we are closing
					
			mEndpoint->Send(kServerClosingMessage, strlen(kServerClosingMessage));
			mInDisconnect = true;
			mEndpoint->SendDisconnect();
/* FB (
			Suspend();					//wait for T_ORDREL message
) FB */
				//	NOTE: You could continue to read data from the remote connection
				//	at this point (instead of Suspending the thread) if you care about 
				//	receiving all it's data.
/* FB ( */
			char theChar;
			while (mEndpoint->ReceiveChar(theChar))
				LThread::Yield ();
/* ) FB */
		} else {
			mEndpoint->Disconnect();
		}
		mEndpoint->Unbind();
	}
	catch(...) { }

	mMasterServer->AddToConnectionCount(-1);
	mResponderMaster->ServerThreadDied();

/* FB ( */
	LStr255 endScript = "Set Users `Evaluate {Users}-1`;Export Users;Execute \"ToolDaemon Logoff\" \"";
	endScript += mFoundUserName;
	endScript += "\";If {Users} == 0;Quit;End";
	sendDoScript (& mProcessSerialNumber, endScript.TextPtr (),
		endScript.Length (), mQueueID);
/* ) FB */

	return nil;
}
