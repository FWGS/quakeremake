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
#ifndef ITEMS_H
#define ITEMS_H

class CItem : public CBaseEntity
{
public:
	void EXPORT 	PlaceItem( void );
	void		StartItem( void );
	CBaseEntity*	Respawn( void );
	void		SetObjectCollisionBox( void );
	void	EXPORT ItemTouch( CBaseEntity *pOther );
	void	EXPORT Materialize( void );
	virtual BOOL MyTouch( CBasePlayer *pPlayer ) { return FALSE; };
};

//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo. 
//=========================================================
class CWeaponBox : public CBaseEntity
{
public:
	void Precache( void );
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	static CWeaponBox *DropBackpack( CBaseEntity *pVictim, int weapon );
	void SetObjectCollisionBox( void );
};

#endif // ITEMS_H
