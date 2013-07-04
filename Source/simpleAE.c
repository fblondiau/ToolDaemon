#include "simpleAE.h"

#define kToolServerSignature 'MPSX'
#define kApplicationType 'APPL'

OSErr sendDoScript (ProcessSerialNumber * process, char * scriptBuffer,
	unsigned long scriptLen, long transactionID)
{
	OSErr theOSErr;
	AEDesc myAddressDesc;

	theOSErr = AECreateDesc (typeProcessSerialNumber, (Ptr) process,
			sizeof (ProcessSerialNumber), & myAddressDesc);
	if (theOSErr == noErr)
	{
		AppleEvent aeEvent;

		if ((scriptLen == 1) && (* scriptBuffer == '.'))
			theOSErr = AECreateAppleEvent ('MPS ', 'abrt', // 'misc','dosc', // 
				& myAddressDesc, kAutoGenerateReturnID, transactionID, & aeEvent);
		else if (scriptLen)
		{
			theOSErr = AECreateAppleEvent ('MPS ', 'scpt', // 'misc','dosc', // 
					& myAddressDesc, kAutoGenerateReturnID, transactionID, & aeEvent);
			if (theOSErr == noErr)
			{
				theOSErr = AEPutParamPtr (& aeEvent, keyDirectObject, typeChar, scriptBuffer,
					scriptLen);
				if (theOSErr != noErr)
					AEDisposeDesc (& aeEvent);
			}
		}
		else
			theOSErr = AECreateAppleEvent ('MPS ', 'stat', // 'misc','dosc', // 
				& myAddressDesc, kAutoGenerateReturnID, transactionID, & aeEvent);

		if (theOSErr == noErr)
		{
			AppleEvent theReply;

			theOSErr = AESend (& aeEvent, & theReply, kAEQueueReply + kAENeverInteract,
				kAENormalPriority, kAEDefaultTimeout, nil, nil);
			AEDisposeDesc (& aeEvent);
		}

		AEDisposeDesc (& myAddressDesc);
	}
	
	return theOSErr;
}
