#include <LEditText.h>
#include <LGADialog.h>
#include <LFile.h>
#include <LPreferencesFile.h>
#include <LPushButton.h>
#include <LPictureControl.h>
#include <LStaticText.h>
#include <LCheckBox.h>

#include <string.h>

#include "ic.h"
#include "endecode.h"
#include "base64.h"
#include "CPrefs.h"

CPrefs			gPrefs;

const ResIDT	PPob_ServerSettings				= 128;

const PaneIDT	ServerSettings_EditCount		= FOUR_CHAR_CODE('NUML');
const PaneIDT	ServerSettings_EditPort			= FOUR_CHAR_CODE('PORT');
const PaneIDT	ServerSettings_CheckUDP			= FOUR_CHAR_CODE('_UDP');
const PaneIDT	ServerSettings_EditAdminPass	= FOUR_CHAR_CODE('ADMP');
const PaneIDT	ServerSettings_EditUserName		= FOUR_CHAR_CODE('USNA');
const PaneIDT	ServerSettings_EditUserKey		= FOUR_CHAR_CODE('USKY');

const PaneIDT	ServerSettings_RegisterPict		= FOUR_CHAR_CODE('REGI');
const PaneIDT	ServerSettings_StatKey			= FOUR_CHAR_CODE('KEY_');
const PaneIDT	ServerSettings_ThanksStatText	= FOUR_CHAR_CODE('REG2');


typedef struct loginPrefsBlock
{
	unsigned long creator;
	unsigned long version;
	
	unsigned long startTime; /* to reset delays, restart ! */

	unsigned long nbLoginFailed;
	unsigned long dateLoginFailed;

	unsigned long dateLoginSuccess;

	Str255	clientHostName;
} loginPrefsRec, * loginPrefsPtr, ** loginPrefsHandle;

typedef struct prefsBlock
{
	unsigned long creator;
	unsigned long version;
	
	unsigned long date;
	unsigned long nbLogin;
	unsigned long nbStartup;
	unsigned long flags;
			/* bit 0 = dont use DNS */
			/* bit 1 = log enabled */
			/* bit 2 = log detailed */

	unsigned long count;
	unsigned long port;

	Str255	scrambledAdminPass;

	Str32	userName;
	Str32	regKey;
} prefsRec, * prefsPtr, ** prefsHandle;

// ---------------------------------------------------------------------------
//		¥ CPrefs
// ---------------------------------------------------------------------------
CPrefs::CPrefs (void)
{
	mPreferencesFile = new LPreferencesFile ("\pToolDaemon Prefs", true);
	mLogFile = new LPreferencesFile ("\pToolDaemon Log", true);

	mNbLogin = 0;
	mNbStartup = 0;
	
	mCount = 4;
	mPort = 23;

	mAdminPass = "";
	mUserName = "\p";
	mUserKey = "\p";

	GetDateTime (& mStartTime);
	
	try
	{
		mPreferencesFile -> OpenDataFork (fsRdPerm);
		mPreferencesFile -> CloseDataFork ();
	}
	
	catch (...)
	{
		mPreferencesFile -> CreateNewDataFile ('DwFt', 'PREF', smSystemScript);

		WritePrefs ();
	}

	ReadPrefs ();
	
	try
	{
		mLogFile -> OpenDataFork (fsRdPerm);
		mLogFile -> CloseDataFork ();
	}
	
	catch (...)
	{
		mLogFile -> CreateNewDataFile ('ttxt', 'TEXT', smSystemScript);
	}
}

// ---------------------------------------------------------------------------
//		¥ getLoginInfoOf
// ---------------------------------------------------------------------------
OSErr CPrefs::getLoginInfoOf (StringPtr userName, long * nbSecsToWait,
	short * nbFailed, long * dateLoginSuccess, StringPtr clientHostName)
{
	OSErr returnedValue;

	returnedValue = noErr;

	try
	{
		mPreferencesFile -> OpenResourceFork (fsRdPerm);
		mPreferencesFile -> CloseResourceFork ();
	}
	catch (...)
	{
		mPreferencesFile -> CreateNewFile ('DwFt', 'PREF', smSystemScript);
	}

	try
	{
		loginPrefsHandle tmpHandle;

		mPreferencesFile -> OpenResourceFork (fsRdPerm);
		tmpHandle = (loginPrefsHandle) Get1NamedResource ('logI', userName);
		if (tmpHandle)
		{
			if (nbFailed)
				* nbFailed = (** tmpHandle).nbLoginFailed;

			if (nbSecsToWait)
			{
				if ((** tmpHandle).nbLoginFailed)
				{
					unsigned long now;
					long tmpLong;
					unsigned long timeToWait;
					unsigned long waitOne = 1;
					unsigned long waitTwo = 2;

					GetDateTime (& now);
					
					tmpLong = (** tmpHandle).nbLoginFailed;
					while (tmpLong)
					{
						timeToWait = waitOne + waitTwo;
						waitOne = waitTwo;
						waitTwo = timeToWait;

						tmpLong --;
					};
					
					* nbSecsToWait = (** tmpHandle).dateLoginFailed + timeToWait - now;
					if (* nbSecsToWait < 0)
						* nbSecsToWait = 0;

					if ((** tmpHandle).startTime != mStartTime)
						* nbSecsToWait = 0;
				}
				else
					* nbSecsToWait = 0;
			}

			if (dateLoginSuccess)
				* dateLoginSuccess = (** tmpHandle).dateLoginSuccess;
			if (clientHostName)
				LString::CopyPStr ((** tmpHandle).clientHostName, clientHostName,
					(** tmpHandle).clientHostName [0] + 1);
		}
		else
		{
			if (nbSecsToWait)
				* nbSecsToWait = 0;
			if (nbFailed)
				* nbFailed = 0;
			if (dateLoginSuccess)
				* dateLoginSuccess = 0;
			if (clientHostName)
				* clientHostName = 0;
		}
			
		mPreferencesFile -> CloseResourceFork ();
	}
	catch (...)
	{
		returnedValue = ResError ();
	}

	return returnedValue;
}

// ---------------------------------------------------------------------------
//		¥ setLoginInfoOf
// ---------------------------------------------------------------------------
OSErr CPrefs::setLoginInfoOf (StringPtr userName, Boolean isOK,
	StringPtr clientHostName)
{
	short returnedValue;

	returnedValue = 0;

	try
	{
		mPreferencesFile -> OpenResourceFork (fsRdPerm);
		mPreferencesFile -> CloseResourceFork ();
	}
	catch (...)
	{
		mPreferencesFile -> CreateNewFile ('DwFt', 'PREF', smSystemScript);
	}

	try
	{
		loginPrefsHandle tmpHandle;

		mPreferencesFile -> OpenResourceFork (fsRdWrPerm);
		tmpHandle = (loginPrefsHandle) Get1NamedResource ('logI', userName);
		if (! tmpHandle)
		{
			tmpHandle = (loginPrefsHandle) NewHandleClear (sizeof (loginPrefsRec));
			if (tmpHandle)
				AddResource ((Handle) tmpHandle, 'logI', UniqueID ('logI'), userName);
		}

		if (tmpHandle)
		{
			(** tmpHandle).creator = 'DwFt';
			(** tmpHandle).version = 0x01000000;

			(** tmpHandle).startTime = mStartTime;
			
			if (isOK)
			{
				GetDateTime (& (** tmpHandle).dateLoginSuccess);
				(** tmpHandle).nbLoginFailed = 0;
			}
			else
			{
				GetDateTime (& (** tmpHandle).dateLoginFailed);
				(** tmpHandle).nbLoginFailed ++;
			}

			if (clientHostName)
			{
				for (short i = 0; i < 256; i++)
					(** tmpHandle).clientHostName [i] = 0;

				LString::CopyPStr (clientHostName, (** tmpHandle).clientHostName,
					clientHostName [0] + 1);
			}

			ChangedResource ((Handle) tmpHandle);
		}
			
		mPreferencesFile -> CloseResourceFork ();
	}
	catch (...)
	{
	}

	return returnedValue;
}

// ---------------------------------------------------------------------------
//		¥ resetDelays
// ---------------------------------------------------------------------------
void CPrefs::resetDelays (void)
{
	GetDateTime (& mStartTime);
}

// ---------------------------------------------------------------------------
//		¥ getLoginPrompt
// ---------------------------------------------------------------------------
char * CPrefs::getLoginPrompt (char * buf)
{
	if (notRegistered())
	{
		if (mNbLogin > 40)
		{
			short i;
			char bell [2] = { 7, 0 };

			strcpy (buf, "Please, visit http://order.kagi.com/?QJ4 to register...\n\r");
			for (i = 50; i < 100; i+=4)
				if (mNbLogin > i)
					strcat (buf, bell);
		}
		else if (mNbLogin > 25)
			strcpy (buf, "If you don't register, who will?\n\r");
		else if (mNbLogin > 12)
			strcpy (buf, "Don't forget the ToolDaemon registration!\n\r");
		else
			strcpy (buf, "Unregistred version.\n\r");
	}
	else
		* buf = 0;

	return buf;
}


// ---------------------------------------------------------------------------
//		¥ ~CPrefs
// ---------------------------------------------------------------------------
CPrefs::~CPrefs (void)
{
	WritePrefs ();
	
	delete mPreferencesFile;
}

// LStr255 gAdminPass;

// ---------------------------------------------------------------------------
//		¥ ReadPrefs
// ---------------------------------------------------------------------------
void CPrefs::ReadPrefs (void)
{
	try
	{
		mPreferencesFile -> OpenDataFork (fsRdPerm);
		prefsHandle tmpHandle = (prefsHandle) mPreferencesFile -> ReadDataFork ();
		
		if (tmpHandle)
		{
			if ((** tmpHandle).creator == 'DwFt')
			{
				switch ((** tmpHandle).version)
				{
					case 0x01000000:
					case 0x02500000:
						mNbLogin = (** tmpHandle).nbLogin;
						mNbStartup = (** tmpHandle).nbStartup;
						
						mDontUseDNS = !! ((** tmpHandle).flags & 1);

						mCount = (** tmpHandle).count;
						mPort = (** tmpHandle).port;

						mAdminPass = (** tmpHandle).scrambledAdminPass;
						decode (mAdminPass);
//						gAdminPass = mAdminPass;

						mUserName = (** tmpHandle).userName;
						mUserKey = (** tmpHandle).regKey;
						
						if ((** tmpHandle).version == 0x02500000)
						{
							mLogEnabled = !! ((** tmpHandle).flags & 2);
							mLogDetailed = !! ((** tmpHandle).flags & 4);
						}
						else
						{
							mLogEnabled = true;
							mLogDetailed = false;
						}
						break;

					default:
						break;
				}
			}
			::DisposeHandle ((Handle) tmpHandle);
		}
		mPreferencesFile -> CloseDataFork ();
	}
	
	catch (...) { };
}

// ---------------------------------------------------------------------------
//		¥ WriteLog
// ---------------------------------------------------------------------------
void CPrefs::WriteLog (Boolean isDetailedLogInfo, StringPtr userName,
	StringPtr userAddress, char * action, OSErr theErr)
{
	if (mLogEnabled && * action)
	{
		if ((! isDetailedLogInfo) || mLogDetailed)
		{
			SInt32	bytesWritten;
			LStr255 logBuffer;
			Str32 dateBuffer;
			Str32 timeBuffer;
			unsigned long currentDate;
			
			GetDateTime (& currentDate);
			DateString (currentDate, false, dateBuffer, nil);
			TimeString (currentDate, true, timeBuffer, nil);

			logBuffer = dateBuffer;
			logBuffer += "\t";
			logBuffer += timeBuffer;
			logBuffer += "\t";
			logBuffer += userName;
			logBuffer += "\t";
			logBuffer += userAddress;
			logBuffer += "\t";
			if (isDetailedLogInfo)
				logBuffer += "\t";
			logBuffer += action;
			if (theErr)
			{
				logBuffer += " (";
				logBuffer += theErr;
				logBuffer += ")";
			}
			logBuffer += "\r";

			mLogFile -> OpenDataFork (fsWrPerm);

			OSErr err = ::SetFPos(mLogFile -> GetDataForkRefNum (), fsFromLEOF, 0);
			ThrowIfOSErr_(err);
			
			bytesWritten = logBuffer.Length ();
			err = ::FSWrite (mLogFile -> GetDataForkRefNum (), & bytesWritten,
				logBuffer.TextPtr ());
			ThrowIfOSErr_(err);
			
			mLogFile -> CloseDataFork ();
		}
	}
}

// ---------------------------------------------------------------------------
//		¥ WritePrefs
// ---------------------------------------------------------------------------
void CPrefs::WritePrefs (void)
{
	prefsRec defaultPrefsRec;

	defaultPrefsRec.creator = 'DwFt';
	defaultPrefsRec.version = 0x02500000;
	GetDateTime (& defaultPrefsRec.date);

	defaultPrefsRec.nbLogin = mNbLogin;
	defaultPrefsRec.nbStartup = mNbStartup;
	defaultPrefsRec.flags =
		(mDontUseDNS ? 1 : 0) |
		(mLogEnabled ? 2 : 0) |
		(mLogDetailed ? 4 : 0);
	defaultPrefsRec.count = mCount;
	defaultPrefsRec.port = mPort;
	
	encode (mAdminPass);
	
	LString::CopyPStr (mAdminPass, defaultPrefsRec.scrambledAdminPass,
		mAdminPass.Length () + 1);

	decode (mAdminPass);
//	gAdminPass = mAdminPass;

	LString::CopyPStr (mUserName, defaultPrefsRec.userName,
		mUserName.Length () + 1);
	LString::CopyPStr (mUserKey, defaultPrefsRec.regKey,
		mUserKey.Length () + 1);

	mPreferencesFile -> OpenDataFork (fsWrPerm);
	mPreferencesFile -> WriteDataFork (& defaultPrefsRec, sizeof (defaultPrefsRec));
	mPreferencesFile -> CloseDataFork ();
}

// ---------------------------------------------------------------------------
//		¥ notRegistered
// ---------------------------------------------------------------------------
Boolean
CPrefs::notRegistered(void)
{
	Str255 decoded;
	
	if (mUserName [0] >= 4)
	{
		
		outBase64 (& decoded [1], mUserKey);
		decoded [0] = strlen ((char *) & decoded [1]);
		decode (decoded);
		if (decoded [0] > 3)
			decoded [0] -= 3;

		return ! EqualString (mUserName, decoded, false, false);
	}
	else
		return true;
}

// ---------------------------------------------------------------------------
//		¥ DoPreferences
// ---------------------------------------------------------------------------
void
CPrefs::DoPreferences(Boolean cantCancel, LCommander * theCommander)
{
	StDialogHandler prefBox(PPob_ServerSettings, theCommander);
	MessageT hitMessage;
	LWindow * theDialog = prefBox.GetDialog();
	
	// Set old values
	LEditText* countField = dynamic_cast<LEditText*>
								(theDialog->FindPaneByID(ServerSettings_EditCount));
	ThrowIfNil_ (countField);
	countField->SetValue(mCount);

	LEditText* portField = dynamic_cast<LEditText*>
								(theDialog->FindPaneByID(ServerSettings_EditPort));
	ThrowIfNil_ (portField);
	portField->SetValue(mPort);

	LEditText* adminPassField = dynamic_cast<LEditText*>
								(theDialog->FindPaneByID(ServerSettings_EditAdminPass));
	ThrowIfNil_ (adminPassField);
	adminPassField->SetDescriptor(mAdminPass);

	LEditText* userNameField = dynamic_cast<LEditText*>
								(theDialog->FindPaneByID(ServerSettings_EditUserName));
	ThrowIfNil_ (userNameField);
	userNameField->SetDescriptor(mUserName);

	LEditText* userKeyField = dynamic_cast<LEditText*>
								(theDialog->FindPaneByID(ServerSettings_EditUserKey));
	ThrowIfNil_ (userKeyField);
	userKeyField->SetDescriptor(mUserKey);

	LPushButton* cancelButton = dynamic_cast<LPushButton*>
								(theDialog->FindPaneByID('CNCL'));
	ThrowIfNil_ (cancelButton);
	if (cantCancel)
		cancelButton->Disable (); 

	LCheckBox* dontUseDNSCheckBox = dynamic_cast<LCheckBox*>
								(theDialog->FindPaneByID('noDS'));
	ThrowIfNil_ (dontUseDNSCheckBox);
	dontUseDNSCheckBox->SetValue (mDontUseDNS); 

	LCheckBox* logEnabledCheckBox = dynamic_cast<LCheckBox*>
								(theDialog->FindPaneByID('logE'));
	ThrowIfNil_ (logEnabledCheckBox);
	logEnabledCheckBox->SetValue (mLogEnabled); 

	LCheckBox* logDetailedCheckBox = dynamic_cast<LCheckBox*>
								(theDialog->FindPaneByID('logD'));
	ThrowIfNil_ (logDetailedCheckBox);
	logDetailedCheckBox->SetValue (mLogDetailed);
	
	if (! mLogEnabled)
		logDetailedCheckBox->Disable ();

	if (notRegistered ())
	{
		LStaticText* theThanks = dynamic_cast<LStaticText*>
			(theDialog->FindPaneByID(ServerSettings_ThanksStatText));
		ThrowIfNil_ (theThanks);

		theThanks->Hide ();
	}
	else
	{
		LPictureControl* theRegistrationRequest = dynamic_cast<LPictureControl*>
			(theDialog->FindPaneByID(ServerSettings_RegisterPict));
		ThrowIfNil_ (theRegistrationRequest);

		LStaticText* theKeyRequest = dynamic_cast<LStaticText*>
			(theDialog->FindPaneByID(ServerSettings_StatKey));
		ThrowIfNil_ (theKeyRequest);

		theRegistrationRequest->Hide ();
		theKeyRequest->Hide ();
		userKeyField->Hide ();
	}

	theDialog->Show();
	
	while (true)
	{
		hitMessage = prefBox.DoDialog();
		
		if (1000 == hitMessage)
			checkICLaunchURL ("\phttp://order.kagi.com/?QJ4");

		if (1001 == hitMessage)
		{
			if (logEnabledCheckBox->GetValue ())
				logDetailedCheckBox->Enable ();
			else
				logDetailedCheckBox->Disable ();
		}

		if (msg_OK == hitMessage || msg_Cancel == hitMessage)
			break;
	}
	
	// if we hit ok, save the pref info
	if (msg_OK == hitMessage)
	{
		mCount = countField->GetValue ();
		if (mCount < 1)
			mCount = 1;

		mDontUseDNS = dontUseDNSCheckBox->GetValue ();
		mLogEnabled = logEnabledCheckBox->GetValue ();
		mLogDetailed = logDetailedCheckBox->GetValue ();

		mPort = portField->GetValue ();

		adminPassField->GetDescriptor (mAdminPass);
//		gAdminPass = mAdminPass;

		userNameField->GetDescriptor (mUserName);
		userKeyField->GetDescriptor (mUserKey);
	}
}
