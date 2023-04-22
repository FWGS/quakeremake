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

extern int gmsgTempEntity;

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define CHTHON_OUT_SOUND		1
#define CHTHON_SIGHT_SOUND		2
#define CHTHON_LAUNCH_LEFT_BALL	3
#define CHTHON_LAUNCH_RIGHT_BALL	4
#define CHTHON_DEATH_SOUND		5
#define CHTHON_DEATH_SPLASH		6

class CChthon : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void AI_Idle( void );
	void AI_Face( void );
	void AI_Walk( float flDist );
	void AI_Run_Missile( void );

	void EXPORT Awake( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void LaunchMissile( const Vector &p, int iAttachment );
};

LINK_ENTITY_TO_CLASS( monster_boss, CChthon );

//=========================================================
// Spawn
//=========================================================
void CChthon :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ))
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SetUse( &CChthon::Awake );
	SetThink( &CQuakeMonster::MonsterThink );

	// add one monster to stat
	gpWorld->total_monsters++;
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CChthon :: Precache()
{
	PRECACHE_MODEL( "models/boss.mdl" );
	PRECACHE_MODEL( "models/lavaball.mdl" );

	PRECACHE_SOUND( "weapons/rocket1i.wav" );
	PRECACHE_SOUND( "boss1/out1.wav" );
	PRECACHE_SOUND( "boss1/sight1.wav" );
	PRECACHE_SOUND( "misc/power.wav" );
	PRECACHE_SOUND( "boss1/throw.wav" );
	PRECACHE_SOUND( "boss1/pain.wav" );
	PRECACHE_SOUND( "boss1/death.wav" );
}

void CChthon :: Awake( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	pev->solid	= SOLID_BBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->takedamage	= DAMAGE_NO;

	SET_MODEL(ENT(pev), "models/boss.mdl");
	UTIL_SetSize( pev, Vector( -128, -128, -24 ), Vector( 128, 128, 256 ));

	if( g_iSkillLevel == SKILL_EASY )
		pev->health = 1;
	else
		pev->health = 3;

	m_hEnemy = pActivator;

	MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
		WRITE_BYTE( TE_LAVASPLASH );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
	MESSAGE_END();

	pev->yaw_speed = 20;

	SetUse( NULL );

	m_iAIState = STATE_IDLE;
	SetActivity( ACT_USE ); // rise Chthon
	MonsterThink();
}

void CChthon :: AI_Idle( void )
{
	if( m_fSequenceFinished )
	{
		// do a first attack!
		AI_Run_Missile ();
	}
}

void CChthon :: AI_Face( void )
{
	// go for another player if multi player
	if(( m_hEnemy != NULL && m_hEnemy->pev->health <= 0.0f ) || RANDOM_FLOAT( 0, 1.0f ) < 0.02f )
	{
		m_hEnemy = UTIL_FindEntityByClassname( m_hEnemy, "player" );
		if( m_hEnemy == NULL )
			m_hEnemy = UTIL_FindEntityByClassname( m_hEnemy, "player" );
	}

	CQuakeMonster::AI_Face();
}

void CChthon :: AI_Walk( float flDist )
{
	if (m_iAttackState == ATTACK_MISSILE)
	{
		AI_Run_Missile();
		return;
	}

	if( m_fSequenceFinished )
	{
		// just a switch between walk and attack
		if( m_hEnemy == NULL || m_hEnemy->pev->health <= 0 )
		{
			m_iAttackState = ATTACK_NONE;
			m_iAIState = STATE_WALK;
			SetActivity( ACT_WALK ); // play walk animation
		}
		else if( m_Activity == ACT_MELEE_ATTACK1 || m_Activity == ACT_SMALL_FLINCH )
		{
			m_iAIState = STATE_WALK;
			m_iAttackState = ATTACK_MISSILE;
		}
		else if( m_Activity == ACT_BIG_FLINCH )
		{
			m_iAIState = STATE_WALK;
			m_iAttackState = ATTACK_NONE;
			SetActivity( ACT_DIEVIOLENT );
		}
		else if( m_Activity == ACT_DIEVIOLENT )
		{
			m_iAIState = STATE_DEAD;
			gpWorld->killed_monsters++;

			// just an event to increase internal client counter
			MESSAGE_BEGIN( MSG_ALL, gmsgKilledMonster );
			MESSAGE_END();

			if( m_hEnemy == NULL ) m_hEnemy = gpWorld;
			SUB_UseTargets( m_hEnemy, USE_TOGGLE, 0 );
			UTIL_Remove( this );
			return;
		}
          }

	AI_Face();
}

void CChthon :: AI_Run_Missile( void )
{
	m_iAIState = STATE_WALK;
	m_iAttackState = ATTACK_NONE;	// wait for sequence ends
	SetActivity( ACT_MELEE_ATTACK1 );
}

int CChthon :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	EMIT_SOUND( edict(), CHAN_VOICE, "boss1/pain.wav", 1, ATTN_NORM );

	pev->health--;

	if( pev->health > 0 )
	{
		SetActivity( ACT_SMALL_FLINCH ); 
		m_iAIState = STATE_WALK;
		m_iAttackState = ATTACK_NONE;
	}
	else
	{
		SetActivity( ACT_BIG_FLINCH ); 
		m_iAIState = STATE_WALK;
		m_iAttackState = ATTACK_NONE;
	}

	return 1;
}

void CChthon :: LaunchMissile( const Vector &p, int iAttachment )
{
	if( m_hEnemy == NULL ) return;

	Vector vecAngles = UTIL_VecToAngles( m_hEnemy->pev->origin - pev->origin );
	UTIL_MakeVectors( vecAngles );

	Vector vecSrc = pev->origin + p.x * gpGlobals->v_forward + p.y * gpGlobals->v_right + p.z * Vector( 0, 0, 1 );
	Vector vecEnd, vecDir;

// classic quake method is more better
//	GetAttachment( iAttachment, vecSrc );

	// lead the player on hard mode
	if( g_iSkillLevel > SKILL_MEDIUM )
	{
		float t = ( m_hEnemy->pev->origin - vecSrc ).Length() / 300.0f;
		Vector vec = m_hEnemy->pev->velocity;
		vec.z = 0;
		vecEnd = m_hEnemy->pev->origin + t * vec;
	}
	else
	{
		vecEnd = m_hEnemy->pev->origin;
	}

	vecDir = (vecEnd - vecSrc).Normalize();

	CNail *pFireBall = CNail::CreateNail( vecSrc, vecDir, this );

	if( pFireBall )
	{
		EMIT_SOUND( edict(), CHAN_WEAPON, "boss1/throw.wav", 1.0, ATTN_NORM );
		SET_MODEL( ENT(pFireBall->pev), "models/lavaball.mdl" );
		pFireBall->SetTouch( &CNail::ExplodeTouch ); // rocket explosion
		pFireBall->pev->avelocity = Vector( 200, 100, 300 );
		pFireBall->pev->velocity = vecDir * 300;
	}
}

void CChthon :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case CHTHON_OUT_SOUND:
		EMIT_SOUND( edict(), CHAN_WEAPON, "boss1/out1.wav", 1.0, ATTN_NORM );
		break;
	case CHTHON_SIGHT_SOUND:
		EMIT_SOUND( edict(), CHAN_VOICE, "boss1/sight1.wav", 1.0, ATTN_NORM );
		break;
	case CHTHON_LAUNCH_RIGHT_BALL:
		LaunchMissile( Vector( 100, -100, 200 ), 1 );
		break;
	case CHTHON_LAUNCH_LEFT_BALL:
		LaunchMissile( Vector( 100, 100, 200 ), 2 );
		break;
	case CHTHON_DEATH_SOUND:
		EMIT_SOUND( edict(), CHAN_WEAPON, "boss1/death.wav", 1.0, ATTN_NORM );
		break;
	case CHTHON_DEATH_SPLASH:
		EMIT_SOUND( edict(), CHAN_WEAPON, "boss1/out1.wav", 1.0, ATTN_NORM );

		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_LAVASPLASH );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
		MESSAGE_END();
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}
