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
#define SCOURGE_BEGIN_MOVE	1
#define SCOURGE_MOVE_TRIGGER	2
#define SCOURGE_TAIL_ATTACK	3
#define SCOURGE_SPIKE_LEFT	4
#define SCOURGE_SPIKE_RIGHT	5

class CScourgeTrigger : public CBaseEntity
{
public:
	static CScourgeTrigger *CreateTrigger( Vector vecOrigin, CBaseEntity *pOwner );
	void Spawn( void );
	void Think( void );
	void Touch( CBaseEntity *pOther );
};

LINK_ENTITY_TO_CLASS( scourgetrig, CScourgeTrigger );

class CScourge : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
	BOOL MonsterHasMeleeAttack( void ) { return TRUE; }
	void MonsterMeleeAttack( void );
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
	void MonsterStrafe( BOOL bLeft );
	void MonsterCustom( void );

	void PainSound( void );
	void IdleSound( void );
	void DeathSound( void );
	void AlternateThink( void );

	void AI_Idle( void );
	void AI_Right( float flDist );
	void AI_Left( float flDist );

	void FireSpikes( float ox );
	void TailAttack( void );

	static const char *pSightSounds[];
	static const char *pPainSounds[];

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hAITrigger;
	int m_bSpawnSilent;
	int m_bSpawnMulti;
	int m_fAttackFinished;
	int m_fInAttack;
};

LINK_ENTITY_TO_CLASS( monster_scourge, CScourge );

CScourgeTrigger *CScourgeTrigger :: CreateTrigger( Vector vecOrigin, CBaseEntity *pOwner )
{
	CScourgeTrigger *pTrigger = GetClassPtr( (CScourgeTrigger *)NULL );

	pTrigger->pev->classname = MAKE_STRING("scourgetrig");
	UTIL_SetOrigin( pTrigger->pev, vecOrigin );
	pTrigger->m_hEnemy = pOwner;
	pTrigger->Spawn();

	return pTrigger;
}

void CScourgeTrigger :: Spawn( void )
{
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize( pev, Vector( -64, -64, -24 ), Vector( 64, 64, 64 ));
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1f, 1.1f );
}

void CScourgeTrigger :: Think( void )
{
	CBaseEntity *pTarg;

	if (m_hEnemy == NULL || m_hEnemy->pev->health <= 0.0f)
	{
		UTIL_Remove (this);
		return;
	}

	pTarg = m_hEnemy;

	UTIL_MakeVectors (pTarg->pev->angles);
	UTIL_SetOrigin (pev, pTarg->pev->origin + (gpGlobals->v_forward * 300.0f));
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CScourgeTrigger :: Touch( CBaseEntity *pOther )
{
	CScourge *pScourge;

	if (pOther->pev->flags & (FL_MONSTER|FL_CLIENT))
		return;

	if (pOther->pev->movetype != MOVETYPE_FLYMISSILE)
		return;

	pScourge = (CScourge *)((CBaseEntity *)m_hEnemy);

	if (m_hEnemy == NULL || pScourge->pev->health <= 0)
	{
		UTIL_Remove (this);
		return;
	}

	Vector dir = (pScourge->pev->origin - pOther->pev->origin).Normalize();

	if (DotProduct( dir, pOther->pev->velocity.Normalize( )) < 0.8f)
		return;

	if (gpGlobals->time > pev->dmgtime)
	{
		pScourge->MonsterStrafe (RANDOM_LONG( 0, 1 ));
		pev->dmgtime = gpGlobals->time + 1.5f;
	}
}


TYPEDESCRIPTION CScourge :: m_SaveData[] = 
{
	DEFINE_FIELD( CScourge, m_hAITrigger, FIELD_EHANDLE ),
	DEFINE_FIELD( CScourge, m_bSpawnSilent, FIELD_BOOLEAN ),
	DEFINE_FIELD( CScourge, m_bSpawnMulti, FIELD_BOOLEAN ),
	DEFINE_FIELD( CScourge, m_fInAttack, FIELD_CHARACTER ),
	DEFINE_FIELD( CScourge, m_fAttackFinished, FIELD_BOOLEAN ),
}; IMPLEMENT_SAVERESTORE( CScourge, CQuakeMonster );

const char *CScourge::pPainSounds[] = 
{
	"scourge/pain.wav",
	"scourge/pain2.wav",
};

const char *CScourge::pSightSounds[] = 
{
	"scourge/sight.wav",
};

void CScourge :: MonsterSight( void )
{
	EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pSightSounds, ATTN_NORM );
}

BOOL CScourge :: MonsterCheckAttack( void )
{
	TraceResult tr;
	float chance;

	// see if any entities are in the way of the shot
	Vector spot1 = EyePosition();
	Vector spot2 = m_hEnemy->EyePosition();

	if ((spot1 - spot2).Length() <= 100.0f)
	{
		if (Q_CanDamage(m_hEnemy, this))
		{
			m_iAttackState = ATTACK_MELEE;
			return TRUE;
		}
	}

	if (gpGlobals->time < m_flAttackFinished)
		return FALSE;

	if (!m_fEnemyVisible)
		return FALSE;

	chance = spot2.z - spot1.z;

	if (chance > 64.0f)
		return FALSE;
	else if (chance < -200.0f)
		return FALSE;

	if ((spot1 - spot2).Length() > 1000.0f)
		return FALSE;

	if ((spot1 - spot2).Length() < 150.0f)
		return FALSE;

	UTIL_TraceLine( spot1, spot2, dont_ignore_monsters, ENT(pev), &tr );

	if (tr.fInOpen && tr.fInWater)
		return FALSE;	// sight line crossed contents

	if (tr.pHit != m_hEnemy->edict())
		return FALSE;	// don't have a clear shot

	// missile attack
	m_iAttackState = ATTACK_MISSILE;
	AttackFinished (RANDOM_FLOAT(2.0f, 4.0f));

	return TRUE;
}

void CScourge :: FireSpikes( float ox )
{
	Vector src, vec;

	AI_Face();
	UTIL_MakeVectors (pev->angles);

	src = pev->origin - Vector(0, 0, 19) + gpGlobals->v_right * ox + gpGlobals->v_forward * 14.0f;
	vec = (m_hEnemy->pev->origin + (200.0f * gpGlobals->v_forward)) - src;
	vec = vec.Normalize();

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/rocket1i.wav", 1, ATTN_NORM );
	CNail::CreateNail( src, vec, this );

	m_flAttackFinished = gpGlobals->time + 0.2f;
}

void CScourge :: TailAttack( void )
{
	Vector delta;
	float ldmg;

	AI_Face ();

	delta = m_hEnemy->pev->origin - pev->origin;

	if (delta.Length() > 100.0f)
		return;

	if (!Q_CanDamage (m_hEnemy, this))
		return;

	ldmg = (RANDOM_FLOAT( 0.0f, 1.0f ) + RANDOM_FLOAT( 0.0f, 1.0f ) + RANDOM_FLOAT( 0.0f, 1.0f )) * 40.0f;
	m_hEnemy->TakeDamage (pev, pev, ldmg, DMG_SLASH);
	EMIT_SOUND( edict(), CHAN_VOICE, "shambler/smack.wav", 1, ATTN_NORM);

	SpawnMeatSpray (pev->origin + gpGlobals->v_forward * 16.0f, RANDOM_FLOAT( -50.0f, 50.0f) * gpGlobals->v_right);
}

void CScourge :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CScourge :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 8;
}

void CScourge :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 14;
}

void CScourge :: MonsterStrafe( BOOL bLeft )
{
	if (bLeft) SetActivity( ACT_STRAFE_LEFT );
	else SetActivity( ACT_STRAFE_RIGHT );
	m_iAIState = STATE_CUSTOM;
	m_bSpawnSilent = 1;
	AlternateThink();
}

void CScourge :: MonsterCustom( void )
{
	if (m_Activity == ACT_STRAFE_LEFT)
		AI_Left(20);
	if (m_Activity == ACT_STRAFE_RIGHT)
		AI_Right(20);

	if (m_fSequenceFinished)
		MonsterRun();
}

void CScourge :: MonsterMissileAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_RANGE_ATTACK1 );
	m_bSpawnSilent = 0;
	AlternateThink();
	AttackFinished (RANDOM_FLOAT(2.0f, 4.0f));
}

void CScourge :: MonsterMeleeAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_MELEE_ATTACK1 );
	m_bSpawnSilent = 0;
	AlternateThink();
	AttackFinished (RANDOM_FLOAT(0.0f, 2.0f));
}

void CScourge :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	if (RANDOM_FLOAT( 0.0f, 50.0f) > flDamage)
		return; // didn't flinch

	if( pev->pain_finished > gpGlobals->time )
		return;

	EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pPainSounds, ATTN_NORM );

	m_iAIState = STATE_PAIN;
	SetActivity( ACT_BIG_FLINCH );
	m_flMonsterSpeed = 0;

	pev->pain_finished = gpGlobals->time + 2.0f;
}

void CScourge :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	if (m_hAITrigger != NULL)
		UTIL_Remove (m_hAITrigger);

	// clear sound
	EMIT_SOUND( edict(), CHAN_BODY, "misc/null.wav", 1.0, ATTN_IDLE );

	if( ShouldGibMonster( iGib ))
	{
		EMIT_SOUND( edict(), CHAN_VOICE, "player/udeath.wav", 1.0, ATTN_NORM );
		CGib::ThrowHead ("models/h_scourg.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib2.mdl", pev);
		CGib::ThrowGib ("models/gib3.mdl", pev);
		UTIL_Remove( this );
		return;
	}

	// regular death
	EMIT_SOUND( edict(), CHAN_VOICE, "scourge/pain2.wav", 1.0, ATTN_NORM );
}

void CScourge :: MonsterAttack( void )
{
	if (m_iAttackState == ATTACK_MELEE)
	{
		AI_Charge(3);
	}

	if (m_fSequenceFinished)
	{
		AttackFinished (RANDOM_FLOAT( 1.0f, 4.0f ));
		MonsterRun();
	}
}

void CScourge :: AlternateThink( void )
{
	if (m_hAITrigger == NULL)
		m_hAITrigger = CScourgeTrigger :: CreateTrigger( pev->origin, this );

	if ((m_bSpawnSilent == 0) && (m_bSpawnMulti == 1))
	{
		EMIT_SOUND( edict(), CHAN_BODY, "misc/null.wav", 1.0, ATTN_IDLE );
	}
	else if ((m_bSpawnSilent == 1) && (m_bSpawnMulti == 0))
	{
		EMIT_SOUND( edict(), CHAN_BODY, "scourge/walk.wav", 1.0, ATTN_IDLE );
	}
	m_bSpawnMulti = m_bSpawnSilent;
}

void CScourge :: AI_Right( float flDist )
{
	WalkMove ((pev->angles.y + 90.0f), flDist);
}

void CScourge :: AI_Left( float flDist )
{
	WalkMove ((pev->angles.y + 270.0f), flDist);
}

void CScourge :: AI_Idle( void )
{
	m_bSpawnSilent = FALSE;	// reset each frame
	CQuakeMonster::AI_Idle();
	AlternateThink();
}

//=========================================================
// Spawn
//=========================================================
void CScourge :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ) || !g_registered )
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/scor.mdl");
	UTIL_SetSize( pev, Vector( -32, -32, -24 ), Vector( 32, 32, 64 ));

	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->health	= 300;
	pev->yaw_speed	= 60;

	WalkMonsterInit ();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CScourge :: Precache()
{
	PRECACHE_MODEL( "models/scor.mdl" );
	PRECACHE_MODEL( "models/h_scourg.mdl" );
	PRECACHE_MODEL( "models/laser.mdl" );
	PRECACHE_MODEL( "models/spike.mdl" );

	PRECACHE_SOUND_ARRAY( pSightSounds );
	PRECACHE_SOUND_ARRAY( pPainSounds );

	PRECACHE_SOUND ("misc/null.wav");
	PRECACHE_SOUND ("scourge/idle.wav");
	PRECACHE_SOUND ("scourge/tailswng.wav");
	PRECACHE_SOUND ("scourge/walk.wav");
	PRECACHE_SOUND ("shambler/smack.wav");
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CScourge :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case SCOURGE_BEGIN_MOVE:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.1f )
			EMIT_SOUND( edict(), CHAN_VOICE, "scourge/idle.wav", 1.0, ATTN_IDLE );
		m_bSpawnSilent = 1;
		AlternateThink();
		break;
	case SCOURGE_MOVE_TRIGGER:
		AlternateThink();
		break;
	case SCOURGE_TAIL_ATTACK:
		TailAttack();
		break;
	case SCOURGE_SPIKE_LEFT:
		FireSpikes( 40 );
		break;
	case SCOURGE_SPIKE_RIGHT:
		FireSpikes( -40 );
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}