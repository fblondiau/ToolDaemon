// ===========================================================================
//	FinalPrefix.h		©1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

	// Bring in the project's precompiled header for the given target
	
#if __POWERPC__
	#include "FinalPrefixHeadersPPC++"		
#else
	#include "FinalPrefixHeaders68K++"
#endif

	// Needed for dnr.c to compile
#ifndef true
	#define true 1
#endif

#ifndef false
	#define false 0
#endif
