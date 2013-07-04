// ===========================================================================
//	CPrefs.h
// ===========================================================================

#ifndef _H_CPrefs
#define _H_CPrefs
#pragma once

#include <LPreferencesFile.h>

class CPrefs
{
public:
	CPrefs (void);
	
	short	GetCount()			{ return mCount; }
	short	GetPort()			{ return mPort; }

	short	GetDontUseDNS()		{ return mDontUseDNS; }

	short	GetLogEnabled()		{ return mLogEnabled; }
	short	GetLogDetailed()	{ return mLogDetailed; }

	LStr255	GetAdminPass()		{ return mAdminPass; }
	LStr255	GetUserName()		{ return mUserName; }
	LStr255	GetUserKey()		{ return mUserKey; }

	void DoPreferences(Boolean cantCancel, LCommander * theCommander);
	Boolean notRegistered (void);

	void rememberLogin (void)	{ mNbLogin ++; };

	char * getLoginPrompt		(char *);
	OSErr getLoginInfoOf (StringPtr, long *, short *, long *, StringPtr);
	OSErr setLoginInfoOf (StringPtr, Boolean, StringPtr);

	void 	WriteLog (Boolean, StringPtr, StringPtr, char *, OSErr);

	void resetDelays		(void);

protected:
	unsigned long			mCount;
	unsigned long			mPort;

	unsigned long			mNbLogin;
	unsigned long			mNbStartup;

	Boolean					mDontUseDNS;

	Boolean					mLogEnabled;
	Boolean					mLogDetailed;

	unsigned long			mStartTime;

	LStr255					mAdminPass;
	LStr255					mUserName;
	LStr255					mUserKey;

	LPreferencesFile		* mPreferencesFile;
	LPreferencesFile		* mLogFile;

	void 	WritePrefs (void);
	void 	ReadPrefs (void);

	virtual					~CPrefs();
};

extern CPrefs gPrefs;

#endif // _H_CPrefs
