/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monster.h"
#include  "animation.h"
#include	"weapons.h"
#include	"skill.h"
#include	"player.h"
#include  "gamerules.h"
#include  "decals.h"

#define TAR_TIME_TO_EXLPODE		45.0f	// makes a tarbaby like a snark :-)

class CTarBaby : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMeleeAttack( void ) { return TRUE; }
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
	BOOL MonsterHasPain( void ) { return FALSE; }
	void MonsterMeleeAttack( void );
	void MonsterMissileAttack( void );

	void MonsterAttack( void );
	void MonsterKilled( entvars_t *pevAttacker, int iGib );
	int BloodColor( void ) { return DONT_BLEED; }

	void EXPORT JumpTouch( CBaseEntity *pOther );
	void EXPORT TarExplosion( void );

	void AI_Run_Melee( void );
	void AI_Run_Missile( void );

	void MonsterSight( void );
	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );
	void MonsterBounce( void );
};

LINK_ENTITY_TO_CLASS( monster_tarbaby, CTarBaby );

void CTarBaby :: MonsterSight( void )
{
	EMIT_SOUND( edict(), CHAN_VOICE, "blob/sight1.wav", 1.0, ATTN_NORM );
}

void CTarBaby :: MonsterMissileAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_LEAP );
}

void CTarBaby :: MonsterMeleeAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_LEAP );
}

void CTarBaby :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
	pev->pain_finished = 0;
	pev->framerate = 1.0f;
}

void CTarBaby :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 2;
	pev->pain_finished = 0;
	pev->framerate = 1.0f;
}

void CTarBaby :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 2;
	pev->framerate = 1.0f;
}

void CTarBaby :: AI_Run_Melee( void )
{
	AI_Face();
}

void CTarBaby :: AI_Run_Missile( void )
{
	AI_Face();
}

void CTarBaby :: MonsterAttack( void )
{
	if (!pev->pain_finished)
		pev->pain_finished = gpGlobals->time + TAR_TIME_TO_EXLPODE;

	float speedFactor = 2.0f - ((pev->pain_finished - gpGlobals->time) / (TAR_TIME_TO_EXLPODE));

	// multiply framerate by time to explode
	pev->framerate = 1.0f * speedFactor;

	if( speedFactor >= 2.0f )
	{
		// time to self-destruction
		MonsterKilled( gpWorld->pev, GIB_ALWAYS );
		return;
	}

	if( m_Activity == ACT_LEAP )
		AI_Face(); // doesn't change when is flying

	if( !m_fSequenceFinished ) return;

	if( m_Activity == ACT_LEAP )
	{
		MonsterBounce();
		SetActivity( ACT_FLY );
	}
	else if( m_Activity == ACT_FLY )
	{
		if( ++pev->impulse == 4 )
		{
			SetActivity( ACT_LEAP );
		}
	}
}

void CTarBaby :: MonsterBounce( void )
{
	pev->movetype = MOVETYPE_BOUNCE;
	SetTouch( &CTarBaby::JumpTouch );

	UTIL_MakeVectors(pev->angles);

	pev->velocity = gpGlobals->v_forward * 600.0f + Vector( 0, 0, 200.0f );
	pev->velocity.z += RANDOM_FLOAT( 0.0f, 1.0f ) * 150.0f;
	pev->flags &= ~FL_ONGROUND;
	pev->impulse = 0;
	pev->origin.z++;
}

void CTarBaby :: JumpTouch( CBaseEntity *pOther )
{
	if (pev->health <= 0)
		return;
		
	if (pOther->pev->takedamage && !FStrEq( STRING( pev->classname ), STRING( pOther->pev->classname )))
	{
		if ( pev->velocity.Length() > 400 )
		{
			float ldmg = 10 + RANDOM_FLOAT( 0.0f, 10.0f );
			pOther->TakeDamage (pev, pev, ldmg, DMG_GENERIC);	
		}
	}
	else
	{
		EMIT_SOUND( edict(), CHAN_WEAPON, "blob/land1.wav", 1.0, ATTN_NORM );
	}

	if (!ENT_IS_ON_FLOOR( edict() ))
	{
		if (pev->flags & FL_ONGROUND)
		{
			pev->movetype = MOVETYPE_STEP;

			// jump randomly to not get hung up
			SetTouch( NULL );
			MonsterRun();
		}
		return;	// not on ground yet
	}

	SetTouch( NULL );

	if (m_hEnemy != NULL && m_hEnemy->pev->health > 0)
	{
		m_iAIState = STATE_ATTACK;
		SetActivity( ACT_LEAP );
	}
	else
	{
		pev->movetype = MOVETYPE_STEP;
		pev->pain_finished = 0;	// explode cancelling
		MonsterRun();
	}
}

void CTarBaby :: TarExplosion( void )
{
	Q_RadiusDamage( this, this, 120, gpWorld );

	EMIT_SOUND( edict(), CHAN_VOICE, "blob/death1.wav", 1.0, ATTN_NORM );

	Vector vecSrc = pev->origin - ( 8 * pev->velocity.Normalize( ));

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_TAREXPLOSION );
		WRITE_COORD( vecSrc.x );
		WRITE_COORD( vecSrc.y );
		WRITE_COORD( vecSrc.z );
	MESSAGE_END();

	UTIL_Remove( this );
}

void CTarBaby :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	SetThink( &CTarBaby::TarExplosion );
	pev->nextthink = gpGlobals->time + 0.2f;
	pev->renderfx = kRenderFxExplode;
}

//=========================================================
// Spawn
//=========================================================
void CTarBaby :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ) || !g_registered )
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/tarbaby.mdl");
	UTIL_SetSize( pev, Vector( -16, -16, -24 ), Vector( 16, 16, 40 ));

	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->health	= 80;

	WalkMonsterInit ();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CTarBaby :: Precache()
{
	PRECACHE_MODEL( "models/tarbaby.mdl" );

	PRECACHE_SOUND( "blob/death1.wav" );
	PRECACHE_SOUND( "blob/hit1.wav" );
	PRECACHE_SOUND( "blob/land1.wav" );
	PRECACHE_SOUND( "blob/sight1.wav" );
}
