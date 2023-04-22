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
//
//  quakedef.h

// this file is included by both the game-dll and the client-dll,

#ifndef CDLL_DLL_H
#define CDLL_DLL_H

#define HIDEHUD_HUD			( 1<<0 )
#define HIDEHUD_ALL			( 1<<1 )

#define HUD_PRINTNOTIFY		1
#define HUD_PRINTCONSOLE		2
#define HUD_PRINTTALK		3
#define HUD_PRINTCENTER		4

// Quake custom messages for gmsgTempEntity
#define TE_SPIKE			0
#define TE_SUPERSPIKE		1
// TE_GUNSHOT already defined
// TE_EXPLOSION already defined
// TE_TAREXPLOSION already defined
#define TE_LIGHTNING1		5
#define TE_LIGHTNING2		6
#define TE_WIZSPIKE			7
#define TE_KNIGHTSPIKE		8
#define TE_LIGHTNING3		9
// TE_LAVASPLASH already defined
// TE_TELEPORT already defined
// TE_EXPLOSION2 already defined

//
// Quake items
//
// weapons
#define IT_AXE			(1<<12)
#define IT_SHOTGUN			(1<<0)
#define IT_SUPER_SHOTGUN		(1<<1)
#define IT_NAILGUN			(1<<2)
#define IT_SUPER_NAILGUN		(1<<3)
#define IT_GRENADE_LAUNCHER		(1<<4)
#define IT_ROCKET_LAUNCHER		(1<<5)
#define IT_LIGHTNING		(1<<6)
#define IT_EXTRA_WEAPON		(1<<7)

// ammo
#define IT_SHELLS			(1<<8)
#define IT_NAILS			(1<<9)
#define IT_ROCKETS			(1<<10)
#define IT_CELLS			(1<<11)

// armor
#define IT_ARMOR1			(1<<13)
#define IT_ARMOR2			(1<<14)
#define IT_ARMOR3			(1<<15)
#define IT_SUPERHEALTH		(1<<16)

// keys
#define IT_KEY1			(1<<17)
#define IT_KEY2			(1<<18)

// artifacts
#define IT_INVISIBILITY		(1<<19)
#define IT_INVULNERABILITY		(1<<20)
#define IT_SUIT			(1<<21)
#define IT_QUAD			(1<<22)

//
// Quake stats are integers communicated to the client by the server
//
#define	STAT_HEALTH		0
#define	STAT_FRAGS		1
#define	STAT_WEAPON		2
#define	STAT_AMMO			3
#define	STAT_ARMOR		4
#define	STAT_WEAPONFRAME		5
#define	STAT_SHELLS		6
#define	STAT_NAILS		7
#define	STAT_ROCKETS		8
#define	STAT_CELLS		9
#define	STAT_ACTIVEWEAPON		10
#define	STAT_TOTALSECRETS		11
#define	STAT_TOTALMONSTERS		12
#define	STAT_SECRETS		13		// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS		14		// bumped by svc_killedmonster
#define	MAX_STATS			32

#endif// CDLL_DLL_H