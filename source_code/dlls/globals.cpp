/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== globals.cpp ========================================================

  DLL-wide global variable definitions.
  They're all defined here, for convenient centralization.
  Source files that need them should "extern ..." declare each
  variable, to better document what globals they care about.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

DLL_GLOBAL ULONG		g_ulFrameCount;
DLL_GLOBAL ULONG		g_ulModelIndexEyes;
DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
DLL_GLOBAL Vector		g_vecAttackDir;
DLL_GLOBAL int		g_iSkillLevel;
DLL_GLOBAL int		gDisplayTitle;
DLL_GLOBAL int		g_iWorldType;
DLL_GLOBAL BOOL		g_fGameOver;
DLL_GLOBAL BOOL		g_fXashEngine;
DLL_GLOBAL const Vector	g_vecZero = Vector(0,0,0);
DLL_GLOBAL const Vector	g_bonusColor = Vector( 215, 186, 69 );
DLL_GLOBAL int		g_levelParams[10];	// changelevel params
DLL_GLOBAL BOOL		g_changelevel = FALSE;
DLL_GLOBAL int		g_intermission_running;
#ifdef HIPNOTIC
DLL_GLOBAL int		g_intermission_sequence;
#endif /* HIPNOTIC */
DLL_GLOBAL float		g_intermission_exittime;
#ifdef HIPNOTIC
DLL_GLOBAL float		g_intermission_seqtime;
#endif /* HIPNOTIC */
DLL_GLOBAL char		g_sNextMap[64];
DLL_GLOBAL BOOL		g_registered = FALSE;
#ifndef HIPNOTIC
DLL_GLOBAL int		g_iXashEngineBuildNumber;
#else /* HIPNOTIC */
DLL_GLOBAL int		g_iXashEngineBuildNumber;
DLL_GLOBAL BOOL		g_fHornActive = FALSE;
DLL_GLOBAL BOOL		g_fDischarged = FALSE;
DLL_GLOBAL BOOL		g_fEmpathyUsed = FALSE;
DLL_GLOBAL CBaseEntity	*g_pHornCharmer = NULL;
DLL_GLOBAL CBaseEntity	*g_pPlayerDecoy = NULL;
#endif /* HIPNOTIC */
