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
#ifdef HIPNOTIC
#include "client.h"
#endif /* HIPNOTIC */
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"
#include "shake.h"
#include "decals.h"
#include "doors.h"

extern DLL_GLOBAL BOOL		g_fXashEngine;

/*QUAKED misc_fireball (0 .5 .8) (-8 -8 -8) (8 8 8)
Lava Balls
*/
class CFireBallSource : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	Think( void );
};

class CFireBall : public CBaseEntity
{
public:
	void EXPORT LavaTouch( CBaseEntity *pOther )
	{
		if( pev->waterlevel > 0 )
		{
			UTIL_Remove( this );
			return;
		}

		pOther->TakeDamage( pev, pev, 20, DMG_BURN );

		TraceResult tr = UTIL_GetGlobalTrace();

		if( pOther->pev->solid != SOLID_BSP )
		{
			UTIL_Remove( this );
			return;
		}

		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 255;
		pev->pain_finished = gpGlobals->time;

		pev->vuser1 = tr.vecPlaneNormal * -1;
		UTIL_DecalTrace( &tr, DECAL_SCORCH1 + RANDOM_LONG( 0, 1 ));

		SetThink( &CFireBall::DieThink );
		SetTouch( NULL );

		pev->renderfx = kRenderLavaDeform;
		pev->movetype = MOVETYPE_NONE;

		pev->nextthink = gpGlobals->time + 0.001;
		pev->animtime = gpGlobals->time + 0.1;
	}

	void EXPORT DieThink( void )
	{
		float flDegree = (gpGlobals->time - pev->pain_finished) / 1.0f;

		pev->renderamt = 255 - (255 * flDegree);
		pev->nextthink = gpGlobals->time + 0.001;

		if( pev->renderamt <= 200 ) pev->effects &= ~EF_FULLBRIGHT;

		if( pev->renderamt <= 0 ) UTIL_Remove( this );
	}
};

LINK_ENTITY_TO_CLASS( misc_fireball, CFireBallSource );
LINK_ENTITY_TO_CLASS( fireball, CFireBall );

void CFireBallSource :: Precache( void )
{
	PRECACHE_MODEL ("models/lavaball.mdl");
}

void CFireBallSource :: Spawn( void )
{
	Precache ();

	if (!pev->speed)
		pev->speed = 1000;
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 5.0 );
}

void CFireBallSource :: Think( void )
{
	CFireBall *pFireBall = GetClassPtr( (CFireBall *)NULL );

	pFireBall->pev->solid = SOLID_TRIGGER;
	pFireBall->pev->movetype = MOVETYPE_TOSS;
	pFireBall->pev->velocity.x = RANDOM_FLOAT( -50, 50 );
	pFireBall->pev->velocity.y = RANDOM_FLOAT( -50, 50 );
	pFireBall->pev->velocity.z = pev->speed + RANDOM_FLOAT( 0, 200 );
	pFireBall->pev->classname = MAKE_STRING( "fireball" );
	pFireBall->pev->avelocity.x = RANDOM_FLOAT( -50, 50 );
	pFireBall->pev->avelocity.y = RANDOM_FLOAT( -50, 50 );
	pFireBall->pev->avelocity.z = RANDOM_FLOAT( -50, 50 );
	pFireBall->SetTouch( &CFireBall::LavaTouch );
	pFireBall->pev->vuser1 = Vector( 1, 1, 1 );

	if( g_fXashEngine )
	{
		// NOTE: this has effect only in Xash3D
		pFireBall->pev->effects = EF_FULLBRIGHT;
	}

	SET_MODEL( ENT(pFireBall->pev), "models/lavaball.mdl" );
	UTIL_SetOrigin( pFireBall->pev, pev->origin );

	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 3, 8 );
}

/*QUAKED misc_explobox (0 .5 .8) (0 0 0) (32 32 64)
TESTING THING
*/
class CExploBox : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Precache( void );
	void EXPORT FallInit( void );
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	virtual void Killed( entvars_t *pevAttacker, int iGib );

	int	m_idShard;
};

LINK_ENTITY_TO_CLASS( misc_explobox, CExploBox );
LINK_ENTITY_TO_CLASS( misc_explobox2, CExploBox );

void CExploBox :: Precache( void )
{
	if( FClassnameIs( pev, "misc_explobox2" ))
		PRECACHE_MODEL ("models/b_exbox2.bsp");
	else PRECACHE_MODEL ("models/b_explob.bsp");

	m_idShard = PRECACHE_MODEL( "models/crategib.mdl" );
}

void CExploBox :: Spawn( void )
{
	Precache();

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_FLY;
	pev->origin.z += 6;	// quake code

	if( FClassnameIs( pev, "misc_explobox2" ))
		SET_MODEL( ENT(pev), "models/b_exbox2.bsp" );
	else SET_MODEL( ENT(pev), "models/b_explob.bsp" );

	pev->health = 20;
	pev->takedamage = DAMAGE_AIM;

	SetThink( &CExploBox::FallInit );
	if (g_fXashEngine)
		pev->nextthink = gpGlobals->time + 0.2;
	else pev->nextthink = gpGlobals->time + 1.0; // make sure what client is changed hulls
}

void CExploBox :: FallInit( void )
{
	if (UTIL_DropToFloor(this) == 0)
	{
		ALERT( at_error, "Item %s fell out of level at %f,%f,%f\n", STRING( pev->classname ), pev->origin.x, pev->origin.y, pev->origin.z );
		UTIL_Remove( this );
		return;
	}

	pev->movetype = MOVETYPE_PUSHSTEP;
	UTIL_SetOrigin( pev, pev->origin );
}

int CExploBox :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if (!pev->takedamage)
		return 0;

	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		Killed( pevAttacker, GIB_NORMAL );
		return 0;
	}

	return 1;
}

void CExploBox :: Killed( entvars_t *pevAttacker, int iGib )
{
	pev->takedamage = DAMAGE_NO;

	Vector vecSrc = pev->origin + Vector( 0, 0, 32 );

	Q_RadiusDamage(this, this, 160, gpWorld);

	UTIL_ScreenShake( pev->origin, 16.0f, 4.0f, 0.8f, 500.0f );

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_EXPLOSION );
		WRITE_COORD( vecSrc.x );
		WRITE_COORD( vecSrc.y );
		WRITE_COORD( vecSrc.z );
	MESSAGE_END();

	// spawn gibs
	Vector vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSpot );
		WRITE_BYTE( TE_BREAKMODEL );

		// position
		WRITE_COORD( vecSpot.x );
		WRITE_COORD( vecSpot.y );
		WRITE_COORD( vecSpot.z );

		// size
		WRITE_COORD( pev->size.x );
		WRITE_COORD( pev->size.y );
		WRITE_COORD( pev->size.z );

		// velocity
		WRITE_COORD( 0 ); 
		WRITE_COORD( 0 );
		WRITE_COORD( 0 );

		// randomization
		WRITE_BYTE( 10 ); 

		// Model
		WRITE_SHORT( m_idShard );	//model id#

		// # of shards
		WRITE_BYTE( 0 );	// let client decide

		// duration
		WRITE_BYTE( 50 );	// 5 seconds

		// flags
		WRITE_BYTE( BREAK_METAL );
	MESSAGE_END();

	SetThink( &CExploBox::SUB_Remove );
	pev->nextthink = pev->nextthink + 0.1;
}

class CBubble : public CBaseEntity
{
public:
	void Touch( CBaseEntity *pOther );
	void Think( void );
	void Split( void );
	void Spawn( void );
};

void CBubble :: Spawn( void )
{
	pev->classname = MAKE_STRING( "bubble" );

	SET_MODEL(ENT(pev), "sprites/s_bubble.spr" );
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize( pev, Vector( -8, -8, -8 ), Vector( 8, 8, 8 ));

	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;
	pev->velocity = Vector( 0, 0, 15 );
	pev->frame = 0;

	pev->nextthink = gpGlobals->time + 0.5;
}

void CBubble :: Touch( CBaseEntity *pOther )
{
	if( FStrEq( STRING( pOther->pev->classname ), STRING( pev->classname )))
		return;	// touch another bubble
	UTIL_Remove( this );
}

void CBubble :: Split( void )
{
	CBubble *pBubble = GetClassPtr( (CBubble *)NULL);

	pBubble->pev->origin = pev->origin;
	pBubble->Spawn();
	pBubble->pev->velocity = pev->velocity;
	pBubble->pev->frame = 1;
	pBubble->pev->impulse = 10;

	// split one big bubble on two half-sized
	pev->frame = 1;
	pev->impulse = 10;
}

void CBubble :: Think( void )
{
	float	rnd1, rnd2, rnd3;

	pev->impulse++;

	if( pev->impulse == 4 )
		Split();

	if( pev->impulse >= 20 || pev->waterlevel != 3 )
	{
		// lifetime expired
		UTIL_Remove( this );
		return;
	}

	rnd1 = pev->velocity.x + RANDOM_FLOAT( -10, 10 );
	rnd2 = pev->velocity.y + RANDOM_FLOAT( -10, 10 );
	rnd3 = pev->velocity.z + RANDOM_FLOAT( 10, 20 );

	if( rnd1 > 10 ) rnd1 = 5;
	if( rnd1 < -10 ) rnd1 = -5;
		
	if( rnd2 > 10 ) rnd2 = 5;
	if( rnd2 < -10 ) rnd2 = -5;
		
	if( rnd3 < 10 ) rnd3 = 15;
	if( rnd3 > 30 ) rnd3 = 25;
	
	pev->velocity.x = rnd1;
	pev->velocity.y = rnd2;
	pev->velocity.z = rnd3;
		
	pev->nextthink = gpGlobals->time + 0.5;
}

LINK_ENTITY_TO_CLASS( bubble, CBubble );

/*QUAKED air_bubbles (0 .5 .8) (-8 -8 -8) (8 8 8)

testing air bubbles
*/
LINK_ENTITY_TO_CLASS( air_bubbles, CBubbleSource );

void CBubbleSource :: Precache( void )
{
	PRECACHE_MODEL ("sprites/s_bubble.spr");
}

void CBubbleSource :: Spawn( void )
{
	if( g_pGameRules->IsDeathmatch())
	{
		// g-cont. how many traffic requires this pretty small bubbles???
		REMOVE_ENTITY( ENT(pev));
		return;
	}

	Precache ();

	pev->nextthink = gpGlobals->time + (pev->button) ? 0.1 : 1;
}

void CBubbleSource :: Think( void )
{
	CBubble *pBubble = GetClassPtr( (CBubble *)NULL);

	pBubble->pev->origin = pev->origin;
	pBubble->Spawn();

	if( pev->button )
	{
		if( ++pev->impulse >= pev->air_finished )
		{
			REMOVE_ENTITY( edict( ));
			return;
		}
	}

	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.5, 1.5 );
}

#define SF_SUPERSPIKE	1
#define SF_LASER		2
#ifdef HIPNOTIC
#define SF_LAVABALL		4
#define SF_ROCKET		8
#endif /* HIPNOTIC */

class CSpikeShooter : public CBaseToggle
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	Think( void );
#ifdef HIPNOTIC
	void	KeyValue( KeyValueData *pkvd );
#endif /* HIPNOTIC */
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

#ifndef HIPNOTIC
/*QUAKED trap_shooter (0 .5 .8) (-8 -8 -8) (8 8 8) superspike laser
Continuously fires spikes.
"wait" time between spike (1.0 default)
"nextthink" delay before firing first spike, so multiple shooters can be stagered.
*/
#else /* HIPNOTIC */
/*QUAKED trap_shooter (0 .5 .8) (-8 -8 -8) (8 8 8) superspike laser lavaball rocket silent
Continuously fires spikes.
"wait" time between spike (1.0 default)
"nextthink" delay before firing first spike, so multiple shooters can be stagered.
*/
#endif /* HIPNOTIC */
LINK_ENTITY_TO_CLASS( trap_shooter, CSpikeShooter );

#ifndef HIPNOTIC
/*QUAKED trap_spikeshooter (0 .5 .8) (-8 -8 -8) (8 8 8) superspike laser
When triggered, fires a spike in the direction set in QuakeEd.
Laser is only for REGISTERED.
*/
#else /* HIPNOTIC */
/*QUAKED trap_spikeshooter (0 .5 .8) (-8 -8 -8) (8 8 8) superspike laser lavaball rocket silent
When triggered, fires a spike in the direction set in QuakeEd.
Laser is only for REGISTERED.
*/
#endif /* HIPNOTIC */
LINK_ENTITY_TO_CLASS( trap_spikeshooter, CSpikeShooter );

#ifdef HIPNOTIC
/*QUAKED trap_switched_shooter (0 .5 .8) (-8 -8 -8) (8 8 8) superspike laser lavaball rocket silent
Continuously fires spikes.
"wait" time between spike (1.0 default)
"nextthink" delay before firing first spike, so multiple shooters can be stagered.
"state" 0 initially off, 1 initially on. (0 default)
*/
LINK_ENTITY_TO_CLASS( trap_switched_shooter, CSpikeShooter );

void CSpikeShooter :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "state"))
	{
		pev->button = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}
#endif /* HIPNOTIC */

void CSpikeShooter::Precache( void )
{
	if (pev->spawnflags & SF_LASER)
	{
		PRECACHE_MODEL ("models/laser.mdl");
		PRECACHE_SOUND ("enforcer/enfire.wav");
		PRECACHE_SOUND ("enforcer/enfstop.wav");
	}
#ifdef HIPNOTIC
	else if (pev->spawnflags & SF_LAVABALL)
	{
		PRECACHE_MODEL ("models/lavarock.mdl");
		PRECACHE_SOUND ("misc/spike.wav");
	}
	else if (pev->spawnflags & SF_ROCKET)
	{
		PRECACHE_MODEL ("models/missile.mdl");
		PRECACHE_SOUND ("weapons/sgun1.wav");
	}
	else
		PRECACHE_SOUND ("weapons/spike2.wav");
#endif /* HIPNOTIC */
}

void CSpikeShooter::Spawn( void )
{
	Precache ();

	SetMovedir(pev);

#ifndef HIPNOTIC
	if (FClassnameIs( pev, "trap_shooter" ))
#else /* HIPNOTIC */
	if (FClassnameIs( pev, "trap_shooter" ) || FClassnameIs( pev, "trap_switched_shooter" ))
#endif /* HIPNOTIC */
	{
		if (m_flWait == 0.0f) m_flWait = 1;
		pev->nextthink = gpGlobals->time + m_flWait;
#ifdef HIPNOTIC

		if( FClassnameIs( pev, "trap_shooter" ))
			pev->button = TRUE;
#endif /* HIPNOTIC */
	}
}

void CSpikeShooter::Think( void )
{
#ifndef HIPNOTIC
	Use( this, this, USE_TOGGLE, 0 );
#else /* HIPNOTIC */
	if( pev->button )
		Use( this, this, USE_TOGGLE, 0 );
#endif /* HIPNOTIC */
	pev->nextthink = gpGlobals->time + m_flWait;
}

void CSpikeShooter::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
#ifdef HIPNOTIC
	if( FClassnameIs( pev, "trap_switched_shooter" ))
	{
		pev->button = !pev->button;
		return;
	}

#endif /* HIPNOTIC */
	if (pev->spawnflags & SF_LASER)
	{
		CLaser *pLaser;

#ifndef HIPNOTIC
		EMIT_SOUND( edict(), CHAN_VOICE, "enforcer/enfire.wav", 1, ATTN_NORM );
#else /* HIPNOTIC */
		if( !FBitSet( pev->spawnflags, SF_TRAP_SILENT ))
			EMIT_SOUND( edict(), CHAN_VOICE, "enforcer/enfire.wav", 1, ATTN_NORM );
#endif /* HIPNOTIC */

		pLaser = CLaser::LaunchLaser( pev->origin, pev->movedir, this );
	}
#ifdef HIPNOTIC
	else if (pev->spawnflags & SF_LAVABALL)
	{
		if( !FBitSet( pev->spawnflags, SF_TRAP_SILENT ))
			EMIT_SOUND( edict(), CHAN_VOICE, "misc/spike.wav", 1, ATTN_NORM );

		CRocket *pLava = CRocket::CreateRocket( pev->origin, pev->movedir, this );

		if( !pLava ) return;
		SET_MODEL(ENT(pLava->pev), "models/lavarock.mdl" );
		UTIL_SetSize( pLava->pev, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ));
		pLava->pev->velocity = pev->movedir * 300.0f; // set lavaball speed
		pLava->pev->avelocity = Vector( 0, 0, 400 );
	}
	else if (pev->spawnflags & SF_ROCKET)
	{
		if( !FBitSet( pev->spawnflags, SF_TRAP_SILENT ))
			EMIT_SOUND( edict(), CHAN_VOICE, "weapons/sgun1.wav", 1, ATTN_NORM);

		CRocket::CreateRocket( pev->origin + pev->movedir * 8, pev->movedir, this );
	}
#endif /* HIPNOTIC */
	else
	{
		CNail *pNail;

#ifndef HIPNOTIC
		EMIT_SOUND( edict(), CHAN_VOICE, "weapons/spike2.wav", 1, ATTN_NORM );
#else /* HIPNOTIC */
		if( !FBitSet( pev->spawnflags, SF_TRAP_SILENT ))
			EMIT_SOUND( edict(), CHAN_VOICE, "weapons/spike2.wav", 1, ATTN_NORM );
#endif /* HIPNOTIC */

		if (pev->spawnflags & SF_SUPERSPIKE)
			pNail = CNail::CreateSuperNail( pev->origin, pev->movedir, this );
		else pNail = CNail::CreateNail( pev->origin, pev->movedir, this );

		if (pNail) pNail->pev->velocity = pev->movedir * 500;
	}
}

class CEventLighting : public CBaseEntity
{
public:
	void	EXPORT LightingFire( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( event_lightning, CEventLighting );

void CEventLighting :: LightingFire( void )
{
	Vector	p1, p2;
	CBaseDoor	*pDoor1, *pDoor2;

	pDoor1 = (CBaseDoor *)CBaseEntity::Instance( pev->enemy );
	pDoor2 = (CBaseDoor *)CBaseEntity::Instance( pev->owner );

	if (gpGlobals->time >= pev->dmgtime)
	{	
		// done here, put the terminals back up
		pDoor1->DoorGoDown();
		pDoor2->DoorGoDown();
		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink( &CEventLighting::LightingFire );

	p1 = (pDoor1->pev->mins + pDoor1->pev->maxs) * 0.5f;
	p1.z = pDoor1->pev->absmin.z - 16;

	p2 = (pDoor2->pev->mins + pDoor2->pev->maxs) * 0.5f;
	p2.z = pDoor2->pev->absmin.z - 16;

	UTIL_Sparks( p1 );
	UTIL_Sparks( p2 );

	// compensate for length of bolt
	p2 = p2 - (p2 - p1).Normalize() * 110;

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_LIGHTNING3 );
		WRITE_ENTITY( 0 );
		WRITE_COORD( p1.x );
		WRITE_COORD( p1.y );
		WRITE_COORD( p1.z );
		WRITE_COORD( p2.x );
		WRITE_COORD( p2.y );
		WRITE_COORD( p2.z );
	MESSAGE_END();
}

void CEventLighting :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (pev->dmgtime >= gpGlobals->time + 1)
		return;

	CBaseDoor	*pDoor1, *pDoor2;

	pDoor1 = (CBaseDoor *)UTIL_FindEntityByString( NULL, "target", "lightning" );
	pDoor2 = (CBaseDoor *)UTIL_FindEntityByString( pDoor1, "target", "lightning" );

	if (!pDoor1 || !pDoor2)
	{
		ALERT( at_error, "event_lightning: missing lightning targets\n" );
		return;
	}
	
	if((pDoor1->m_toggle_state != TS_AT_TOP && pDoor1->m_toggle_state != TS_AT_BOTTOM)
	|| (pDoor2->m_toggle_state != TS_AT_TOP && pDoor2->m_toggle_state != TS_AT_BOTTOM)
	|| (pDoor1->m_toggle_state != pDoor2->m_toggle_state ))
	{
		return;
	}

	// don't let the electrodes go back up until the bolt is done
	pDoor1->pev->nextthink = -1;
	pDoor2->pev->nextthink = -1;
	pev->dmgtime = gpGlobals->time + 1;

	// store door pointers into standard fields
	pev->enemy = ENT( pDoor1->pev );
	pev->owner = ENT( pDoor2->pev );

	EMIT_SOUND(ENT(pev), CHAN_VOICE, "misc/power.wav", 1, ATTN_NORM);
	LightingFire();		

	// advance the boss pain if down
	CBaseEntity *pBoss = UTIL_FindEntityByClassname( NULL, "monster_boss" );
	if (!pBoss) return;

	// update enemy (who activated the shock)
	pBoss->m_hEnemy = pActivator;

	if (pDoor1->m_toggle_state != TS_AT_BOTTOM && pBoss->pev->health > 0)
	{
		pBoss->TakeDamage( pev, pActivator->pev, 1, DMG_SHOCK );
	}	
}

#ifdef HIPNOTIC
class CInfoCommand : public CBaseEntity
{
public:
	void Activate( void ) { if( pev->message ) SERVER_COMMAND( (char *)STRING( pev->message )); }
};

LINK_ENTITY_TO_CLASS( info_command, CInfoCommand );

/*QUAKED func_exploder (0.4 0 0) (0 0 0) (8 8 8) particles
  Spawns an explosion when triggered.  Triggers any targets.

  "dmg" specifies how much damage to cause.  Negative values
  indicate no damage.  Default or 0 indicates 120.
  "volume" volume at which to play explosions (default 1.0)
  "speed" attenuation for explosions (default normal)
*/
class CFuncExploder : public CBaseDelay
{
public:
	void Spawn( void );
	void Precache( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT Explosion( void );
};

LINK_ENTITY_TO_CLASS( func_exploder, CFuncExploder );

void CFuncExploder :: Precache( void )
{
	PRECACHE_SOUND( "misc/shortexp.wav" );
	PRECACHE_SOUND( "misc/longexpl.wav" );
}

void CFuncExploder :: Spawn( void )
{
	Precache ();

	if( pev->dmg == 0.0f )
		pev->dmg = 120;
	if( pev->dmg < 0.0f )
		pev->dmg = 0;

	if( pev->speed == 0.0f )
		pev->speed = 1.0f;
	if( m_flVolume == 0.0f )
		m_flVolume = 1.0f;
}

void CFuncExploder :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_hActivator = pActivator;

	if( m_flDelay )
	{
		SetThink( &CFuncExploder::Explosion );
		pev->nextthink = gpGlobals->time + m_flDelay;
		m_flDelay = 0.0f;
	}
	else
	{
		// fire immediately
		Explosion ();
	}
}

void CFuncExploder :: Explosion( void )
{
	SUB_UseTargets( m_hActivator, USE_TOGGLE, 0.0f );

	if( pev->dmg < 120.0f )
	{
		EMIT_SOUND(ENT(pev), CHAN_AUTO, "misc/shortexp.wav", m_flVolume, pev->speed);
	}
	else
	{
		EMIT_SOUND(ENT(pev), CHAN_AUTO, "misc/longexpl.wav", m_flVolume, pev->speed);
	}

	UTIL_ScreenShake( pev->origin, 16.0f, 2.0f, 0.5f, 250.0f );
	Q_RadiusDamage(this, m_hActivator, pev->dmg, NULL);

	if( pev->spawnflags & 1 )
	{
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_EXPLOSION3 );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
		MESSAGE_END();
	}

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_EXPLOSION_SPRITE );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
	MESSAGE_END();

	UTIL_Remove( this );
}

/*QUAKED func_multi_exploder (0.4 0 0) ?
  Spawns an explosion when triggered.  Triggers any targets.
  size of brush determines where explosions will occur.

  "dmg" specifies how much damage to cause from each explosion
  Negative values indicate no damage.  Default or 0 indicates 120.
  "delay" delay before exploding (Default 0 seconds)
  "duration" how long to explode for (Default 1 second)
  "wait" time between each explosion (default 0.25 seconds)
  "volume" volume to play explosion sound at (default 0.5)
  "speed" attenuation for explosions (default normal)

*/
LINK_ENTITY_TO_CLASS( func_multi_exploder, CFuncMultiExploder );

void CFuncMultiExploder :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		pev->dmgtime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CFuncMultiExploder :: Precache( void )
{
	PRECACHE_SOUND( "misc/shortexp.wav" );
	PRECACHE_SOUND( "misc/longexpl.wav" );
}

void CFuncMultiExploder :: Spawn( void )
{
	Precache ();

	// set size and link into world
	SET_MODEL(ENT(pev), STRING(pev->model) );
	pev->modelindex = 0;
	pev->model = 0;

	if( pev->dmg == 0.0f )
		pev->dmg = 120;
	if( pev->dmg < 0.0f )
		pev->dmg = 0;

	if( pev->dmgtime == 0.0f )
		pev->dmgtime = 1.0f;
	if( pev->speed == 0.0f )
		pev->speed = 1.0f;
	if( m_flVolume == 0.0f )
		m_flVolume = 1.0f;
	if( m_flWait == 0.0f )
		m_flWait = 0.25f;

	pev->button = FALSE;
}

void CFuncMultiExploder :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_hActivator = pActivator;

	if( m_flDelay )
	{
		SetThink( &CFuncMultiExploder::ExplosionThink );
		pev->nextthink = gpGlobals->time + m_flDelay;
		m_flDelay = 0.0f;
	}
	else
	{
		// fire immediately
		SetThink( &CFuncMultiExploder::ExplosionThink );
		ExplosionThink ();
	}
}

void CFuncMultiExploder :: ExplosionThink( void )
{
	pev->nextthink = gpGlobals->time + m_flWait;

	if( !pev->button )
	{
		pev->button = TRUE;
		pev->dmgtime = gpGlobals->time + pev->dmgtime;
		SUB_UseTargets( m_hActivator, USE_TOGGLE, 0.0f );
	}

	if( gpGlobals->time > pev->dmgtime )
	{
		UTIL_Remove( this );
		return;
	}

	CBaseEntity *pExpl = GetClassPtr( (CPointEntity *)NULL );
	pExpl->pev->classname = MAKE_STRING( "info_notnull" );
	pExpl->pev->owner = pev->owner;
	pExpl->pev->dmg = pev->dmg;
	pExpl->pev->origin.x = pev->absmin.x + (RANDOM_FLOAT(0, 1) * (pev->absmax.x - pev->absmin.x));
	pExpl->pev->origin.y = pev->absmin.y + (RANDOM_FLOAT(0, 1) * (pev->absmax.y - pev->absmin.y));
	pExpl->pev->origin.z = pev->absmin.z + (RANDOM_FLOAT(0, 1) * (pev->absmax.z - pev->absmin.z));
	EMIT_SOUND(ENT(pExpl->pev), CHAN_AUTO, "misc/shortexp.wav", m_flVolume, pev->speed);
	UTIL_ScreenShake( pExpl->pev->origin, 16.0f, 2.0f, 0.5f, 250.0f );
	Q_RadiusDamage(pExpl, m_hActivator, pev->dmg, NULL);

	if( pev->spawnflags & 1 )
	{
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_EXPLOSION3 );
			WRITE_COORD( pExpl->pev->origin.x );
			WRITE_COORD( pExpl->pev->origin.y );
			WRITE_COORD( pExpl->pev->origin.z );
		MESSAGE_END();
	}

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_EXPLOSION_SPRITE );
		WRITE_COORD( pExpl->pev->origin.x );
		WRITE_COORD( pExpl->pev->origin.y );
		WRITE_COORD( pExpl->pev->origin.z );
	MESSAGE_END();

	UTIL_Remove( pExpl );
}

void CFuncMultiExploder :: MultiExplosion( const Vector &loc, float flRad, float flDamage, float dur, float pause, float vol )
{
	CFuncMultiExploder *pMultiExp = GetClassPtr( (CFuncMultiExploder *)NULL );

	if( !pMultiExp ) return;

	pMultiExp->pev->classname = MAKE_STRING( func_multi_exploder );
	pMultiExp->pev->origin = loc;
	pMultiExp->pev->dmg = flDamage;
	pMultiExp->pev->dmgtime = dur;
	pMultiExp->m_flWait = pause;
	pMultiExp->m_flVolume = vol;

	pMultiExp->pev->absmin = pMultiExp->pev->origin - (flRad * Vector( 1, 1, 1 ));
	pMultiExp->pev->absmax = pMultiExp->pev->origin + (flRad * Vector( 1, 1, 1 ));

	pMultiExp->SetThink( &CFuncMultiExploder :: ExplosionThink );
	pMultiExp->ExplosionThink();
}

class CRubblePiece : public CBaseEntity
{
public:
	void Spawn( const char *szGibModel );
	void Touch( CBaseEntity *pOther );
	static void ThrowRubble( const char *szGibName, const Vector &vecOrigin );
};

LINK_ENTITY_TO_CLASS( rubble, CRubblePiece );

void CRubblePiece :: ThrowRubble( const char *szGibName, const Vector &vecOrigin )
{
	CRubblePiece *pRubble = GetClassPtr( (CRubblePiece *)NULL );

	pRubble->Spawn( szGibName );
	pRubble->pev->origin = vecOrigin;
	pRubble->pev->classname = MAKE_STRING( "rubble" );
}

void CRubblePiece :: Spawn( const char *szGibModel )
{
	SET_MODEL(ENT(pev), szGibModel);
	UTIL_SetSize (pev, g_vecZero, g_vecZero);

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	pev->velocity.x = RANDOM_FLOAT( -70, 70 );
	pev->velocity.y = RANDOM_FLOAT( -70, 70 );
	pev->velocity.z = RANDOM_FLOAT( 0, 210 );
	pev->avelocity.x = RANDOM_FLOAT( 0, 600 );
	pev->avelocity.y = RANDOM_FLOAT( 0, 600 );
	pev->avelocity.z = RANDOM_FLOAT( 0, 600 );

	SetThink( &CBaseEntity::SUB_Remove );

	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 23.0f, 32.0f );
	pev->dmgtime = gpGlobals->time;
}

void CRubblePiece :: Touch( CBaseEntity *pOther )
{
	if( gpGlobals->time < pev->dmgtime )
		return;
	   
	if( pOther->pev->takedamage )
	{
		pOther->TakeDamage( pev, pev, 10, DMG_GENERIC );
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "zombie/z_hit.wav", 1, ATTN_NORM);
		pev->dmgtime = gpGlobals->time + 0.1;
	}
}

class CFuncRubble : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

/*QUAKED func_rubble (0.4 0.4 0.2) (0 0 0) (32 32 32)
  Spawns random sized rubble when triggered.  
  
  "count" is the number of pieces of rubble to spawn.  Default is 1.
*/
LINK_ENTITY_TO_CLASS( func_rubble, CFuncRubble );

/*QUAKED func_rubble1 (0.4 0.4 0.2) (0 0 0) (8 8 8)
  Spawns small rubble when triggered.  
  
  "count" is the number of pieces of rubble to spawn.  Default is 1.
*/
LINK_ENTITY_TO_CLASS( func_rubble1, CFuncRubble );

/*QUAKED func_rubble2 (0.4 0.4 0.2) (0 0 0) (16 16 16)
  Spawns medium rubble when triggered.  
  
  "count" is the number of pieces of rubble to spawn.  Default is 1.
*/
LINK_ENTITY_TO_CLASS( func_rubble2, CFuncRubble );

/*QUAKED func_rubble3 (0.4 0.4 0.2) (0 0 0) (32 32 32)
  Spawns large rubble when triggered.  
  
  "count" is the number of pieces of rubble to spawn.  Default is 1.
*/
LINK_ENTITY_TO_CLASS( func_rubble3, CFuncRubble );

void CFuncRubble :: Precache( void )
{
	PRECACHE_MODEL ("models/rubble1.mdl");
	PRECACHE_MODEL ("models/rubble2.mdl");
	PRECACHE_MODEL ("models/rubble3.mdl");
	PRECACHE_SOUND ("zombie/z_hit.wav");
}

void CFuncRubble :: Spawn( void )
{
	Precache ();

	if( FClassnameIs( pev, "func_rubble" ))
		m_flCnt = 0.0f;
	else if( FClassnameIs( pev, "func_rubble1" ))
		m_flCnt = 1.0f;
	else if( FClassnameIs( pev, "func_rubble2" ))
		m_flCnt = 2.0f;
	else if( FClassnameIs( pev, "func_rubble3" ))
		m_flCnt = 3.0f;
}

void CFuncRubble :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int index = 0;
	
	do 
	{
		int which = m_flCnt;

		if( m_flCnt == 0.0f )
			which = RANDOM_FLOAT( 1, 3 );

		switch( which )
		{
		case 1: CRubblePiece::ThrowRubble( "models/rubble1.mdl", pev->origin ); break;
		case 2: CRubblePiece::ThrowRubble( "models/rubble3.mdl", pev->origin ); break;
		default: CRubblePiece::ThrowRubble("models/rubble2.mdl", pev->origin ); break;
		}
	} while( ++index < m_flCount );
}

/*QUAKED func_earthquake (0 0 0.5) (0 0 0) (32 32 32)
Causes an earthquake.  Triggers targets.

"dmg" is the duration of the earthquake.  Default is 0.8 seconds.
*/
class CFuncEarthQuake : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( func_earthquake, CFuncEarthQuake );

void CFuncEarthQuake :: Spawn( void )
{
	if( !pev->dmg ) pev->dmg = 0.8f;
}

void CFuncEarthQuake :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
	value = gpGlobals->time + pev->dmg;

	if( value > pPlayer->m_flEarthQuakeTime )
		pPlayer->m_flEarthQuakeTime = value;
}

/*QUAKED info_startendtext (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
 start the end text
*/

class CStartEndText : public CBaseEntity
{
public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
LINK_ENTITY_TO_CLASS( info_startendtext, CStartEndText );

void CStartEndText::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	g_intermission_running = 1;
	ExitIntermission (INDEXENT( 1 ));
}

/*QUAKED effect_teleport (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
 Create a teleport effect when triggered
*/
class CTeleportEffect : public CBaseEntity
{
public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
LINK_ENTITY_TO_CLASS( effect_teleport, CTeleportEffect );

void CTeleportEffect::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CTeleFog::CreateFog( pev->origin );
}


/*QUAKED effect_finale (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) useplayer nodecoy
 start the finale sequence
 "target" the camera to go to.
 "mdl" if useplayer is specified, this is a
 path corner with target of the next
 path_corner to run to.
 if use player isn't specified this becomes
 the spawn point as well.
 "spawnfunction" next routine to run
 "delay" time to wait until running routine
 useplayer - use the current player as
             decoy location.
 nodecoy - no decoy, only the camera
*/
class CFinaleEffect : public CBaseEntity
{
public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );
	void BecomeDecoy( string_t target, const Vector &origin );
};

LINK_ENTITY_TO_CLASS( effect_finale, CFinaleEffect );

// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
void CFinaleEffect :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "mangle"))	// a quake alias
	{
		UTIL_StringToVector( (float *)pev->angles, pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "mdl"))
	{
		pev->netname = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CFinaleEffect :: BecomeDecoy( string_t target, const Vector &origin )
{
	CBaseEntity *pDecoy;

	pDecoy = CBaseEntity :: Create( "monster_decoy", origin, g_vecZero );
	pDecoy->pev->target = target;	// copy path_corner name
}

void CFinaleEffect :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pPlayer;

	if (pev->button) return;

	pev->button = TRUE;

	// find the intermission spot
	CBaseEntity *pTarget = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ));

	if (!pTarget)
	{
		ALERT( at_error, "no target in finale!\n" );
		return;
	}

	//setup decoy
	if (!FBitSet( pev->spawnflags, 2 ))
	{
		CBaseEntity *pPath = UTIL_FindEntityByTargetname( NULL, STRING( pev->netname ));

		if (!pPath)
		{
			ALERT( at_error, "no path target while spawnflags 2 is not set!\n" );
			return;
		}

		if (FBitSet(pev->spawnflags, 1))
		{
			pPlayer = UTIL_PlayerByIndex (1);
			BecomeDecoy( pPath->pev->target, pPlayer->pev->origin );
		}
		else
		{
			BecomeDecoy( pPath->pev->target, pPath->pev->origin );
		}
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		pPlayer = UTIL_PlayerByIndex (i);

		SET_VIEW( pPlayer->edict(), pTarget->edict() );
		pPlayer->pev->takedamage = DAMAGE_NO;
		pPlayer->pev->solid = SOLID_NOT;
		pPlayer->pev->movetype = MOVETYPE_NONE;
		pPlayer->pev->modelindex = 0;
		pPlayer->pev->v_angle = pTarget->pev->angles;
		pPlayer->pev->fixangle = TRUE;
		pPlayer->m_iItems = 0;
		UTIL_SetOrigin( pPlayer->pev, pTarget->pev->origin );
	}
}
#endif /* HIPNOTIC */
