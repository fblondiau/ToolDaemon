#include <string.h>

#include "Base64.h"

char inBaseAlphabet [64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
	'w', 'x', 'y', 'z', '0', '1', '2', '3', 
	'4', '5', '6', '7', '8', '9', '+', '/'
};

char outBaseAlphabet [256] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12 ,13 ,14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 ,40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

unsigned char * inBase64 (unsigned char * encoded, const unsigned char * s)
{
	unsigned char * encodedPos = encoded;
	const unsigned char * sPos = s;
	unsigned int pos = 2;
	unsigned int rest;

	while (* sPos)
	{
		switch (pos --)
		{
			case 2:
				* encodedPos ++ = inBaseAlphabet [* sPos >> 2];
				rest = (* sPos++ & 0x03) << 4;
				break;
			case 1:
				* encodedPos ++ = inBaseAlphabet [rest + (* sPos >> 4)];
				rest = (* sPos++ & 0x0F) << 2;
				break;
			case 0:
				* encodedPos ++ = inBaseAlphabet [rest + (* sPos >> 6)];
				* encodedPos ++ = inBaseAlphabet [* sPos++ & 0x3F];
				pos = 2;
				break;
		}
	}

	switch (pos)
	{
		case 1:
			* encodedPos ++ = inBaseAlphabet [rest];
			* encodedPos ++ = '=';
			* encodedPos ++ = '=';
			break;
		case 0:
			* encodedPos ++ = inBaseAlphabet [rest];
			* encodedPos ++ = '=';
			break;
	}

	* encodedPos = 0;

	return encoded;
}

unsigned char * outBase64 (unsigned char * decoded, const unsigned char * s)
{
	unsigned char * decodedPos = decoded;
	const unsigned char * sPos = s;
	unsigned int pos = 3;

	while ((* sPos) && (* sPos != '='))
	{
		char tmpChar;
		
		tmpChar = outBaseAlphabet [* sPos ++];
		if (tmpChar != -1)
		{
			switch (pos --)
			{
				case 3:
					* decodedPos = tmpChar << 2;
					break;
				case 2:
					* decodedPos ++ += tmpChar >> 4;
					* decodedPos = (tmpChar & 0x0F) << 4;
					break;
				case 1:
					* decodedPos ++ += tmpChar >> 2;
					* decodedPos = (tmpChar & 0x03) << 6;
					break;
				case 0:
					* decodedPos ++ += tmpChar;
					pos = 3;
					break;
			}
		}
	}

	* decodedPos = 0;

	return decoded;
}
