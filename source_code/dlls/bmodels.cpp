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

===== bmodels.cpp ========================================================

  spawn, think, and use functions for entities that use brush models

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

#ifdef HIPNOTIC
#define SF_TOGGLEWALL_START_OFF	1

#endif /* HIPNOTIC */
//
// BModelOrigin - calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
//
Vector VecBModelOrigin( entvars_t* pevBModel )
{
	return pevBModel->absmin + ( pevBModel->size * 0.5 );
}

// =================== FUNC_WALL ==============================================

/*QUAKED func_wall (0 .5 .8) ?
This is just a solid wall if not inhibited
*/
class CFuncWall : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( func_wall, CFuncWall );

void CFuncWall :: Spawn( void )
{
	pev->angles	= g_vecZero;
	pev->movetype	= MOVETYPE_PUSH;  // so it doesn't get pushed by anything
	pev->solid	= SOLID_BSP;

#ifdef HIPNOTIC
	// some func_walls are teleports.
	pev->effects |= EF_NOWATERCSG;

#endif /* HIPNOTIC */
	SET_MODEL( ENT(pev), STRING(pev->model) );
	
	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}

void CFuncWall :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ShouldToggle( useType, (int)( pev->frame )) )
		pev->frame = 1 - pev->frame;
}

#ifdef HIPNOTIC
/*QUAKED func_breakawaywall (0 .5 .8) ?
Special walltype that removes itself when triggered.
*/
class CBreakAwayWall : public CFuncWall
{
public:
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( func_breakawaywall, CBreakAwayWall );

void CBreakAwayWall :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// g-cont. strange entity. Why they not used just func_wall + killtarget instead?
	UTIL_Remove( this );
}

/*QUAKED func_togglewall (0 .5 .8) ? START_OFF
Creates a invisible wall that can be toggled on and off.

START_OFF wall doesn't block until triggered.

"noise" is the sound to play when wall is turned off.
"noise1" is the sound to play when wall is blocking.
"dmg" is the amount of damage to cause when touched.
*/
class CFuncToggleWall : public CFuncWall
{
public:
	void	Precache( void );
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	Touch( CBaseEntity *pOther );
	void	TurnOff( void );
	void	TurnOn( void );
	BOOL	IsOn( void );
};

LINK_ENTITY_TO_CLASS( func_togglewall, CFuncToggleWall );

void CFuncToggleWall :: Precache( void )
{
	PRECACHE_SOUND( (char *)STRING( pev->noise ));
	PRECACHE_SOUND( (char *)STRING( pev->noise1 ));
}

void CFuncToggleWall :: Spawn( void )
{
	CFuncWall::Spawn();

	if( !pev->noise )
		pev->noise = MAKE_STRING( "misc/null.wav" );

	if( !pev->noise1 )
		pev->noise1 = MAKE_STRING( "misc/null.wav" );

	pev->effects |= EF_NODRAW;

	Precache ();

	if( pev->spawnflags & SF_TOGGLEWALL_START_OFF )
		TurnOff();
	else TurnOn();
}

void CFuncToggleWall :: TurnOff( void )
{
	pev->solid = SOLID_NOT;
	UTIL_SetOrigin( pev, pev->origin );
	EMIT_SOUND( edict(), CHAN_VOICE, STRING( pev->noise ), 1.0, ATTN_NORM );
}

void CFuncToggleWall :: TurnOn( void )
{
	pev->solid = SOLID_BSP;
	UTIL_SetOrigin( pev, pev->origin );
	EMIT_SOUND( edict(), CHAN_VOICE, STRING( pev->noise1 ), 1.0, ATTN_NORM );
}

BOOL CFuncToggleWall :: IsOn( void )
{
	if ( pev->solid == SOLID_NOT )
		return FALSE;
	return TRUE;
}

void CFuncToggleWall :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int on = IsOn();

	if( ShouldToggle( useType, on ))
	{
		if( on ) TurnOff();
		else TurnOn();
	}
}

void CFuncToggleWall :: Touch( CBaseEntity *pOther )
{
	if( !pev->dmg || gpGlobals->time < m_flAttackFinished )
		return;

	pOther->TakeDamage( pev, pev, pev->dmg, DMG_GENERIC );
	m_flAttackFinished = gpGlobals->time + 0.5f;
}

#endif /* HIPNOTIC */
/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
#ifndef HIPNOTIC
class CFuncIllusionary : public CBaseToggle 
#else /* HIPNOTIC */
class CFuncIllusionary : public CBaseEntity
#endif /* HIPNOTIC */
{
public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( func_illusionary, CFuncIllusionary );

void CFuncIllusionary :: Spawn( void )
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_NONE;  
	pev->solid = SOLID_NOT;// always solid_not 
#ifdef HIPNOTIC

#endif /* HIPNOTIC */
	SET_MODEL( ENT(pev), STRING(pev->model) );
#ifdef HIPNOTIC

	pev->effects |= EF_NOWATERCSG;
#endif /* HIPNOTIC */
	
	// I'd rather eat the network bandwidth of this than figure out how to save/restore
	// these entities after they have been moved to the client, or respawn them ala Quake
	// Perhaps we can do this in deathmatch only.
	//	MAKE_STATIC(ENT(pev));
}

/*QUAKED func_episodegate (0 .5 .8) ? E1 E2 E3 E4
This bmodel will appear if the episode has allready been completed, so players can't reenter it.
*/
class CFuncEpisodeGate : public CBaseToggle 
{
public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( func_episodegate, CFuncEpisodeGate );

void CFuncEpisodeGate :: Spawn( void )
{
	if (!((int)gpWorld->serverflags & pev->spawnflags))
	{
		REMOVE_ENTITY( ENT( pev ));
		return;
	}

	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH;	// so it doesn't get pushed by anything  
	pev->solid = SOLID_BSP;// always solid_not 
	SET_MODEL( ENT(pev), STRING(pev->model) );

	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}

/*QUAKED func_bossgate (0 .5 .8) ?
This bmodel appears unless players have all of the episode sigils.
*/
class CFuncBossGate : public CBaseToggle 
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( func_bossgate, CFuncBossGate );

void CFuncBossGate :: Spawn( void )
{
	if ( ((int)gpWorld->serverflags & 15 ) == 15 )
	{
		REMOVE_ENTITY( ENT( pev )); // all episodes completed
		return;
	}

	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH;	// so it doesn't get pushed by anything  
	pev->solid = SOLID_BSP;// always solid_not 
	SET_MODEL( ENT(pev), STRING(pev->model) );

	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}

void CFuncBossGate :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ShouldToggle( useType, (int)( pev->frame )) )
		pev->frame = 1 - pev->frame;
#ifdef HIPNOTIC
}

class CFuncBobbingWater : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Think( void );
};

LINK_ENTITY_TO_CLASS( func_bobbingwater, CFuncBobbingWater );

void CFuncBobbingWater :: Spawn( void )
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_STEP;
	pev->solid = SOLID_NOT;

	SET_MODEL( ENT(pev), STRING(pev->model) );

	m_flCount = 0;
	m_flCnt = pev->size.z / 2.0f;

	if( !pev->speed ) pev->speed = 4.0f;
	pev->speed = 360.0f / pev->speed;

	pev->nextthink = gpGlobals->time + 0.01f;
	pev->ltime = gpGlobals->time;
}

void CFuncBobbingWater :: Think( void )
{
	m_flCount += pev->speed * ( gpGlobals->time - pev->ltime );

	if( m_flCount > 360 )
		m_flCount -= 360.0f;

	Vector ang( m_flCount, 0.0f, 0.0f );
	UTIL_MakeVectors( ang );

	pev->origin.z = gpGlobals->v_forward.z * m_flCnt;
	UTIL_SetOrigin( pev, pev->origin );
	pev->ltime = gpGlobals->time;
	pev->nextthink = gpGlobals->time + 0.01;
#endif /* HIPNOTIC */
}
