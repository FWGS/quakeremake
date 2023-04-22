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

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"
#include "shake.h"

extern int gmsgItemPickup;
extern DLL_GLOBAL int		g_iWorldType;
extern DLL_GLOBAL BOOL		g_fXashEngine;

void CItem::StartItem( void )
{
	SetThink( PlaceItem );
	pev->nextthink = gpGlobals->time + 0.2;	// items start after other solids
}

void CItem::PlaceItem( void )
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	pev->origin.z += 6;	// quake code

	SetTouch( ItemTouch );

	if (UTIL_DropToFloor(this) == 0)
	{
		ALERT( at_error, "Item %s fell out of level at %f,%f,%f\n", STRING( pev->classname ), pev->origin.x, pev->origin.y, pev->origin.z );
		UTIL_Remove( this );
		return;
	}

	// g-cont: e3m2 has key on a moving platform. link them together to prevent loosing key
	if( g_fXashEngine && !FNullEnt( pev->groundentity ) && VARS( pev->groundentity )->movetype == MOVETYPE_PUSH )
	{ 
		edict_t	*gnd = pev->groundentity;

		ALERT( at_aiconsole, "%s linked with %s (%s)\n", STRING( pev->classname ),
		STRING( VARS( gnd )->classname ), STRING( VARS( gnd )->targetname )); 
		pev->movetype = MOVETYPE_COMPOUND;	// set movewith type
		pev->aiment = pev->groundentity;	// set parent
	}
}

void CItem::SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + pev->mins;
	pev->absmax = pev->origin + pev->maxs;

	// to make items easier to pick up and allow them to be grabbed off
	// of shelves, the abs sizes are expanded
	pev->absmin.x -= 15;
	pev->absmin.y -= 15;
	pev->absmax.x += 15;
	pev->absmax.y += 15;
}

void CItem::ItemTouch( CBaseEntity *pOther )
{
	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// ok, a player is touching this item, but can he have it?
	if ( !g_pGameRules->CanHaveItem( pPlayer, this ))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch( pPlayer ))
	{
		// health touch sound
		EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, STRING( pev->noise ), 1, ATTN_NORM );

		// send bonus flash (same as command "bf\n")
		BONUS_FLASH( pPlayer->edict() );

		SUB_UseTargets( pOther, USE_TOGGLE, 0 );
		SetTouch( NULL );
		
		// player grabbed the item. 
		g_pGameRules->PlayerGotItem( pPlayer, this );
		if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
		{
			Respawn(); 
		}
		else
		{
			UTIL_Remove( this );
		}
	}
}

CBaseEntity* CItem::Respawn( void )
{
	SetTouch( NULL );
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin( pev, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.

	SetThink ( Materialize );
	pev->nextthink = g_pGameRules->FlItemRespawnTime( this ); 
	return this;
}

void CItem::Materialize( void )
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "items/itembk2.wav", 1, ATTN_NORM );
		pev->effects &= ~EF_NODRAW;
	}

	SetTouch( ItemTouch );
}

#define SF_HEALTH_ROTTEN 	1
#define SF_HEALTH_MEGA	2

/*QUAKED item_health (.3 .3 1) (0 0 0) (32 32 32) rotten megahealth
Health box. Normally gives 25 points.
Rotten box heals 5-10 points,
megahealth will add 100 health, then 
rot you down to your maximum health limit, 
one point per second.
*/
class CItemHealth : public CItem
{
	void Spawn( void )
	{ 
		Precache( );

		if (pev->spawnflags & SF_HEALTH_ROTTEN)
		{
			SET_MODEL(ENT(pev), "models/b_bh10.bsp" );
			pev->noise = MAKE_STRING( "items/r_item1.wav" ); 
			pev->health = 15;
		}
		else if (pev->spawnflags & SF_HEALTH_MEGA)
		{
			SET_MODEL(ENT(pev), "models/b_bh100.bsp" );
			pev->noise = MAKE_STRING( "items/r_item2.wav" ); 
			pev->health = 100;
		}
		else
		{
			SET_MODEL(ENT(pev), "models/b_bh25.bsp" );
			pev->noise = MAKE_STRING( "items/health1.wav" ); 
			pev->health = 25;
		}

		UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 32, 32, 56 ));
		StartItem ();
	}

	void Precache( void )
	{
		if (pev->spawnflags & SF_HEALTH_ROTTEN)
		{
			PRECACHE_MODEL( "models/b_bh10.bsp" );
			PRECACHE_SOUND( "items/r_item1.wav" );
			pev->noise = MAKE_STRING( "items/r_item1.wav" ); 
		}
		else if (pev->spawnflags & SF_HEALTH_MEGA)
		{
			PRECACHE_MODEL( "models/b_bh100.bsp" );
			PRECACHE_SOUND( "items/r_item2.wav" );
			pev->noise = MAKE_STRING( "items/r_item2.wav" ); 
		}
		else
		{
			PRECACHE_MODEL( "models/b_bh25.bsp" );
			PRECACHE_SOUND( "items/health1.wav" );
			pev->noise = MAKE_STRING( "items/health1.wav" ); 
		}
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( FStrEq( STRING( pev->model ), "models/b_bh100.bsp" )) // Megahealth?  Ignore max_health...
		{
			if ( pPlayer->pev->health >= 250 )
				return FALSE;

			// allow to override max_health here!
			if (!pPlayer->TakeHealth( pev->health, DMG_GENERIC, TRUE ))
				return FALSE;

			// has megahealth!
			pPlayer->m_iItems |= IT_SUPERHEALTH;
			pPlayer->m_flCheckHealthTime = gpGlobals->time + 5;
		}
		else
		{
			if (!pPlayer->TakeHealth( pev->health, DMG_GENERIC ))
				return FALSE;
		}

		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "You receive %.f health\n", pev->health ));	
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS( item_health, CItemHealth );

/*QUAKED item_sigil (0 .5 .8) (-16 -16 -24) (16 16 32) E1 E2 E3 E4
End of level sigil, pick up to end episode and return to jrstart.
*/
class CItemSigil : public CItem
{
	void Spawn( void )
	{ 
		Precache( );

		if (pev->spawnflags & 1)
			SET_MODEL(ENT(pev), "models/end1.mdl" );
		else if (pev->spawnflags & 2)
			SET_MODEL(ENT(pev), "models/end2.mdl" );
		else if (pev->spawnflags & 4)
			SET_MODEL(ENT(pev), "models/end3.mdl" );
		else if (pev->spawnflags & 8)
			SET_MODEL(ENT(pev), "models/end4.mdl" );

		pev->noise = MAKE_STRING( "misc/runekey.wav" ); 
		UTIL_SetSize( pev, Vector( -16, -16, -24 ), Vector( 16, 16, 32 ));
		StartItem ();
	}

	void Precache( void )
	{
		PRECACHE_SOUND( "misc/runekey.wav" );

		if (pev->spawnflags & 1)
			PRECACHE_MODEL( "models/end1.mdl" );
		else if (pev->spawnflags & 2)
			PRECACHE_MODEL( "models/end2.mdl" );
		else if (pev->spawnflags & 4)
			PRECACHE_MODEL( "models/end3.mdl" );
		else if (pev->spawnflags & 8)
			PRECACHE_MODEL( "models/end4.mdl" );
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		CenterPrint( pPlayer->pev, "You got the rune!" );
		gpWorld->serverflags |= (pev->spawnflags & 15);
		pPlayer->m_iItems |= (gpWorld->serverflags << 28); // store runes as high bits
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS( item_sigil, CItemSigil );

class CItemKey : public CItem
{
	void Spawn( void )
	{ 
		Precache( );

		BOOL fSilver = FClassnameIs( pev, "item_key1" );

		switch( g_iWorldType )
		{
		case WORLDTYPE_MEDIEVAL:
			pev->noise = MAKE_STRING( "misc/medkey.wav" );
			pev->netname = fSilver ? MAKE_STRING( "silver key" ) : MAKE_STRING( "gold key" );
			break;
		case WORLDTYPE_RUNIC:
			pev->noise = MAKE_STRING( "misc/runekey.wav" );
			pev->netname = fSilver ? MAKE_STRING( "silver runekey" ) : MAKE_STRING( "gold runekey" );
			break;
		case WORLDTYPE_PRESENT:
			pev->noise = MAKE_STRING( "misc/basekey.wav" );
			pev->netname = fSilver ? MAKE_STRING( "silver keycard" ) : MAKE_STRING( "gold keycard" );
			break;
		}

		pev->team = fSilver ? IT_KEY1 : IT_KEY2; 
		SET_MODEL(ENT(pev), STRING(pev->model) );
		UTIL_SetSize( pev, Vector( -16, -16, -24 ), Vector( 16, 16, 32 ));
		StartItem ();
	}

	void Precache( void )
	{
		BOOL fSilver = FClassnameIs( pev, "item_key1" );

		char tmpstring[64];
		char color = fSilver ? 's' : 'g';
		char type = (g_iWorldType == WORLDTYPE_MEDIEVAL) ? 'w' : (g_iWorldType == WORLDTYPE_RUNIC) ? 'm' : 'b';

		sprintf( tmpstring, "models/%c_%c_key.mdl", type, color );

		if( FStringNull( pev->model ))
			pev->model = ALLOC_STRING( tmpstring );

		PRECACHE_MODEL( (char*)STRING( pev->model ));

		switch( g_iWorldType )
		{
		case WORLDTYPE_MEDIEVAL:
			PRECACHE_SOUND( "misc/medkey.wav" );
			pev->noise = MAKE_STRING( "misc/medkey.wav" );
			break;
		case WORLDTYPE_RUNIC:
			PRECACHE_SOUND( "misc/runekey.wav" );
			pev->noise = MAKE_STRING( "misc/runekey.wav" );
			break;
		case WORLDTYPE_PRESENT:
			PRECACHE_SOUND( "misc/basekey.wav" );
			pev->noise = MAKE_STRING( "misc/basekey.wav" );
			break;
		}
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if (pPlayer->m_iItems & pev->team)
			return FALSE;

		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "You got the %s\n", STRING( pev->netname )));
		pPlayer->m_iItems |= pev->team;

		return TRUE;
	}
};

/*QUAKED item_key1 (0 .5 .8) (-16 -16 -24) (16 16 32)
SILVER key
In order for keys to work
you MUST set your maps
worldtype to one of the
following:
0: medieval
1: metal
2: base
*/
LINK_ENTITY_TO_CLASS( item_key1, CItemKey );

/*QUAKED item_key2 (0 .5 .8) (-16 -16 -24) (16 16 32)
GOLD key
In order for keys to work
you MUST set your maps
worldtype to one of the
following:
0: medieval
1: metal
2: base
*/
LINK_ENTITY_TO_CLASS( item_key2, CItemKey );

class CItemArmor : public CItem
{
	void Precache( void )
	{
		PRECACHE_MODEL ("models/armor.mdl");
	}

	void Spawn( void )
	{ 
		Precache( );

		pev->noise = MAKE_STRING( "items/armor1.wav" );
		SET_MODEL(ENT(pev), "models/armor.mdl");

		UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 56 ));

		if( FClassnameIs( pev, "item_armor2" ))
			pev->skin = 1;
		else if( FClassnameIs( pev, "item_armorInv" ))
			pev->skin = 2;

		StartItem ();
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		float	type, value;
		int	bit;

		if (FClassnameIs( pev, "item_armor1" ))
		{
			type = 0.3;
			value = 100;
			bit = IT_ARMOR1;
		}
		if (FClassnameIs( pev, "item_armor2" ))
		{
			type = 0.6;
			value = 150;
			bit = IT_ARMOR2;
		}
		if (FClassnameIs( pev, "item_armorInv" ))
		{
			type = 0.8;
			value = 200;
			bit = IT_ARMOR3;
		}

		if ((pPlayer->pev->armortype * pPlayer->pev->armorvalue) >= (type * value))
			return FALSE;

		CLIENT_PRINTF( pPlayer->edict(), print_console, "You got armor\n" );

		pPlayer->pev->armortype = type;
		pPlayer->pev->armorvalue = value;

		pPlayer->m_iItems &= ~(IT_ARMOR1|IT_ARMOR2|IT_ARMOR3);
		pPlayer->m_iItems |= bit;

		return TRUE;		
	}
};

/*QUAKED item_armor1 (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(item_armor1, CItemArmor);

/*QUAKED item_armor2 (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(item_armor2, CItemArmor);

/*QUAKED item_armorInv (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(item_armorInv, CItemArmor);

class CItemArtifact : public CItem
{
	void Precache( void )
	{
		if (FClassnameIs( pev, "item_artifact_invulnerability" ))
		{
			PRECACHE_MODEL ("models/invulner.mdl");
			PRECACHE_SOUND( "items/protect.wav" );
			PRECACHE_SOUND( "items/protect2.wav" );
			PRECACHE_SOUND( "items/protect3.wav" );
		}
		else if (FClassnameIs( pev, "item_artifact_envirosuit" ))
		{
			PRECACHE_MODEL ("models/suit.mdl");
			PRECACHE_SOUND( "items/suit.wav" );
			PRECACHE_SOUND( "items/suit2.wav" );
		}
		else if (FClassnameIs( pev, "item_artifact_invisibility" ))
		{
			PRECACHE_MODEL ("models/invisibl.mdl");
			PRECACHE_SOUND( "items/inv1.wav" );
			PRECACHE_SOUND( "items/inv2.wav" );
			PRECACHE_SOUND( "items/inv3.wav" );
		}
		else if (FClassnameIs( pev, "item_artifact_super_damage" ))
		{
			PRECACHE_MODEL ("models/quaddama.mdl");
			PRECACHE_SOUND( "items/damage.wav" );
			PRECACHE_SOUND( "items/damage2.wav" );
			PRECACHE_SOUND( "items/damage3.wav" );
		}
	}

	void Spawn( void )
	{ 
		Precache( );

		if (FClassnameIs( pev, "item_artifact_invulnerability" ))
		{
			SET_MODEL(ENT(pev), "models/invulner.mdl");
			pev->noise = MAKE_STRING( "items/protect.wav" );
			pev->netname = MAKE_STRING("Pentagram of Protection");
			pev->team = IT_INVULNERABILITY;
		}
		else if (FClassnameIs( pev, "item_artifact_envirosuit" ))
		{
			SET_MODEL(ENT(pev), "models/suit.mdl");
			pev->noise = MAKE_STRING( "items/suit.wav" );
			pev->netname = MAKE_STRING("Biosuit");
			pev->team = IT_SUIT;
		}
		else if (FClassnameIs( pev, "item_artifact_invisibility" ))
		{
			SET_MODEL(ENT(pev), "models/invisibl.mdl");
			pev->noise = MAKE_STRING( "items/inv1.wav" );
			pev->netname = MAKE_STRING("Ring of Shadows");
			pev->team = IT_INVISIBILITY;
		}
		else if (FClassnameIs( pev, "item_artifact_super_damage" ))
		{
			SET_MODEL(ENT(pev), "models/quaddama.mdl");
			pev->noise = MAKE_STRING( "items/damage.wav" );
			pev->netname = MAKE_STRING("Quad Damage");
			pev->team = IT_QUAD;
		}

		UTIL_SetSize( pev, Vector( -16, -16, -24 ), Vector( 16, 16, 32 ));
		StartItem ();
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		// do the apropriate action
		if (FClassnameIs( pev, "item_artifact_envirosuit" ))
		{
			pPlayer->m_flRadSuitTime = 1;
			pPlayer->m_flRadSuitFinished = gpGlobals->time + 30;
		}
	
		if (FClassnameIs( pev, "item_artifact_invulnerability" ))
		{
			pPlayer->m_flInvincibleTime = 1;
			pPlayer->m_flInvincibleFinished = gpGlobals->time + 30;
		}

		if (FClassnameIs( pev, "item_artifact_invisibility" ))
		{
			pPlayer->m_flInvisibleTime = 1;
			pPlayer->m_flInvisibleFinished = gpGlobals->time + 30;
		}

		if (FClassnameIs( pev, "item_artifact_super_damage" ))
		{
			pPlayer->m_flSuperDamageTime = 1;
			pPlayer->m_flSuperDamageFinished = gpGlobals->time + 30;
		}

		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "You got the %s\n", STRING( pev->netname )));
		pPlayer->m_iItems |= pev->team;

		return TRUE;		
	}
};

/*QUAKED item_artifact_invulnerability (0 .5 .8) (-16 -16 -24) (16 16 32)
Player is invulnerable for 30 seconds
*/
LINK_ENTITY_TO_CLASS(item_artifact_invulnerability, CItemArtifact);

/*QUAKED item_artifact_envirosuit (0 .5 .8) (-16 -16 -24) (16 16 32)
Player takes no damage from water or slime for 30 seconds
*/
LINK_ENTITY_TO_CLASS(item_artifact_envirosuit, CItemArtifact);

/*QUAKED item_artifact_invisibility (0 .5 .8) (-16 -16 -24) (16 16 32)
Player is invisible for 30 seconds
*/
LINK_ENTITY_TO_CLASS(item_artifact_invisibility, CItemArtifact);

/*QUAKED item_artifact_super_damage (0 .5 .8) (-16 -16 -24) (16 16 32)
The next attack from the player will do 4x damage
*/
LINK_ENTITY_TO_CLASS(item_artifact_super_damage, CItemArtifact);

#define SF_WEAPON_BIG2 	1

class CItemAmmo : public CItem
{
	void Precache( void )
	{
		if (FClassnameIs( pev, "item_shells" ))
		{
			if (pev->spawnflags & SF_WEAPON_BIG2)
				PRECACHE_MODEL ("models/b_shell1.bsp");
			else
				PRECACHE_MODEL ("models/b_shell0.bsp"); 
		}
		else if (FClassnameIs( pev, "item_spikes" ))
		{
			if (pev->spawnflags & SF_WEAPON_BIG2)
				PRECACHE_MODEL ("models/b_nail1.bsp");
			else
				PRECACHE_MODEL ("models/b_nail0.bsp"); 
		}
		else if (FClassnameIs( pev, "item_rockets" ))
		{
			if (pev->spawnflags & SF_WEAPON_BIG2)
				PRECACHE_MODEL ("models/b_rock1.bsp");
			else
				PRECACHE_MODEL ("models/b_rock0.bsp"); 
		}
		else if (FClassnameIs( pev, "item_cells" ))
		{
			if (pev->spawnflags & SF_WEAPON_BIG2)
				PRECACHE_MODEL ("models/b_batt1.bsp");
			else
				PRECACHE_MODEL ("models/b_batt0.bsp"); 
		}
	}

	void Spawn( void )
	{ 
		// support for item_weapon
		if (FClassnameIs( pev, "item_weapon" ))
		{
			if (pev->spawnflags & 1)
				pev->classname = MAKE_STRING( "item_shells" );
			if (pev->spawnflags & 2)
				pev->classname = MAKE_STRING( "item_rockets" );
			if (pev->spawnflags & 4)
				pev->classname = MAKE_STRING( "item_spikes" );
			if (pev->spawnflags & 8)
				pev->spawnflags = SF_WEAPON_BIG2;
			else pev->spawnflags = 0;
		}

		Precache( );

		if (FClassnameIs( pev, "item_shells" ))
		{
			if (pev->spawnflags & SF_WEAPON_BIG2)
			{
				SET_MODEL(ENT(pev), "models/b_shell1.bsp");
				pev->armorvalue = 40;
			}
			else
			{
				SET_MODEL(ENT(pev), "models/b_shell0.bsp"); 
				pev->armorvalue = 20;
			}
			pev->netname = MAKE_STRING("shells");
			pev->team = (IT_SHOTGUN|IT_SUPER_SHOTGUN);
		}
		else if (FClassnameIs( pev, "item_spikes" ))
		{
			if (pev->spawnflags & SF_WEAPON_BIG2)
			{
				SET_MODEL(ENT(pev), "models/b_nail1.bsp");
				pev->armorvalue = 50;
			}
			else
			{
				SET_MODEL(ENT(pev), "models/b_nail0.bsp"); 
				pev->armorvalue = 25;
			}
			pev->netname = MAKE_STRING("nails");
			pev->team = (IT_NAILGUN|IT_SUPER_NAILGUN);
		}
		else if (FClassnameIs( pev, "item_rockets" ))
		{
			if (pev->spawnflags & SF_WEAPON_BIG2)
			{
				SET_MODEL(ENT(pev), "models/b_rock1.bsp");
				pev->armorvalue = 10;
			}
			else
			{
				SET_MODEL(ENT(pev), "models/b_rock0.bsp"); 
				pev->armorvalue = 5;
			}
			pev->netname = MAKE_STRING("rockets");
			pev->team = (IT_GRENADE_LAUNCHER|IT_ROCKET_LAUNCHER);
		}
		else if (FClassnameIs( pev, "item_cells" ))
		{
			if (pev->spawnflags & SF_WEAPON_BIG2)
			{
				SET_MODEL(ENT(pev), "models/b_batt1.bsp");
				pev->armorvalue = 12;
			}
			else
			{
				SET_MODEL(ENT(pev), "models/b_batt0.bsp"); 
				pev->armorvalue = 6;
			}
			pev->netname = MAKE_STRING("cells");
			pev->team = IT_LIGHTNING;
		}
		pev->noise = MAKE_STRING( "weapons/lock4.wav" );

		UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 32, 32, 56 ));
		StartItem ();
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		int best = pPlayer->W_BestWeapon();

		// do the apropriate action
		if (pev->team & (IT_SHOTGUN|IT_SUPER_SHOTGUN))
		{
			if (pPlayer->ammo_shells >= 100)
				return FALSE;
			pPlayer->ammo_shells += pev->armorvalue;
		}
	
		if (pev->team & (IT_NAILGUN|IT_SUPER_NAILGUN))
		{
			if (pPlayer->ammo_nails >= 200)
				return FALSE;
			pPlayer->ammo_nails += pev->armorvalue;
		}
		if (pev->team & (IT_GRENADE_LAUNCHER|IT_ROCKET_LAUNCHER))
		{
			if (pPlayer->ammo_rockets >= 100)
				return FALSE;
			pPlayer->ammo_rockets += pev->armorvalue;
		}

		if (pev->team & IT_LIGHTNING)
		{
			if (pPlayer->ammo_cells >= 200)
				return FALSE;
			pPlayer->ammo_cells += pev->armorvalue;
		}

		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "You got the %s\n", STRING( pev->netname )));

		pPlayer->CheckAmmo();

		// change to a better weapon if appropriate
		if( pPlayer->m_iWeapon == best )
		{
			pPlayer->m_iWeapon = pPlayer->W_BestWeapon();
			pPlayer->W_SetCurrentAmmo ();
		}

		pPlayer->W_SetCurrentAmmo ();

		return TRUE;		
	}
};

/*QUAKED item_shells (0 .5 .8) (0 0 0) (32 32 32) big
*/
LINK_ENTITY_TO_CLASS(item_shells, CItemAmmo);

/*QUAKED item_spikes (0 .5 .8) (0 0 0) (32 32 32) big
*/
LINK_ENTITY_TO_CLASS(item_spikes, CItemAmmo);

/*QUAKED item_rockets (0 .5 .8) (0 0 0) (32 32 32) big
*/
LINK_ENTITY_TO_CLASS(item_rockets, CItemAmmo);

/*QUAKED item_cells (0 .5 .8) (0 0 0) (32 32 32) big
*/
LINK_ENTITY_TO_CLASS(item_cells, CItemAmmo);

/*QUAKED item_weapon (0 .5 .8) (0 0 0) (32 32 32) shotgun rocket spikes big
DO NOT USE THIS!!!! IT WILL BE REMOVED!
*/
LINK_ENTITY_TO_CLASS(item_weapon, CItemAmmo);

class CItemWeapon : public CItem
{
	void Precache( void )
	{
		if (FClassnameIs( pev, "weapon_supershotgun" ))
		{
			PRECACHE_MODEL ("models/g_shot.mdl");
		}
		else if (FClassnameIs( pev, "weapon_nailgun" ))
		{
			PRECACHE_MODEL ("models/g_nail.mdl");
		}
		else if (FClassnameIs( pev, "weapon_supernailgun" ))
		{
			PRECACHE_MODEL ("models/g_nail2.mdl");
		}
		else if (FClassnameIs( pev, "weapon_grenadelauncher" ))
		{
			PRECACHE_MODEL ("models/g_rock.mdl"); 
		}
		else if (FClassnameIs( pev, "weapon_rocketlauncher" ))
		{
			PRECACHE_MODEL ("models/g_rock2.mdl"); 
		}
		else if (FClassnameIs( pev, "weapon_lightning" ))
		{
			PRECACHE_MODEL ("models/g_light.mdl"); 
		}
	}

	void Spawn( void )
	{ 
		Precache( );
		if (FClassnameIs( pev, "weapon_supershotgun" ))
		{
			SET_MODEL(ENT(pev), "models/g_shot.mdl");
			pev->netname = MAKE_STRING("Double-barrelled Shotgun");
			pev->team = IT_SUPER_SHOTGUN;
		}
		else if (FClassnameIs( pev, "weapon_nailgun" ))
		{
			SET_MODEL(ENT(pev), "models/g_nail.mdl");
			pev->netname = MAKE_STRING("nailgun");
			pev->team = IT_NAILGUN;
		}
		else if (FClassnameIs( pev, "weapon_supernailgun" ))
		{
			SET_MODEL(ENT(pev), "models/g_nail2.mdl");
			pev->netname = MAKE_STRING("Super Nailgun");
			pev->team = IT_SUPER_NAILGUN;
		}
		else if (FClassnameIs( pev, "weapon_grenadelauncher" ))
		{
			SET_MODEL(ENT(pev), "models/g_rock.mdl"); 
			pev->netname = MAKE_STRING("Grenade Launcher");
			pev->team = IT_GRENADE_LAUNCHER;
		}
		else if (FClassnameIs( pev, "weapon_rocketlauncher" ))
		{
			SET_MODEL(ENT(pev), "models/g_rock2.mdl"); 
			pev->netname = MAKE_STRING("Rocket Launcher");
			pev->team = IT_ROCKET_LAUNCHER;
		}
		else if (FClassnameIs( pev, "weapon_lightning" ))
		{
			SET_MODEL(ENT(pev), "models/g_light.mdl"); 
			pev->netname = MAKE_STRING("Thunderbolt");
			pev->team = IT_LIGHTNING;
		}
		pev->noise = MAKE_STRING( "weapons/pkup.wav" );

		UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 56 ));
		StartItem ();
	}

	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		BOOL leave;

		if (gpGlobals->deathmatch == 2.0 || gpGlobals->coop)
			leave = TRUE;
		else leave = FALSE;

		// do the apropriate action
		if (pev->team & IT_SHOTGUN)
		{
			if (leave && (pPlayer->m_iItems & IT_SHOTGUN) )
				return FALSE;
			pPlayer->ammo_shells += 5;
		}
		if (pev->team & (IT_NAILGUN|IT_SUPER_NAILGUN))
		{
			if (leave && pPlayer->m_iItems & (IT_NAILGUN|IT_SUPER_NAILGUN))
				return FALSE;
			pPlayer->ammo_nails += 30;
		}	
		if (pev->team & (IT_GRENADE_LAUNCHER|IT_ROCKET_LAUNCHER))
		{
			if (leave && pPlayer->m_iItems & (IT_GRENADE_LAUNCHER|IT_ROCKET_LAUNCHER))
				return FALSE;
			pPlayer->ammo_rockets += 5;
		}
		if (pev->team & IT_LIGHTNING)
		{
			if (leave && (pPlayer->m_iItems & IT_LIGHTNING) )
				return FALSE;
			pPlayer->ammo_cells += 15;
		}

		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "You got the %s\n", STRING( pev->netname )));

		pPlayer->CheckAmmo();

		pPlayer->m_iItems |= pev->team;

		// change to a better weapon if appropriate
		if (gpGlobals->deathmatch)
			pPlayer->m_iWeapon = pPlayer->W_BestWeapon();
		else pPlayer->m_iWeapon = pev->team;

		pPlayer->W_SetCurrentAmmo ();

		return !leave;		
	}
};

/*QUAKED weapon_supershotgun (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(weapon_supershotgun, CItemWeapon);

/*QUAKED weapon_nailgun (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(weapon_nailgun, CItemWeapon);

/*QUAKED weapon_supernailgun (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(weapon_supernailgun, CItemWeapon);

/*QUAKED weapon_grenadelauncher (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(weapon_grenadelauncher, CItemWeapon);

/*QUAKED weapon_rocketlauncher (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(weapon_rocketlauncher, CItemWeapon);

/*QUAKED weapon_lightning (0 .5 .8) (-16 -16 0) (16 16 32)
*/
LINK_ENTITY_TO_CLASS(weapon_lightning, CItemWeapon);

//*********************************************************
// weaponbox code:
//*********************************************************
LINK_ENTITY_TO_CLASS( backpack, CWeaponBox );

//=========================================================
//
//=========================================================
CWeaponBox *CWeaponBox::DropBackpack( CBaseEntity *pVictim, int weapon )
{
	if (!pVictim || !(pVictim->ammo_shells + pVictim->ammo_nails + pVictim->ammo_rockets + pVictim->ammo_cells))
		return NULL; // nothing in it

	CWeaponBox *pBackpack = GetClassPtr((CWeaponBox *)NULL );

	UTIL_SetOrigin( pBackpack->pev, pVictim->pev->origin + Vector( 0, 0, 24 ));

	pBackpack->pev->velocity.x = RANDOM_FLOAT( -100, 100 );
	pBackpack->pev->velocity.y = RANDOM_FLOAT( -100, 100 );
	pBackpack->pev->velocity.z = 100;
	pBackpack->Spawn();
	pBackpack->pev->classname = MAKE_STRING( "backpack" );

	// copy weapon and ammo
	pBackpack->pev->team = weapon;
	pBackpack->ammo_shells = pVictim->ammo_shells;
	pBackpack->ammo_nails = pVictim->ammo_nails;
	pBackpack->ammo_rockets = pVictim->ammo_rockets;
	pBackpack->ammo_cells = pVictim->ammo_cells;

	return pBackpack;
}

void CWeaponBox::Precache( void )
{
	PRECACHE_MODEL("models/backpack.mdl");
}

//=========================================================
// CWeaponBox - Spawn 
//=========================================================
void CWeaponBox::Spawn( void )
{
	Precache( );

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	SET_MODEL( ENT(pev), "models/backpack.mdl");
	UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 56 ));

	pev->nextthink = gpGlobals->time + 120;	// remove after 2 minutes
	SetThink( SUB_Remove );
}

//=========================================================
// CWeaponBox - Touch: try to add my contents to the toucher
// if the toucher is a player.
//=========================================================
void CWeaponBox::Touch( CBaseEntity *pOther )
{
	if( !pOther->IsPlayer( ) || !pOther->IsAlive())
	{
		// only players may touch a weaponbox.
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	int best = pPlayer->W_BestWeapon();

	// change weapons
	pPlayer->ammo_shells += ammo_shells;
	pPlayer->ammo_nails += ammo_nails;
	pPlayer->ammo_rockets += ammo_rockets;
	pPlayer->ammo_cells += ammo_cells;
	pPlayer->m_iItems |= pev->team;
	pPlayer->CheckAmmo();

	CLIENT_PRINTF( pPlayer->edict(), print_console, "You get " );

	if (ammo_shells)
		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "%i shells", ammo_shells ));

	if (ammo_nails)
		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "%i nails", ammo_nails ));

	if (ammo_rockets)
		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "%i rockets", ammo_rockets ));

	if (ammo_cells)
		CLIENT_PRINTF( pPlayer->edict(), print_console, UTIL_VarArgs( "%i cells", ammo_cells ));

	CLIENT_PRINTF( pPlayer->edict(), print_console, "\n" );

	if (pPlayer->m_iWeapon == best)
		pPlayer->m_iWeapon = pPlayer->W_BestWeapon();
	pPlayer->W_SetCurrentAmmo();

	EMIT_SOUND( pOther->edict(), CHAN_ITEM, "weapons/lock4.wav", 1, ATTN_NORM );

	// send bonus flash (same as command "bf\n")
	BONUS_FLASH( pPlayer->edict() );

	SetTouch(NULL);
	UTIL_Remove(this);
}

void CWeaponBox::SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + pev->mins;
	pev->absmax = pev->origin + pev->maxs;

	// to make items easier to pick up and allow them to be grabbed off
	// of shelves, the abs sizes are expanded
	pev->absmin.x -= 15;
	pev->absmin.y -= 15;
	pev->absmax.x += 15;
	pev->absmax.y += 15;
}