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

extern int gmsgTempEntity;

#define SF_SPAWN_CRUCIFIED	1
#ifdef HIPNOTIC
#define SF_SPAWN_LYINGDOWN	4
#endif /* HIPNOTIC */

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ZOMBIE_CURCIFIED_IDLE_SOUND	1
#define ZOMBIE_WALK_IDLE_SOUND	2
#define ZOMBIE_RUN_IDLE_SOUND		3
#define ZOMBIE_RIGHTHAND_ATTACK	4
#define ZOMBIE_LEFTHAND_ATTACK	5
#define ZOMBIE_FALL_SOUND		6
#define ZOMBIE_TEMPORARY_DEAD		7

class CZombie : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
	void MonsterMissileAttack( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void MonsterPain( CBaseEntity *pAttacker, float flDamage );
	int BloodColor( void ) { return BLOOD_COLOR_RED; }

	void MonsterSight( void );
	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );

	virtual void AI_Idle( void );
	void ThrowMeat( int iAttachment, Vector vecOffset );

	static const char *pIdleSounds[];
	static const char *pPainSounds[];
	static const char *pDeathSounds[];

	void EXPORT ZombieDefeated( void );
#ifdef HIPNOTIC
	void EXPORT ZombieLyingDown( void );
#endif /* HIPNOTIC */

	void Killed( entvars_t *pevAttacker, int iGib );
#ifdef HIPNOTIC
	void SetPainState( void );
#endif /* HIPNOTIC */
};

LINK_ENTITY_TO_CLASS( monster_zombie, CZombie );

const char *CZombie::pIdleSounds[] = 
{
	"zombie/z_idle.wav",
	"zombie/z_idle1.wav",
};

const char *CZombie::pPainSounds[] = 
{
	"zombie/z_pain.wav",
	"zombie/z_pain1.wav",
};

void CZombie :: AI_Idle( void )
{
	if( FBitSet( pev->spawnflags, SF_SPAWN_CRUCIFIED ))
		return;	// stay idle

	if (FindTarget ())
		return;
	
	if (gpGlobals->time > m_flPauseTime)
	{
		MonsterWalk();
		return;
	}

	// change angle slightly
}

void CZombie :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CZombie :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 1;
}

void CZombie :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 4;
	pev->impulse = 0;	// not in pain
	pev->frags = 0;	// not dead
}

void CZombie :: MonsterMissileAttack( void )
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_MELEE_ATTACK1 );
}

void CZombie :: MonsterSight( void )
{
	EMIT_SOUND( edict(), CHAN_VOICE, "zombie/z_idle.wav", 1.0, ATTN_NORM );
}

void CZombie :: ThrowMeat( int iAttachment, Vector vecOffset )
{
	Vector vecOrigin;
#if 0
	GetAttachment( iAttachment, vecOrigin, vecTemp );
	vecOffset = g_vecZero;
#else
	vecOrigin = pev->origin;
#endif
	EMIT_SOUND( edict(), CHAN_WEAPON, "zombie/z_shot1.wav", 1.0, ATTN_NORM );
	CZombieMissile :: CreateMissile( vecOrigin, vecOffset, pev->angles, this );

	MonsterRun ();
}

void CZombie :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	pev->health = 60;		// allways reset health

	if( flDamage < 9 )
		return;		// totally ignore

	if( m_iAIState != STATE_PAIN )
	{
		// play pain sounds if not in pain
		EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pPainSounds, ATTN_NORM ); 
	}

	m_iAIState = STATE_PAIN;

	if( pev->impulse == 2 )
		return; // down on ground, so don't reset any counters

	// go down immediately if a big enough hit
	if( flDamage >= 25 )
	{
		SetActivity( ACT_BIG_FLINCH );
		pev->impulse = 2;
		AI_Pain( 2 );
		return;
	}
	
	if( pev->impulse )
	{
		// if hit again in next gre seconds while not in pain frames, definately drop
		pev->pain_finished = gpGlobals->time + 3;
		return; // currently going through an animation, don't change
	}
	
	if( pev->pain_finished > gpGlobals->time )
	{
		// hit again, so drop down
		SetActivity( ACT_BIG_FLINCH );
		pev->impulse = 2;
		AI_Pain( 2 );
		return;
	}

	// go into one of the fast pain animations	
	pev->impulse = 1;

	SetActivity( ACT_SMALL_FLINCH );
	AI_PainForward( 3 );
}

void CZombie :: Killed( entvars_t *pevAttacker, int iGib )
{
	gpWorld->killed_monsters++;

	if( m_hEnemy == NULL )
		m_hEnemy = CBaseEntity::Instance( pevAttacker );
	MonsterDeathUse( m_hEnemy, this, USE_TOGGLE, 0.0f );

	// just an event to increase internal client counter
	MESSAGE_BEGIN( MSG_ALL, gmsgKilledMonster );
	MESSAGE_END();

	EMIT_SOUND( edict(), CHAN_VOICE, "zombie/z_gib.wav", 1.0, ATTN_NORM );
	CGib::ThrowHead ("models/h_zombie.mdl", pev);
	CGib::ThrowGib ("models/gib1.mdl", pev);
	CGib::ThrowGib ("models/gib2.mdl", pev);
	CGib::ThrowGib ("models/gib3.mdl", pev);
	UTIL_Remove( this );
}

#ifdef HIPNOTIC
void CZombie :: SetPainState( void )
{
	SetActivity( ACT_BIG_FLINCH );
	m_iAIState = STATE_PAIN;
	pev->frame = 100.0f;
	pev->frags = 1.0f;
	pev->impulse = 2;
}

#endif /* HIPNOTIC */
//=========================================================
// Spawn
//=========================================================
void CZombie :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ))
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/zombie.mdl");
	UTIL_SetSize( pev, Vector( -16, -16, -24 ), Vector( 16, 16, 40 ));

	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->health	= 60;

	if( FBitSet( pev->spawnflags, SF_SPAWN_CRUCIFIED ))
	{
		pev->movetype	= MOVETYPE_NONE;
		pev->takedamage	= DAMAGE_NO;

		// static monster as furniture
		m_iAIState = STATE_IDLE;
		SetActivity( ACT_SLEEP );

		SetThink( &CQuakeMonster::MonsterThink );
		pev->nextthink = gpGlobals->time + (RANDOM_LONG( 1, 10 ) * 0.1f);
	}
	else
	{
		WalkMonsterInit ();
#ifdef HIPNOTIC

		if( FBitSet( pev->spawnflags, SF_SPAWN_LYINGDOWN ))
		{
			pev->takedamage = DAMAGE_AIM;
			pev->ideal_yaw = pev->angles.y;

			if (!pev->yaw_speed)
				pev->yaw_speed = 20;

			SetEyePosition();
			SetUse( &CQuakeMonster::MonsterUse );
                              SetPainState();

			pev->flags |= FL_MONSTER;
			m_flPauseTime = 99999999;
			SetThink( &CZombie::ZombieLyingDown );
			pev->nextthink = gpGlobals->time + 0.1f;

			StopAnimation();
		}
#endif /* HIPNOTIC */
	}
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombie :: Precache()
{
	PRECACHE_MODEL( "models/zombie.mdl" );
	PRECACHE_MODEL( "models/h_zombie.mdl" );
	PRECACHE_MODEL( "models/zom_gib.mdl" );

	PRECACHE_SOUND_ARRAY( pIdleSounds );
	PRECACHE_SOUND_ARRAY( pPainSounds );

	PRECACHE_SOUND( "zombie/z_fall.wav" );
	PRECACHE_SOUND( "zombie/z_miss.wav" );
	PRECACHE_SOUND( "zombie/z_hit.wav" );
	PRECACHE_SOUND( "zombie/z_shot1.wav" );
	PRECACHE_SOUND( "zombie/z_gib.wav" );
	PRECACHE_SOUND( "zombie/idle_w2.wav" );	// crucified
}

void CZombie :: ZombieDefeated( void )
{
	EMIT_SOUND( edict(), CHAN_VOICE, "zombie/z_idle.wav", 1.0, ATTN_IDLE );

	pev->health = 60;
	pev->solid = SOLID_SLIDEBOX;

	if( !WALK_MOVE( ENT(pev), 0, 0, WALKMOVE_NORMAL ))
	{
		// no space to standing up (e.g. player blocked)
		pev->solid = SOLID_NOT;
		pev->nextthink = gpGlobals->time + 5.0f;
	}
	else
	{
		ResetSequenceInfo( );
		SetThink( &CQuakeMonster::MonsterThink );
		pev->nextthink = gpGlobals->time + 0.1f;
	}
#ifdef HIPNOTIC
}

void CZombie :: ZombieLyingDown( void )
{
	pev->health = 60;

	if( FindTarget ())
	{
		SetPainState(); // restore pain state
		ResetSequenceInfo( );
		SetThink( &CQuakeMonster::MonsterThink );
	}

	pev->nextthink = gpGlobals->time + 0.1f;
#endif /* HIPNOTIC */
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombie :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case ZOMBIE_CURCIFIED_IDLE_SOUND:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.05f )
			EMIT_SOUND( edict(), CHAN_VOICE, "zombie/idle_w2.wav", 1.0, ATTN_STATIC );
		pev->framerate = RANDOM_FLOAT( 0.5f, 1.1f ); // randomize animation speed
		break;
	case ZOMBIE_WALK_IDLE_SOUND:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.05f )
			EMIT_SOUND( edict(), CHAN_VOICE, "zombie/z_idle.wav", 1.0, ATTN_IDLE );
		break;
	case ZOMBIE_RUN_IDLE_SOUND:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.05f )
		{
			EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pIdleSounds, ATTN_IDLE ); 
		}
		break;
	case ZOMBIE_RIGHTHAND_ATTACK:
		ThrowMeat( 1, Vector( -10, 22, 30 ));
		break;
	case ZOMBIE_LEFTHAND_ATTACK:
		ThrowMeat( 2, Vector( -10, -24, 29 ));
		break;
	case ZOMBIE_FALL_SOUND:
		if( !pev->frags )
			EMIT_SOUND( edict(), CHAN_VOICE, "zombie/z_fall.wav", 1.0, ATTN_NORM );
		break;
	case ZOMBIE_TEMPORARY_DEAD:
		if( !pev->frags )
		{
			SetThink( &CZombie::ZombieDefeated );
			pev->nextthink = gpGlobals->time + 5.0f;
			StopAnimation(); // stop the animation!
			pev->solid = SOLID_NOT;
			pev->frags = 1.0f;
		}
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}
