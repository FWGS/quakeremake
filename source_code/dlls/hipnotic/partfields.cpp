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
#include "doors.h"
#include "items.h"
#include "gamerules.h"

// func_particlefield
#define SF_USE_COUNTER		1

// func_counter
#define SF_COUNTER_TOGGLE		1
#define SF_COUNTER_LOOP		2
#define SF_COUNTER_STEP		4
#define SF_COUNTER_RESET		8
#define SF_COUNTER_RANDOM		16
#define SF_COUNTER_FINISHCOUNT	32
#define SF_COUNTER_START_ON		64

/*QUAKED func_particlefield (0 .5 .8) ? USE_COUNT
Creates a brief particle flash roughly the size of the defining
brush each time it is triggered.

USE_COUNT when the activator is a func_counter, the field will only
   activate when count is equal to "cnt".  Same as using a func_oncount
   to trigger.

"cnt" is the count to activate on when USE_COUNT is set.
"color" is the color of the particles.  Default is 192 (yellow).
"count" is the density of the particles.  Default is 2.
"noise" is the sound to play when triggered.  Do not use a looping sound here.
"dmg" is the amount of damage to cause when touched.
*/
class CFuncPartField : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );

	void EXPORT FieldXY( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT FieldXZ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT FieldYZ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void Touch( CBaseEntity *pOther );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	Vector	m_vecDest;
	Vector	m_vecDest1;
	Vector	m_vecDest2;
};

TYPEDESCRIPTION CFuncPartField::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncPartField, m_vecDest, FIELD_VECTOR ),
	DEFINE_FIELD( CFuncPartField, m_vecDest1, FIELD_VECTOR ),
	DEFINE_FIELD( CFuncPartField, m_vecDest2, FIELD_VECTOR ),
}; IMPLEMENT_SAVERESTORE( CFuncPartField, CBaseEntity );

LINK_ENTITY_TO_CLASS( func_particlefield, CFuncPartField );

void CFuncPartField :: Precache( void )
{
	if( pev->noise )
		PRECACHE_SOUND( (char *)STRING( pev->noise ));
}

void CFuncPartField :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "color"))
	{
		// store color into impulse
		pev->impulse = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CFuncPartField :: Spawn( void )
{
	if( !pev->impulse )
		pev->impulse = 192;	// yellow color

	if( !m_flCount )
		m_flCount = 2;

	Precache ();

	SET_MODEL( ENT(pev), STRING(pev->model) );
	pev->origin = (pev->mins + pev->maxs) * 0.5f;
	UTIL_SetOrigin( pev, pev->origin );

	// g-cont. remove the model because we have custom origin
	// so FIND_CLIENT_IN_PVS should use them instead of absmin\absmax
	pev->modelindex = pev->model = 0;  	
	pev->ltime = gpGlobals->time;
	pev->effects |= EF_NODRAW;

	m_vecDest = pev->maxs - pev->mins - Vector( 16.0f, 16.0f, 16.0f );
	m_vecDest1 = pev->mins + Vector( 8.0f, 8.0f, 8.0f ) - pev->origin;
	m_vecDest2 = pev->maxs + Vector( 7.9f, 7.9f, 7.9f ) - pev->origin;

	if( m_vecDest.x > m_vecDest.z )
	{
		if( m_vecDest.y > m_vecDest.z )
		{
			SetUse( &CFuncPartField::FieldXY );
			m_vecDest1.z = ( m_vecDest1.z + m_vecDest2.z ) / 2;
		}
		else
		{
			SetUse( &CFuncPartField::FieldXZ );
			m_vecDest1.y = ( m_vecDest1.y + m_vecDest2.y ) / 2;
		}
	}
	else
	{
		if( m_vecDest.y > m_vecDest.x )
		{
			SetUse( &CFuncPartField::FieldYZ );
			m_vecDest1.x = ( m_vecDest1.x + m_vecDest2.x ) / 2;
		}
		else
		{
			SetUse( &CFuncPartField::FieldXZ );
			m_vecDest1.y = ( m_vecDest1.y + m_vecDest2.y ) / 2;
		}
	}
}

void CFuncPartField :: Touch( CBaseEntity *pOther )
{
	if( !pev->dmg ) return;

	if( gpGlobals->time > pev->ltime || gpGlobals->time < m_flAttackFinished )
		return;

	pOther->TakeDamage( pev, pev, pev->dmg, DMG_GENERIC );
	m_flAttackFinished = gpGlobals->time + 0.5f;
}

void CFuncPartField :: FieldXZ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if(( pev->spawnflags & SF_USE_COUNTER ) && ( pActivator->GetCount() != m_flCnt ))
		return;

	pev->ltime = gpGlobals->time + 0.25f;

	if( pev->noise )
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise ), 1, ATTN_NORM);

	// Only show particles if client is visible.
	// This helps to keep network traffic down to a minimum.
	if( FNullEnt( FIND_CLIENT_IN_PVS( edict() )) && !g_intermission_running)
		return;

	Vector start = m_vecDest1 + pev->origin;
	Vector end   = m_vecDest2 + pev->origin;
	Vector pos;

	pos.y = start.y;
	pos.z = start.z;

	while( pos.z <= end.z )
	{
		pos.x = start.x;
		while( pos.x <= end.x )
		{
			UTIL_ParticleEffect( pos, g_vecZero, pev->impulse, m_flCount );
			pos.x = pos.x + 16;
		}
		pos.z = pos.z + 16;
	}
}

void CFuncPartField :: FieldYZ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if(( pev->spawnflags & SF_USE_COUNTER ) && ( pActivator->GetCount() != m_flCnt ))
		return;

	pev->ltime = gpGlobals->time + 0.25f;

	if( pev->noise )
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise ), 1, ATTN_NORM);

	// Only show particles if client is visible.
	// This helps to keep network traffic down to a minimum.
	if( FNullEnt( FIND_CLIENT_IN_PVS( edict() )) && !g_intermission_running)
		return;

	Vector start = m_vecDest1 + pev->origin;
	Vector end   = m_vecDest2 + pev->origin;
	Vector pos;

	pos.x = start.x;
	pos.z = start.z;

	while( pos.z < end.z )
	{
		pos.y = start.y;
		while( pos.y < end.y )
		{
			UTIL_ParticleEffect( pos, g_vecZero, pev->impulse, m_flCount );
			pos.y = pos.y + 16;
		}
		pos.z = pos.z + 16;
	}
}

void CFuncPartField :: FieldXY( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if(( pev->spawnflags & SF_USE_COUNTER ) && ( pActivator->GetCount() != m_flCnt ))
		return;

	pev->ltime = gpGlobals->time + 0.25f;

	if( pev->noise )
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise ), 1, ATTN_NORM);

	// Only show particles if client is visible.
	// This helps to keep network traffic down to a minimum.
	if( FNullEnt( FIND_CLIENT_IN_PVS( edict() )) && !g_intermission_running)
		return;

	Vector start = m_vecDest1 + pev->origin;
	Vector end   = m_vecDest2 + pev->origin;
	Vector pos;

	pos.x = start.x;
	pos.z = start.z;

	while( pos.x < end.x )
	{
		pos.y = start.y;
		while( pos.y < end.y )
		{
			UTIL_ParticleEffect( pos, g_vecZero, pev->impulse, m_flCount );
			pos.y = pos.y + 16;
		}
		pos.x = pos.x + 16;
	}
}

/*QUAKED func_counter (0 0 0.5) (0 0 0) (32 32 32) TOGGLE LOOP STEP RESET RANDOM FINISHCOUNT START_ON
TOGGLE causes the counter to switch between an on and off state
each time the counter is triggered.

LOOP causes the counter to repeat infinitly.  The count resets to zero
after reaching the value in "count".

STEP causes the counter to only increment when triggered.  Effectively,
this turns the counter into a relay with counting abilities.

RESET causes the counter to reset to 0 when restarted.

RANDOM causes the counter to generate random values in the range 1 to "count"
at the specified interval.

FINISHCOUNT causes the counter to continue counting until it reaches "count"
before shutting down even after being set to an off state.

START_ON causes the counter to be on when the level starts.

"count" specifies how many times to repeat the event.  If LOOP is set,
it specifies how high to count before reseting to zero.  Default is 10.

"wait"  the length of time between each trigger event. Default is 1 second.

"delay" how much time to wait before firing after being switched on.
*/
class CFuncCounter : public CBaseToggle
{
public:
	void Spawn( void );
	void EXPORT CounterOn( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT CounterOff( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT CounterThink( void );

	float GetCount( void ) { return pev->dmg; }
};

LINK_ENTITY_TO_CLASS( func_counter, CFuncCounter );

void CFuncCounter :: Spawn( void )
{
	if( !m_flWait )
	{
		m_flWait = 1.0f;
	}

	m_flCount = floor( m_flCount );

	if( m_flCount <= 0.0f )
		m_flCount = 10.0f;

	m_flCnt = 0;
	pev->dmg = 0;

	SetUse( &CFuncCounter::CounterOff );

	if( pev->spawnflags & SF_COUNTER_START_ON )
	{
		// g-cont. for auto start activator will be himself
		// and condition "pActivator->GetCount()" works fine
		SetThink( &CBaseEntity::SUB_CallUseToggle );
		pev->nextthink = gpGlobals->time + 0.1f;
	}
}

void CFuncCounter :: CounterOn( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_hActivator = pActivator;

	if(( m_flCnt != 0.0f ) && ( pev->spawnflags & SF_COUNTER_FINISHCOUNT ))
	{
		pev->button = TRUE;
		return;
	}

	SetUse( &CFuncCounter::CounterOff );
	pev->button = FALSE;
	SetThink( NULL );
}

void CFuncCounter :: CounterOff( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_hActivator = pActivator;
	pev->button = FALSE;

	if( pev->spawnflags & SF_COUNTER_TOGGLE )
	{
		SetUse( &CFuncCounter::CounterOn );
	}
	else
	{
		SetUse( NULL );
	}

	if( pev->spawnflags & SF_COUNTER_RESET )
	{
		m_flCnt = 0.0f;
		pev->dmg = 0.0f;
	}

	SetThink( &CFuncCounter::CounterThink );

	if( m_flDelay )
	{
		pev->nextthink = gpGlobals->time + m_flDelay;
	}
	else
	{
		CounterThink ();
	}
}

void CFuncCounter :: CounterThink( void )
{
	m_flCnt += 1.0f;

	if( pev->spawnflags & SF_COUNTER_RANDOM )
	{
		pev->dmg = RANDOM_FLOAT( 0.0f, 1.0f ) * m_flCount;
		pev->dmg = floor( pev->dmg ) + 1.0f;
	}
	else
	{
		pev->dmg = m_flCnt;
	}

	SUB_UseTargets( this, USE_TOGGLE, 0.0f );
	pev->nextthink = gpGlobals->time + m_flWait;

	if( pev->spawnflags & SF_COUNTER_STEP )
	{
		// activator will be pass through again
		CounterOn( this, this, USE_TOGGLE, 0.0f );
	}

	if( m_flCnt >= m_flCount )
	{
		m_flCnt = 0.0f;

		if(( pev->button ) || !FBitSet( pev->spawnflags, SF_COUNTER_LOOP ))
		{
			if( pev->spawnflags & SF_COUNTER_TOGGLE )
			{
				// activator will be pass through again
				CounterOn( this, this, USE_TOGGLE, 0.0f );
			}
			else
			{
				UTIL_Remove( this );
			}
		}
	}
}

/*QUAKED func_oncount (0 0 0.5) (0 0 0) (16 16 16)
Must be used as the target for func_counter.  When the counter
reaches the value set by count, func_oncount triggers its targets.

"count" specifies the value to trigger on.  Default is 1.

"delay" how much time to wait before firing after being triggered.
*/
class CFuncOnCount : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( func_oncount, CFuncOnCount );

void CFuncOnCount :: Spawn( void )
{
	m_flCount = floor( m_flCount );

	if( m_flCount <= 0.0f )
		m_flCount = 1.0f;
}

void CFuncOnCount :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( pActivator->GetCount() == m_flCount )
		SUB_UseTargets( pActivator, USE_TOGGLE, 0.0f );
}
