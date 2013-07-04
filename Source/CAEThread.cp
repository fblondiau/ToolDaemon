// ===========================================================================
//	CAEThread.cp		©1999 DouWère
// ===========================================================================
//	This is the threaded version of the AE replyer process.

#include <LSharedQueue.h>

#include <cstring>

#include "CAEThread.h"

// ---------------------------------------------------------------------------
//	• CAEThread								[public]
// ---------------------------------------------------------------------------
//	Constructor

CAEThread::CAEThread(
				LTCPEndpoint *		inNetworkEndpoint,
				LSharedQueue *		inQueue)
	: LThread(	false,
				thread_DefaultStack,
				LThread::threadOption_Default,
				nil),
	mEndpoint(inNetworkEndpoint),
	mQueue(inQueue)
{
}


// ---------------------------------------------------------------------------
//	• ~CAEThread								[public, virtual]
// ---------------------------------------------------------------------------
//	Destructor

CAEThread::~CAEThread()
{
}


// ---------------------------------------------------------------------------
//	• Run												[protected, virtual]
// ---------------------------------------------------------------------------

void*
CAEThread::Run()
{
	LLink * data;
	while (true)
	{
//		try
//		{
			data = mQueue -> Next ();
			CAELLink * AEdata = dynamic_cast<CAELLink *>(data);
			if (AEdata)
			{
				mEndpoint -> Send (* (AEdata -> GetTextHandle ()),
					GetHandleSize (AEdata -> GetTextHandle ()));
			}

			DisposeHandle (AEdata -> GetTextHandle ());
			delete data;
//		}
//		catch (...) { };
	}
}

