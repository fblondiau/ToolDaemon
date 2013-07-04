// ===========================================================================
//	PrefixCommon.h		©1999 Metrowerks Inc. All rights reserved.
// ===========================================================================
//	This file contains settings/material common to all targets for the
//	prefix file/precompiled header.


	// Option for using PowerPlant namespace
#define PP_Uses_PowerPlant_Namespace		0	// OFF, don't use PowerPlant namespace

	// Standard Dialogs
#define PP_StdDialogs_Option				PP_StdDialogs_Conditional	// use conditional standard dialog
																		// implementation

	// Force the use of new types
#define PP_Uses_Old_Integer_Types			0	// OFF, use new types

	// Don't allow use of the obsolete AllowTargetSwitch
#define PP_Obsolete_AllowTargetSwitch		0

	// Don't show release notes warnings for old projects
#define PP_Suppress_Notes_20				1

	// Thread Classes
#define THREAD_DEBUG						0	// Off
#define THREAD_PROFILE						0	// Off
#define THREAD_INACTIVITY					0	// Off
#define THREAD_PROTECT_TIMETASK				0	// Off

	// Networking classes
#define USE_MACTCP							1
#define USE_OPENTPT							1

