#ifndef __ERRORS__
#include <Errors.h>
#endif

#include "endecode.h"

OSErr encode (StringPtr s)
{
	OSErr theOSErr;

	if (s)
	{
		short i;
		short lastBit;

		lastBit = 0;
		for (i = 1; i <= s [0]; i++)
		{
			short j;
			char origin, destination;

			origin = s [i];
			destination = 0;
			for (j = 128; j; j >>= 1)
			{
				short newBit;

				newBit = !! (origin & j);
				if (newBit != lastBit)
					destination += j;
				lastBit = newBit;
			}
			s [i] = destination;
		}
		theOSErr = noErr;
	}
	else
		theOSErr = paramErr;
	
	return theOSErr;
}

OSErr decode (StringPtr s)
{
	OSErr theOSErr;

	if (s)
	{
		short i;
		short currentBit;

		currentBit = 0;
		for (i = 1; i <= s [0]; i++)
		{
			short j;
			char origin, destination;

			origin = s [i];
			destination = 0;
			for (j = 128; j; j >>= 1)
			{
				if (origin & j)
					currentBit = ! currentBit;
				if (currentBit)
					destination = destination + j;
			}
			s [i] = destination;
		}
		theOSErr = noErr;
	}
	else
		theOSErr = paramErr;
	
	return theOSErr;
}

/*****/
