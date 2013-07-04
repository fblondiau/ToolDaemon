// ===========================================================================
//	CNetServerApp.h			©1995-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CNetServerApp
#define _H_CNetServerApp
#pragma once

#include <LDocApplication.h>


class CNetServerApp : public LDocApplication {

public:
							CNetServerApp();
	virtual					~CNetServerApp();

/* FB ( */
	virtual void			Initialize();
	virtual void			DoPreferences (Boolean cantCancel);
	virtual void			SendAECreateDocument();
/* ) FB */	

	virtual LModelObject*	MakeNewDocument();

/* FB ( */
	virtual void InstallCallBacks();

protected:
	static long getTransactionID (const AppleEvent*	inAppleEvent);

	static pascal OSErr diagAndOutputEventHandler(
								const AppleEvent*	inAppleEvent,
								AppleEvent*			outAEReply,
								SInt32				inRefCon);
	static pascal OSErr answerEventHandler(
								const AppleEvent*	inAppleEvent,
								AppleEvent*			outAEReply,
								SInt32				inRefCon);
/* ) FB */

/* FB (
	virtual	void			OpenDocument(
								FSSpec*			inMacFSSpec);
	virtual void			PrintDocument(
								FSSpec*			inmacFSSpec);
	virtual	void			ChooseDocument();
) FB */

	virtual Boolean			ObeyCommand(
									CommandT			inCommand,
									void				*ioParam);

	virtual void			FindCommandStatus(
									CommandT			inCommand,
									Boolean				&outEnabled,
									Boolean				&outUsesMark,
									UInt16				&outMark,
									Str255				outName);


protected:
	virtual	void			StartUp();
	
			void			RegisterClasses();

};

#endif // _H_CNetServerApp
