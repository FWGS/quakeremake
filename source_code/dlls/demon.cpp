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

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define DEMON_IDLE_SOUND	1
#define DEMON_LEAP		2
#define DEMON_ATTACK_RIGHT	3
#define DEMON_ATTACK_LEFT	4

class CDemon : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMeleeAttack( void ) { return TRUE; }
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
	void MonsterMeleeAttack( void );
	void MonsterMissileAttack( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void MonsterAttack( void );
	BOOL MonsterCheckAttack( void );
	void MonsterKilled( entvars_t *pevAttacker, int iGib );
	void MonsterPain( CBaseEntity *pAttacker, float flDamage );
	int BloodColor( void ) { return BLOOD_COLOR_RED; }
	BOOL CheckMelee( void );
	BOOL CheckJump( void );

	void EXPORT JumpTouch( CBaseEntity *pOther );
	void MonsterLeap( void );
	void MonsterMelee( float flDamage );

	void MonsterSight( void );
	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );
};

LINK_ENTITY_TO_CLASS( monster_demon1, CDemon );

void CDemon :: MonsterSight( void )
{
	EMIT_SOUND( edict(), CHAN_VOICE, "demon/sight2.wav", 1.0, ATTN_NORM );
}

void CDemon :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	if( m_pfnTouch == JumpTouch )
		return;

	if( pev->pain_finished > gpGlobals->time )
		return;

	EMIT_SOUND( edict(), CHAN_VOICE, "demon/dpain1.wav", 1.0, ATTN_NORM );
	pev->pain_finished = gpGlobals->time + 1;

	if (RANDOM_FLOAT( 0.0f, 1.0f ) * 200 > flDamage )
		return; // didn't flinch

	m_iAIState = STATE_PAIN;
	SetActivity( ACT_BIG_FLINCH );
}

/*
==============
CheckDemonMelee

Returns TRUE if a melee attack would hit right now
==============
*/
BOOL CDemon :: CheckMelee( void )
{
	if( m_iEnemyRange == RANGE_MELEE )
	{	
		// FIXME: check canreach
		m_iAttackState = ATTACK_MELEE;
		return TRUE;
	}
	return FALSE;
}

/*
==============
CheckDemonJump

==============
*/
BOOL CDemon :: CheckJump( void )
{
	if (pev->origin.z + pev->mins.z > m_hEnemy->pev->origin.z + m_hEnemy->pev->mins.z + 0.75f * m_hEnemy->pev->size.z)
		return FALSE;
		
	if (pev->origin.z + pev->maxs.z < m_hEnemy->pev->origin.z + m_hEnemy->pev->mins.z + 0.25f * m_hEnemy->pev->size.z)
		return FALSE;
		
	Vector dist = m_hEnemy->pev->origin - pev->origin;
	dist.z = 0;
	
	float d = dist.Length();
	
	if (d < 100)
		return FALSE;
		
	if (d > 200)
	{
		if (RANDOM_FLOAT(0.0f, 1.0f) < 0.9f)
			return FALSE;
	}

	return TRUE;
}

BOOL CDemon :: MonsterCheckAttack( void )
{
	// if close enough for slashing, go for it
	if (CheckMelee ())
	{
		m_iAttackState = ATTACK_MELEE;
		pev->impulse = ATTACK_MELEE;
		return TRUE;
	}
	
	if (CheckJump ())
	{
		EMIT_SOUND( edict(), CHAN_VOICE, "demon/djump.wav", 1.0, ATTN_NORM );
		m_iAttackState = ATTACK_MISSILE;
		pev->impulse = ATTACK_MISSILE;
		return TRUE;
	}
	
	return FALSE;
}

void CDemon :: MonsterLeap( void )
{
	AI_Face();
	
	SetTouch( JumpTouch );
	UTIL_MakeVectors( pev->angles );

	pev->origin.z++;
	pev->velocity = gpGlobals->v_forward * 600 + Vector( 0, 0, 250 );
	pev->flags &= ~FL_ONGROUND;
}

void CDemon :: MonsterMelee( float side )
{
	if (m_hEnemy == NULL)
		return;

	AI_Face();
	WalkMove( pev->ideal_yaw, 12 );	// allow a little closing

	Vector delta = m_hEnemy->pev->origin - pev->origin;

	if (delta.Length() > 100)
		return;

	if (!Q_CanDamage( m_hEnemy, this ))
		return;

	EMIT_SOUND( edict(), CHAN_VOICE, "demon/dhit2.wav", 1.0, ATTN_NORM );		
	float ldmg = 10.0f + 5.0f * RANDOM_FLOAT( 0.0f, 1.0f );
	m_hEnemy->TakeDamage (pev, pev, ldmg, DMG_GENERIC);

	UTIL_MakeVectors( pev->angles );
	SpawnMeatSpray (pev->origin + gpGlobals->v_forward * 16, side * gpGlobals->v_right);
}

void CDemon :: MonsterMissileAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_LEAP );
}

void CDemon :: MonsterMeleeAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_MELEE_ATTACK1 );
}

void CDemon :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CDemon :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 6;
}

void CDemon :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 18;
}

void CDemon :: MonsterAttack( void )
{
	if( pev->impulse == ATTACK_MELEE )
	{
		if( !m_pfnTouch )
			AI_Face();
	}
	else if( pev->impulse == ATTACK_MISSILE )
	{
		AI_Charge( 18 );
	}

	if( m_iAIState == STATE_ATTACK && m_fSequenceFinished )
	{
		pev->impulse = ATTACK_NONE;	// reset shadow of attack state
		MonsterRun();
	}
}

void CDemon :: JumpTouch( CBaseEntity *pOther )
{
	if (pev->health <= 0)
		return;
		
	if (pOther->pev->takedamage)
	{
		if ( pev->velocity.Length() > 400 )
		{
			float ldmg = 40 + RANDOM_FLOAT( 0.0f, 10.0f );
			pOther->TakeDamage (pev, pev, ldmg, DMG_GENERIC);	
		}
	}

	if (!ENT_IS_ON_FLOOR( edict() ))
	{
		if (pev->flags & FL_ONGROUND)
		{
			// jump randomly to not get hung up
			SetTouch( NULL );
			m_iAIState = STATE_ATTACK;
			SetActivity( ACT_LEAP );
		}
		return;	// not on ground yet
	}

	SetTouch( NULL );
	MonsterRun();
}

void CDemon :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	if( ShouldGibMonster( iGib ))
	{
		EMIT_SOUND( edict(), CHAN_VOICE, "player/udeath.wav", 1.0, ATTN_NORM );
		CGib::ThrowHead ("models/h_demon.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		UTIL_Remove( this );
		return;
	}

	// regular death
	EMIT_SOUND( edict(), CHAN_VOICE, "demon/ddeath.wav", 1.0, ATTN_NORM );
}

//=========================================================
// Spawn
//=========================================================
void CDemon :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ))
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/demon.mdl");
	UTIL_SetSize( pev, Vector( -32, -32, -24 ), Vector( 32, 32, 64 ));

	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->health	= 300;

	WalkMonsterInit ();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CDemon :: Precache()
{
	PRECACHE_MODEL( "models/demon.mdl" );
	PRECACHE_MODEL( "models/h_demon.mdl" );

	PRECACHE_SOUND( "demon/ddeath.wav" );
	PRECACHE_SOUND( "demon/dhit2.wav" );
	PRECACHE_SOUND( "demon/djump.wav" );
	PRECACHE_SOUND( "demon/dpain1.wav" );
	PRECACHE_SOUND( "demon/idle1.wav" );
	PRECACHE_SOUND( "demon/sight2.wav" );
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CDemon :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case DEMON_IDLE_SOUND:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.05f )
			EMIT_SOUND( edict(), CHAN_VOICE, "demon/idle1.wav", 1.0, ATTN_IDLE );
		break;
	case DEMON_LEAP:
		MonsterLeap();
		break;
	case DEMON_ATTACK_RIGHT:
		MonsterMelee( 200.0f );
		break;
	case DEMON_ATTACK_LEFT:
		MonsterMelee( -200.0f );
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}