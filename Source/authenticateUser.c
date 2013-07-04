#include <Folders.h>

#include "UGLibrary.h"
#include "UGLibErrors.h"
#include "authenticateUser.h"

OSErr authenticateUser (StringPtr enteredUserNamePtr, StringPtr passwordPtr,
	StringPtr administratorKeyPtr, StringPtr foundUserNamePtr)
{
	OSErr theOSErr, tmpOSErr;
	short foundVRefNum;
	long foundDirID;

	foundUserNamePtr [0] = 0;

	theOSErr = FindFolder (kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder,
		& foundVRefNum, & foundDirID);
	if (theOSErr == noErr)
	{
		UGParamBlockRec * theUserGroupPBPtr;
		
		theUserGroupPBPtr = (UGParamBlockRec *) NewPtrClear (sizeof (UGParamBlockRec));
		if (theUserGroupPBPtr != NULL)
		{
			Str31 defaultNamePtr;

			defaultNamePtr [0] = 0;
			
			theUserGroupPBPtr -> ugFilePB.ugNamePtr = defaultNamePtr;
			theUserGroupPBPtr -> ugFilePB.ugVRefNum = foundVRefNum;
			theUserGroupPBPtr -> ugFilePB.ugDirID = foundDirID;
			theUserGroupPBPtr -> ugFilePB.ugAdminKey = administratorKeyPtr;

			theOSErr = UGOpenFile (theUserGroupPBPtr, false);
			if (theOSErr == noErr)
			{
				theUserGroupPBPtr -> ugUserPB.ugNamePtr = enteredUserNamePtr;
				theUserGroupPBPtr -> ugUserPB.ugUserID = 0;
				theUserGroupPBPtr -> ugUserPB.ugPassword = passwordPtr;

				theOSErr = UGAuthenticateUser (theUserGroupPBPtr, false);

/* maybe, diacritical signs are missing : try to find a similar user */
				if (theOSErr == ugUserNotFound)
				{
					foundUserNamePtr [0] = 0;

					theUserGroupPBPtr -> ugUserPB.ugNamePtr = foundUserNamePtr;
					theUserGroupPBPtr -> ugUserPB.ugUserID = 0;
					theUserGroupPBPtr -> ugUserPB.ugGroupID = 0;
					theUserGroupPBPtr -> ugUserPB.ugPosOffset = 0;
					theOSErr = UGGetUserInfo (theUserGroupPBPtr, false);
					while (theOSErr == noErr)
					{
						if (IdenticalString (enteredUserNamePtr, foundUserNamePtr, NULL) == 0)
						{
							theUserGroupPBPtr -> ugUserPB.ugUserID = 0;
							theUserGroupPBPtr -> ugUserPB.ugPassword = passwordPtr;
							
							theOSErr = UGAuthenticateUser (theUserGroupPBPtr, false);
							break;
						}
						else
						{
							theUserGroupPBPtr -> ugUserPB.ugPosOffset = 1;
							theOSErr = UGGetUserInfo (theUserGroupPBPtr, false);
						}
					}
				}
				else
				{
					BlockMoveData (enteredUserNamePtr, foundUserNamePtr,
						enteredUserNamePtr [0] + 1);
//					foundUserNamePtr [0] = 0;

//					theUserGroupPBPtr -> ugUserPB.ugNamePtr = foundUserNamePtr;
//					theUserGroupPBPtr -> ugUserPB.ugGroupID = 0;
//					theUserGroupPBPtr -> ugUserPB.ugPosOffset = 0;
//					tmpOSErr = UGGetUserInfo (theUserGroupPBPtr, false);
				}

				tmpOSErr = UGCloseFile (theUserGroupPBPtr, false);
			}

			DisposePtr ((Ptr) theUserGroupPBPtr);
		}
	}
	
	if (theOSErr)
		foundUserNamePtr [0] = 0;

	return theOSErr;
}
