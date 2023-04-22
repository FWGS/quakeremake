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
#include  "weapons.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ENF_END_ATTACK	1
#define ENF_DROP_BACKPACK	2
#define ENF_SHOOT		3
#define ENF_IDLE_SOUND	4

class CEnforcer : public CQuakeMonster
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

	void MonsterSight( void );
	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );

	void MonsterFire( void );
	void PainSound( void );
	void IdleSound( void );
	void DeathSound( void );

	static const char *pSightSounds[];
	static const char *pPainSounds[];

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_fAttackFinished;
	int m_fInAttack;
};

LINK_ENTITY_TO_CLASS( monster_enforcer, CEnforcer );

TYPEDESCRIPTION CEnforcer :: m_SaveData[] = 
{
	DEFINE_FIELD( CEnforcer, m_fInAttack, FIELD_CHARACTER ),
	DEFINE_FIELD( CEnforcer, m_fAttackFinished, FIELD_BOOLEAN ),
}; IMPLEMENT_SAVERESTORE( CEnforcer, CQuakeMonster );

const char *CEnforcer::pPainSounds[] = 
{
	"enforcer/pain1.wav",
	"enforcer/pain2.wav",
};

const char *CEnforcer::pSightSounds[] = 
{
	"enforcer/sight1.wav",
	"enforcer/sight2.wav",
	"enforcer/sight3.wav",
	"enforcer/sight4.wav",
};

void CEnforcer :: MonsterSight( void )
{
	EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pSightSounds, ATTN_NORM );
}

void CEnforcer :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CEnforcer :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 3;
}

void CEnforcer :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 12;
}

void CEnforcer :: MonsterMissileAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_RANGE_ATTACK1 );
}

void CEnforcer :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	if( pev->pain_finished > gpGlobals->time )
		return;

	EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pPainSounds, ATTN_NORM );

	m_iAIState = STATE_PAIN;
	SetActivity( ACT_BIG_FLINCH );
	m_flMonsterSpeed = 0;

	pev->pain_finished = gpGlobals->time + (SequenceDuration() - 0.3f);
}

void CEnforcer :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	if( ShouldGibMonster( iGib ))
	{
		EMIT_SOUND( edict(), CHAN_VOICE, "player/udeath.wav", 1.0, ATTN_NORM );
		CGib::ThrowHead ("models/h_mega.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib2.mdl", pev);
		CGib::ThrowGib ("models/gib3.mdl", pev);
		UTIL_Remove( this );
		return;
	}

	// regular death
	EMIT_SOUND( edict(), CHAN_VOICE, "enforcer/death1.wav", 1.0, ATTN_NORM );
}

void CEnforcer :: MonsterAttack( void )
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

void CEnforcer :: MonsterFire( void )
{
	if( m_fInAttack > 1 ) return;

	AI_Face();

	EMIT_SOUND( edict(), CHAN_VOICE, "enforcer/enfire.wav", 1.0, ATTN_NORM );
	UTIL_MakeVectors(pev->angles);

	Vector vecOrg = pev->origin + gpGlobals->v_forward * 30.0f + gpGlobals->v_right * 8.5 + Vector( 0, 0, 16 );
	Vector vecDir = (m_hEnemy->pev->origin - pev->origin).Normalize();

	CLaser::LaunchLaser( vecOrg, vecDir, this );

	pev->effects |= EF_MUZZLEFLASH;
	m_fInAttack++;
}

//=========================================================
// Spawn
//=========================================================
void CEnforcer :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ) || !g_registered )
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/enforcer.mdl");
	UTIL_SetSize( pev, Vector( -16, -16, -24 ), Vector( 16, 16, 40 ));

	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->health	= 80;

	WalkMonsterInit ();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CEnforcer :: Precache()
{
	PRECACHE_MODEL( "models/enforcer.mdl" );
	PRECACHE_MODEL( "models/h_mega.mdl" );
	PRECACHE_MODEL( "models/laser.mdl" );

	PRECACHE_SOUND_ARRAY( pSightSounds );
	PRECACHE_SOUND_ARRAY( pPainSounds );

	PRECACHE_SOUND( "enforcer/death1.wav" );
	PRECACHE_SOUND( "enforcer/enfire.wav" );
	PRECACHE_SOUND( "enforcer/enfstop.wav" );
	PRECACHE_SOUND( "enforcer/idle1.wav" );
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CEnforcer :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case ENF_SHOOT:
		MonsterFire();
		break;
	case ENF_END_ATTACK:
		m_fInAttack = 2;
		m_fAttackFinished = TRUE;
		break;
	case ENF_DROP_BACKPACK:
		ammo_cells = 5;
		CWeaponBox::DropBackpack( this, 0 );
		break;
	case ENF_IDLE_SOUND:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.2f )
			EMIT_SOUND( edict(), CHAN_VOICE, "enforcer/idle1.wav", 1.0, ATTN_IDLE );
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}