// ===========================================================================
//	LURL.h						                     © 1998-1999 Donald E. Carlile.
// ===========================================================================
//  Based heavily on
// ===========================================================================
//	LURL.h						PowerPlant 1.9.3	©1997-1998 Metrowerks Inc.
// ===========================================================================
//	Class for parsing and creating URL's.
//  I have modified it to  reflect the capitalization convention for URLS, to
//  create file URLs correctly, and to be a dunamic class, with the URL reflecting
//  the current settings for host, path, etc.

#ifndef _H_LURL
#define _H_LURL
#pragma once

/* FB ( */
#include <cctype>
/* ) FB */
#include <cstring>
#include <string>
#include <LFTPConnection.h>

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

#define kURLFTPType 		"ftp"
#define kURLMAILTOType		"mailto"
#define kURLHTTPType		"http"
#define kURLFILEType		"file"

//Specific URL Types supported by this object. Others
//	will be parsed into scheme and schemepart only
enum EURLType {
	url_Unknown = 0,
	url_FTP,
	url_MAILTO,
	url_HTTP,
	url_FILE
};

typedef enum EncodingMask {
    url_Aplha			= 0x1,
    url_PAlpha			= 0x2,
    url_Path            = 0x4
} EncodingMask;

class	LURL {
public:
							LURL();
							LURL(const char * inURL);
							LURL(const char * inURL, SInt32 inLength);
							LURL(EURLType inURLType, const char * inSchemePart);
							~LURL();

	virtual void			SetURL(const char * inURL, SInt32 inLength = 0);
	virtual const char *	GetURL();

	virtual void			SetScheme(const char * inURL);
	virtual void			SetScheme(EURLType inURLType);
	virtual const char *	GetScheme();
	virtual EURLType		GetSchemeType();

	virtual void			SetSchemePart(const char * inURL);
	virtual const char *	GetSchemePart();

	virtual void			SetHost(const char * inHost);
	virtual const char *	GetHost()
								{ return mHost.c_str(); }

	virtual void			SetPort(SInt16 inPort)
								{ mPort = inPort; BuildURL(); }
	virtual SInt16			GetPort()
								{ return mPort; }

	//Protocol Specific
	virtual void			SetUserName(const char * inUsername)
								{ mUsername = inUsername; BuildURL(); }
	virtual const char *	GetUserName()
								{ return mUsername.c_str(); }

	virtual void			SetPassword(const char * inPassword);
	virtual const char *	GetPassword()
								{ return mPassword.c_str(); }

	virtual void			SetPath(const char * inPath)
								{ mPath = inPath; BuildURL();}
	virtual const char *	GetPath()
								{ return mPath.c_str(); }
								
	virtual void			SetSearchPart(const char * inSearch)
								{ mHTTPSearchPart = inSearch; }
	virtual const char *	GetSearchPart()
								{ return mHTTPSearchPart.c_str(); }

	virtual void			SetFTPTypeCode(EFTPTransferType inTypeCode)
								{ mFTPTypeCode = inTypeCode; }

	virtual EFTPTransferType	GetFTPTypeCode()
								{ return mFTPTypeCode; }
protected:
	
	virtual void			InitMembers();
	virtual void			ResetMembers();
	virtual void			BuildURL();
	virtual void			DecodeReservedChars(char * tempURL);
	virtual void			EncodeReservedChars (const char * str, 
												PP_STD::string & outEncoded,
												EncodingMask mask = url_Path);
	inline char				ConvertHex(char inChar)
							{
							    return  inChar >= '0' && inChar <= '9' ?  inChar - '0' 
							    	    : inChar >= 'A' && inChar <= 'F'? inChar - 'A' + 10
							    	    : inChar - 'a' + 10;
							}
	
	PP_STD::string			mURL;
	PP_STD::string			mScheme;
	PP_STD::string			mSchemePart;
	
	//Protocol Specific
	EURLType 				mSchemeType;
	PP_STD::string			mUsername;
	PP_STD::string			mPassword;
	Boolean					mHavePassword;
	PP_STD::string			mHost;
	SInt16 					mPort;
	PP_STD::string			mPath;
	EFTPTransferType		mFTPTypeCode;
	PP_STD::string			mHTTPSearchPart;
};


#if PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif
