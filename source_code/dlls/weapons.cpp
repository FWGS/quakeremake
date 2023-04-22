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

===== weapons.cpp ========================================================

  Quake weaponry

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "decals.h"
#include "items.h"
#include "skill.h"

extern DLL_GLOBAL BOOL		g_fXashEngine;

DLL_GLOBAL short	g_sModelIndexBubbles;// holds the index for the bubbles model
DLL_GLOBAL short	g_sModelIndexBloodDrop;// holds the sprite index for the initial blood
DLL_GLOBAL short	g_sModelIndexBloodSpray;// holds the sprite index for splattered blood

/*
==============================================================================

MULTI-DAMAGE

Collects multiple small damages into a single damage

==============================================================================
*/

MULTIDAMAGE gMultiDamage;

//
// ClearMultiDamage - resets the global multi damage accumulator
//
void ClearMultiDamage(void)
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount	= 0;
}

//
// ApplyMultiDamage - inflicts contents of global multi damage register on gMultiDamage.pEntity
//
void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker )
{
	if ( !gMultiDamage.pEntity )
		return;

	if( gMultiDamage.pEntity->pev->takedamage )
		gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, DMG_GENERIC );
}

void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType)
{
	if ( !pEntity )
		return;

	if ( pEntity != gMultiDamage.pEntity )
	{
		ApplyMultiDamage(pevInflictor,pevInflictor); // UNDONE: wrong attacker!
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount	= flDamage;
	}

	gMultiDamage.amount += flDamage;
}

BOOL IsSkySurface( CBaseEntity *pEnt, const Vector &point, const Vector &vecDir )
{
	Vector vecSrc = point + vecDir * -2.0f;
	Vector vecEnd = point + vecDir * 2.0f;

	const char *pTex = TRACE_TEXTURE( pEnt->edict(), vecSrc, vecEnd );

	if( pTex != NULL && !strnicmp( pTex, "sky", 3 ))
		return TRUE;
	return FALSE;
}

LINK_ENTITY_TO_CLASS( spike, CNail );
#ifdef HIPNOTIC
LINK_ENTITY_TO_CLASS( hiplaser, CNail );
#endif /* HIPNOTIC */

//=========================================================
CNail *CNail::CreateNail( Vector vecOrigin, Vector vecDir, CBaseEntity *pOwner )
{
	CNail *pNail = GetClassPtr((CNail *)NULL );

	UTIL_SetOrigin( pNail->pev, vecOrigin );

	pNail->pev->velocity = vecDir * 1000;
	pNail->pev->angles = UTIL_VecToAngles( vecDir );
	pNail->pev->owner = pOwner->edict();
	pNail->Spawn();
	pNail->pev->classname = MAKE_STRING( "spike" );
	pNail->pev->animtime = gpGlobals->time;
	pNail->pev->framerate = 1.0;

	return pNail;
}

CNail *CNail::CreateSuperNail( Vector vecOrigin, Vector vecDir, CBaseEntity *pOwner )
{
	CNail *pNail = CreateNail( vecOrigin, vecDir, pOwner );

	// super nails simply do more damage
	pNail->pev->dmg = 18;
	pNail->pev->skin = 1;
	return pNail;
}

CNail *CNail::CreateKnightSpike( Vector vecOrigin, Vector vecDir, CBaseEntity *pOwner )
{
	CNail *pNail = CreateNail( vecOrigin, vecDir, pOwner );

	SET_MODEL( ENT(pNail->pev), "models/k_spike.mdl" );
	pNail->pev->velocity = vecDir * 300; // knightspikes goes slow
	pNail->pev->effects |= EF_FULLBRIGHT;

	return pNail;
}

#ifdef HIPNOTIC
CNail *CNail::CreateHipLaser( Vector vecOrigin, Vector vecDir, CBaseEntity *pOwner )
{
	CNail *pLaser = GetClassPtr((CNail *)NULL );

	EMIT_SOUND (ENT(pOwner->pev),CHAN_WEAPON, "hipweap/laserg.wav", 1, ATTN_NORM);
	UTIL_SetOrigin( pLaser->pev, vecOrigin );

	pLaser->pev->speed = 1000.0f;
	pLaser->pev->velocity = vecDir * pLaser->pev->speed;
	pLaser->pev->movedir = pLaser->pev->velocity;
	pLaser->pev->angles = UTIL_VecToAngles( vecDir );
	pLaser->pev->owner = pOwner->edict();
	pLaser->pev->enemy = pOwner->edict();
	pLaser->Spawn();
	SET_MODEL( ENT(pLaser->pev), "models/lasrspik.mdl" );
	pLaser->pev->classname = MAKE_STRING( "hiplaser" );
	pLaser->pev->animtime = gpGlobals->time;
	pLaser->pev->avelocity = Vector( 0, 0, 400 );
	pLaser->pev->air_finished = gpGlobals->time + 5.0f;
	pLaser->SetThink( &CNail::LaserThink );
	pLaser->pev->nextthink = gpGlobals->time + 0.1f;
	pLaser->SetTouch( &CNail::LaserTouch );
	pLaser->pev->framerate = 1.0f;
	pLaser->pev->dmg = 18;

	if( RANDOM_FLOAT(0.0f, 1.0f) < 0.15f)
		pLaser->pev->effects |= EF_DIMLIGHT;

	return pLaser;
}

#endif /* HIPNOTIC */
//=========================================================
void CNail::Spawn( void )
{
	Precache();

	// Setup
	pev->movetype = MOVETYPE_FLYMISSILE;
	pev->solid = SOLID_BBOX;
	
	// Safety removal
	pev->nextthink = gpGlobals->time + 6;
	SetThink( SUB_Remove );
	
	// Touch
	SetTouch( NailTouch );

	// Model
	SET_MODEL( ENT(pev), "models/spike.mdl" );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	// Damage
	pev->dmg = 9;
}

//=========================================================
void CNail::NailTouch( CBaseEntity *pOther )
{
	if (pOther->pev->solid == SOLID_TRIGGER)
		return;

	TraceResult tr = UTIL_GetGlobalTrace();

	// Remove if we've hit skybrush
	if ( IsSkySurface( pOther, tr.vecEndPos, pev->velocity.Normalize( ) ) )
	{
		UTIL_Remove( this );
		return;
	}

	// Hit something that bleeds
	if (pOther->pev->takedamage)
	{
		CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
		if( !pOwner ) pOwner = gpWorld;

		if ( g_pGameRules->PlayerRelationship( pOther, pOwner ) != GR_TEAMMATE )
			SpawnBlood( pev->origin, pOther->BloodColor(), pev->dmg );

		if( !FNullEnt( pOwner ))
			pOther->TakeDamage( pev, pOwner->pev, pev->dmg, DMG_GENERIC );
		else 
			pOther->TakeDamage( pev, pev, pev->dmg, DMG_GENERIC );
	}
	else
	{
		if ( pOther->pev->solid == SOLID_BSP || pOther->pev->movetype == MOVETYPE_PUSHSTEP )
		{
			Vector vecDir = pev->velocity.Normalize( );

			CBaseEntity *pEntity = NULL;
			Vector point = pev->origin - vecDir * 3;

			MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
				if( FStrEq( STRING (pev->model), "models/w_spike.mdl" ))
					WRITE_BYTE( TE_WIZSPIKE );
				else if( FStrEq( STRING (pev->model), "models/k_spike.mdl" ))
					WRITE_BYTE( TE_KNIGHTSPIKE );
				else
					WRITE_BYTE( pev->skin ? TE_SUPERSPIKE : TE_SPIKE );

				WRITE_COORD( point.x );
				WRITE_COORD( point.y );
				WRITE_COORD( point.z );
			MESSAGE_END();

			if ( strcmp( "models/spike.mdl", STRING(pev->model)) )
			{
				// wizard and knight spikes aren't stuck in the walls
				UTIL_Remove( this );
				return;
			}

			// too many overlapped spikes looks ugly
			while ((pEntity = UTIL_FindEntityInSphere( pEntity, point, 1 )) != NULL)
			{
				if( FClassnameIs( pEntity->pev, "spike" ) && pEntity != this )
				{
					UTIL_Remove( this );
					return;
				}
                              }

			UTIL_SetOrigin( pev, point );
			pev->angles = UTIL_VecToAngles( vecDir );
			pev->solid = SOLID_NOT;
			pev->velocity = Vector( 0, 0, 0 );
			pev->angles.z = RANDOM_LONG(0, 360);

			if (g_fXashEngine)
			{
				// g-cont. Setup movewith feature
				pev->movetype = MOVETYPE_COMPOUND;	// set movewith type
				pev->aiment = ENT( pOther->pev );	// set parent
			}

			SetTouch( NULL );
			return;	// stay in brush
		}
	}

	UTIL_Remove( this );
}

void CNail::ExplodeTouch( CBaseEntity *pOther )
{
	if( pOther->edict() == pev->owner )
		return; // don't explode on owner

	TraceResult tr = UTIL_GetGlobalTrace();

	// Remove if we've hit skybrush
	if ( IsSkySurface( pOther, tr.vecEndPos, pev->velocity.Normalize( ) ) )
	{
		UTIL_Remove( this );
		return;
	}

	UTIL_ScreenShake( pev->origin, 20.0f, 4.0f, 0.7f, 350.0f );

	float dmg = 100 + RANDOM_FLOAT( 0.0f, 20.0f );
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if( !pOwner ) pOwner = gpWorld;

	if( pOther->pev->health && pOther->pev->takedamage )
	{
		if( FClassnameIs( pOther->pev, "monster_shambler" ))
			dmg *= 0.5f; // mostly immune
		pOther->TakeDamage( pev, pOwner->pev, dmg, DMG_BLAST );
	}

	Q_RadiusDamage( this, pOwner, 120, pOther );

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

#ifdef HIPNOTIC
void CNail::LaserTouch( CBaseEntity *pOther )
{
	pev->owner = NULL;
	pev->impulse++;

	TraceResult tr = UTIL_GetGlobalTrace();

	// Remove if we've hit skybrush
	if ( IsSkySurface( pOther, tr.vecEndPos, pev->velocity.Normalize( ) ) )
	{
		UTIL_Remove( this );
		return;
	}

	Vector oldvel = pev->movedir.Normalize();
	Vector spot1 = pev->origin - (16.0f * oldvel);
	Vector spot2 = pev->origin + (16.0f * oldvel);
	UTIL_TraceLine( spot1, spot2, dont_ignore_monsters, edict(), &tr );

	pev->origin = tr.vecEndPos;

	if (pOther->pev->takedamage && pOther->pev->health)
	{
		if (pev->enemy == pOther->edict())
			pev->dmg *= 0.5f;

		SpawnBlood( pev->origin, pOther->BloodColor(), pev->dmg );// a little surface blood.
		TraceBleed( pev->dmg, oldvel, &tr, DMG_GENERIC );
		pOther->TakeDamage( pev, VARS(pev->enemy), pev->dmg, DMG_GENERIC );
	}
	else if ((pev->impulse == 3) || (RANDOM_FLOAT(.0f, 1.0f) < 0.15f))
	{
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_GUNSHOT );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
		MESSAGE_END();
	}
	else
	{
		pev->dmg *= 0.9f;
		pev->velocity = oldvel+(2*tr.vecPlaneNormal);
		pev->velocity = pev->velocity.Normalize() * pev->speed;
		pev->movedir = pev->velocity;
		pev->flags &= ~FL_ONGROUND;

		EMIT_SOUND (ENT(pev), CHAN_WEAPON, "hipweap/laserric.wav", 1, ATTN_STATIC);
		return;
	}

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "enforcer/enfstop.wav", 1, ATTN_STATIC);
	UTIL_Remove(this);
}

void CNail::LaserThink( void )
{
	if( gpGlobals->time > pev->air_finished)
	{
		SetTouch( NULL );
		UTIL_Remove( this );
		return;
	}

	pev->flags &= ~FL_ONGROUND;

	pev->velocity = pev->movedir;
	pev->angles = UTIL_VecToAngles(pev->velocity);
	pev->nextthink = gpGlobals->time + 0.1f;
}

#endif /* HIPNOTIC */
LINK_ENTITY_TO_CLASS( laser, CLaser );

//=========================================================
CLaser *CLaser::LaunchLaser( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CLaser *pLaser = GetClassPtr((CLaser *)NULL );

	UTIL_SetOrigin( pLaser->pev, vecOrigin );

	pLaser->pev->velocity = vecAngles * 600;
	pLaser->pev->angles = UTIL_VecToAngles( vecAngles );
#ifdef HIPNOTIC
	pLaser->pev->spawnflags = pOwner->pev->spawnflags;
#endif /* HIPNOTIC */
	pLaser->pev->owner = pOwner->edict();
	pLaser->Spawn();
	pLaser->pev->classname = MAKE_STRING( "laser" );

	return pLaser;
}

//=========================================================
void CLaser::Spawn( void )
{
	Precache();

	// Setup
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;
	pev->effects = EF_DIMLIGHT;
	
	// Safety removal
	pev->nextthink = gpGlobals->time + 5;
	SetThink( SUB_Remove );
	
	// Touch
	SetTouch( LaserTouch );

	// Model
	SET_MODEL( ENT(pev), "models/laser.mdl" );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	// Damage
	pev->dmg = 9;
}

//=========================================================
void CLaser::LaserTouch( CBaseEntity *pOther )
{
	if (pOther->pev->solid == SOLID_TRIGGER)
		return;

	TraceResult tr = UTIL_GetGlobalTrace();

	// Remove if we've hit skybrush
	if ( IsSkySurface( pOther, tr.vecEndPos, pev->velocity.Normalize( ) ) )
	{
		UTIL_Remove( this );
		return;
	}

#ifndef HIPNOTIC
	EMIT_SOUND(ENT(pOther->pev), CHAN_WEAPON, "enforcer/enfstop.wav", 1, ATTN_NORM);
#else /* HIPNOTIC */
	if( !FBitSet( pev->spawnflags, SF_TRAP_SILENT ))
		EMIT_SOUND(ENT(pOther->pev), CHAN_WEAPON, "enforcer/enfstop.wav", 1, ATTN_NORM);
#endif /* HIPNOTIC */

	// Hit something that bleeds
	if (pOther->pev->takedamage)
	{
		CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
		if( !pOwner ) pOwner = gpWorld;

		if ( g_pGameRules->PlayerRelationship( pOther, pOwner ) != GR_TEAMMATE )
			SpawnBlood( pev->origin, pOther->BloodColor(), pev->dmg );

		pOther->TakeDamage( pev, pOwner->pev, pev->dmg, DMG_GENERIC );
	}
	else
	{
		if ( pOther->pev->solid == SOLID_BSP || pOther->pev->movetype == MOVETYPE_PUSHSTEP )
		{
			Vector vecDir = pev->velocity.Normalize( );
			Vector point = pev->origin - vecDir * 8;

			MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
				WRITE_BYTE( TE_GUNSHOT );
				WRITE_COORD( point.x );
				WRITE_COORD( point.y );
				WRITE_COORD( point.z );
			MESSAGE_END();
		}
	}

	UTIL_Remove( this );
}

LINK_ENTITY_TO_CLASS( grenade, CRocket );
LINK_ENTITY_TO_CLASS( rocket, CRocket );
#ifdef HIPNOTIC
LINK_ENTITY_TO_CLASS( proximity_grenade, CRocket );
#endif /* HIPNOTIC */

//=========================================================
CRocket *CRocket::CreateRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CRocket *pRocket = GetClassPtr( (CRocket *)NULL );
	
	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	SET_MODEL(ENT(pRocket->pev), "models/missile.mdl");
	pRocket->Spawn();
	pRocket->pev->classname = MAKE_STRING("missile");
	pRocket->pev->owner = pOwner->edict();

	// Setup
	pRocket->pev->movetype = MOVETYPE_FLYMISSILE;
	pRocket->pev->solid = SOLID_BBOX;
		
	// Velocity
	pRocket->pev->velocity = vecAngles * 1000;
	pRocket->pev->angles = UTIL_VecToAngles( vecAngles );
	
	// Touch
	pRocket->SetTouch( CRocket::RocketTouch );

	// Safety Remove
	pRocket->pev->nextthink = gpGlobals->time + 5;
	pRocket->SetThink( SUB_Remove );

	// Effects
//	pRocket->pev->effects |= EF_LIGHT;

	return pRocket;
} 

//=========================================================
CRocket *CRocket::CreateGrenade( Vector vecOrigin, Vector vecVelocity, CBaseEntity *pOwner )
{
	CRocket *pRocket = GetClassPtr( (CRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	SET_MODEL(ENT(pRocket->pev), "models/grenade.mdl");
	pRocket->Spawn();
	pRocket->pev->classname = MAKE_STRING("grenade");
	pRocket->pev->owner = pOwner->edict();

	// Setup
	pRocket->pev->movetype = MOVETYPE_BOUNCE;
	pRocket->pev->solid = SOLID_BBOX;

	pRocket->pev->avelocity = Vector(300,300,300);
	
	// Velocity
	pRocket->pev->velocity = vecVelocity;
	pRocket->pev->angles = UTIL_VecToAngles(vecVelocity);
	pRocket->pev->friction = 0.5;

	// Touch
	pRocket->SetTouch( CRocket::GrenadeTouch );

	if (FClassnameIs( pOwner->pev, "monster_ogre"))
		pRocket->pev->dmg = 40.0f;
	else pRocket->pev->dmg = 120.0f;

	// set newmis duration
	if ( gpGlobals->deathmatch == 4 )
	{
		pRocket->m_flAttackFinished = gpGlobals->time + 1.1;	// What's this used for?
		if (pOwner)
			pOwner->TakeDamage( pOwner->pev, pOwner->pev, 10, DMG_GENERIC );
	}

	pRocket->pev->nextthink = gpGlobals->time + 2.5;
	pRocket->SetThink( CRocket::GrenadeExplode );

	return pRocket;
}

#ifdef HIPNOTIC
CRocket *CRocket::CreateProxGrenade( Vector vecOrigin, Vector vecVelocity, CBaseEntity *pOwner )
{
	CRocket *pRocket = GetClassPtr( (CRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	SET_MODEL(ENT(pRocket->pev), "models/proxbomb.mdl");
	UTIL_SetSize(pRocket->pev, Vector( -1, -1, -1), Vector(1, 1, 1));
	pRocket->pev->classname = MAKE_STRING("proximity_grenade");
	pRocket->pev->owner = pOwner->edict();
	pRocket->pev->enemy = pOwner->edict();

	// Setup
	pRocket->pev->movetype = MOVETYPE_TOSS;
	pRocket->pev->effects |= EF_FULLBRIGHT;	// FIXME: make an additive fullbright texture for bomb model
	pRocket->pev->solid = SOLID_BBOX;

	pRocket->pev->avelocity = Vector(100,600,100);
	
	// Velocity
	pRocket->pev->velocity = vecVelocity;
	pRocket->pev->angles = UTIL_VecToAngles(vecVelocity);

	// Touch
	pRocket->SetTouch( &CRocket::ProximityTouch );
	pRocket->pev->dmg = 95.0f;

	// set newmis duration
	if ( gpGlobals->deathmatch == 4 )
	{
		pRocket->m_flAttackFinished = gpGlobals->time + 1.1;	// What's this used for?
		if (pOwner)
			pOwner->TakeDamage( pOwner->pev, pOwner->pev, 10, DMG_GENERIC );
	}

	pRocket->pev->nextthink = gpGlobals->time + 2.0f;
	pRocket->SetThink( &CRocket::ProximityThink );
	pRocket->m_flDelay = gpGlobals->time + 15.0f + RANDOM_FLOAT( 0.0f, 10.0f );
   
	return pRocket;
}

#endif /* HIPNOTIC */
//=========================================================
void CRocket::Spawn( void )
{
	Precache();

	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );
}

//=========================================================
void CRocket::RocketTouch ( CBaseEntity *pOther )
{
	TraceResult tr = UTIL_GetGlobalTrace();

	// Remove if we've hit skybrush
	if ( IsSkySurface( pOther, tr.vecEndPos, pev->velocity.Normalize( ) ) )
	{
		UTIL_Remove( this );
		return;
	}

	// Do touch damage
	float flDmg = RANDOM_FLOAT( 100, 120 );
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if( !pOwner ) pOwner = gpWorld;

	if (pOther->pev->health && pOther->pev->takedamage)
	{
		if (FClassnameIs( pOther->pev, "monster_shambler"))
			flDmg *= 0.5f; // mostly immune

		pOther->TakeDamage( pev, pOwner->pev, flDmg, DMG_GENERIC );
	}

	// Don't do radius damage to the other, because all the damage was done in the impact
	Q_RadiusDamage(this, pOwner, 120, pOther);

	// Finish and remove
	Explode();
}

//=========================================================
void CRocket::GrenadeTouch( CBaseEntity *pOther )
{
	if (pOther->pev->takedamage == DAMAGE_AIM)
	{
		GrenadeExplode();
		return;
	}

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/bounce.wav", 1, ATTN_NORM);

	if (pev->velocity == g_vecZero)
		pev->avelocity = g_vecZero;
}

//=========================================================
void CRocket::GrenadeExplode()
{
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if( !pOwner ) pOwner = gpWorld;

	Q_RadiusDamage(this, pOwner, pev->dmg, NULL);

	// Finish and remove
	Explode();
}

//=========================================================
void CRocket::Explode()
{
	pev->origin = pev->origin - 8 * pev->velocity.Normalize();

	UTIL_ScreenShake( pev->origin, 16.0f, 2.0f, 0.5f, 250.0f );

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_EXPLOSION );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
	MESSAGE_END();

	UTIL_Remove( this );
}

#ifdef HIPNOTIC
void CRocket::ProximityThink( void )
{
	if ((gpGlobals->time > m_flDelay) || (gpWorld->num_prox_grenades > 15))
	{
		ProximityGrenadeExplode();
		return;
	}

	pev->owner = NULL;
	pev->takedamage = DAMAGE_YES;

	TraceResult tr;
	CBaseEntity *pEnt = NULL;
	BOOL bBlowup = FALSE;

	while(( pEnt = UTIL_FindEntityInSphere( pEnt, pev->origin, 140 )) != NULL )
	{
		if (( pEnt != this ) && (pEnt->pev->health > 0.0f) && (pEnt->pev->flags & (FL_CLIENT|FL_MONSTER)) && !FStrEq( STRING( pEnt->pev->classname ), STRING( pev->classname ) ))
		{
			// velocity based detector for player
			if( pEnt->pev->flags & FL_CLIENT && pEnt->pev->velocity == g_vecZero );
			else bBlowup = TRUE;
                    }

		if( FStrEq( STRING( pEnt->pev->classname ), STRING( pev->classname ) ) && pEnt->pev->movetype != MOVETYPE_COMPOUND)
			bBlowup = TRUE;

		UTIL_TraceLine(pev->origin, pEnt->pev->origin, ignore_monsters, ENT(pev), &tr);
		if (tr.flFraction != 1.0f)
			bBlowup = FALSE;	// cancelling explosion

		if (bBlowup)
		{
			EMIT_SOUND (ENT(pev), CHAN_WEAPON, "hipweap/proxwarn.wav", 1, ATTN_NORM);
			ProximityGrenadeExplode();
			pev->nextthink = gpGlobals->time + 0.3f;
			return;
		}
	}

	pev->nextthink = gpGlobals->time + 0.25f;
}

void CRocket::ProximityGrenadeExplode( void )
{
	gpWorld->num_prox_grenades--;
	m_iDeathType = MAKE_STRING( "exploding" );
	pev->nextthink = gpGlobals->time + 0.1f;
	pev->owner = pev->enemy;
	SetThink( GrenadeExplode );
}

void CRocket::ProximityTouch( CBaseEntity *pOther )
{
	if( FStrEq( STRING( pOther->pev->classname ), STRING( pev->classname ) ))
		return;

	if (pev->movetype == MOVETYPE_COMPOUND)
		return;

	if( pOther->pev->takedamage == DAMAGE_AIM)
	{
		ProximityGrenadeExplode();
		return;
	}

	if( pOther->pev->solid != SOLID_BSP )
		return;

	// nice point to attach for
	UTIL_SetSize(pev, Vector( -8, -8, -8), Vector(8, 8, 8));
	EMIT_SOUND (ENT(pev), CHAN_WEAPON, "weapons/bounce.wav", 1, ATTN_NORM); // bounce sound

	pev->movetype = MOVETYPE_COMPOUND;	// set movewith type
	pev->aiment = pOther->edict();	// set parent
	SetTouch( NULL );
}

#endif /* HIPNOTIC */
LINK_ENTITY_TO_CLASS( zombie_missile, CZombieMissile );

CZombieMissile *CZombieMissile :: CreateMissile( Vector vecOrigin, Vector vecOffset, Vector vecAngles, CBaseEntity *pOwner )
{
	if( !pOwner || pOwner->m_hEnemy == NULL )
		return NULL;

	UTIL_MakeVectors( vecAngles );

	// calc org
	Vector org = vecOrigin + vecOffset.x * gpGlobals->v_forward + vecOffset.y * gpGlobals->v_right + (vecOffset.z - 24) * gpGlobals->v_up;

	CZombieMissile *pMeat = GetClassPtr((CZombieMissile *)NULL );

	UTIL_SetOrigin( pMeat->pev, org );

	pMeat->pev->classname = MAKE_STRING( "zombie_missile" );
	pMeat->pev->velocity = (pOwner->m_hEnemy->pev->origin - org).Normalize() * 600;
	pMeat->pev->velocity.z = 200;
	pMeat->pev->avelocity = Vector( 3000, 1000, 2000 );
	pMeat->pev->owner = pOwner->edict();
	pMeat->pev->solid = SOLID_BBOX;
	pMeat->Spawn();
	pMeat->SetTouch( MeatTouch );

	// Safety removal
	pMeat->pev->nextthink = gpGlobals->time + 2.5f;
	pMeat->SetThink( SUB_Remove );

	// done
	return pMeat;
}

CZombieMissile *CZombieMissile :: CreateSpray( Vector vecOrigin, Vector vecVelocity )
{
	CZombieMissile *pMeat = GetClassPtr((CZombieMissile *)NULL );

	UTIL_SetOrigin( pMeat->pev, vecOrigin );
	pMeat->pev->classname = MAKE_STRING( "zombie_missile" );
	pMeat->pev->velocity = vecVelocity;
	pMeat->pev->velocity.z += 250 + RANDOM_FLOAT( 0.0f, 50.0f );
	pMeat->pev->avelocity = Vector( 3000, 1000, 2000 );
	pMeat->pev->solid = SOLID_NOT;
	pMeat->Spawn();

	// Safety removal
	pMeat->pev->nextthink = gpGlobals->time + 1.0f;
	pMeat->SetThink( SUB_Remove );

	// done
	return pMeat;
}

void CZombieMissile :: Spawn( void )
{
	Precache();

	// Setup
	pev->movetype = MOVETYPE_BOUNCE;
	pev->dmg = 10;

	SET_MODEL( ENT(pev), "models/zom_gib.mdl" );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );
}

void CZombieMissile :: Precache( void )
{
	PRECACHE_MODEL( "models/zom_gib.mdl" );

	PRECACHE_SOUND( "zombie/z_hit.wav" );
	PRECACHE_SOUND( "zombie/z_miss.wav" );
}

void CZombieMissile :: MeatTouch( CBaseEntity *pOther )
{
	if( pOther->edict() == pev->owner )
		return; // don't explode on owner

	if( pOther->pev->takedamage )
	{
		CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);

		if( !FNullEnt( pOwner ))
			pOther->TakeDamage( pev, pOwner->pev, pev->dmg, DMG_GENERIC );
		else 
			pOther->TakeDamage( pev, pev, pev->dmg, DMG_GENERIC );

		EMIT_SOUND( edict(), CHAN_WEAPON, "zombie/z_hit.wav", 1.0, ATTN_NORM );
		UTIL_Remove( this );
		return;
	}

	EMIT_SOUND( edict(), CHAN_WEAPON, "zombie/z_miss.wav", 1.0, ATTN_NORM );	// bounce sound

	TraceResult tr = UTIL_GetGlobalTrace();
	UTIL_DecalTrace( &tr, DECAL_BLOOD1 + RANDOM_LONG( 0, 5 ));

	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;

	SetTouch( NULL );
	UTIL_Remove( this );
}

LINK_ENTITY_TO_CLASS( shal_missile, CShalMissile );

CShalMissile *CShalMissile :: CreateMissile( Vector vecOrigin, Vector vecVelocity )
{
	CShalMissile *pMiss = GetClassPtr((CShalMissile *)NULL );

	UTIL_SetOrigin( pMiss->pev, vecOrigin );
	pMiss->pev->classname = MAKE_STRING( "shal_missile" );
	pMiss->pev->avelocity = Vector( 300, 300, 300 );
	pMiss->pev->velocity = vecVelocity;
	pMiss->Spawn();

	// done
	return pMiss;
}

void CShalMissile :: Spawn( void )
{
	Precache();

	// Setup
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_FLYMISSILE;

	SET_MODEL( ENT(pev), "models/v_spike.mdl" );
	UTIL_SetSize(pev, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ));	// allow to explode with himself
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( ShalTouch );
	SetThink( ShalHome );
}

void CShalMissile :: Precache( void )
{
	PRECACHE_MODEL( "models/v_spike.mdl" );
}

void CShalMissile :: ShalHome( void )
{
	if( !pev->enemy || pev->enemy->v.health <= 0.0f )
	{
		CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
		if( pOwner ) pOwner->pev->impulse--; // decrease missiles		
		REMOVE_ENTITY( edict() );
		return;
	}

	CBaseEntity *pEnemy = CBaseEntity::Instance(pev->enemy);
	Vector vecTmp = pEnemy->pev->origin + Vector( 0, 0, 10 );
	Vector vecDir = ( vecTmp - pev->origin ).Normalize();

	if( g_iSkillLevel == SKILL_NIGHTMARE )
		pev->velocity = vecDir * 350.0f;
	else pev->velocity = vecDir * 250.0f;

	pev->nextthink = gpGlobals->time + 0.2f;
}

void CShalMissile :: ShalTouch( CBaseEntity *pOther )
{
	if( pOther->edict() == pev->owner )
		return; // don't explode on owner

	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);

	if( !pOwner ) pOwner = gpWorld; // shalrath is gibbed
	else pOwner->pev->impulse--; // decrease missiles

	if( FClassnameIs( pOther->pev, "monster_zombie" ))
		pOther->TakeDamage( pev, pev, 110, DMG_GENERIC );
	Q_RadiusDamage( this, pOwner, 40, gpWorld );

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_EXPLOSION );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
	MESSAGE_END();

	pev->velocity = g_vecZero;
	SetThink( NULL );
	SetTouch( NULL );

	UTIL_Remove( this );
}

#ifdef HIPNOTIC
LINK_ENTITY_TO_CLASS( mjolnir_beam, CMjolnirLightning );

CMjolnirLightning *CMjolnirLightning :: CreateLightning( CBaseEntity *pPrev, CBaseEntity *pOwner, const Vector &vecAngles, float flDist )
{
	CMjolnirLightning *pBolt = GetClassPtr((CMjolnirLightning *)NULL );

	UTIL_SetOrigin( pBolt->pev, pOwner->pev->origin );
	pBolt->pev->classname = MAKE_STRING( "mjolnir_beam" );
	pBolt->pev->owner = pOwner->edict();
	pBolt->pev->v_angle.y = vecAngles.y;
	pBolt->pev->enemy = pPrev->edict();	// lastvictim!!!
	pBolt->pev->frags = flDist;
	pBolt->Spawn();

	// done
	return pBolt;
}

void CMjolnirLightning :: Spawn( void )
{
	m_flDelay = gpGlobals->time + 0.8f;
	pev->nextthink = gpGlobals->time;	// fire immediately
}

BOOL CMjolnirLightning :: TargetVisible( CBaseEntity *pTarg )
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

void CMjolnirLightning :: Think( void )
{
	if (gpGlobals->time > m_flDelay)
	{
		if (m_hEnemy != NULL)
			m_hEnemy->m_bStruckByMjolnir = FALSE;
		UTIL_Remove(this);
		return;
	}

	TraceResult trace;
	int oldstate = pev->button;

	if (pev->button==0)
	{
		// look in our immediate vicinity
		m_hEnemy = NULL;
		CBaseEntity *pEnt = NULL, *pBest = NULL;
		float flStartDist = pev->frags;

		while(( pEnt = UTIL_FindEntityInSphere( pEnt, pev->owner->v.origin, pev->frags )) != NULL )
		{
			if( !FBitSet( pEnt->pev->flags, FL_NOTARGET ) && FBitSet( pEnt->pev->flags, FL_CLIENT|FL_MONSTER ))
			{
				if (TargetVisible( pEnt ) && pEnt->edict() != pev->owner->v.owner && (pEnt->pev->health > 0))
				{
					float flDist = (pEnt->pev->origin - pev->enemy->v.origin).Length();
					if( flDist < flStartDist && !pEnt->m_bStruckByMjolnir )
					{
						flStartDist = flDist;
						pBest = pEnt;
					}
				}
			}
		}

		if (pBest != NULL)
		{
			pev->button = 1;
			m_hEnemy = pBest;
			m_hEnemy->m_bStruckByMjolnir = TRUE;
		}
		else
		{
			UTIL_MakeVectors(pev->v_angle);
			Vector org = pev->owner->v.origin;
			Vector end = org + gpGlobals->v_forward * 200;
			end += gpGlobals->v_right * RANDOM_FLOAT(-200.0f, 200.0f);

			UTIL_TraceLine( org, end, ignore_monsters, edict(), &trace );

			MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
				WRITE_BYTE( TE_LIGHTNING2 );
				WRITE_ENTITY( entindex() );
				WRITE_COORD( org.x );
				WRITE_COORD( org.y );
				WRITE_COORD( org.z );
				WRITE_COORD( trace.vecEndPos.x );
				WRITE_COORD( trace.vecEndPos.y );
				WRITE_COORD( trace.vecEndPos.z );
			MESSAGE_END();

			pev->nextthink = gpGlobals->time + 0.1f;
			return;
		}
	}

	if (m_hEnemy == NULL)
	{
		pev->nextthink = gpGlobals->time + 0.1f;
		pev->button = 0;	// reset state
		return;
	}

	Vector org = pev->enemy->v.origin;
	Vector dst = m_hEnemy->pev->absmin + 0.25f * (m_hEnemy->pev->absmax - m_hEnemy->pev->absmin);
	dst += (RANDOM_FLOAT( 0.0f, 0.5f ) * (m_hEnemy->pev->absmax - m_hEnemy->pev->absmin));

	UTIL_TraceLine( org, dst, ignore_monsters, pev->owner->v.owner, &trace );

	if (trace.flFraction != 1.0f || m_hEnemy->pev->health <= 0.0f )
	{
		m_hEnemy->m_bStruckByMjolnir = FALSE;
		pev->nextthink = gpGlobals->time + 0.1f;
		pev->button = 0;	// reset state
		return;
	}

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_LIGHTNING2 );
		WRITE_ENTITY( entindex() );
		WRITE_COORD( org.x );
		WRITE_COORD( org.y );
		WRITE_COORD( org.z );
		WRITE_COORD( trace.vecEndPos.x );
		WRITE_COORD( trace.vecEndPos.y );
		WRITE_COORD( trace.vecEndPos.z );
	MESSAGE_END();

	Vector vec = (m_hEnemy->pev->origin - pev->owner->v.origin).Normalize();
	float flDot = DotProduct( vec, pev->owner->v.movedir );
	float flDmg;

	CBaseEntity *pAttacker = CBaseEntity::Instance(pev->owner->v.owner);

	if (oldstate==0)
		flDmg = 80.0f;
	else
		flDmg = 30.0f;

	if (flDot > 0.3f)
		CBasePlayer::LightningDamage (org, trace.vecEndPos, pAttacker, flDmg, vec );
	else
		CBasePlayer::LightningDamage (org, trace.vecEndPos, pAttacker, flDmg * 0.5f, vec );

	pev->nextthink = gpGlobals->time + 0.2f;
}

#endif /* HIPNOTIC */
// Plays quad sound if needed
void CBasePlayer::SuperDamageSound( void )
{
	if ( m_iItems & IT_QUAD )
	{
		if ( m_flSuperDamageSound < gpGlobals->time)
		{
			EMIT_SOUND( edict(), CHAN_ITEM, "items/damage3.wav", 1.0, ATTN_NORM );
			m_flSuperDamageSound = gpGlobals->time + 1;
		}
	}
}

//================================================================================================
// WEAPON SELECTION
//================================================================================================
// Return the ID of the best weapon being carried by the player
int CBasePlayer::W_BestWeapon()
{
	if (pev->waterlevel <= 1 && ammo_cells >= 1 && (m_iItems & IT_LIGHTNING) )
		return IT_LIGHTNING;
#ifdef HIPNOTIC
	else if(ammo_cells >= 1 && (m_iItems & IT_LASER_CANNON) )
		return IT_LASER_CANNON;
#endif /* HIPNOTIC */
	else if(ammo_nails >= 2 && (m_iItems & IT_SUPER_NAILGUN) )
		return IT_SUPER_NAILGUN;
	else if(ammo_shells >= 2 && (m_iItems & IT_SUPER_SHOTGUN) )
		return IT_SUPER_SHOTGUN;
	else if(ammo_nails >= 1 && (m_iItems & IT_NAILGUN) )
		return IT_NAILGUN;
	else if(ammo_shells >= 1 && (m_iItems & IT_SHOTGUN)  )
		return IT_SHOTGUN;
#ifndef HIPNOTIC
		
#else /* HIPNOTIC */
	else if( m_iItems & IT_MJOLNIR )
		return IT_MJOLNIR;
#endif /* HIPNOTIC */
	return IT_AXE;
}

// Weapon setup after weapon switch
void CBasePlayer::W_SetCurrentAmmo( int sendanim /* = 1 */ )
{
	m_iItems &= ~(IT_SHELLS | IT_NAILS | IT_ROCKETS | IT_CELLS);
#ifndef HIPNOTIC
	int	iszViewModel = 0;
#else /* HIPNOTIC */

	int iszViewModel = 0;
#endif /* HIPNOTIC */
	char *viewmodel = "";
	int iszWeaponModel = 0;
	char *szAnimExt;
	
	// Find out what weapon the player's using
	if (m_iWeapon == IT_AXE)
	{
		m_pCurrentAmmo = NULL;
		viewmodel = "models/v_axe.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		szAnimExt = "crowbar";
		iszWeaponModel = MAKE_STRING("models/p_crowbar.mdl");
	}
	else if (m_iWeapon == IT_SHOTGUN)
	{
		m_pCurrentAmmo = &ammo_shells;
		viewmodel = "models/v_shot.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_shot.mdl");
		m_iItems |= IT_SHELLS;
		szAnimExt = "shotgun";
	}
	else if (m_iWeapon == IT_SUPER_SHOTGUN)
	{
		m_pCurrentAmmo = &ammo_shells;
		viewmodel = "models/v_shot2.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_shot2.mdl");
		m_iItems |= IT_SHELLS;
		szAnimExt = "shotgun";
	}
	else if (m_iWeapon == IT_NAILGUN)
	{
		m_pCurrentAmmo = &ammo_nails;
		viewmodel = "models/v_nail.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_nail.mdl");
		m_iItems |= IT_NAILS;
		szAnimExt = "mp5";
	}
	else if (m_iWeapon == IT_SUPER_NAILGUN)
	{
		m_pCurrentAmmo = &ammo_nails;
		viewmodel = "models/v_nail2.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_nail2.mdl");
		m_iItems |= IT_NAILS;
		szAnimExt = "mp5";
	}
	else if (m_iWeapon == IT_GRENADE_LAUNCHER)
	{
		m_pCurrentAmmo = &ammo_rockets;
		viewmodel = "models/v_rock.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		m_iItems |= IT_ROCKETS;
		iszWeaponModel = MAKE_STRING("models/p_rock.mdl");
		szAnimExt = "gauss";
	}
	else if (m_iWeapon == IT_ROCKET_LAUNCHER)
	{
		m_pCurrentAmmo = &ammo_rockets;
		viewmodel = "models/v_rock2.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		m_iItems |= IT_ROCKETS;
		iszWeaponModel = MAKE_STRING("models/p_rock2.mdl");
		szAnimExt = "gauss";
	}
	else if (m_iWeapon == IT_LIGHTNING)
	{
		m_pCurrentAmmo = &ammo_cells;
		viewmodel = "models/v_light.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_light.mdl");
		m_iItems |= IT_CELLS;
		szAnimExt = "gauss";
	}
#ifdef HIPNOTIC
	else if (m_iWeapon == IT_LASER_CANNON)
	{
		m_pCurrentAmmo = &ammo_cells;
		viewmodel = "models/v_laser.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_laser.mdl");
		m_iItems |= IT_CELLS;
		szAnimExt = "gauss";
	}
	else if (m_iWeapon == IT_MJOLNIR)
	{
		m_pCurrentAmmo = &ammo_cells;
		viewmodel = "models/v_hammer.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_hammer.mdl");
		m_iItems |= IT_CELLS;
		szAnimExt = "crowbar";
	}
	else if (m_iWeapon == IT_PROXIMITY_GUN)
	{
		m_pCurrentAmmo = &ammo_rockets;
		viewmodel = "models/v_prox.mdl";
		iszViewModel = MAKE_STRING(viewmodel);
		iszWeaponModel = MAKE_STRING("models/p_prox.mdl");
		m_iItems |= IT_ROCKETS;
		szAnimExt = "gauss";
	}
#endif /* HIPNOTIC */
	else
	{
		m_pCurrentAmmo = NULL;
	}

	pev->viewmodel = iszViewModel;

	pev->weaponmodel = iszWeaponModel;
	strcpy( m_szAnimExtention, szAnimExt );
}

void CBasePlayer::CheckAmmo( void )
{
	if (ammo_shells > 100)
		ammo_shells = 100;
	if (ammo_nails > 200)
		ammo_nails = 200;
	if (ammo_rockets > 100)
		ammo_rockets = 100;               
	if (ammo_cells > 200)
		ammo_cells = 200;         
}

// Return TRUE if the weapon still has ammo
BOOL CBasePlayer::W_CheckNoAmmo( void )
{
	if ( m_pCurrentAmmo && *m_pCurrentAmmo > 0 )
		return TRUE;

	if ( m_iWeapon == IT_AXE )
		return TRUE;
#ifdef HIPNOTIC

	if ( m_iWeapon == IT_MJOLNIR )
		return TRUE;
#endif /* HIPNOTIC */
	
	m_iWeapon = W_BestWeapon();
	W_SetCurrentAmmo();
	return FALSE;
}

// Change to the specified weapon
void CBasePlayer::W_ChangeWeapon( int iWeaponNumber )
{
	int iWeapon = 0;
	BOOL bHaveAmmo = TRUE;
	
	if (iWeaponNumber == 1)
	{
		iWeapon = IT_AXE;
	}
	else if (iWeaponNumber == 2)
	{
		iWeapon = IT_SHOTGUN;
		if (ammo_shells < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 3)
	{
		iWeapon = IT_SUPER_SHOTGUN;
		if (ammo_shells < 2)
			bHaveAmmo = FALSE;
	}               
	else if (iWeaponNumber == 4)
	{
		iWeapon = IT_NAILGUN;
		if (ammo_nails < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 5)
	{
		iWeapon = IT_SUPER_NAILGUN;
		if (ammo_nails < 2)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 6)
	{
#ifndef HIPNOTIC
		iWeapon = IT_GRENADE_LAUNCHER;
#else /* HIPNOTIC */
		if (m_iWeapon == IT_GRENADE_LAUNCHER)
			iWeapon = IT_PROXIMITY_GUN;
		else
			iWeapon = IT_GRENADE_LAUNCHER;
#endif /* HIPNOTIC */
		if (ammo_rockets < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 7)
	{
		iWeapon = IT_ROCKET_LAUNCHER;
		if (ammo_rockets < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 8)
	{
		iWeapon = IT_LIGHTNING;
		
		if (ammo_cells < 1)
			bHaveAmmo = FALSE;
	}
#ifdef HIPNOTIC
	else if (iWeaponNumber == 225)
	{
		iWeapon = IT_LASER_CANNON;
		if (ammo_cells < 1)
			bHaveAmmo = FALSE;
	}
	else if (iWeaponNumber == 226)
	{
		iWeapon = IT_MJOLNIR;
	}
#endif /* HIPNOTIC */

	// Have the weapon?
	if ( !(m_iItems & iWeapon) )
	{       
#ifndef HIPNOTIC
		CLIENT_PRINTF( edict(), print_console, "no weapon.\n" );
		return;
#else /* HIPNOTIC */
		if (iWeapon == IT_GRENADE_LAUNCHER)
		{
			iWeapon = IT_PROXIMITY_GUN;

			if ( !(m_iItems & iWeapon) )
			{
				CLIENT_PRINTF( edict(), print_console, "no weapon.\n" );
				return;
			}

			if (ammo_rockets < 1)
				bHaveAmmo = FALSE;
		}
		else
		{
			CLIENT_PRINTF( edict(), print_console, "no weapon.\n" );
			return;
		}
#endif /* HIPNOTIC */
	}
	
	// Have ammo for it?
	if ( !bHaveAmmo )
	{
		CLIENT_PRINTF( edict(), print_console, "not enough ammo.\n" );
		return;
	}

	// Set weapon, update ammo
	m_iWeapon = iWeapon;
	W_SetCurrentAmmo();
}

// Go to the next weapon with ammo
void CBasePlayer::W_CycleWeaponCommand( void )
{
	while (1)
	{
		BOOL bHaveAmmo = TRUE;

#ifndef HIPNOTIC
		if (m_iWeapon == IT_LIGHTNING)
		{
			m_iWeapon = IT_EXTRA_WEAPON;
		}
		else if (m_iWeapon == IT_EXTRA_WEAPON)
#else /* HIPNOTIC */
		if (m_iWeapon == IT_MJOLNIR)
#endif /* HIPNOTIC */
		{
			m_iWeapon = IT_AXE;
		}
		else if (m_iWeapon == IT_AXE)
		{
			m_iWeapon = IT_SHOTGUN;
			if (ammo_shells < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_SHOTGUN)
		{
			m_iWeapon = IT_SUPER_SHOTGUN;
			if (ammo_shells < 2)
				bHaveAmmo = FALSE;
		}               
		else if (m_iWeapon == IT_SUPER_SHOTGUN)
		{
			m_iWeapon = IT_NAILGUN;
			if (ammo_nails < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_NAILGUN)
		{
			m_iWeapon = IT_SUPER_NAILGUN;
			if (ammo_nails < 2)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_SUPER_NAILGUN)
		{
			m_iWeapon = IT_GRENADE_LAUNCHER;
			if (ammo_rockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_GRENADE_LAUNCHER)
		{
#ifdef HIPNOTIC
			m_iWeapon = IT_PROXIMITY_GUN;
			if (ammo_rockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_GRENADE_LAUNCHER)
		{
			m_iWeapon = IT_ROCKET_LAUNCHER;
			if (ammo_rockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_PROXIMITY_GUN)
		{
#endif /* HIPNOTIC */
			m_iWeapon = IT_ROCKET_LAUNCHER;
			if (ammo_rockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_ROCKET_LAUNCHER)
		{
			m_iWeapon = IT_LIGHTNING;
			if (ammo_cells < 1)
				bHaveAmmo = FALSE;
		}
#ifdef HIPNOTIC
		else if (m_iWeapon == IT_LIGHTNING)
		{
			m_iWeapon = IT_LASER_CANNON;
			if (ammo_cells < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_LASER_CANNON)
		{
			m_iWeapon = IT_MJOLNIR;
		}
#endif /* HIPNOTIC */
	
		if ( (m_iItems & m_iWeapon) && bHaveAmmo )
		{
			W_SetCurrentAmmo();
			return;
		}
	}

}

// Go to the prev weapon with ammo
void CBasePlayer::W_CycleWeaponReverseCommand( void )
{
	while (1)
	{
		BOOL bHaveAmmo = TRUE;

#ifndef HIPNOTIC
		if (m_iWeapon == IT_EXTRA_WEAPON)
#else /* HIPNOTIC */
		if (m_iWeapon == IT_MJOLNIR)
		{
			m_iWeapon = IT_LASER_CANNON;
			if (ammo_cells < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_LASER_CANNON)
#endif /* HIPNOTIC */
		{
			m_iWeapon = IT_LIGHTNING;
#ifdef HIPNOTIC
			if (ammo_cells < 1)
				bHaveAmmo = FALSE;
#endif /* HIPNOTIC */
		}
		else if (m_iWeapon == IT_LIGHTNING)
		{
			m_iWeapon = IT_ROCKET_LAUNCHER;
			if (ammo_rockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_ROCKET_LAUNCHER)
		{
#ifdef HIPNOTIC
			m_iWeapon = IT_PROXIMITY_GUN;
			if (ammo_rockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_PROXIMITY_GUN)
		{
#endif /* HIPNOTIC */
			m_iWeapon = IT_GRENADE_LAUNCHER;
			if (ammo_rockets < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_GRENADE_LAUNCHER)
		{
			m_iWeapon = IT_SUPER_NAILGUN;
			if (ammo_nails < 2)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_SUPER_NAILGUN)
		{
			m_iWeapon = IT_NAILGUN;
			if (ammo_nails < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_NAILGUN)
		{
			m_iWeapon = IT_SUPER_SHOTGUN;
			if (ammo_shells < 2)
				bHaveAmmo = FALSE;
		}               
		else if (m_iWeapon == IT_SUPER_SHOTGUN)
		{
			m_iWeapon = IT_SHOTGUN;
			if (ammo_shells < 1)
				bHaveAmmo = FALSE;
		}
		else if (m_iWeapon == IT_SHOTGUN)
		{
			m_iWeapon = IT_AXE;
		}
		else if (m_iWeapon == IT_AXE)
		{
#ifndef HIPNOTIC
			m_iWeapon = IT_EXTRA_WEAPON;
		}
		else if (m_iWeapon == IT_EXTRA_WEAPON)
		{
			m_iWeapon = IT_LIGHTNING;
			if (ammo_cells < 1)
				bHaveAmmo = FALSE;
#else /* HIPNOTIC */
			m_iWeapon = IT_MJOLNIR;
#endif /* HIPNOTIC */
		}

#ifndef HIPNOTIC
	
#endif /* ! HIPNOTIC */
		if ( (m_iItems & m_iWeapon) && bHaveAmmo )
		{
			W_SetCurrentAmmo();
			return;
		}
	}

}

//================================================================================================
// WEAPON FUNCTIONS
//================================================================================================
// Returns true if the inflictor can directly damage the target.  Used for explosions and melee attacks.
float Q_CanDamage(CBaseEntity *pTarget, CBaseEntity *pInflictor) 
{
	TraceResult trace;

	// bmodels need special checking because their origin is 0,0,0
	if (pTarget->pev->movetype == MOVETYPE_PUSH)
	{
		UTIL_TraceLine( pInflictor->pev->origin, 0.5 * (pTarget->pev->absmin + pTarget->pev->absmax), ignore_monsters, NULL, &trace );
		if (trace.flFraction == 1)
			return TRUE;
		CBaseEntity *pEntity = CBaseEntity::Instance(trace.pHit);
		if (pEntity == pTarget)
			return TRUE;
		return FALSE;
	}
	
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin, ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin + Vector(15,15,0), ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin + Vector(-15,-15,0), ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin + Vector(-15,15,0), ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;
	UTIL_TraceLine( pInflictor->pev->origin, pTarget->pev->origin + Vector(15,-15,0), ignore_monsters, NULL, &trace );
	if (trace.flFraction == 1)
		return TRUE;

	return FALSE;
}

// Quake Bullet firing
void CBasePlayer::FireBullets( entvars_t *pev, int iShots, Vector vecDir, Vector vecSpread )
{
	TraceResult trace;
	UTIL_MakeVectors(pev->v_angle);

	Vector vecSrc = pev->origin + (gpGlobals->v_forward * 10);
	vecSrc.z = pev->absmin.z + (pev->size.z * 0.7);
	ClearMultiDamage();

	while ( iShots > 0 )
	{
		Vector vecPath = vecDir + ( RANDOM_FLOAT( -1, 1 ) * vecSpread.x * gpGlobals->v_right ) + ( RANDOM_FLOAT( -1, 1 ) * vecSpread.y * gpGlobals->v_up );
		Vector vecEnd = vecSrc + ( vecPath * 2048 );

		if( g_iSkillLevel == SKILL_EASY )
			gpGlobals->trace_flags = FTRACE_SIMPLEBOX; // ignore hitboxes for easy skills

		UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &trace );

		if (trace.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( trace.pHit );
			pEntity->TraceAttack( pev, 4, vecPath, &trace, DMG_BULLET );
		}

		iShots--;
	}

	ApplyMultiDamage( pev, pev );
}

// Quake Radius damage
void Q_RadiusDamage( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, CBaseEntity *pIgnore )
{
	CBaseEntity *pEnt = NULL;

	while ( (pEnt = UTIL_FindEntityInSphere( pEnt, pInflictor->pev->origin, flDamage+40 )) != NULL )
	{
		if (pEnt != pIgnore)
		{
			if (pEnt->pev->takedamage)
			{
				Vector vecOrg = pEnt->pev->origin + ((pEnt->pev->mins + pEnt->pev->maxs) * 0.5);
				float flPoints = 0.5 * (pInflictor->pev->origin - vecOrg).Length();
				if (flPoints < 0)
					flPoints = 0;
				flPoints = flDamage - flPoints;
				
				if (pEnt == pAttacker)
					flPoints = flPoints * 0.5;
				if (flPoints > 0)
				{
					if ( Q_CanDamage( pEnt, pInflictor ) )
					{
						// shambler takes half damage from all explosions
						if (FClassnameIs( pEnt->pev, "monster_shambler"))
							pEnt->TakeDamage( pInflictor->pev, pAttacker->pev, flPoints * 0.5f, DMG_GENERIC );
						else
							pEnt->TakeDamage( pInflictor->pev, pAttacker->pev, flPoints, DMG_GENERIC );
					}
				}
			}
		}
	}
}

// Lightning hit a target
void LightningHit(CBaseEntity *pTarget, CBaseEntity *pAttacker, Vector vecHitPos, float flDamage, TraceResult *ptr, Vector vecDir ) 
{
	SpawnBlood( vecHitPos, BLOOD_COLOR_RED, flDamage * 1.5 );

	if ( g_pGameRules->PlayerRelationship( pTarget, pAttacker ) != GR_TEAMMATE )
	{
		pTarget->TakeDamage( pAttacker->pev, pAttacker->pev, flDamage, DMG_GENERIC );
		pTarget->TraceBleed( flDamage, vecDir, ptr, DMG_BULLET ); // have to use DMG_BULLET or it wont spawn.
	}
}

void SpawnMeatSpray( Vector vecOrigin, Vector vecVelocity )
{
	CZombieMissile :: CreateSpray( vecOrigin, vecVelocity );
}

/*
================
SpawnBlood
================
*/
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips( vecSpot, g_vecAttackDir, bloodColor, (int)flDamage );
}

//
// EjectBrass - tosses a brass shell from passed origin at passed velocity
//
void EjectBrass ( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecOrigin );
		WRITE_BYTE( TE_MODEL);
		WRITE_COORD( vecOrigin.x);
		WRITE_COORD( vecOrigin.y);
		WRITE_COORD( vecOrigin.z);
		WRITE_COORD( vecVelocity.x);
		WRITE_COORD( vecVelocity.y);
		WRITE_COORD( vecVelocity.z);
		WRITE_ANGLE( rotation );
		WRITE_SHORT( model );
		WRITE_BYTE ( soundtype);
		WRITE_BYTE ( 25 );// 2.5 seconds
	MESSAGE_END();
}

// Lightning Damage
void CBasePlayer::LightningDamage( Vector p1, Vector p2, CBaseEntity *pAttacker, float flDamage, Vector vecDir)
{
	TraceResult trace;
	Vector vecThru = (p2 - p1).Normalize();
	vecThru.x = 0 - vecThru.y;
	vecThru.y = vecThru.x;
	vecThru.z = 0;
	vecThru = vecThru * 16;

	CBaseEntity *pEntity1 = NULL;
	CBaseEntity *pEntity2 = NULL;

	gpGlobals->trace_flags = FTRACE_SIMPLEBOX; // ignore hitboxes for now

	// Hit first target?
	UTIL_TraceLine( p1, p2, dont_ignore_monsters, ENT(pAttacker->pev), &trace );
	CBaseEntity *pEntity = CBaseEntity::Instance(trace.pHit);
#ifndef HIPNOTIC
	if (pEntity && pEntity->pev->takedamage)
#else /* HIPNOTIC */
	BOOL fCanDamage = (pEntity->IsPlayer() ? (((CBasePlayer *)pEntity)->m_flWetSuitFinished == 0.0f) : TRUE );

	if (pEntity && pEntity->pev->takedamage && fCanDamage)
#endif /* HIPNOTIC */
	{
		LightningHit(pEntity, pAttacker, trace.vecEndPos, flDamage, &trace, vecDir );
	}
	pEntity1 = pEntity;

	gpGlobals->trace_flags = FTRACE_SIMPLEBOX; // ignore hitboxes for now

	// Hit second target?
	UTIL_TraceLine( p1 + vecThru, p2 + vecThru, dont_ignore_monsters, ENT(pAttacker->pev), &trace );
	pEntity = CBaseEntity::Instance(trace.pHit);
#ifndef HIPNOTIC
	if (pEntity && pEntity != pEntity1 && pEntity->pev->takedamage)
#else /* HIPNOTIC */
	fCanDamage = (pEntity->IsPlayer() ? (((CBasePlayer *)pEntity)->m_flWetSuitFinished == 0.0f) : TRUE );

	if (pEntity && pEntity != pEntity1 && pEntity->pev->takedamage && fCanDamage)
#endif /* HIPNOTIC */
	{
		LightningHit(pEntity, pAttacker, trace.vecEndPos, flDamage, &trace, vecDir );
	}
	pEntity2 = pEntity;

	gpGlobals->trace_flags = FTRACE_SIMPLEBOX; // ignore hitboxes for now

	// Hit third target?
	UTIL_TraceLine( p1 - vecThru, p2 - vecThru, dont_ignore_monsters, ENT(pAttacker->pev), &trace );
	pEntity = CBaseEntity::Instance(trace.pHit);
#ifndef HIPNOTIC
	if (pEntity && pEntity != pEntity1 && pEntity != pEntity2 && pEntity->pev->takedamage)
#else /* HIPNOTIC */
	fCanDamage = (pEntity->IsPlayer() ? (((CBasePlayer *)pEntity)->m_flWetSuitFinished == 0.0f) : TRUE );

	if (pEntity && pEntity != pEntity1 && pEntity != pEntity2 && pEntity->pev->takedamage && fCanDamage)
#endif /* HIPNOTIC */
	{
		LightningHit(pEntity, pAttacker, trace.vecEndPos, flDamage, &trace, vecDir );
	}
}

//================================================================================================
// WEAPON FIRING
//================================================================================================
// Axe
void CBasePlayer::W_FireAxe( void )
{
	TraceResult trace;
	Vector vecSrc = pev->origin + Vector(0, 0, 16);

	// Swing forward 64 units
	UTIL_MakeVectors(pev->v_angle);
	UTIL_TraceLine( vecSrc, vecSrc + (gpGlobals->v_forward * 64), dont_ignore_monsters, ENT(pev), &trace );
	if (trace.flFraction == 1.0)
	{
		// play miss animation
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/ax1.wav", 1, ATTN_NORM);
		SendWeaponAnim( 2 );
		return;
	}	

	// we make sound, so show hostile
	m_flShowHostile = gpGlobals->time + 1.0f;

	Vector vecOrg = trace.vecEndPos - gpGlobals->v_forward * 4;

	CBaseEntity *pEntity = CBaseEntity::Instance(trace.pHit);
	if (pEntity && pEntity->pev->takedamage)
	{
		pEntity->m_bAxHitMe = TRUE;
		int iDmg = 20;
		if (gpGlobals->deathmatch > 3)
			iDmg = 75;

		pEntity->TakeDamage( pev, pev, iDmg, DMG_GENERIC );

		if ( g_pGameRules->PlayerRelationship( this, pEntity ) != GR_TEAMMATE )
			 SpawnBlood( vecOrg, BLOOD_COLOR_RED, iDmg * 4 ); // Make a lot of Blood!
	}
	else
	{
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, "player/axhit2.wav", 1, ATTN_NORM);
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_GUNSHOT );
			WRITE_COORD( vecOrg.x );
			WRITE_COORD( vecOrg.y );
			WRITE_COORD( vecOrg.z );
		MESSAGE_END();
	}
}

// Single barrel shotgun
void CBasePlayer::W_FireShotgun( void )
{
	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/guncock.wav", 1, ATTN_NORM );
	UTIL_MakeVectors( pev->v_angle );

	Vector vecDir;
	GET_AIM_VECTOR( edict(), 1000.0f, vecDir );
	FireBullets( pev, 6, vecDir, Vector(0.04, 0.04, 0) );
}

// Double barrel shotgun
void CBasePlayer::W_FireSuperShotgun( void )
{
	if (*m_pCurrentAmmo == 1)
	{
		W_FireShotgun();
		return;
	}

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 2;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/shotgn2.wav", 1, ATTN_NORM );
	UTIL_MakeVectors( pev->v_angle );

	Vector vecDir;
	GET_AIM_VECTOR( edict(), 1000.0f, vecDir );
	FireBullets( pev, 14, vecDir, Vector(0.14, 0.08, 0) );
}

// Rocket launcher
void CBasePlayer::W_FireRocket( void )
{
	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/sgun1.wav", 1, ATTN_NORM );

	// Create the rocket
	UTIL_MakeVectors( pev->v_angle );
	Vector vecOrg = pev->origin + (gpGlobals->v_forward * 8) + Vector(0,0,16);

	Vector vecDir;
	GET_AIM_VECTOR( edict(), 1000.0f, vecDir );
	CRocket *pRocket = CRocket::CreateRocket( vecOrg, vecDir, this );
}

// Grenade launcher
void CBasePlayer::W_FireGrenade( void )
{       
	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/grenade.wav", 1, ATTN_NORM );

	// Get initial velocity
	UTIL_MakeVectors( pev->v_angle );
	Vector vecVelocity;
	if ( pev->v_angle.x )
	{
		vecVelocity = gpGlobals->v_forward * 600 + gpGlobals->v_up * 200 + RANDOM_FLOAT(-1,1) * gpGlobals->v_right * 10 + RANDOM_FLOAT(-1,1) * gpGlobals->v_up * 10;
	}
	else
	{
		GET_AIM_VECTOR( edict(), 1000.0f, vecVelocity );
		vecVelocity = vecVelocity * 600;
		vecVelocity.z = 200;
	}

#ifdef HIPNOTIC
	pev->punchangle.x = -2;

#endif /* HIPNOTIC */
	// Create the grenade
	CRocket *pRocket = CRocket::CreateGrenade( pev->origin, vecVelocity, this );
}

#ifdef HIPNOTIC
// Proximity grenade launcher
void CBasePlayer::W_FireProximityGrenade( void )
{
	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "hipweap/proxbomb.wav", 1, ATTN_NORM );
	gpWorld->num_prox_grenades++;

	// Get initial velocity
	UTIL_MakeVectors( pev->v_angle );
	Vector vecVelocity;
	if ( pev->v_angle.x )
	{
		vecVelocity = gpGlobals->v_forward * 600 + gpGlobals->v_up * 200 + RANDOM_FLOAT(-1,1) * gpGlobals->v_right * 10 + RANDOM_FLOAT(-1,1) * gpGlobals->v_up * 10;
	}
	else
	{
		GET_AIM_VECTOR( edict(), 1000.0f, vecVelocity );
		vecVelocity = vecVelocity * 600;
		vecVelocity.z = 200;
	}

	pev->punchangle.x = -2;

	// Create the grenade
	CRocket *pRocket = CRocket::CreateProxGrenade( pev->origin, vecVelocity, this );
}

// Laser cannon
void CBasePlayer::W_FireLaser( void )
{
	UTIL_MakeVectors (pev->v_angle);

	Vector out = gpGlobals->v_forward;
	out.z = 0.0f;

	out = out.Normalize();
	Vector org = pev->origin + (6.0f * gpGlobals->v_up) + (12.0f * out);
	Vector dir = UTIL_GetAimVector( edict(), 1000.0f );
	float aofs = 6 * 0.707f;

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;

	org = org + (aofs * gpGlobals->v_right);
	org = org - (aofs * gpGlobals->v_up);
	CNail::CreateHipLaser( org, dir, this );
	org = org - (2 * aofs * gpGlobals->v_right);
	CNail::CreateHipLaser( org, dir, this );
	pev->punchangle.x = -1.0f;
}

void CBasePlayer::W_SpawnMjolnirBase( TraceResult *ptr )
{
	CBaseEntity *pSource = GetClassPtr( (CPointEntity *)NULL );
	pSource->pev->classname = MAKE_STRING( "info_notnull" );
	UTIL_SetOrigin( pSource->pev, ptr->vecEndPos );

	pSource->pev->owner = edict();
	pSource->m_bStruckByMjolnir = TRUE;
	pSource->SetThink( &CBaseEntity :: SUB_Remove );
	pSource->pev->nextthink = gpGlobals->time + 1.0f;

	EMIT_SOUND (ENT(pSource->pev), CHAN_AUTO, "hipweap/mjolslap.wav", 1, ATTN_NORM);
	EMIT_SOUND (ENT(pSource->pev), CHAN_WEAPON, "hipweap/mjolhit.wav", 1, ATTN_NORM);
	UTIL_MakeVectors(pev->v_angle);
	pSource->pev->movedir = gpGlobals->v_forward;

	CMjolnirLightning :: CreateLightning( pSource, pSource, pev->angles, 350.0f );
	CMjolnirLightning :: CreateLightning( pSource, pSource, pev->angles, 350.0f );
	CMjolnirLightning :: CreateLightning( pSource, pSource, pev->angles, 350.0f );
	CMjolnirLightning :: CreateLightning( pSource, pSource, pev->angles, 350.0f );
}

// Mjolnir
void CBasePlayer::W_FireHammer( BOOL hasAmmo )
{
	TraceResult tr;

	m_flNextAttack = UTIL_WeaponTimeBase() + 0.4f;
	Vector source = pev->origin + Vector( 0, 0, 16 );

	UTIL_MakeVectors (pev->v_angle);

	UTIL_TraceLine( source, source + gpGlobals->v_forward * 36.0f, dont_ignore_monsters, edict(), &tr );

	if( tr.flFraction == 1.0f && ( ammo_cells >= 15 ))
	{
		source += gpGlobals->v_forward * 36.0f;
		UTIL_TraceLine( source, source - gpGlobals->v_up * 50.0f, dont_ignore_monsters, edict(), &tr );

		if(( tr.flFraction > 0.3f && tr.flFraction < 1.0f ))
		{
			// explode if under water
			if (pev->waterlevel > 1)
			{
				if ( (gpGlobals->deathmatch > 3) && (RANDOM_FLOAT(0, 1) <= 0.5) )
				{
					TakeDamage( pev, pev, 4000, DMG_GENERIC );
				}
				else
				{
					float flCellsBurnt = *m_pCurrentAmmo;
					*m_pCurrentAmmo = 0;
					W_SetCurrentAmmo();
					g_fDischarged = TRUE;
					Q_RadiusDamage( this, this, 35 * flCellsBurnt, NULL );
					g_fDischarged = FALSE;
				}
				return;
			}

			m_flNextAttack = UTIL_WeaponTimeBase() + 1.5f;
			*m_pCurrentAmmo -= 15;
			W_SpawnMjolnirBase( &tr );
			SendWeaponAnim( 2 );
			return;
		}
	}

	Vector org = tr.vecEndPos - gpGlobals->v_forward * 4;
	CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

	if (pEntity && pEntity->pev->takedamage)
	{
		float flDamage = 50.0f;
		if (FClassnameIs( pEntity->pev, "monster_zombie"))
			flDamage = 70.0f;
		pEntity->m_bAxHitMe = TRUE;

		pEntity->TakeDamage( pev, pev, flDamage, DMG_GENERIC );

		if ( g_pGameRules->PlayerRelationship( this, pEntity ) != GR_TEAMMATE )
			 SpawnBlood( org, pEntity->BloodColor(), flDamage );
	}
	else
	{
		// hit wall
		if (tr.flFraction != 1.0f)
		{
			EMIT_SOUND (ENT(pev), CHAN_WEAPON, "hipweap/mjoltink.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
				WRITE_BYTE( TE_GUNSHOT );
				WRITE_COORD( org.x );
				WRITE_COORD( org.y );
				WRITE_COORD( org.z );
			MESSAGE_END();
		}
		else
		{
			EMIT_SOUND (ENT(pev), CHAN_WEAPON, "knight/sword1.wav", 1, ATTN_NORM);
			SendWeaponAnim( 3 );
		}
	}

	m_flNextAttack = UTIL_WeaponTimeBase() + 0.4f;
}

#endif /* HIPNOTIC */
// Lightning Gun
void CBasePlayer::W_FireLightning( void )
{
	if (*m_pCurrentAmmo < 1)
	{
		m_iWeapon = W_BestWeapon ();
		W_SetCurrentAmmo();
		return;
	}

	bool playsound = false;

	// Make lightning sound every 0.6 seconds
	if ( m_flLightningSoundTime <= gpGlobals->time )
	{
		EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/lhit.wav", 1, ATTN_NORM);
		m_flLightningSoundTime = gpGlobals->time + 0.6;
	}

	// explode if under water
	if (pev->waterlevel > 1)
	{
		if ( (gpGlobals->deathmatch > 3) && (RANDOM_FLOAT(0, 1) <= 0.5) )
		{
			TakeDamage( pev, pev, 4000, DMG_GENERIC );
		}
		else
		{
			float flCellsBurnt = *m_pCurrentAmmo;
			*m_pCurrentAmmo = 0;
			W_SetCurrentAmmo();
#ifdef HIPNOTIC
			g_fDischarged = TRUE;
#endif /* HIPNOTIC */
			Q_RadiusDamage( this, this, 35 * flCellsBurnt, NULL );
#ifdef HIPNOTIC
			g_fDischarged = FALSE;
#endif /* HIPNOTIC */
			return;
		}
	}

	if (gpGlobals->deathmatch != 4 )
	{
		// eat ammo every 0.1 secs
		if ( m_flLightningTime <= gpGlobals->time )
		{
			m_flLightningTime = gpGlobals->time + 0.1;
			*m_pCurrentAmmo -= 1;
		}
	}

	// Lightning bolt effect
	TraceResult trace;
	Vector vecOrg = pev->origin + Vector(0,0,16);
	UTIL_MakeVectors( pev->v_angle + pev->punchangle );

	// adjust lightning origin
	Vector tmpSrc = EyePosition() + gpGlobals->v_forward * 20 + gpGlobals->v_up * -8;
	UTIL_TraceLine( vecOrg, vecOrg + (gpGlobals->v_forward * 600), ignore_monsters, ENT(pev), &trace );

	if (trace.fAllSolid == FALSE)
	{
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_LIGHTNING2 );
			WRITE_ENTITY( entindex() );
			WRITE_COORD( tmpSrc.x );
			WRITE_COORD( tmpSrc.y );
			WRITE_COORD( tmpSrc.z );
			WRITE_COORD( trace.vecEndPos.x );
			WRITE_COORD( trace.vecEndPos.y );
			WRITE_COORD( trace.vecEndPos.z );
		MESSAGE_END();
	}

	Vector vecDir = gpGlobals->v_forward + ( 0.001 * gpGlobals->v_right ) + ( 0.001 * gpGlobals->v_up );
	// Do damage
	LightningDamage(pev->origin, trace.vecEndPos + (gpGlobals->v_forward * 4), this, 15, vecDir );
}

// Super Nailgun
void CBasePlayer::W_FireSuperSpikes( void )
{
	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 2;
	m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/spike2.wav", 1, ATTN_NORM );
	UTIL_MakeVectors( pev->v_angle );

	// Fire the Nail
	Vector vecDir;
	GET_AIM_VECTOR( edict(), 1000.0f, vecDir );
	CNail *pNail = CNail::CreateSuperNail( pev->origin + Vector(0,0,16)+(gpGlobals->v_forward * 10), vecDir, this );
}

// Nailgun
void CBasePlayer::W_FireSpikes( void )
{
	// If we're wielding the Super nailgun and we've got ammo for it, fire Super nails
	if (*m_pCurrentAmmo >= 2 && m_iWeapon == IT_SUPER_NAILGUN)
	{
		W_FireSuperSpikes();
		return;
	}

	// Swap to next best weapon if this one just ran out
	if (*m_pCurrentAmmo < 1)
	{
		m_iWeapon = W_BestWeapon ();
		W_SetCurrentAmmo();
		return;
	}

	// Fire left then right
	if (m_iNailOffset == 2)
		m_iNailOffset = -2;
	else
		m_iNailOffset = 2;

	if (gpGlobals->deathmatch != 4 )
		*m_pCurrentAmmo -= 1;
	m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/rocket1i.wav", 1, ATTN_NORM );

	// Fire the nail
	UTIL_MakeVectors( pev->v_angle );
	Vector vecDir;
	GET_AIM_VECTOR( edict(), 1000.0f, vecDir );
	CNail *pNail = CNail::CreateNail( pev->origin + Vector(0,0,10) + (gpGlobals->v_right * m_iNailOffset), vecDir, this );
}

//===============================================================================
// PLAYER WEAPON USE
//===============================================================================
void CBasePlayer::W_Attack( void )
{
	// Out of ammo?
	if ( !W_CheckNoAmmo() )
		return;

	// g-cont. don't update shaft animation it's looped
	if ( m_iWeapon != IT_LIGHTNING || pev->weaponanim != 1 )
		SendWeaponAnim( 1 );

	// g-cont. no react for monsters when player fire axe
	if (m_iWeapon != IT_AXE)
		m_flShowHostile = gpGlobals->time + 1.0f;

	if (m_iWeapon == IT_AXE)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

		W_FireAxe();

		// Delay attack for 0.3
		m_flAxeFire = gpGlobals->time + 0.3;
	}
	else if (m_iWeapon == IT_SHOTGUN)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

		W_FireShotgun();
	}
	else if (m_iWeapon == IT_SUPER_SHOTGUN)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.7;

		W_FireSuperShotgun();
	}
	else if (m_iWeapon == IT_NAILGUN)
	{	
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;
	
		W_FireSpikes();
	}
	else if (m_iWeapon == IT_SUPER_NAILGUN)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;
		W_FireSpikes();
	}
	else if (m_iWeapon == IT_GRENADE_LAUNCHER)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;

		W_FireGrenade();
	}
#ifdef HIPNOTIC
	else if (m_iWeapon == IT_PROXIMITY_GUN)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;

		W_FireProximityGrenade();
	}
#endif /* HIPNOTIC */
	else if (m_iWeapon == IT_ROCKET_LAUNCHER)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.8;
			
		W_FireRocket();
	}
	else if (m_iWeapon == IT_LIGHTNING)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.01;

		// Play the lightning start sound if gun just started firing
		if (m_afButtonPressed & IN_ATTACK)
			EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/lstart.wav", 1, ATTN_NORM);

		W_FireLightning();
#ifdef HIPNOTIC
	}
	else if (m_iWeapon == IT_LASER_CANNON)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;
		W_FireLaser();
	}
	else if (m_iWeapon == IT_MJOLNIR)
	{
		m_flNextAttack = UTIL_WeaponTimeBase() + 0.8;
		W_FireHammer(( ammo_cells >= 30 ) ? TRUE : FALSE );
#endif /* HIPNOTIC */
	}

	// Make player attack
	if ( pev->health >= 0 )
		SetAnimation( PLAYER_ATTACK1 );
}
