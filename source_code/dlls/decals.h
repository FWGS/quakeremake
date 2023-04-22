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
#ifndef DECALS_H
#define DECALS_H

//
// Dynamic Decals
//
enum decal_e 
{	
	DECAL_GUNSHOT1 = 0, 
	DECAL_GUNSHOT2,
	DECAL_GUNSHOT3,
	DECAL_GUNSHOT4,
	DECAL_GUNSHOT5,
	DECAL_SCORCH1,
	DECAL_SCORCH2,
	DECAL_BLOOD1, 
	DECAL_BLOOD2, 
	DECAL_BLOOD3, 
	DECAL_BLOOD4, 
	DECAL_BLOOD5, 
	DECAL_BLOOD6, 
	DECAL_YBLOOD1, 
	DECAL_YBLOOD2, 
	DECAL_YBLOOD3, 
	DECAL_YBLOOD4, 
	DECAL_YBLOOD5, 
	DECAL_YBLOOD6, 
	DECAL_SPIT1,
	DECAL_SPIT2,
};

typedef struct 
{
	char	*name;
	int	index;
} DLL_DECALLIST;

extern DLL_DECALLIST gDecals[];

#endif	// DECALS_H
