#include "IC.h"

#include <InternetConfig.h>

static ICInstance inst;
static unsigned short ICStarted = 0;
static Boolean prefsFound = false;

Boolean checkICStart (void)
{
	if (ICStarted == 0)
	{
		if (ICStart (& inst, '????') == noErr)
		{
			prefsFound = (ICGeneralFindConfigFile (inst, true, false, 0, nil) == noErr);

			ICStarted ++;
		}
	}
	else
		ICStarted ++;

	return ICStarted != 0;
}

Boolean checkICStop (void)
{
	ICStarted --;

	if (ICStarted == 0)
		ICStop (inst);

	return ICStarted == 0;
}

Boolean canUseIC (void)
{
	return ICStarted && prefsFound;
}

void checkICLaunchURL (StringPtr URL)
{
	if (checkICStart ())
	{
		long start, end;

		start = 0;
		end = URL [0];

		ICLaunchURL (inst, "\p", (char *) & URL [1], end, & start, & end);
		
		checkICStop ();
	}
}

StringPtr checkICGetKey (StringPtr result, ConstStr255Param key)
{
	if (result)
	{
		if (checkICStart ())
		{
			long size;
			ICAttr attr;

			size = 255;
			if (ICGetPref (inst, key, & attr, (Ptr) result, & size) == noErr)
				result [0] = size - 1;
			else
				result [0] = 0;

			checkICStop ();
		}
		else
			result [0] = 0;
	}

	return result;
}
