// ===========================================================================
//	CSetWithID.cp		©1999 DouWre
// ===========================================================================
//	This is the threaded version of the AE replyer process.

#include "CSetWithID.h"

long			CSetWithID::sLastID = 0;
CSetWithID * 	CSetWithID::sHead = nil;

// ---------------------------------------------------------------------------
//	¥ CSetWithID
// ---------------------------------------------------------------------------
//	Default Constructor

CSetWithID::CSetWithID()
{
	// initialise member variables to known values
	mID	= 0;
	mData	= NULL;
	mNext	= NULL;
}

// ---------------------------------------------------------------------------
//	¥ ~CSetWithID
// ---------------------------------------------------------------------------
//	Destructor  -- just a placeholder, really

CSetWithID::~CSetWithID()
{
	// there's nothing to do
}


// ---------------------------------------------------------------------------
//	¥ AddItem									[public, virtual]
// ---------------------------------------------------------------------------

long CSetWithID::AddItem(LSharedQueue * data)
{
	sLastID ++;

	mID = sLastID;
	mData = data;
	mNext = sHead;

	sHead = this;

	return sLastID;
}

// ---------------------------------------------------------------------------
//	¥ GetItem									[public, virtual]
// ---------------------------------------------------------------------------

LSharedQueue * CSetWithID::GetData(long ID)
{
	CSetWithID * next;

	next = sHead;
	while (next)
	{
		if (next -> mID == ID)
			return next -> mData;
		next = next -> mNext;
	}

	return nil;
}

// ---------------------------------------------------------------------------
//	¥ RemoveItem								[public, virtual]
// ---------------------------------------------------------------------------

void CSetWithID::GetAndRemoveItem(long ID)
{
	CSetWithID * toKill = nil;

	if (sHead)
	{
		if (sHead -> mID == ID)
		{
			toKill = sHead;
			sHead = toKill -> mNext;
		}
		else
		{
			CSetWithID * next = sHead;
			while (next && next -> mNext)
			{
				if (next -> mNext -> mID == ID)
				{
					toKill = next -> mNext;
					next -> mNext =  toKill -> mNext;
				}
				next = next -> mNext;
			}
		}
	}

	if (toKill)
		delete toKill;
}

