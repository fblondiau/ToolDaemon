// =================================================================================
//	AppConstants.h				©1995-1999 Metrowerks Inc. All rights reserved.
// =================================================================================

#ifndef _H_AppConstants
#define _H_AppConstants
#pragma once

#include <PP_Types.h>

	// Used in CNetServerApp
	
const ResIDT	ALRT_NoThreadManager		= 1000;
const ResIDT	PPob_ServerSettings			= 128;
const PaneIDT	ServerSettings_EditCount	= FOUR_CHAR_CODE('NUML');
const PaneIDT	ServerSettings_EditPort		= FOUR_CHAR_CODE('PORT');
const PaneIDT	ServerSettings_CheckUDP		= FOUR_CHAR_CODE('_UDP');

	// Used in CTCPResponder
	
const ResIDT	PPob_TCPTerminalWindow	= 129;
const PaneIDT	pTerminal				= FOUR_CHAR_CODE('TERM');

	// Used in CSimpleTCPServer

const ResIDT	PPob_WaitIngWindow	= 131;
const PaneIDT	pPortItemID			= FOUR_CHAR_CODE('PORT');
const PaneIDT	pAddress			= FOUR_CHAR_CODE('ADDR');
const PaneIDT	pMaxConnection		= FOUR_CHAR_CODE('MCON');
const PaneIDT	pCurrConnection		= FOUR_CHAR_CODE('CCON');
const PaneIDT	pRejectedConnection	= FOUR_CHAR_CODE('RCON');

/* FB ( */
const ResIDT	ALRT_NoTCPIP				= 1001;
const PaneIDT	pCurrConnectionBar	= FOUR_CHAR_CODE('CCVM');
/* ) FB */

	// Used in CSimpleUDPServer

const ResIDT	PPob_UDPTerminalWindow	= 130;
//const PaneIDT	pPortItemID				= FOUR_CHAR_CODE('PORT');
//const PaneIDT	pAddress				= FOUR_CHAR_CODE('ADDR');
//const PaneIDT	pTerminal				= FOUR_CHAR_CODE('TERM');


#endif // _H_AppConstants
