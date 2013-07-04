// ===========================================================================
//	CAEThread.h		©1995-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CAEThread
#define _H_CAEThread
#pragma once

#include <LThread.h>
#include <LTCPEndpoint.h>
#include <LSharedQueue.h>
#include <LLink.h>

class CAELLink : public LLink
{
public:
	CAELLink (Handle inTextHandle) : LLink ()
	{
		mTextHandle = inTextHandle;
	}
	Handle	GetTextHandle()		{ return mTextHandle; }
protected:
	Handle	mTextHandle;
};

class CAEThread : public LThread
{
public:
									CAEThread(
											LTCPEndpoint *		inNetworkEndpoint,
											LSharedQueue *		inQueue);
	virtual							~CAEThread();

protected:
	virtual void*					Run();

			LSharedQueue *			mQueue;
			LTCPEndpoint *			mEndpoint;
};

#endif // _H_CAEThread
