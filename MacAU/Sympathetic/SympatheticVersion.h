/*
 *	File:		SympatheticVersion.h
 *
 *	Version:	1.0
 *
 *	Copyright:  Copyright (c) 2026 Panaudio, Panaudio uses the MIT license
 */

#ifndef __SympatheticVersion_h__
#define __SympatheticVersion_h__

#ifdef DEBUG
	#define kSympatheticVersion 0xFFFFFFFF
#else
	#define kSympatheticVersion 0x00010000
#endif

//~~~~~~~~~~~~~~  Change!!! ~~~~~~~~~~~~~~~~~~~~~//
#define Sympathetic_COMP_MANF		'Pana'
#define Sympathetic_COMP_SUBTYPE	'ssym'
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#endif
