// ===========================================================================
//	CAEThread.h		©1995-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CSetWithID
#define _H_CSetWithID
#pragma once

#include <LThread.h>
#include <LTCPEndpoint.h>
#include <LSharedQueue.h>

class CSetWithID
{
public:
			CSetWithID ();
virtual			~CSetWithID ();

			long					AddItem (LSharedQueue *);
static		LSharedQueue *			GetData (long ID);
static		void					GetAndRemoveItem (long ID);
protected:
			static	long			sLastID;
			static CSetWithID * 	sHead;

			long					mID;
			LSharedQueue *			mData;
			CSetWithID *			mNext;
};

#endif // _H_CSetWithID
