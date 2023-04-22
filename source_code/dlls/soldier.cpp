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
#include	"items.h"
#include	"skill.h"
#include	"player.h"
#include  "gamerules.h"
#include  "decals.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ARMY_END_ATTACK	1
#define ARMY_DROP_BACKPACK	2
#define ARMY_SHOOT		3
#define ARMY_IDLE_SOUND	4

class CSoldier : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
	void MonsterMissileAttack( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void MonsterAttack( void );
	void MonsterKilled( entvars_t *pevAttacker, int iGib );
	void MonsterPain( CBaseEntity *pAttacker, float flDamage );
	int BloodColor( void ) { return BLOOD_COLOR_RED; }
	BOOL MonsterCheckAttack( void );

	void MonsterSight( void );
	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );

	void MonsterFire( void );

	static const char *pPainSounds[];

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_fAttackFinished;
	int m_fInAttack;
};

LINK_ENTITY_TO_CLASS( monster_army, CSoldier );

TYPEDESCRIPTION CSoldier :: m_SaveData[] = 
{
	DEFINE_FIELD( CSoldier, m_fInAttack, FIELD_BOOLEAN ),
	DEFINE_FIELD( CSoldier, m_fAttackFinished, FIELD_BOOLEAN ),
}; IMPLEMENT_SAVERESTORE( CSoldier, CQuakeMonster );

const char *CSoldier::pPainSounds[] = 
{
	"soldier/pain1.wav",
	"soldier/pain2.wav",
};

void CSoldier :: MonsterSight( void )
{
	EMIT_SOUND( edict(), CHAN_VOICE, "soldier/sight1.wav", 1.0, ATTN_NORM );
}

void CSoldier :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CSoldier :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 2.5;
}

void CSoldier :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 8;
}

void CSoldier :: MonsterMissileAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_RANGE_ATTACK1 );
}

void CSoldier :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	if( pev->pain_finished > gpGlobals->time )
		return;

	EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pPainSounds, ATTN_NORM );

	m_iAIState = STATE_PAIN;
	SetActivity( ACT_BIG_FLINCH );
	m_flMonsterSpeed = 0;

	pev->pain_finished = gpGlobals->time + (SequenceDuration() - 0.3f);
}

void CSoldier :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	if( ShouldGibMonster( iGib ))
	{
		EMIT_SOUND( edict(), CHAN_VOICE, "player/udeath.wav", 1.0, ATTN_NORM );
		CGib::ThrowHead ("models/h_guard.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib2.mdl", pev);
		CGib::ThrowGib ("models/gib3.mdl", pev);
		UTIL_Remove( this );
		return;
	}

	// regular death
	EMIT_SOUND( edict(), CHAN_VOICE, "soldier/death1.wav", 1.0, ATTN_NORM );
}

BOOL CSoldier::MonsterCheckAttack( void )
{
	Vector spot1, spot2;
	CBaseEntity *pTarg;
	float chance;

	pTarg = m_hEnemy;
	
	// see if any entities are in the way of the shot
	spot1 = EyePosition();
	spot2 = pTarg->EyePosition();

	TraceResult tr;
	UTIL_TraceLine( spot1, spot2, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr );

	if (tr.fInOpen && tr.fInWater)
		return FALSE;	// sight line crossed contents

	if (tr.pHit != pTarg->edict())
		return FALSE;	// don't have a clear shot
			
	
	// missile attack
	if (gpGlobals->time < m_flAttackFinished)
		return FALSE;
		
	if (m_iEnemyRange == RANGE_FAR)
		return FALSE;
		
	if (m_iEnemyRange == RANGE_MELEE)
		chance = 0.9f;
	else if (m_iEnemyRange == RANGE_NEAR)
		chance = 0.4f;
	else if (m_iEnemyRange == RANGE_MID)
		chance = 0.05f;
	else
		chance = 0;

	if (RANDOM_FLOAT(0, 1) < chance)
	{
		MonsterMissileAttack();
		AttackFinished (1 + RANDOM_FLOAT(0, 1));
		if (RANDOM_FLOAT(0, 1) < 0.3f)
			m_fLeftY = !m_fLeftY;

		return TRUE;
	}

	return FALSE;
}

void CSoldier :: MonsterAttack( void )
{
	if( m_fAttackFinished )
	{
		m_fAttackFinished = FALSE;
		m_fInAttack = FALSE;

		if( CheckRefire())
		{
			MonsterMissileAttack ();
		}
		else
		{
			MonsterRun();
		}
	}

	AI_Face();
}

void CSoldier :: MonsterFire( void )
{
	if( m_fInAttack ) return;

	AI_Face();

	EMIT_SOUND( edict(), CHAN_VOICE, "soldier/sattck1.wav", 1.0, ATTN_NORM );

	// fire somewhat behind the player, so a dodging player is harder to hit
	Vector vecDir = ((m_hEnemy->pev->origin - m_hEnemy->pev->velocity * 0.2f) - pev->origin).Normalize();

	CBasePlayer::FireBullets( pev, 4, vecDir, Vector(0.1, 0.1, 0) );

	pev->effects |= EF_MUZZLEFLASH;
	m_fInAttack = TRUE;
}

//=========================================================
// Spawn
//=========================================================
void CSoldier :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ))
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/soldier.mdl");
	UTIL_SetSize( pev, Vector( -16, -16, -24 ), Vector( 16, 16, 40 ));

	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->health	= 30;

	WalkMonsterInit ();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CSoldier :: Precache()
{
	PRECACHE_MODEL( "models/soldier.mdl" );
	PRECACHE_MODEL( "models/h_guard.mdl" );

	PRECACHE_SOUND_ARRAY( pPainSounds );

	PRECACHE_SOUND( "soldier/death1.wav" );
	PRECACHE_SOUND( "soldier/idle.wav" );
	PRECACHE_SOUND( "soldier/sattck1.wav" );
	PRECACHE_SOUND( "soldier/sight1.wav" );
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CSoldier :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case ARMY_SHOOT:
		MonsterFire();
		break;
	case ARMY_END_ATTACK:
		m_fInAttack = TRUE;
		m_fAttackFinished = TRUE;
		break;
	case ARMY_DROP_BACKPACK:
		ammo_shells = 5;
		CWeaponBox::DropBackpack( this, 0 );
		break;
	case ARMY_IDLE_SOUND:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.2f )
			EMIT_SOUND( edict(), CHAN_VOICE, "soldier/idle.wav", 1.0, ATTN_IDLE );
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}