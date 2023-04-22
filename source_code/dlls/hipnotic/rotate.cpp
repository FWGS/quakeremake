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
#include "rotate.h"
#include "gamerules.h"

// a debug command that show clip brushes instead of rotational brushes
void Cmd_ShowRotateClip_f( void )
{
	edict_t *pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );

	for( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if( pEdict->free || pEdict->v.flags & FL_KILLME )	// Not in use
			continue;

		if( FClassnameIs( pEdict, "func_movewall" ) || FClassnameIs( pEdict, "rotate_object" ))
			pEdict->v.effects = (pEdict->v.effects ^ EF_NODRAW);
	}
}

/*QUAKED info_rotate (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as the point of rotation for rotatable objects.
*/
class CInfoRotate : public CBaseEntity
{
public:
	void Spawn( void ) { SetThink( &CBaseEntity::SUB_Remove ); pev->nextthink = gpGlobals->time + 2.0f; }
};

LINK_ENTITY_TO_CLASS( info_rotate, CInfoRotate );

/*QUAKED func_rotate_entity (0 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE START_ON
Creates an entity that continually rotates.  Can be toggled on and
off if targeted.

TOGGLE = allows the rotation to be toggled on/off

START_ON = wether the entity is spinning when spawned.  If TOGGLE is 0, entity can be turned on, but not off.

If "deathtype" is set with a string, this is the message that will appear when a player is killed by the train.

"rotate" is the rate to rotate.
"target" is the center of rotation.
"speed"  is how long the entity takes to go from standing still to full speed and vice-versa.
*/
class CFuncRotate : public CBaseEntity
{
public:
	void RotateTargets( void );
	void RotateTargetsFinal( void );
	void SetTargetOrigin( void );
	void LinkRotateTargets( void );
	void SetDamageOnTargets( float flDmg );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	void EXPORT FirstThink( void );	// link rotate targets etc
	void EXPORT NormalThink( void );

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );

	int m_rotate_state;
	Vector m_vecRotate;
	float m_flEndTime;
};

TYPEDESCRIPTION CFuncRotate::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncRotate, m_rotate_state, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncRotate, m_vecRotate, FIELD_VECTOR ),
	DEFINE_FIELD( CFuncRotate, m_flEndTime, FIELD_TIME ),
}; IMPLEMENT_SAVERESTORE( CFuncRotate, CBaseEntity );

LINK_ENTITY_TO_CLASS( func_rotate_entity, CFuncRotate );

void CFuncRotate :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "rotate"))
	{
		UTIL_StringToVector( m_vecRotate, pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CFuncRotate :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	if( pev->speed != 0 )
	{
		// auto-compute accelerate interval
		m_flCnt = 1.0f / pev->speed;
	}

	SetThink( &CFuncRotate::FirstThink );
	pev->nextthink = gpGlobals->time + 0.1f;
	pev->ltime = gpGlobals->time;
}

// rotate object routines
void CFuncRotate :: RotateTargets( void )
{
	Vector vx, vy, vz;
	Vector org;

	UTIL_MakeVectors( pev->angles );

	CBaseEntity *pEnt = UTIL_FindEntityByTargetname( gpWorld, STRING( pev->target ));

	while( pEnt != NULL )
	{
		if( pEnt->m_iRotateType == OBJECT_SETORIGIN )
		{
			org = pEnt->oldorigin;
			vx = ( gpGlobals->v_forward * org.x );
			vy = ( gpGlobals->v_right   * org.y );
			vy = vy * -1;
			vz = ( gpGlobals->v_up      * org.z );
			pEnt->neworigin = vx + vy + vz;
			UTIL_SetOrigin( pEnt->pev, pEnt->neworigin + pev->origin );
		}
		else if( pEnt->m_iRotateType == OBJECT_ROTATE )
		{
			pEnt->pev->angles = pev->angles;
			org = pEnt->oldorigin;
			vx = ( gpGlobals->v_forward * org.x );
			vy = ( gpGlobals->v_right   * org.y );
			vy = vy * -1;
			vz = ( gpGlobals->v_up      * org.z );
			pEnt->neworigin = vx + vy + vz;
			UTIL_SetOrigin( pEnt->pev, pEnt->neworigin + pev->origin );
		}
		else if( pEnt->m_iRotateType == OBJECT_MOVEWALL )
		{
			org = pEnt->oldorigin;
			vx = ( gpGlobals->v_forward * org.x );
			vy = ( gpGlobals->v_right   * org.y );
			vy = vy * -1;
			vz = ( gpGlobals->v_up      * org.z );
			pEnt->neworigin = vx + vy + vz;
			pEnt->neworigin = pev->origin - oldorigin + (pEnt->neworigin - pEnt->oldorigin);
			pEnt->pev->velocity = (pEnt->neworigin - pEnt->pev->origin) * 25;
		}

		pEnt = UTIL_FindEntityByTargetname( pEnt, STRING( pev->target ));
	}
}

void CFuncRotate :: RotateTargetsFinal( void )
{
	CBaseEntity *pEnt = UTIL_FindEntityByTargetname( gpWorld, STRING( pev->target ));

	while( pEnt != NULL )
	{
		pEnt->pev->velocity = g_vecZero;

		if( pEnt->m_iRotateType == OBJECT_ROTATE )
			pEnt->pev->angles = pev->angles;

		pEnt = UTIL_FindEntityByTargetname( pEnt, STRING( pev->target ));
	}
}

void CFuncRotate :: SetTargetOrigin( void )
{
	CBaseEntity *pEnt = UTIL_FindEntityByTargetname( gpWorld, STRING( pev->target ));

	while( pEnt != NULL )
	{
		pEnt->pev->velocity = g_vecZero;

		if( pEnt->m_iRotateType == OBJECT_MOVEWALL )
			UTIL_SetOrigin( pEnt->pev, pev->origin - oldorigin + (pEnt->neworigin - pEnt->oldorigin) );
                    else UTIL_SetOrigin( pEnt->pev, pEnt->neworigin + pev->origin );

		pEnt = UTIL_FindEntityByTargetname( pEnt, STRING( pev->target ));
	}
}

void CFuncRotate :: LinkRotateTargets( void )
{
	CBaseEntity *pEnt = UTIL_FindEntityByTargetname( gpWorld, STRING( pev->target ));

	oldorigin = pev->origin;

	while( pEnt != NULL )
	{
		if( FClassnameIs( pEnt->pev, "rotate_object" ))
		{
			pEnt->m_iRotateType = OBJECT_ROTATE;
			pEnt->oldorigin = pEnt->pev->origin - oldorigin;
			pEnt->neworigin = pEnt->pev->origin - oldorigin;
			pEnt->pev->owner = edict();
                    }
		else if( FClassnameIs( pEnt->pev, "func_movewall" ))
		{
			pEnt->m_iRotateType = OBJECT_MOVEWALL;
			pEnt->oldorigin = pEnt->Center() - oldorigin;
			pEnt->neworigin = pEnt->oldorigin;
			pEnt->pev->owner = edict();
		}
                    else
                    {
			pEnt->m_iRotateType = OBJECT_SETORIGIN;
			pEnt->oldorigin = pEnt->pev->origin - oldorigin;
			pEnt->neworigin = pEnt->pev->origin - oldorigin;
                    }

		pEnt = UTIL_FindEntityByTargetname( pEnt, STRING( pev->target ));
	}
}

void CFuncRotate :: SetDamageOnTargets( float flDmg )
{
	CBaseEntity *pEnt = UTIL_FindEntityByTargetname( gpWorld, STRING( pev->target ));

	while( pEnt != NULL )
	{
		if( FClassnameIs( pEnt->pev, "trigger_hurt" ))
		{
			// hurt_setdamage( float amount );
			pEnt->pev->dmg = flDmg;
			if( !flDmg ) pEnt->pev->solid = SOLID_NOT;
			else pEnt->pev->solid = SOLID_TRIGGER;
			pEnt->pev->nextthink = -1;
                    }
		else if( FClassnameIs( pEnt->pev, "func_movewall" ))
		{
			pEnt->pev->dmg = flDmg;
		}

		pEnt = UTIL_FindEntityByTargetname( pEnt, STRING( pev->target ));
	}
}

void CFuncRotate :: FirstThink( void )
{
	LinkRotateTargets();

	if( pev->spawnflags & SF_ROTATE_START_ON )
	{
		m_rotate_state = STATE_ACTIVE;
		pev->nextthink = gpGlobals->time + 0.01;
		pev->ltime = gpGlobals->time;
		SetThink( &CFuncRotate::NormalThink );
	}
	else
	{
		m_rotate_state = STATE_INACTIVE;
		SetThink( NULL );
	}
}

void CFuncRotate :: NormalThink( void )
{
	float t = gpGlobals->time - pev->ltime;
	pev->ltime = gpGlobals->time;

	if( m_rotate_state == STATE_SPEEDINGUP )
	{
		m_flCount += m_flCnt * t;

		if( m_flCount > 1.0f )
			m_flCount = 1.0f;

		// get rate of rotation
		t = t * m_flCount;
	}
	else if( m_rotate_state == STATE_SLOWINGDOWN )
	{
		m_flCount -= m_flCnt * t;

		if ( m_flCount < 0.0f )
		{
			m_rotate_state = STATE_INACTIVE;
			RotateTargetsFinal();
			SetThink( NULL );
			return;
		}

		// get rate of rotation
		t = t * m_flCount;
	}

	pev->angles += ( m_vecRotate * t );
	UTIL_NormalizeAngles( pev->angles );

	RotateTargets();
	pev->nextthink = gpGlobals->time + 0.01;
}

void CFuncRotate :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// change to alternate textures
	pev->frame = 1 - pev->frame;

	if( m_rotate_state == STATE_ACTIVE )
	{
		if( pev->spawnflags & SF_ROTATE_TOGGLE )
		{
			if( pev->speed )
			{
				m_flCount = 1.0f;
				m_rotate_state = STATE_SLOWINGDOWN;
			}
			else
			{
				m_rotate_state = STATE_INACTIVE;
				SetThink( NULL );
			}
		}
	}
	else if( m_rotate_state == STATE_INACTIVE )
	{
		SetThink( &CFuncRotate::NormalThink );
		pev->nextthink = gpGlobals->time + 0.01f;
		pev->ltime = gpGlobals->time;

		if( pev->speed )
		{
			m_flCount = 0.0f;
			m_rotate_state = STATE_SPEEDINGUP;
		}
		else
		{
			m_rotate_state = STATE_ACTIVE;
		}
	}
	else if( m_rotate_state == STATE_SPEEDINGUP )
	{
		if( pev->spawnflags & SF_ROTATE_TOGGLE )
		{
			m_rotate_state = STATE_SLOWINGDOWN;
		}
	}
	else
	{
		m_rotate_state = STATE_SPEEDINGUP;
	}
}

//************************************************
//
// Train with rotation functionality
//
//************************************************
/*QUAKED path_rotate (0.5 0.3 0) (-8 -8 -8) (8 8 8) ROTATION ANGLES STOP NO_ROTATE DAMAGE MOVETIME SET_DAMAGE
 Path for rotate_train.

 ROTATION tells train to rotate at rate specified by "rotate".  Use '0 0 0' to stop rotation.

 ANGLES tells train to rotate to the angles specified by "angles" while traveling to this path_rotate.  Use values < 0 or > 360 to guarantee that it turns in a certain direction.  Having this flag set automatically clears any rotation.

 STOP tells the train to stop and wait to be retriggered.

 NO_ROTATE tells the train to stop rotating when waiting to be triggered.

 DAMAGE tells the train to cause damage based on "dmg".

 MOVETIME tells the train to interpret "speed" as the length of time to take moving from one corner to another.

 SET_DAMAGE tells the train to set all targets damage to "dmg"

 "noise" contains the name of the sound to play when train stops.
 "noise1" contains the name of the sound to play when train moves.
 "event" is a target to trigger when train arrives at path_rotate.
*/
class CPathRotate : public CBaseEntity
{
public:
	void KeyValue( KeyValueData *pkvd );
	void Precache( void );
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( path_rotate, CPathRotate );

void CPathRotate :: Precache( void )
{
	if( pev->noise )
		PRECACHE_SOUND( (char *)STRING( pev->noise ));
	if( pev->noise1 )
		PRECACHE_SOUND( (char *)STRING( pev->noise1 ));
}

void CPathRotate :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		pev->frags = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "rotate"))
	{
		// HACKHACK: store rotate field into view_ofs
		UTIL_StringToVector( pev->view_ofs, pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CPathRotate :: Spawn( void )
{
	Precache();
}

/*QUAKED func_rotate_train (0 .5 .8) (-8 -8 -8) (8 8 8)
In path_rotate, set speed to be the new speed of the train after it reaches
the path change.  If speed is -1, the train will warp directly to the next
path change after the specified wait time.  If MOVETIME is set on the
path_rotate, the train to interprets "speed" as the length of time to
take moving from one corner to another.

"noise" contains the name of the sound to play when train stops.
"noise1" contains the name of the sound to play when train moves.
Both "noise" and "noise1" defaults depend upon "sounds" variable and
can be overridden by the "noise" and "noise1" variable in path_rotate.

Also in path_rotate, if STOP is set, the train will wait until it is
retriggered before moving on to the next goal.

Trains are moving platforms that players can ride.
"path" specifies the first path_rotate and is the starting position.
If the train is the target of a button or trigger, it will not begin moving until activated.
The func_rotate_train entity is the center of rotation of all objects targeted by it.

If "deathtype" is set with a string, this is the message that will appear when a player is killed by the train.

speed	default 100
dmg      default  0
sounds
1) ratchet metal
*/
class CFuncRotateTrain : public CFuncRotate
{
public:
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Precache( void );

	int m_sounds;
	Vector m_vecDest1;
	Vector m_vecDest2;
	Vector m_vecFinalDest;
	Vector m_vecFinalAngle;
	float m_flDuration;

	void (CFuncRotateTrain::*m_pfnThink2)(void);

	// callbacks
	void EXPORT TrainFind( void );
	void EXPORT TrainThink( void );
	void EXPORT TrainWait( void );
	void EXPORT TrainStop( void );
	void EXPORT TrainNext( void );
};

TYPEDESCRIPTION CFuncRotateTrain::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncRotateTrain, m_sounds, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncRotateTrain, m_vecDest1, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CFuncRotateTrain, m_vecDest2, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CFuncRotateTrain, m_vecFinalDest, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CFuncRotateTrain, m_flDuration, FIELD_TIME ),
	DEFINE_FIELD( CFuncRotateTrain, m_pfnThink2, FIELD_FUNCTION ),
	DEFINE_FIELD( CFuncRotateTrain, m_vecFinalAngle, FIELD_VECTOR ),
}; IMPLEMENT_SAVERESTORE( CFuncRotateTrain, CFuncRotate );

LINK_ENTITY_TO_CLASS( func_rotate_train, CFuncRotateTrain );

void CFuncRotateTrain :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deathtype"))
	{
		m_iDeathType = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "path"))
	{
		// alias for "path"
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CFuncRotate::KeyValue( pkvd );
	}
}

void CFuncRotateTrain :: Precache( void )
{
	PRECACHE_SOUND( (char *)STRING( pev->noise ));
	PRECACHE_SOUND( (char *)STRING( pev->noise1 ));
}

void CFuncRotateTrain :: Spawn( void )
{
	if( !pev->speed )
		pev->speed = 100;

	if( !pev->target )
	{
		ALERT( at_error, "rotate_train without a target\n" );
		REMOVE_ENTITY( edict() );
		return;
	}

	if( !pev->noise )
	{
		if( m_sounds == 0 )
			pev->noise = MAKE_STRING( "misc/null.wav" );
		else pev->noise = MAKE_STRING( "plats/train2.wav" );
	}

	if( !pev->noise1 )
	{
		if( m_sounds == 0 )
			pev->noise1 = MAKE_STRING( "misc/null.wav" );
		else pev->noise1 = MAKE_STRING( "plats/train1.wav" );
	}

	Precache ();

	m_flCnt = 1;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_STEP; // g-cont. ???

	UTIL_SetOrigin( pev, pev->origin );

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	pev->ltime = gpGlobals->time;
	pev->nextthink = pev->ltime + 0.1f;
	m_flEndTime = pev->ltime + 0.1f;
	SetThink( &CFuncRotateTrain::TrainThink );
	SetThink2( &CFuncRotateTrain::TrainFind );
	m_rotate_state = STATE_FIND;

	m_flDuration = 1.0f;	// 1 / duration
	m_flCnt = 0.1f;		// start time
	m_vecDest2 = g_vecZero;	// delta
	m_vecDest1 = pev->origin;	// original position

	pev->flags |= FL_ONGROUND;
}

void CFuncRotateTrain :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( m_pfnThink2 != &CFuncRotateTrain::TrainFind )
	{
		if( pev->velocity != g_vecZero )
			return; // already activated

		if( m_pfnThink2 )
			(this->*m_pfnThink2)();
	}
}

void CFuncRotateTrain :: TrainFind( void )
{
	m_rotate_state = STATE_FIND;
	LinkRotateTargets();

	// the first target is the point of rotation.
	// the second target is the path.
	edict_t *pTarget = FIND_ENTITY_BY_TARGETNAME ( NULL, STRING(pev->netname) );
	if( FNullEnt(pTarget) || !FClassnameIs( pTarget, "path_rotate" ))
	{
		ALERT( at_error, "Next target is not found or not path_rotate\n" );
		SetThink( NULL );
		return;
	}

	// Save the current entity
	m_pGoalEnt = CBaseEntity::Instance( pTarget );

	if( m_pGoalEnt->pev->spawnflags & SF_PATH_ANGLES )
	{
		pev->angles = m_vecFinalAngle = m_pGoalEnt->pev->angles;
		UTIL_NormalizeAngles( m_vecFinalAngle );
	}

	pev->netname = m_pGoalEnt->pev->target;
	UTIL_SetOrigin( pev, m_pGoalEnt->pev->origin );
	SetTargetOrigin();
	RotateTargetsFinal();
	SetThink2( &CFuncRotateTrain::TrainNext );

	if( !pev->targetname )
	{
		// not triggered, so start immediately
		m_flEndTime = pev->ltime + 0.1f;
	}
	else
	{
		m_flEndTime = 0.0f;
	}

	m_flDuration = 1.0f;	// 1 / duration
	m_flCnt = gpGlobals->time;	// start time
	m_vecDest2 = g_vecZero;	// delta
	m_vecDest1 = pev->origin;	// original position
}

void CFuncRotateTrain :: TrainThink( void )
{
	float t = gpGlobals->time - pev->ltime;
	pev->ltime = gpGlobals->time;

	if(( m_flEndTime ) && ( gpGlobals->time >= m_flEndTime ))
	{
		m_flEndTime = 0.0f;

		if( m_rotate_state == STATE_MOVE )
		{
			UTIL_SetOrigin( pev, m_vecFinalDest );
			pev->velocity = g_vecZero;
		}

		if( m_pfnThink2 )
			(this->*m_pfnThink2)();
	}
	else
	{
		float timeelapsed = (gpGlobals->time - m_flCnt) * m_flDuration;

		if( timeelapsed > 1.0f )
			timeelapsed = 1.0f;

		UTIL_SetOrigin( pev, m_vecDest1 + ( m_vecDest2 * timeelapsed ));
	}

	pev->angles += ( m_vecRotate * t );
	UTIL_NormalizeAngles( pev->angles );
	RotateTargets();

	pev->nextthink = gpGlobals->time + 0.01;
}

void CFuncRotateTrain :: TrainWait( void )
{
	m_rotate_state = STATE_WAIT;

	if( !m_pGoalEnt )
	{
		ALERT( at_error, "TrainWait: m_pGoalEnt == NULL!\n" );
		return;
	}

	if( m_pGoalEnt->pev->noise )
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( m_pGoalEnt->pev->noise ), 1, ATTN_NORM);
	}
	else
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise ), 1, ATTN_NORM);
	}

	if( m_pGoalEnt->pev->spawnflags & SF_PATH_ANGLES )
	{
		m_vecRotate = g_vecZero;
		pev->angles = m_vecFinalAngle;
	}

	if( m_pGoalEnt->pev->spawnflags & SF_PATH_NO_ROTATE )
	{
		m_vecRotate = g_vecZero;
	}

	m_flEndTime = pev->ltime + m_pGoalEnt->pev->frags;
	SetThink2( &CFuncRotateTrain::TrainNext );
}

void CFuncRotateTrain :: TrainStop( void )
{
	m_rotate_state = STATE_STOP;

	if( !m_pGoalEnt )
	{
		ALERT( at_error, "TrainStop: m_pGoalEnt == NULL!\n" );
		return;
	}

	if( m_pGoalEnt->pev->noise )
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( m_pGoalEnt->pev->noise ), 1, ATTN_NORM);
	}
	else
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise ), 1, ATTN_NORM);
	}

	if( m_pGoalEnt->pev->spawnflags & SF_PATH_ANGLES )
	{
		m_vecRotate = g_vecZero;
		pev->angles = m_vecFinalAngle;
	}

	if( m_pGoalEnt->pev->spawnflags & SF_PATH_NO_ROTATE )
	{
		m_vecRotate = g_vecZero;
	}

	pev->dmg = 0;
	SetThink2( &CFuncRotateTrain::TrainNext );
}

void CFuncRotateTrain :: TrainNext( void )
{
	m_rotate_state = STATE_NEXT;

	CBaseEntity *pCurrent = m_pGoalEnt;

	edict_t *pTarget = FIND_ENTITY_BY_TARGETNAME ( NULL, STRING(pev->netname) );
	if( FNullEnt(pTarget) || !FClassnameIs( pTarget, "path_rotate" ))
	{
		ALERT( at_error, "Next target is not found or not path_rotate\n" );
		SetThink( NULL );
		return;
	}

	// copy from path_rotate
	if( m_pGoalEnt->pev->noise1 )
		pev->noise1 = m_pGoalEnt->pev->noise1;

	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise1 ), 1, ATTN_NORM);

	// Save the current entity
	m_pGoalEnt = CBaseEntity::Instance( pTarget );
	pev->netname = m_pGoalEnt->pev->target;

	if( !pev->netname )
	{
		ALERT( at_error, "TrainNext: no next target\n" );
		SetThink( NULL );
		return;
	}

	if( m_pGoalEnt->pev->spawnflags & SF_PATH_STOP )
	{
		SetThink2( &CFuncRotateTrain::TrainStop );
	}
	else if( m_pGoalEnt->pev->frags )
	{
		SetThink2( &CFuncRotateTrain::TrainWait );
	}
	else
	{
		SetThink2( &CFuncRotateTrain::TrainNext );
	}

	if( pCurrent->pev->netname )
	{
		if( pCurrent->pev->message )
		{
			CenterPrint( UTIL_PlayerByIndex( 1 )->pev, STRING( pCurrent->pev->message ));
			pCurrent->pev->message = iStringNull;
		}

		// Trigger any events that should happen at the corner.
		FireTargets( STRING(pCurrent->pev->netname), pCurrent, this, USE_TOGGLE, 0.0f );
	}

	if( pCurrent->pev->spawnflags & SF_PATH_ANGLES )
	{
		m_vecRotate = g_vecZero;
		pev->angles = m_vecFinalAngle;
	}

	if( pCurrent->pev->spawnflags & SF_PATH_ROTATION )
	{
		m_vecRotate = pCurrent->pev->view_ofs;
	}

	if( pCurrent->pev->spawnflags & SF_PATH_DAMAGE )
	{
		pev->dmg = pCurrent->pev->dmg;
	}

	if( pCurrent->pev->spawnflags & SF_PATH_SET_DAMAGE )
	{
		SetDamageOnTargets( pCurrent->pev->dmg );
	}

	if( pCurrent->pev->speed == -1 )
	{
		// Warp to the next path_corner
		UTIL_SetOrigin( pev, m_pGoalEnt->pev->origin );
		m_flEndTime = pev->ltime + 0.01f;
		SetTargetOrigin();

		if( m_pGoalEnt->pev->spawnflags & SF_PATH_ANGLES )
		{
			pev->angles = m_pGoalEnt->pev->angles;
		}

		m_flDuration = 1.0f;	// 1.0f / duration
		m_flCnt = gpGlobals->time;	// start time
		m_vecDest2 = g_vecZero;	// delta
		m_vecDest1 = pev->origin;	// original position
		m_vecFinalDest = pev->origin;
	}
	else
	{
		m_rotate_state = STATE_MOVE;
		m_vecFinalDest = m_pGoalEnt->pev->origin;

		// already there?
		if( m_vecFinalDest == pev->origin )
		{
			pev->velocity = g_vecZero;
			m_flEndTime = pev->ltime + 0.1f;

			m_flDuration = 1.0f;	// 1 / duration
			m_flCnt = gpGlobals->time;	// start time
			m_vecDest2 = g_vecZero;	// delta
			m_vecDest1 = pev->origin;	// original position
			m_vecFinalDest = pev->origin;
			return;
		}

		// set destdelta to the vector needed to move
		Vector vDestDelta = m_vecFinalDest - pev->origin;

		// calculate length of vector
		float len = vDestDelta.Length();
		float traveltime;

		if( pCurrent->pev->spawnflags & SF_PATH_MOVETIME )
		{
			traveltime = pCurrent->pev->speed;
		}
		else
		{
			// check if there's a speed change
			if( pCurrent->pev->speed > 0.0f )
				pev->speed = pCurrent->pev->speed;

			if( !pev->speed )
			{
				ALERT( at_error, "No speed is defined!\n" );
				SetThink( NULL );
				return;
			}

			// divide by speed to get time to reach dest
			traveltime = len / pev->speed;
		}

		if( traveltime < 0.1f )
		{
			pev->velocity = g_vecZero;
			m_flEndTime = pev->ltime + 0.1f;

			if( m_pGoalEnt->pev->spawnflags & SF_PATH_ANGLES )
			{
				pev->angles = m_pGoalEnt->pev->angles;
			}
			return;
		}

		// qcc won't take vec/float
		float div = 1.0f / traveltime;

		if( m_pGoalEnt->pev->spawnflags & SF_PATH_ANGLES )
		{
			m_vecFinalAngle = m_pGoalEnt->pev->angles;
			UTIL_NormalizeAngles( m_vecFinalAngle );
			m_vecRotate = ( m_pGoalEnt->pev->angles - pev->angles ) * div;
		}

		// set endtime to trigger a think when dest is reached
		m_flEndTime = pev->ltime + traveltime;

		// scale the destdelta vector by the time spent traveling to get velocity
		pev->velocity = vDestDelta * div;

		m_flDuration = div;      	// 1.0f / duration
		m_flCnt = gpGlobals->time;	// start time
		m_vecDest2 = vDestDelta;	// delta
		m_vecDest1 = pev->origin;	// original position
	}
}

//************************************************
//
// Rotating doors
//
//************************************************

/*QUAKED func_rotate_door (0 .5 .8) (-8 -8 -8) (8 8 8) STAYOPEN
Creates a door that rotates between two positions around a point of
rotation each time it's triggered.

STAYOPEN tells the door to reopen after closing.  This prevents a trigger-
once door from closing again when it's blocked.

"dmg" specifies the damage to cause when blocked.  Defaults to 2.  Negative numbers indicate no damage.
"speed" specifies how the time it takes to rotate

"sounds"
1) medieval (default)
2) metal
3) base
*/
class CFuncRotateDoor : public CFuncRotate
{
public:
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	void EXPORT DoorThink1( void );
	void EXPORT DoorThink2( void );

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );
	void Precache( void );
	void Spawn( void );

	void ReverseDirection( void );
	void GroupReverseDirection( void );

	Vector m_vecAngle1;
	Vector m_vecAngle2;
	Vector m_vecDestAngle;
	int m_sounds;
};

TYPEDESCRIPTION CFuncRotateDoor::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncRotateDoor, m_sounds, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncRotateDoor, m_vecAngle1, FIELD_VECTOR ),
	DEFINE_FIELD( CFuncRotateDoor, m_vecAngle2, FIELD_VECTOR ),
	DEFINE_FIELD( CFuncRotateDoor, m_vecDestAngle, FIELD_VECTOR ),
}; IMPLEMENT_SAVERESTORE( CFuncRotateDoor, CFuncRotate );

LINK_ENTITY_TO_CLASS( func_rotate_door, CFuncRotateDoor );

void CFuncRotateDoor :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "group"))
	{
		// store into entvars to allow engine search by string
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CFuncRotate::KeyValue( pkvd );
	}
}

void CFuncRotateDoor :: Precache( void )
{
	// set the door's "in-motion" sound
	switch (m_sounds)
	{
	default:
	case 1:
		PRECACHE_SOUND ("doors/latch2.wav");
		PRECACHE_SOUND ("doors/winch2.wav");
		PRECACHE_SOUND ("doors/drclos4.wav");
		pev->noise = MAKE_STRING("doors/latch2.wav");
		pev->noise1 = MAKE_STRING("doors/winch2.wav");
		pev->noise2 = MAKE_STRING("doors/drclos4.wav");
		break;
	case 2:
		PRECACHE_SOUND ("doors/airdoor1.wav");
		PRECACHE_SOUND ("doors/airdoor2.wav");
		pev->noise = MAKE_STRING("doors/airdoor2.wav");
		pev->noise1 = MAKE_STRING("doors/airdoor1.wav");
		pev->noise2 = MAKE_STRING("doors/airdoor2.wav");
		break;
	case 3:
		PRECACHE_SOUND ("doors/basesec1.wav");
		PRECACHE_SOUND ("doors/basesec2.wav");
		pev->noise = MAKE_STRING("doors/basesec2.wav");
		pev->noise1 = MAKE_STRING("doors/basesec1.wav");
		pev->noise2 = MAKE_STRING("doors/basesec2.wav");
		break;
	}
}

void CFuncRotateDoor :: Spawn( void )
{
	if( !pev->target )
	{
		ALERT( at_error, "rotate_door without target.\n" );
		REMOVE_ENTITY( edict() );
		return;
	}

	m_vecAngle1 = g_vecZero;
	m_vecAngle2 = pev->angles;
	pev->angles = m_vecAngle1;

	// default to 2 seconds
	if( !pev->speed )
		pev->speed = 2;

	m_flCnt = 0;

	if( !pev->dmg )
	{
		pev->dmg = 2;
	}
	else if( pev->dmg < 0 )
	{
		pev->dmg = 0;
	}

	if (!m_sounds)
		m_sounds = 1;

	Precache();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	UTIL_SetOrigin( pev, pev->origin );
	m_rotate_state = STATE_CLOSED;
}

void CFuncRotateDoor :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if(( m_rotate_state != STATE_OPEN ) && ( m_rotate_state != STATE_CLOSED ))
		return;	// in-moving

	if( !m_flCnt )
	{
		m_flCnt = 1.0f;
		LinkRotateTargets();
	}

	Vector start;

	// change to alternate textures
	pev->frame = 1 - pev->frame;

	if( m_rotate_state == STATE_CLOSED )
	{
		start = m_vecAngle1;
		m_vecDestAngle = m_vecAngle2;
		m_rotate_state = STATE_OPENING;
	}
	else
	{
		start = m_vecAngle2;
		m_vecDestAngle = m_vecAngle1;
		m_rotate_state = STATE_CLOSING;
	}

	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise1 ), 1, ATTN_NORM);

	m_vecRotate = ( m_vecDestAngle - start ) * ( 1.0f / pev->speed );
	SetThink( &CFuncRotateDoor::DoorThink1 );

	pev->nextthink = gpGlobals->time + 0.01f;
	m_flEndTime = gpGlobals->time + pev->speed;
	pev->ltime = gpGlobals->time;
}

void CFuncRotateDoor :: DoorThink1( void )
{
	float t = gpGlobals->time - pev->ltime;
	pev->ltime = gpGlobals->time;

	if( gpGlobals->time < m_flEndTime )
	{
		pev->angles += ( m_vecRotate * t );
		RotateTargets();
	}
	else
	{
		pev->angles = m_vecDestAngle;
		RotateTargets();
		SetThink( &CFuncRotateDoor::DoorThink2 );
	}

	pev->nextthink = gpGlobals->time + 0.01f;
}

void CFuncRotateDoor :: DoorThink2( void )
{
	float t = gpGlobals->time - pev->ltime;
	pev->ltime = gpGlobals->time;

	// change to alternate textures
	pev->frame = 1 - pev->frame;

	pev->angles = m_vecDestAngle;

	if( m_rotate_state == STATE_OPENING )
	{
		m_rotate_state = STATE_OPEN;
	}
	else
	{
		if( pev->spawnflags & SF_DOOR_STAYOPEN )
		{
			GroupReverseDirection();
			return;
		}
		m_rotate_state = STATE_CLOSED;
	}

	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise2 ), 1, ATTN_NORM);
	SetThink( NULL );

	RotateTargetsFinal();
}

void CFuncRotateDoor :: ReverseDirection( void )
{
	Vector start;

	// change to alternate textures
	pev->frame = 1 - pev->frame;

	if( m_rotate_state == STATE_CLOSING )
	{
		start = m_vecAngle1;
		m_vecDestAngle = m_vecAngle2;
		m_rotate_state = STATE_OPENING;
	}
	else
	{
		start = m_vecAngle2;
		m_vecDestAngle = m_vecAngle1;
		m_rotate_state = STATE_CLOSING;
	}

	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING( pev->noise1 ), 1, ATTN_NORM);

	m_vecRotate = ( m_vecDestAngle - start ) * ( 1.0f / pev->speed );
	SetThink( &CFuncRotateDoor::DoorThink1 );

	pev->nextthink = gpGlobals->time + 0.01;
	m_flEndTime = gpGlobals->time + pev->speed - ( m_flEndTime - gpGlobals->time );
	pev->ltime = gpGlobals->time;
}

void CFuncRotateDoor :: GroupReverseDirection( void )
{
	// tell all associated rotaters to reverse direction
	if( pev->netname )
	{
		CBaseEntity *pEnt = UTIL_FindEntityByString( gpWorld, "netname", STRING( pev->netname ));

		while( pEnt != NULL )
		{
			if( FClassnameIs( pEnt->pev, "func_rotate_door" ))
			{
				CFuncRotateDoor *pRotDoor = (CFuncRotateDoor *)pEnt;
				pRotDoor->ReverseDirection();
                              }
			pEnt = UTIL_FindEntityByString( pEnt, "netname", STRING( pev->netname ));
		}
	}
	else
	{
		ReverseDirection();
	}
}

//************************************************
//
// Moving clip walls
//
//************************************************

/*QUAKED func_movewall (0 .5 .8) ? VISIBLE TOUCH NONBLOCKING
Used to emulate collision on rotating objects.

VISIBLE causes brush to be displayed.

TOUCH specifies whether to cause damage when touched by player.

NONBLOCKING makes the brush non-solid.  This is useless if VISIBLE is set.

"dmg" specifies the damage to cause when touched or blocked.
*/
class CMoveWall : public CBaseEntity
{
public:
	void EXPORT WallTouch( CBaseEntity *pOther );
	void EXPORT WallBlocked( CBaseEntity *pOther );
	void Think( void );
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( func_movewall, CMoveWall );

void CMoveWall :: Spawn( void )
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH;

	if( pev->spawnflags & SF_MOVEWALL_NONBLOCKING )
	{
		pev->solid = SOLID_NOT;
	}
	else
	{
		pev->solid = SOLID_BSP;
		SetBlocked( &CMoveWall::WallBlocked );
	}

	if( pev->spawnflags & SF_MOVEWALL_TOUCH )
	{
		SetTouch( &CMoveWall::WallTouch );
	}

	SET_MODEL( ENT(pev), STRING(pev->model) );

	if( !FBitSet( pev->spawnflags, SF_MOVEWALL_VISIBLE ))
	{
		pev->effects |= EF_NODRAW;
	}

	pev->nextthink = gpGlobals->time + 0.01f;
	pev->ltime = gpGlobals->time;
}

void CMoveWall :: WallTouch( CBaseEntity *pOther )
{
	CBaseEntity *pOwner = CBaseEntity::Instance( pev->owner );

	if( !pOwner || gpGlobals->time < pOwner->m_flAttackFinished )
		return;

	if( pev->dmg )
	{
		pOther->TakeDamage( pev, pOwner->pev, pev->dmg, DMG_GENERIC );
		pOwner->m_flAttackFinished = gpGlobals->time + 0.5f;
	}
	else if( pOwner->pev->dmg )
	{
		pOther->TakeDamage( pev, pOwner->pev, pOwner->pev->dmg, DMG_GENERIC );
		pOwner->m_flAttackFinished = gpGlobals->time + 0.5f;
	}
}

void CMoveWall :: WallBlocked( CBaseEntity *pOther )
{
	CBaseEntity *pOwner = CBaseEntity::Instance( pev->owner );

	if( !pOwner || gpGlobals->time < pOwner->m_flAttackFinished )
		return;

	if( FClassnameIs( pOwner->pev, "func_rotate_door" ))
	{
		CFuncRotateDoor *pRotDoor = (CFuncRotateDoor *)pOwner;
		pRotDoor->GroupReverseDirection();
	}

	if( pev->dmg )
	{
		pOther->TakeDamage( pev, pOwner->pev, pev->dmg, DMG_GENERIC );
		pOwner->m_flAttackFinished = gpGlobals->time + 0.5f;
	}
	else if( pOwner->pev->dmg )
	{
		pOther->TakeDamage( pev, pOwner->pev, pOwner->pev->dmg, DMG_GENERIC );
		pOwner->m_flAttackFinished = gpGlobals->time + 0.5f;
	}
}

void CMoveWall :: Think( void )
{
	// g-cont. keep times an actual to allow wall movement
	pev->ltime = gpGlobals->time;
	pev->nextthink = gpGlobals->time + 0.01f;
}

/*QUAKED rotate_object (0 .5 .8) ?
This defines an object to be rotated.  Used as the target of func_rotate_door.
*/
class CRotateObject : public CBaseEntity
{
public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( rotate_object, CRotateObject );
LINK_ENTITY_TO_CLASS( rotate_train, CRotateObject );	// just an alias of rotate_object

void CRotateObject :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	SET_MODEL( ENT(pev), STRING(pev->model) );
	UTIL_SetSize( pev, pev->mins, pev->maxs );
}

/*QUAKED func_clock (0 0 0.5) (0 0 0) (32 32 32)
Creates one hand of a "clock".

Set the angle to be the direction the clock is facing.

"event" is the targetname of the entities to trigger when hand strikes 12.
"cnt" is the time to start at.
"count" is the # of seconds it takes to make a full revolution (seconds is 60, minutes 3600, hours 43200).  default is 60.
*/
class CFuncClock : public CFuncRotate
{
public:
	void EXPORT FirstThink( void );	// link rotate targets etc
	void EXPORT ClockThink( void );

	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( func_clock, CFuncClock );

void CFuncClock :: Spawn( void )
{
	SetMovedir( pev );

	Vector temp = pev->movedir;
	pev->movedir.x = 0.0f - temp.y;
	pev->movedir.y = 0.0f - temp.z;
	pev->movedir.z = 0.0f - temp.x;

	if( !m_flCount ) m_flCount = 60;
	m_flCnt *= ( m_flCount / 12.0f );

	SetThink( &CFuncClock::FirstThink );
	pev->nextthink = gpGlobals->time + 0.1;
	pev->ltime = gpGlobals->time;
}

void CFuncClock :: FirstThink( void )
{
	LinkRotateTargets();
	SetThink( &CFuncClock::ClockThink );

	ClockThink();
}

void CFuncClock :: ClockThink( void )
{
	// How much time has elapsed.
	float seconds = gpGlobals->time + m_flCnt;

	// divide by time it takes for one revolution
	float pos = seconds / m_flCount;

	// chop off non-fractional component
	pos = pos - floor( pos );

	float ang = 360.0f * pos;

	if( pev->netname && ( pev->ltime > ang ))
	{
		// past twelve
		FireTargets( STRING(pev->netname), this, this, USE_TOGGLE, 0.0f );
	}

	pev->angles = pev->movedir * ang;
	RotateTargetsFinal();

	pev->nextthink = gpGlobals->time + 1.0f;
	pev->ltime = ang;
}
