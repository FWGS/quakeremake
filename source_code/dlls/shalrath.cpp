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
#include  "weapons.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define SHALRATH_IDLE_SOUND		1
#define SHALRATH_ATTACK		2
#define SHALRATH_ATTACK_SOUND		3

class CShalrath : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int BloodColor( void ) { return BLOOD_COLOR_RED; }
	void MonsterMissileAttack( void );

	void MonsterPain( CBaseEntity *pAttacker, float flDamage );
	void MonsterKilled( entvars_t *pevAttacker, int iGib );
	void MonsterSight( void );
	void CreateMissile( void );

	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );
	void MonsterAttack( void );

	static const char *pAttackSounds[];
};

LINK_ENTITY_TO_CLASS( monster_shalrath, CShalrath );

const char *CShalrath::pAttackSounds[] = 
{
	"shalrath/attack.wav",
	"shalrath/attack2.wav",
};

void CShalrath :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CShalrath :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 4;
}

void CShalrath :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 4;
}

void CShalrath :: MonsterMissileAttack( void )
{
	// don't launch more than 5 missiles at one time
	if( pev->impulse > 5 ) return;

	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_MELEE_ATTACK1 );
}

void CShalrath :: MonsterAttack( void )
{
	if( m_iAIState == STATE_ATTACK && m_fSequenceFinished )
		MonsterRun();
}

void CShalrath :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	if( ShouldGibMonster( iGib ))
	{
		CGib::ThrowHead ("models/h_shal.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib2.mdl", pev);
		CGib::ThrowGib ("models/gib3.mdl", pev);
		UTIL_Remove( this );
		return;
	}

	// regular death
	EMIT_SOUND( edict(), CHAN_VOICE, "shalrath/death.wav", 1.0, ATTN_NORM );
}

void CShalrath :: CreateMissile( void )
{
	Vector vecDir = ((m_hEnemy->pev->origin + Vector( 0, 0, 10 )) - pev->origin).Normalize();
	Vector vecSrc = pev->origin + Vector( 0, 0, 10 );

	float flDist = (m_hEnemy->pev->origin - pev->origin).Length();

	float flytime = flDist * 0.002;
	if( flytime < 0.1f ) flytime = 0.1f;

	pev->effects |= EF_MUZZLEFLASH;
	EMIT_SOUND( edict(), CHAN_WEAPON, "shalrath/attack2.wav", 1.0, ATTN_IDLE );

	CShalMissile *pMiss = CShalMissile::CreateMissile( vecSrc, vecDir * 400.0f );

	if( pMiss )
	{
		pMiss->pev->owner = edict();
		pMiss->pev->enemy = m_hEnemy->edict();
		pMiss->pev->nextthink = gpGlobals->time + flytime;
		pev->impulse++;
	}
}

void CShalrath :: MonsterSight( void )
{
	EMIT_SOUND( edict(), CHAN_VOICE, "shalrath/sight.wav", 1.0, ATTN_NORM );
}

void CShalrath :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	if( pev->pain_finished > gpGlobals->time )
		return;

	EMIT_SOUND( edict(), CHAN_VOICE, "shalrath/pain.wav", 1.0, ATTN_NORM );

	m_iAIState = STATE_PAIN;
	SetActivity( ACT_BIG_FLINCH );
	pev->pain_finished = gpGlobals->time + 3;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CShalrath :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case SHALRATH_IDLE_SOUND:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.05f )
			EMIT_SOUND( edict(), CHAN_VOICE, "shalrath/idle.wav", 1.0, ATTN_IDLE );
		break;
	case SHALRATH_ATTACK:
		CreateMissile();
		break;
	case SHALRATH_ATTACK_SOUND:
		EMIT_SOUND( edict(), CHAN_VOICE, "shalrath/attack.wav", 1.0, ATTN_IDLE );
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CShalrath :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ) || !g_registered )
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/shalrath.mdl");
	UTIL_SetSize( pev, Vector( -32, -32, -24 ), Vector( 32, 32, 64 ));

	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->health	= 400;

	WalkMonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CShalrath :: Precache()
{
	PRECACHE_MODEL( "models/shalrath.mdl" );
	PRECACHE_MODEL( "models/h_shal.mdl" );
	PRECACHE_MODEL( "models/v_spike.mdl" );

	PRECACHE_SOUND_ARRAY( pAttackSounds );
	PRECACHE_SOUND( "shalrath/death.wav" );
	PRECACHE_SOUND( "shalrath/idle.wav" );
	PRECACHE_SOUND( "shalrath/pain.wav" );
	PRECACHE_SOUND( "shalrath/sight.wav" );
}