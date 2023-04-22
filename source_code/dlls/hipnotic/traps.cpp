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
#include "skill.h"
#include "decals.h"

#define SF_LIGHTNING_RANDOM		1
#define SF_LIGHTNING_BOOM		2

#define SF_UNDERWATER		2

class CLightingBeam : public CBaseDelay
{
public:
	void	Spawn( void );
	void	Think( void );
	static CLightingBeam *CreateBeam( const Vector &p1, const Vector &p2, CBaseEntity *pOwner, float flDmg, float flDelay );
};

LINK_ENTITY_TO_CLASS( trap_beam, CLightingBeam );

CLightingBeam *CLightingBeam :: CreateBeam( const Vector &p1, const Vector &p2, CBaseEntity *pOwner, float flDmg, float flDelay )
{
	CLightingBeam *pBeam = GetClassPtr( (CLightingBeam *)NULL );
	Vector dir = ( p2 - p1 ).Normalize();
	float dst = ( p2 - p1 ).Length();

	// place entity between ligtning start and end to guranteed that entity
	// will be seen player in PVS
	UTIL_SetOrigin( pBeam->pev, p1 + dir * (dst * 0.5f) );
	pBeam->neworigin = p1;
	pBeam->oldorigin = p2;
	pBeam->pev->owner = pOwner->edict();
	pBeam->pev->dmg = flDmg;
	pBeam->m_flDelay = flDelay;
	pBeam->Spawn();

	return pBeam;
}

void CLightingBeam :: Spawn( void )
{
	pev->classname = MAKE_STRING( "trap_beam" );
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CLightingBeam :: Think( void )
{
	if( gpGlobals->time > m_flDelay )
	{
		UTIL_Remove( this );
		return;
	}

	if (!FNullEnt( FIND_CLIENT_IN_PVS( edict() )))
	{
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_LIGHTNING2 );
			WRITE_ENTITY( entindex() );
			WRITE_COORD( neworigin.x );
			WRITE_COORD( neworigin.y );
			WRITE_COORD( neworigin.z );
			WRITE_COORD( oldorigin.x );
			WRITE_COORD( oldorigin.y );
			WRITE_COORD( oldorigin.z );
		MESSAGE_END();
	}

	Vector dir = ( pev->oldorigin - pev->origin ).Normalize();
	CBasePlayer::LightningDamage(pev->origin, pev->oldorigin, CBaseEntity::Instance( pev->owner ), pev->dmg, dir );
	pev->nextthink = gpGlobals->time + 0.1f;
}

class CLightningTrap : public CBaseToggle
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	KeyValue( KeyValueData *pkvd );
	void EXPORT LightningUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT SwitchedUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT FirstThink( void );
	void EXPORT RegularThink( void );
};

/*QUAKED trap_lightning_triggered (0 .5 .8) (-8 -8 -8) (8 8 8) random boom
When triggered, fires lightning in the direction set in QuakeEd.
"wait" how long to wait between blasts (1.0 default)
       if in random mode wait is multiplied by random
"dmg" how much damage lightning should inflict (30 default)
"duration" how long each lightning attack should last (0.1 default)
*/
LINK_ENTITY_TO_CLASS( trap_lightning_triggered, CLightningTrap );

/*QUAKED trap_lightning (0 .5 .8) (-8 -8 -8) (8 8 8) random boom
Continuously fire lightning.
"wait" how long to wait between blasts (1.0 default)
       if in random mode wait is multiplied by random
"nextthink" delay before firing first lightning, so multiple traps can be stagered.
"dmg" how much damage lightning should inflict (30 default)
"duration" how long each lightning attack should last (0.1 default)
*/
LINK_ENTITY_TO_CLASS( trap_lightning, CLightningTrap );

/*QUAKED trap_lightning_switched (0 .5 .8) (-8 -8 -8) (8 8 8) random boom
Continuously fires lightning.
"wait" how long to wait between blasts (1.0 default)
       if in random mode wait is multiplied by random
"nextthink" delay before firing first lightning, so multiple traps can be stagered.
"dmg" how much damage lightning should inflict (30 default)
"duration" how long each lightning attack should last (0.1 default)
"state" 0 (default) initially off, 1 initially on.
*/
LINK_ENTITY_TO_CLASS( trap_lightning_switched, CLightningTrap );

void CLightningTrap :: Precache( void )
{
	PRECACHE_SOUND ("weapons/lhit.wav");
	PRECACHE_SOUND ("weapons/lstart.wav");
}

void CLightningTrap :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		pev->frags = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CLightningTrap :: Spawn( void )
{
	Precache ();

	if( m_flWait == 0 )
		m_flWait = 1.0f;
	if( pev->dmg == 0 )
		pev->dmg = 30.0f;

	if( pev->frags == 0 )
		pev->frags = 0.1f;

	m_flCnt = 0;

	if( FClassnameIs( pev, "trap_lightning_switched" ))
		SetUse( &CLightningTrap::SwitchedUse );
	else SetUse( &CLightningTrap::LightningUse );

	if( FClassnameIs( pev, "trap_lightning" ))
		pev->button = TRUE;

	pev->dmgtime = pev->nextthink;
	SetThink( &CLightningTrap::FirstThink );
	pev->nextthink = gpGlobals->time + 0.25f;
	m_iDeathType = MAKE_STRING( "is electrocuted" );
}

void CLightningTrap :: LightningUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	Vector p1, p2, dir;
	float dst, remainder;

	if( gpGlobals->time >= pev->pain_finished )
	{
		if (pev->spawnflags & SF_LIGHTNING_BOOM)
			EMIT_SOUND (ENT(pev), CHAN_VOICE, "weapons/lstart.wav", 1, ATTN_NORM);
		else EMIT_SOUND (ENT(pev), CHAN_VOICE, "weapons/lhit.wav", 1, ATTN_NORM);

		if (FClassnameIs(pev,"trap_lightning_triggered"))
			pev->pain_finished = gpGlobals->time + 0.1f;
	}

	if( pev->enemy != NULL )
	{
		p1 = pev->origin;
		p2 = pev->enemy->v.origin;
	}
	else
	{
		TraceResult tr;
		UTIL_MakeVectors( pev->angles );
		pev->movedir = gpGlobals->v_forward;
		UTIL_TraceLine (pev->origin, pev->origin + pev->movedir * 600, ignore_monsters, ENT(pev), &tr);
		p1 = pev->origin;
		p2 = tr.vecEndPos;
	}

	// fix up both ends of the lightning
	// lightning bolts are 30 units long each
	dir = ( p2 - p1 ).Normalize();
	dst = ( p2 - p1 ).Length();
	dst = dst / 30.0f;
	remainder = dst - floor( dst );

	if( remainder > 0.0f )
	{
		remainder = remainder - 1.0f;
		// split half the remainder with the front and back
		remainder = remainder * 15.0f;
		p1 = p1 + (remainder * dir);
		p2 = p2 - (remainder * dir);
	}

	if( pev->frags > 0.1f )
	{
		CLightingBeam :: CreateBeam( p1, p2, this, pev->dmg, gpGlobals->time + pev->frags );
	}
	else if (!FNullEnt( FIND_CLIENT_IN_PVS( edict() )))
	{
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_LIGHTNING2 );
			WRITE_ENTITY( entindex() );
			WRITE_COORD( p1.x );
			WRITE_COORD( p1.y );
			WRITE_COORD( p1.z );
			WRITE_COORD( p2.x );
			WRITE_COORD( p2.y );
			WRITE_COORD( p2.z );
		MESSAGE_END();

		CBasePlayer::LightningDamage(p1, p2, this, pev->dmg, dir );
	}
	else
	{
		CBasePlayer::LightningDamage(p1, p2, this, pev->dmg, dir );
	}
}

void CLightningTrap :: SwitchedUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	pev->button = !pev->button;

	if( pev->button )
		pev->nextthink = pev->dmgtime;
}

void CLightningTrap :: FirstThink( void )
{
	CBaseEntity *pTarg = GetNextTarget();

	if( pTarg != NULL )
	{
		m_vecPosition1 = pTarg->pev->origin;
		pev->enemy = pTarg->edict();
	}

	if( FClassnameIs( pev, "trap_lightning_triggered" ))
	{
		SetThink( NULL );
		pev->nextthink = 0;
	}
	else
	{
		SetThink( &CLightningTrap::RegularThink );
		pev->nextthink = pev->dmgtime + m_flWait + pev->ltime;
	}
}

void CLightningTrap :: RegularThink( void )
{
	float timedelay;

	if( pev->button )
	{
		LightningUse( this, this, USE_TOGGLE, 0.0f );
	}

	if( m_flCnt == 0 )
	{
		if( pev->spawnflags & SF_LIGHTNING_RANDOM )
		{
			timedelay = m_flWait * RANDOM_FLOAT( 0, 1 );
		}
		else
		{
			timedelay = m_flWait;
		}

		m_flCnt = 1;
		pev->teleport_time = gpGlobals->time + pev->frags - 0.1f;
		pev->pain_finished = gpGlobals->time + pev->frags - 0.1f;

		if( pev->pain_finished < gpGlobals->time + 0.3f )
			pev->pain_finished = gpGlobals->time + 0.3f;

		if( timedelay < pev->frags )
			timedelay = pev->frags;
		pev->air_finished = gpGlobals->time + timedelay;
	}

	if( gpGlobals->time >= pev->teleport_time )
	{
		m_flCnt = 0;
		pev->nextthink = pev->air_finished;
	}
	else
	{
		pev->nextthink = gpGlobals->time + 0.2f;
	}
}

class CTeslaBeam : public CBaseDelay
{
public:
	void Spawn( void );
	void Think( void );
	static CTeslaBeam *CreateBeam( CBaseEntity *pOwner, CBaseEntity *pTarget, CBaseEntity *pActivator );
};

LINK_ENTITY_TO_CLASS( tesla_beam, CTeslaBeam );

CTeslaBeam *CTeslaBeam :: CreateBeam( CBaseEntity *pOwner, CBaseEntity *pTarget, CBaseEntity *pActivator )
{
	CTeslaBeam *pBeam = GetClassPtr( (CTeslaBeam *)NULL );
	UTIL_SetOrigin( pBeam->pev, pOwner->pev->origin );

	if( pOwner->pev->health )
		pBeam->m_flDelay = gpGlobals->time + pOwner->pev->health;
	else pBeam->m_flDelay = gpGlobals->time + 9999.0f;

	pBeam->m_hEnemy = pTarget;
	pBeam->pev->owner = pOwner->edict();
	pBeam->pev->dmg = pOwner->pev->dmg;
	pBeam->pev->frags = pOwner->pev->frags;
	pBeam->m_iDeathType = pOwner->m_iDeathType;
	pBeam->m_hActivator = pActivator;
	pBeam->Spawn();

	// strucked
	pTarget->m_bStruckByMjolnir = TRUE;

	return pBeam;
}

void CTeslaBeam :: Spawn( void )
{
	pev->classname = MAKE_STRING( "tesla_beam" );
	pev->nextthink = gpGlobals->time;
}

void CTeslaBeam :: Think( void )
{
	CBaseEntity *pOwner = CBaseEntity :: Instance( pev->owner );

	if( !pOwner || m_hEnemy == NULL )
	{
		if( !pOwner )
			ALERT( at_error, "tesla_beam: no owner!!!\n" );

		if( m_hEnemy != NULL )
			m_hEnemy->m_bStruckByMjolnir = FALSE;
		UTIL_Remove( this );
		return;		
	}

	pOwner->pev->impulse = 2;	// attack state

	if( gpGlobals->time > m_flDelay )
	{
		m_hEnemy->m_bStruckByMjolnir = FALSE;
		UTIL_Remove( this );
		return;
	}

	TraceResult tr;
	UTIL_TraceLine( pev->origin, m_hEnemy->pev->origin, ignore_monsters, ignore_glass, ENT(pev), &tr);

	if( tr.flFraction != 1.0 || m_hEnemy->pev->health <= 0.0f || (pev->origin - m_hEnemy->pev->origin).Length() > ( pev->frags + 10 ))
	{
		m_hEnemy->m_bStruckByMjolnir = FALSE;
		UTIL_Remove( this );
		return;
	}

	if (!FNullEnt( FIND_CLIENT_IN_PVS( edict() )))
	{
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_LIGHTNING2 );
			WRITE_ENTITY( entindex() );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( tr.vecEndPos.x );
			WRITE_COORD( tr.vecEndPos.y );
			WRITE_COORD( tr.vecEndPos.z );
		MESSAGE_END();
	}

	Vector dir = ( tr.vecEndPos - pev->origin ).Normalize();
	CBasePlayer::LightningDamage(pev->origin, tr.vecEndPos, m_hActivator, pev->dmg, dir );
	pev->nextthink = gpGlobals->time + 0.1f;
}

class CRadiusTrap : public CBaseToggle
{
public:
	void KeyValue( KeyValueData *pkvd );
	void ScanTargets( void );
	BOOL TargetVisible( CBaseEntity *pTarg );
	void EXPORT TeslaThink( void );
	void EXPORT GravityThink( void );

	// no need to save\restore this
	int m_iNumTargets;
	CBaseEntity *m_pHeadTarget;
};

void CRadiusTrap :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "distance"))
	{
		pev->frags = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "duration"))
	{
		pev->health = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "state"))
	{
		pev->button = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CRadiusTrap :: ScanTargets( void )
{
	CBaseEntity *pEnt = NULL, *pPrev = NULL;

	m_iNumTargets = 0;

	// g-cont. replace this with UTIL_FindEntitiesInPVS ?
	while(( pEnt = UTIL_FindEntityInSphere( pEnt, pev->origin, pev->frags )) != NULL )
	{
		if( !FBitSet( pEnt->pev->flags, FL_NOTARGET ) && FBitSet( pEnt->pev->flags, (int)m_flCnt ))
		{
			if (TargetVisible( pEnt ) && (pEnt->pev->health > 0) && (!pEnt->m_bStruckByMjolnir))
			{
				if( !m_iNumTargets )
					m_pHeadTarget = pEnt;
				else pPrev->m_pNextEnt = pEnt;

				m_iNumTargets++;
				pPrev = pEnt;

				if( m_iNumTargets == m_flCount )
					return;
			}
		}
	}
}

BOOL CRadiusTrap :: TargetVisible( CBaseEntity *pTarg )
{
	TraceResult tr;

	Vector spot1 = EyePosition();
	Vector spot2 = pTarg->EyePosition();

	// see through other monsters
	UTIL_TraceLine(spot1, spot2, ignore_monsters, ignore_glass, ENT(pev), &tr);
	
	if (tr.fInOpen && tr.fInWater)
		return FALSE;		// sight line crossed contents

	if (tr.flFraction == 1.0)
		return TRUE;
	return FALSE;
}

void CRadiusTrap :: TeslaThink( void )
{
	if( pev->button == 0 )	// state
	{
		pev->nextthink = gpGlobals->time + 0.25f;
		return;
	}

	if( pev->impulse == 0 )	// attack state
	{
		ScanTargets ();

		if( m_iNumTargets > 0 )
		{
			if( m_flWait > 0.0f )
				EMIT_SOUND( ENT(pev), CHAN_VOICE, "misc/tesla.wav", 1, ATTN_NORM);
			pev->impulse = 1;
			pev->nextthink = gpGlobals->time + m_flWait;
			return;
		}

		pev->nextthink = gpGlobals->time + 0.25f;

		if( m_flDelay > 0.0f )
		{
			if( gpGlobals->time > pev->air_finished ) // search time
				pev->impulse = 3;
		}
	}
	else if( pev->impulse == 1 )
	{
		ScanTargets ();

		while( m_iNumTargets > 0 )
		{
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "hipweap/mjolhit.wav", 1, ATTN_NORM);
			CTeslaBeam :: CreateBeam( this, m_pHeadTarget, m_hActivator );
			m_pHeadTarget = m_pHeadTarget->m_pNextEnt;
			m_iNumTargets--;
		}

		pev->impulse = 2;
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	else if( pev->impulse == 2 )
	{
		// trying to change attack state but tesla traps will return
		// attack state to 2 until last beam is not dead
		pev->impulse = 3;
		pev->nextthink = gpGlobals->time + 0.2f;
	}
	else if( pev->impulse == 3 )
	{
		// scan targets again
		pev->impulse = 0;
		pev->nextthink = gpGlobals->time + 0.1f;

		if( FClassnameIs( pev, "trap_gods_wrath" ))
			pev->nextthink = -1;
	}
}

void CRadiusTrap :: GravityThink( void )
{
	pev->ltime = gpGlobals->time;
	ScanTargets ();

	while( m_iNumTargets > 0 )
	{
		Vector dir = (pev->origin - m_pHeadTarget->pev->origin).Normalize();
		Vector vel = dir * pev->speed;

		if( m_pHeadTarget->IsPlayer( ) && ( pev->spawnflags & SF_UNDERWATER ))
		{
			CBasePlayer *pPlayer = (CBasePlayer *)m_pHeadTarget;
			if( pPlayer->m_flWetSuitFinished > gpGlobals->time )
				vel *= 0.6f;
		}

		m_pHeadTarget->pev->velocity += vel;
		m_pHeadTarget = m_pHeadTarget->m_pNextEnt;
		m_iNumTargets--;
	}

	pev->nextthink = gpGlobals->time + 0.1f;
}

/*QUAKED trap_tesla_coil (0 .5 .8) (-8 -8 -8) (8 8 8) targetenemies
targets enemies in vicinity and fires at them
"wait" how long build up should be (2 second default)
"dmg" how much damage lightning should inflict (2 + 5*skill default)
"duration" how long each lightning attack should last (continuous default)
"distance" how far the tesla coil should reach (600 default)
"state" on/off for the coil (0 default is off)
"count" number of people to target (2 default)
*/
class CTeslaCoil : public CRadiusTrap
{
public:
	void Spawn( void );
	void Precache( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( trap_tesla_coil, CTeslaCoil );

void CTeslaCoil :: Precache( void )
{
	PRECACHE_SOUND ("misc/tesla.wav");
	PRECACHE_SOUND ("hipweap/mjolhit.wav");   // lightning sound
}

void CTeslaCoil :: Spawn( void )
{
	Precache();

	if( m_flWait == 0.0f )
		m_flWait = 2.0f;

	if( pev->dmg == 0.0f )
		pev->dmg = 2.0f + ( 5.0f * g_iSkillLevel );

	if( pev->health == 0.0f )
		pev->health = -1;

	if( pev->frags == 0.0f )
		pev->frags = 600.0f;

	if( pev->spawnflags & 1 )
		m_flCnt = (FL_CLIENT|FL_MONSTER);
	else m_flCnt = FL_CLIENT;

	if( m_flDelay == 0.0f )
		m_flDelay = -1.0f;

	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.0f, 1.0f );
	SetThink( &CRadiusTrap::TeslaThink );
	m_hActivator = gpWorld;
	m_iNumTargets = 0;
	pev->impulse = 0;

	m_iDeathType = MAKE_STRING( "is electrocuted" );
}

void CTeslaCoil :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	pev->button = !pev->button;
}

/*QUAKED trap_gods_wrath (0 .5 .8) (-8 -8 -8) (8 8 8) targetenemies
targets enemies in vicinity and fires at them
"dmg" how much damage lightning should inflict (5 default)
"duration" how long each lightning attack should last (continuous default)
"distance" how far god's wrath should reach (600 default)
"delay" how long to wait until god calms down
   this is only needed if no one is targetted (5 seconds default)
"count" number of people to target (2 default)
*/
class CGodsWrath : public CRadiusTrap
{
public:
	void Spawn( void );
	void Precache( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( trap_gods_wrath, CGodsWrath );

void CGodsWrath :: Precache( void )
{
	PRECACHE_SOUND ("misc/tesla.wav");
	PRECACHE_SOUND ("hipweap/mjolhit.wav");   // lightning sound
}

void CGodsWrath :: Spawn( void )
{
	Precache();

	m_flWait = 0.0f;

	if( pev->dmg == 0.0f )
		pev->dmg = 2.0f + ( 5.0f * g_iSkillLevel );

	if( pev->health == 0.0f )
		pev->health = -1;

	if( pev->frags == 0.0f )
		pev->frags = 600.0f;

	if( pev->spawnflags & 1 )
		m_flCnt = (FL_CLIENT|FL_MONSTER);
	else m_flCnt = FL_CLIENT;

	if( m_flDelay == 0.0f )
		m_flDelay = 5.0f;

	SetThink( &CRadiusTrap::TeslaThink );
	m_hActivator = gpWorld;
	pev->nextthink = -1;
	m_iNumTargets = 0;
	pev->impulse = 0;
	pev->button = 1;

	m_iDeathType = MAKE_STRING( "suffers the wrath of God" );
}

void CGodsWrath :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( pev->impulse == 0 )
	{
		pev->air_finished = gpGlobals->time + m_flDelay;
		m_hActivator = pActivator;
		TeslaThink ();
	}
}

/*QUAKED trap_gravity_well (.8 .5 0) (-8 -8 -8) (8 8 8) targetenemies UNDERWATER
targets enemies in vicinity and draws them near, gibbing them on contact.

UNDERWATER cuts the pull in half for players wearing the wetsuit

"distance" how far the gravity well should reach (600 default)
"count" number of people to target (2 default)
"speed" is how strong the pull is. (210 default)
"dmg" is how much damage to do each touch. (10000 default)
*/
class CGravityWell : public CRadiusTrap
{
public:
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void Precache( void );
};

LINK_ENTITY_TO_CLASS( trap_gravity_well, CGravityWell );

void CGravityWell :: Spawn( void )
{
	Precache ();

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;

	if( pev->dmg == 0.0f )
		pev->dmg = 800.0f;
	if( pev->speed == 0.0f )
		pev->speed = 210.0f;
	if( pev->frags == 0.0f )
		pev->frags = 600.0f;

	if( pev->spawnflags & 1 )
		m_flCnt = (FL_CLIENT|FL_MONSTER);
	else m_flCnt = FL_CLIENT;

	if( pev->spawnflags & 1 )
		m_flCnt = (FL_CLIENT|FL_MONSTER);
	else m_flCnt = FL_CLIENT;

	SetThink( &CRadiusTrap::GravityThink );
	m_hActivator = gpWorld;
	pev->nextthink = gpGlobals->time + 0.1f;
	pev->ltime = gpGlobals->time;
	pev->pain_finished = 0;
	m_iNumTargets = 0;
	pev->impulse = 0;
	pev->button = 1;
}

//
// gravity_well isn't have model, so we need restore them size here
//
void CGravityWell :: Precache( void )
{
	UTIL_SetSize( pev, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ));
}

void CGravityWell :: Touch( CBaseEntity *pOther )
{
	if( pev->pain_finished > gpGlobals->time )
		return;

	if( pOther->pev->takedamage )
	{
		pOther->TakeDamage( pev, pev, pev->dmg, DMG_GENERIC );
		pev->pain_finished = gpGlobals->time + 0.2f;
	}
}

/*QUAKED trap_spike_mine (0 .5 .8) (-16 -16 0) (16 16 32)
*/
class CSpikeMine : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void EXPORT SpikeHome( void );
	void EXPORT FirstThink( void );
	void EXPORT SpikeTouch( CBaseEntity *pOther );
	void MonsterKilled( entvars_t *pevAttacker, int iGib );
	BOOL MonsterHasPain( void ) { return FALSE; }
	int BloodColor( void ) { return BLOOD_COLOR_RED; }
};

LINK_ENTITY_TO_CLASS( trap_spike_mine, CSpikeMine );

void CSpikeMine :: Precache( void )
{
	PRECACHE_MODEL ("models/spikmine.mdl");
	PRECACHE_SOUND ("weapons/r_exp3.wav");
	PRECACHE_SOUND ("hipitems/spikmine.wav");
}

void CSpikeMine :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ))
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache ();

	SET_MODEL(ENT(pev), "models/spikmine.mdl");
	UTIL_SetSize( pev, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ));

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_FLYMISSILE;
	pev->avelocity = Vector( -50, 100, 150 );

	if( g_iSkillLevel <= 1.0f )
		pev->health = 200;
	else pev->health = 400;

	SetThink( &CSpikeMine::FirstThink );
	SetTouch( &CSpikeMine::SpikeTouch );
	pev->nextthink = gpGlobals->time + 0.2f;

	// add one monster to stat
	gpWorld->total_monsters++;

	pev->flags |= FL_MONSTER;
	m_iDeathType = MAKE_STRING( "was blasted by a spike mine" );
}

void CSpikeMine :: FirstThink( void )
{
	SetThink( &CSpikeMine::SpikeHome );

	pev->nextthink = gpGlobals->time + 0.1f;
	pev->air_finished = 0; // search time
	pev->takedamage = DAMAGE_AIM;
	pev->animtime = gpGlobals->time + 0.1f;
	pev->framerate = 1.0f;

	SetUse( &CQuakeMonster::MonsterUse );
}

void CSpikeMine :: SpikeHome( void )
{
	CBaseEntity *pEnt = NULL, *pBest = NULL;
	float flStartDist = 2000;

	pev->nextthink = gpGlobals->time + 0.2f;

	if( pev->air_finished < gpGlobals->time )
	{
		// g-cont. replace this with UTIL_FindEntitiesInPVS ?
		while(( pEnt = UTIL_FindEntityInSphere( pEnt, pev->origin, 2000.0f )) != NULL )
		{
			if( !FBitSet( pEnt->pev->flags, FL_NOTARGET ) && FBitSet( pEnt->pev->flags, FL_CLIENT ))
			{
				if (TargetVisible( pEnt ) && (pEnt->pev->health > 0))
				{
					float flDist = (pEnt->pev->origin - pev->origin).Length();
					if( flDist < flStartDist )
					{
						flStartDist = flDist;
						pBest = pEnt;
					}
				}
			}
		}
	}

	if( pBest )
	{
		EMIT_SOUND (ENT(pev), CHAN_VOICE, "hipitems/spikmine.wav", 1, ATTN_NORM);
		pev->air_finished = gpGlobals->time + 1.3f;
		m_hEnemy = pBest;
	}

	if (m_hEnemy == NULL)
	{
		EMIT_SOUND (ENT(pev), CHAN_VOICE, "misc/null.wav", 1, ATTN_NORM);
		pev->velocity = g_vecZero;
		return;
	}

	Vector vtemp = m_hEnemy->pev->origin + Vector( 0.0f, 0.0f, 10.0f );
	Vector dir = ( vtemp - pev->origin ).Normalize();

	if( InFront( m_hEnemy ))
	{
		pev->velocity = dir * ((g_iSkillLevel * 50.0f) + 50.0f);
	}
	else
	{
		pev->velocity = dir * ((g_iSkillLevel * 50.0f) + 150.0f);
	}
}

void CSpikeMine :: SpikeTouch( CBaseEntity *pOther )
{
	TraceResult tr = UTIL_GetGlobalTrace();

	if( pev->health > 0.0f )
	{
		if (FClassnameIs( pOther->pev, "trap_spike_mine" ))
			return;
		if (FClassnameIs( pOther->pev, "missile" ))
			return;
		if (FClassnameIs( pOther->pev, "grenade" ))
			return;
		if (FClassnameIs( pOther->pev, "hiplaser" ))
			return;
		if (FClassnameIs( pOther->pev, "proximity_grenade" ))
			return;

		TakeDamage( pev, pev, pev->health + 10.0f, DMG_GENERIC );
	}

	Q_RadiusDamage( this, this, 110, this );

	pev->origin = pev->origin - 8 * pev->velocity.Normalize();

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_EXPLOSION );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
	MESSAGE_END();

	UTIL_DecalTrace( &tr, DECAL_SCORCH1 + RANDOM_LONG( 0, 1 ));

	UTIL_Remove( this );
	SetTouch( NULL );
}

void CSpikeMine :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	Q_RadiusDamage( this, this, 110, this );

	pev->origin = pev->origin - 8 * pev->velocity.Normalize();

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_EXPLOSION );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
	MESSAGE_END();

	UTIL_Remove( this );
	SetTouch( NULL );
}
