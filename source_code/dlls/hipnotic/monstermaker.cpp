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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "monster.h"
#include "gamerules.h"

const char *random_spawn_names[]=
{
{ "monster_dog" },
{ "monster_ogre" },
{ "monster_demon1" },
{ "monster_zombie" },
{ "monster_shambler" },

};

//================
//
// func_spawn
//
//================
/*QUAKED func_spawn (0 .5 .8) (-32 -32 -24) (32 32 64) big/ambush megahealth
This will spawn a thing upon being used. The thing that
is spawned depends upon the value of "spawnfunction".
"spawnclassname" should contain the same value as "spawnfunction".
If "spawnfunction" is unspecified a random monster is chosen.
The angles, target and all flags are passed on
Think of it like setting up a normal entity.
"spawnsilent" set this to 1 if you want a silent spawn.
"spawnmulti" set this to 1 if you want this spawn to be reoccuring.
*/
class CFuncSpawn : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	void	KeyValue( KeyValueData *pkvd );

	BOOL	m_fSpawnSilent;
	BOOL	m_fSpawnMulti;
	string_t	m_iszSpawnClassname;
	string_t	m_iszSpawnFunction;
};

LINK_ENTITY_TO_CLASS( func_spawn, CFuncSpawn );
LINK_ENTITY_TO_CLASS( func_spawn_small, CFuncSpawn );

TYPEDESCRIPTION CFuncSpawn::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncSpawn, m_fSpawnSilent, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFuncSpawn, m_fSpawnMulti, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFuncSpawn, m_iszSpawnClassname, FIELD_STRING ),
	DEFINE_FIELD( CFuncSpawn, m_iszSpawnFunction, FIELD_STRING ),
}; IMPLEMENT_SAVERESTORE( CFuncSpawn, CBaseEntity );

void CFuncSpawn :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "spawnsilent"))
	{
		m_fSpawnSilent = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnmulti"))
	{
		m_fSpawnMulti = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnclassname"))
	{
		m_iszSpawnClassname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnfunction"))
	{
		m_iszSpawnFunction = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CFuncSpawn :: Precache( void )
{
	UTIL_PrecacheOther( STRING( m_iszSpawnClassname ));
}

void CFuncSpawn :: Spawn( void )
{
	if( !m_iszSpawnFunction )
	{
		// random monster selection
		int monsterCount = ARRAYSIZE( random_spawn_names ) - 1;
		m_iszSpawnClassname = MAKE_STRING( random_spawn_names[RANDOM_LONG( 0, monsterCount )] );		
		m_iszSpawnFunction = m_iszSpawnClassname; // makes a copy
	}

	if( !m_iszSpawnClassname )
	{
		ALERT( at_error, "func_spawn: has no \"spawnclassname\" specified. Removed.\n" );
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	if( !FStrEq( STRING( m_iszSpawnClassname ), STRING( m_iszSpawnFunction )))
		ALERT( at_warning, "func_spawn: class: %s, func: %s\n", STRING( m_iszSpawnClassname ), STRING( m_iszSpawnFunction ));

	// precache our entity
	Precache ();
}

void CFuncSpawn :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	edict_t *pent = CREATE_NAMED_ENTITY( m_iszSpawnClassname );

	if( FNullEnt( pent ))
	{
		ALERT ( at_error, "NULL Ent in func_spawn!\n" );
		UTIL_Remove( this );
		return;
	}

	entvars_t *pevCreate = VARS( pent );
	pevCreate->origin = pev->origin;
	pevCreate->angles = pev->angles;
	pevCreate->spawnflags = pev->spawnflags;
	pevCreate->target = pev->target;	

	DispatchSpawn( ENT( pevCreate ) );

	if( pent->free || pent->v.flags & FL_KILLME )
	{
		// entity was rejected for some reasons. kill the monstermaker too
		UTIL_Remove( this );
		return;
	}

	if( !m_fSpawnSilent )
		CTeleFog::CreateFog( pev->origin );

	CBaseEntity *pSpawn = CBaseEntity :: Instance( pevCreate );
	CQuakeMonster *pSpawnedMonster = pSpawn->GetMonster();

	if( pSpawnedMonster != NULL )
	{
		if( g_fHornActive )
		{
			pSpawnedMonster->m_fCharmed = TRUE;
			pSpawnedMonster->m_hCharmer = g_pHornCharmer;
			pevCreate->effects |= EF_DIMLIGHT;
		}

		// update monster count
		MESSAGE_BEGIN( MSG_ALL, gmsgStats );
			WRITE_BYTE( STAT_TOTALMONSTERS );
			WRITE_SHORT( gpWorld->total_monsters );
		MESSAGE_END();
	}

	if( !m_fSpawnMulti )
		UTIL_Remove( this );
}