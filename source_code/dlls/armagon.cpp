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
#include  "items.h"

#define SERVO_VOLUME	0.5f

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ARMAGON_STAND_ATTACK	1
#define ARMAGON_STEP_FALL	2
#define ARMAGON_STEP_SERVO	3
#define ARMAGON_RUN_REGROUP	4
#define ARMAGON_FIRE_LASER	5
#define ARMAGON_FIRE_ROCKET	6
#define ARMAGON_FINISH_ATTACK	7
#define ARMAGON_CHARGE_ONE	8
#define ARMAGON_CHARGE_TWO	9
#define ARMAGON_END_REGROUP	10
#define ARMAGON_BODY_EXPLODE	11
#define ARMAGON_BODY_HIDE	12

class CArmagon : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMeleeAttack( void ) { return TRUE; }
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
//	void MonsterMissileAttack( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

//	void MonsterAttack( void );
	void Killed( entvars_t *pevAttacker, int iGib );
	void MonsterPain( CBaseEntity *pAttacker, float flDamage );
	int BloodColor( void ) { return BLOOD_COLOR_RED; }
	BOOL MonsterCheckAttack( void );
	void EXPORT BodyExplode1( void );
	void EXPORT BodyExplode2( void );

	void MonsterSight( void );
	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );
	void AI_Walk( float flDist );
	void AI_Run( float flDist );

	void MonsterFire( void );
	void BaseThink( void );
	void WalkThink( void );
	void MonsterTurn( float flAngleDelta, float flDelta );
	void MonsterMeleeAttack( void );
	void StandAttack( void );
	void RepulseAttack( void );
	void LaunchLaser( float offset );
	void LaunchRocket( float offset, int iTurn = 0 );
	void MonsterRegroup( void );
	void MonsterCustom( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	static const char *pIdleSounds[];

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLaunchFactor;
	float m_flBodyYaw;
	float m_flIdleTime;
	BOOL m_bRunAttack;
};

LINK_ENTITY_TO_CLASS( monster_armagon, CArmagon );

TYPEDESCRIPTION CArmagon :: m_SaveData[] = 
{
	DEFINE_FIELD( CArmagon, m_flLaunchFactor, FIELD_FLOAT ),
	DEFINE_FIELD( CArmagon, m_flBodyYaw, FIELD_FLOAT ),
	DEFINE_FIELD( CArmagon, m_flIdleTime, FIELD_TIME ),
	DEFINE_FIELD( CArmagon, m_bRunAttack, FIELD_BOOLEAN ),
}; IMPLEMENT_SAVERESTORE( CArmagon, CQuakeMonster );

const char *CArmagon::pIdleSounds[] = 
{
	"armagon/idle1.wav",
	"armagon/idle2.wav",
	"armagon/idle3.wav",
	"armagon/idle4.wav",
};

void CArmagon :: MonsterSight( void )
{
	EMIT_SOUND( edict(), CHAN_VOICE, "armagon/sight.wav", 1.0, 0.1 );
}

void CArmagon :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CArmagon :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 14;
}

void CArmagon :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 14;
}

void CArmagon :: AI_Walk( float flDist )
{
	m_flMoveDistance = flDist;

	MoveToGoal( flDist );
	WalkThink();
}

void CArmagon :: AI_Run( float flDist )
{
	CHANGE_YAW( ENT(pev) );
	m_bRunStraight = TRUE;

	CQuakeMonster :: AI_Run( flDist );
	BaseThink();
}

void CArmagon :: MonsterMeleeAttack( void ) 
{
	m_iAIState = STATE_ATTACK;
	SetActivity( ACT_ARM );

	EMIT_SOUND( edict(), CHAN_BODY, "armagon/footfall.wav", 1.0, ATTN_ARMAGON );
}

void CArmagon :: MonsterCustom( void )
{
	// overleft and overright doing here
	bool bOverleft = (m_Activity == ACT_TURN_LEFT) ? true : false;
	float delta, angdelta;

	CHANGE_YAW( ENT(pev) );
	WalkMove( pev->angles.y, 14.0f );

	if (m_bRunAttack)
	{
		BaseThink();
		return;
	}

	SetBoneController( 0, m_flBodyYaw );

	if (m_flCount == 0.0f)
	{
		if( m_hEnemy != NULL )
		{
			m_flEnemyYaw = UTIL_VecToYaw( m_hEnemy->pev->origin - pev->origin );
			pev->ideal_yaw = m_flEnemyYaw;
		}

		delta = pev->ideal_yaw - pev->angles.y;
		if (bOverleft) delta -= 165.0f;
		else delta += 165.0f;

		if( delta > 180.0f ) delta -= 360.0f;
		if( delta < -180.0f ) delta += 360.0f;
		angdelta = delta - m_flBodyYaw;
	}
	else if (m_flCount == 1.0f)
	{
		if (bOverleft)
			LaunchRocket( 40.0f, 1 );
		else LaunchRocket( -40.0f, 2 );
		return;
	}
	else
	{
		delta = 0.0f;
		angdelta = delta - m_flBodyYaw;
	}

	MonsterTurn( angdelta, delta );
}

void CArmagon :: LaunchLaser( float ofs )
{
	CBaseEntity *pTarg;
	Vector ang, vec;
	float flDist;

	if( m_hEnemy == NULL )
		return;

	pTarg = m_hEnemy;

	ang = pev->angles;
	ang.y += m_flBodyYaw;
	UTIL_MakeVectors( ang );

	Vector src = pev->origin + Vector( 0, 0, 66.0f ) + gpGlobals->v_right * ofs + gpGlobals->v_forward * 84.0f;
	Vector dst = pTarg->EyePosition();

	if (g_iSkillLevel != SKILL_EASY)
	{
		flDist = (dst - src).Length();
		dst += (pTarg->pev->velocity * (flDist / 1000.0f));
	}

	vec = (dst - src).Normalize();
	flDist = DotProduct( vec, gpGlobals->v_forward );

	if (flDist < m_flLaunchFactor)
		vec = gpGlobals->v_forward;

	SetBits( pev->effects, EF_MUZZLEFLASH );

	CNail *pNail = CNail::CreateHipLaser( src, vec, this );
}

void CArmagon :: LaunchRocket( float ofs, int iTurn )
{
	CBaseEntity *pTarg;
	Vector ang, vec;
	float flDist;

	if( m_hEnemy == NULL )
		return;

	pTarg = m_hEnemy;

	ang = pev->angles;
	ang.y += m_flBodyYaw;

	if( iTurn == 1 )
		ang.y += 165.0f;
	else if( iTurn == 2 )
		ang.y -= 165.0f;

	UTIL_MakeVectors( ang );

	Vector src = pev->origin + Vector( 0, 0, 66.0f ) + gpGlobals->v_right * ofs + gpGlobals->v_forward * 84.0f;
	Vector dst = pTarg->EyePosition();

	if (g_iSkillLevel != SKILL_EASY)
	{
		flDist = (dst - src).Length();
		dst += (pTarg->pev->velocity * (flDist / 1000.0f));
	}

	vec = (dst - src).Normalize();
	flDist = DotProduct( vec, gpGlobals->v_forward );

	if (flDist < m_flLaunchFactor)
		vec = gpGlobals->v_forward;
	SetBits( pev->effects, EF_MUZZLEFLASH );

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/sgun1.wav", 1, ATTN_NORM );

	CRocket *pRocket = CRocket::CreateRocket( src, vec, this );
}

void CArmagon :: StandAttack( void )
{
	Vector spot1, spot2;
	CBaseEntity *pTarg;

	if( m_hEnemy == NULL )
	{
		MonsterIdle();
		return;
          }

	pTarg = m_hEnemy;

	// see if any entities are in the way of the shot
	spot1 = EyePosition();
	spot2 = pTarg->EyePosition();

	TraceResult tr;
	UTIL_TraceLine( spot1, spot2, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr );

	if (tr.pHit != pTarg->edict())
	{
		MonsterRun();
		return;
	}

	if (tr.fInOpen && tr.fInWater)
	{
		MonsterRun();
		return;
	}

	if (gpGlobals->time < m_flAttackFinished)
		return;

	float flDist = (spot2 - spot1).Length();

	if ((flDist < 200) && FBitSet( pTarg->pev->flags, FL_CLIENT|FL_MONSTER ))
	{
		RepulseAttack();
		return;
	}

	pev->renderfx = kRenderFxNone;
	pev->button = 0;

	if (flDist > 450)
	{
		MonsterRun();
		return;
	}

	if (RANDOM_FLOAT( 0.0f, 1.0f) < 0.5f)
	{
		m_iAIState = STATE_ATTACK;
		SetActivity( ACT_RANGE_ATTACK2 );
	}
	else
	{
		m_iAIState = STATE_ATTACK;
		SetActivity( ACT_RANGE_ATTACK1 );
	}

	if (m_flCnt == 1.0f)
	{
		MonsterRun();
		return;
	}
}

void CArmagon :: RepulseAttack( void )
{
	BaseThink();

	if (pev->button == 0)
	{
		AttackFinished (0.5f);
		EMIT_SOUND( edict(), CHAN_BODY, "armagon/repel.wav", 1.0, ATTN_NORM );
		pev->rendercolor = Vector( 255.0f, 160.0f, 0.0f );
		pev->renderfx = kRenderFxGlowShell;
		pev->renderamt = 200.0f;
		pev->button = 1;
		return;
	}
	else if (pev->button == 1)
	{
		CBaseEntity *pEnt = NULL;

		while ( (pEnt = UTIL_FindEntityInSphere( pEnt, pev->origin, 300.0f )) != NULL )
		{
			if( !FBitSet( pEnt->pev->flags, FL_NOTARGET ) && FBitSet( pEnt->pev->flags, FL_CLIENT|FL_MONSTER ))
			{
				if (TargetVisible( pEnt ) && (pEnt->pev->health > 0) && (pEnt != this))
				{
					Vector dir = (pEnt->pev->origin - (pev->origin - Vector( 0, 0, 24.0f ))).Normalize();
					pEnt->pev->velocity += dir * 1500.0f;
					ClearBits( pEnt->pev->flags, FL_ONGROUND );
				}
			}
		}

		UTIL_ScreenShake( pev->origin, 16.0f, 8.0f, 1.5f, 1200.0f );
		Q_RadiusDamage (this, this, 60.0f, this);
		pev->renderfx = kRenderFxNone;
		AttackFinished (0.1f);
		pev->button = 0;
	}
	// pev->nextthink = gpGlobals->time + 0.1;
}

void CArmagon :: MonsterTurn( float flAngleDelta, float flDelta )
{
	if( fabs( flAngleDelta ) < 10.0f )
	{
		m_flBodyYaw = flDelta;
	}
	else
	{
		if( flAngleDelta > 5.0f )
		{
			m_flBodyYaw += 9.0f;
		}
		else if( flAngleDelta < -5.0f )
		{
			m_flBodyYaw -= 9.0f;
		}
		else
		{
			m_flBodyYaw = flDelta;
		}
	}
}

void CArmagon :: MonsterRegroup( void )
{
	m_bRunAttack = TRUE;

	if ((m_flCnt == 1.0f) && (gpGlobals->time > m_flAttackFinished))
	{
		float delta = pev->ideal_yaw - pev->angles.y;

		if (delta > 180.0f) delta -= 360.0f;
		if (delta < -180.0f) delta += 360.0f;

		if (delta > 0.0f)
			SetActivity( ACT_TURN_LEFT );
		else SetActivity( ACT_TURN_RIGHT );
		m_iAIState = STATE_CUSTOM;
		m_flCount = 0.0f;
		return;
	}

	if (m_fLeftY == 1)
	{
		if (RANDOM_FLOAT( 0.0f, 1.0f) < 0.5f)
		{
			m_iAIState = STATE_CUSTOM;
			SetActivity( ACT_MELEE_ATTACK2 );
		}
		else
		{
			m_iAIState = STATE_CUSTOM;
			SetActivity( ACT_MELEE_ATTACK1 );
		}

		m_bRunAttack = TRUE;
		m_fLeftY = 0;
	}
}

void CArmagon :: BaseThink( void )
{
	float flDelta;
	float flAngleDelta;
//ReportAIState();
	SetBoneController( 0, m_flBodyYaw );

	if( m_hEnemy != NULL )
	{
		m_flEnemyYaw = UTIL_VecToYaw( m_hEnemy->pev->origin - pev->origin );
		pev->ideal_yaw = m_flEnemyYaw;
	}

	flDelta = pev->ideal_yaw - pev->angles.y;
	m_flCnt = 0.0f;

	if( flDelta > 180.0f )
		flDelta -= 360.0f;
	if( flDelta < -180.0f )
		flDelta += 360.0f;

	if( fabs( flDelta ) > 90.0f )
	{
		flDelta = 0.0f;
		m_flCnt = 1.0f;
	}

	flAngleDelta = flDelta - m_flBodyYaw;

	MonsterTurn( flAngleDelta, flDelta );

	if( pev->health < 0.0f )
		return;

	if( gpGlobals->time > m_flIdleTime )
	{
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.5f )
			EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pIdleSounds, ATTN_ARMAGON );

		m_flIdleTime = gpGlobals->time + 3.0f;
	}
}

void CArmagon :: WalkThink( void )
{
	float flDelta;
	float flAngleDelta;

	SetBoneController( 0, m_flBodyYaw );

	CHANGE_YAW( ENT(pev) );

	flDelta = 0.0f;
	m_flCnt = 0.0f;

	if( flDelta > 180.0f )
		flDelta -= 360.0f;
	if( flDelta < -180.0f )
		flDelta += 360.0f;

	if( fabs( flDelta ) > 90.0f )
	{
		flDelta = 0.0f;
		m_flCnt = 1.0f;
	}

	flAngleDelta = flDelta - m_flBodyYaw;

	MonsterTurn( flAngleDelta, flDelta );

	if( pev->health < 0.0f )
		return;

	if( gpGlobals->time > m_flIdleTime )
	{
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.5f )
			EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pIdleSounds, ATTN_ARMAGON );

		m_flIdleTime = gpGlobals->time + 3.0f;
	}

	CBaseEntity *pClient = UTIL_FindClientInPVS( edict() );
	if (!pClient) return;

	if( TargetVisible( pClient ) && pClient->pev->health > 0.0f )
	{
		m_hEnemy = pClient;
		FoundTarget();
	}
}

void CArmagon :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	if (pev->health <= 0.0f)
		return;		// allready dying, don't go into pain frame

	if( pev->button )
		RepulseAttack();

	if (flDamage < 25.0f)
		return;

	if( pev->pain_finished > gpGlobals->time )
		return;

	EMIT_SOUND( edict(), CHAN_VOICE, "armagon/pain.wav", 1.0, ATTN_NORM );

	m_iAIState = STATE_PAIN;
//	SetActivity( ACT_ARM );

	pev->pain_finished = gpGlobals->time + 2.0f;
}

void CArmagon :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	switch( ptr->iHitgroup)
	{
	case 10:
		if (FBitSet( bitsDamageType, DMG_GENERIC | DMG_BULLET ))
		{
			UTIL_Ricochet( ptr->vecEndPos, 1.0 );
			return;
		}
		break;
	}

	CQuakeMonster :: TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

void CArmagon :: Killed( entvars_t *pevAttacker, int iGib )
{
	SetActivity( ACT_DIESIMPLE );
	m_iAIState = STATE_DEAD;
	pev->takedamage = DAMAGE_NO;
//pev->deadflag = DEAD_DEAD;
	UTIL_SetSize( pev, Vector( -32, -32, -24 ), Vector( 32, 32, 32 ));
//pev->solid = SOLID_NOT;
	pev->health = -100;
	pev->renderfx = kRenderFxNone;

	if( m_hEnemy == NULL )
		m_hEnemy = CBaseEntity::Instance( pevAttacker );

	gpWorld->killed_monsters++;

	if( m_fCharmed )
		pev->effects &= ~EF_DIMLIGHT;

	// just an event to increase internal client counter
	MESSAGE_BEGIN( MSG_ALL, gmsgKilledMonster );
	MESSAGE_END();
}

void CArmagon :: BodyExplode1( void )
{
	if (m_flCnt == 0)
	{
		m_flCount = 0;
	}

	if (m_flCnt < 25)
	{
		if (m_flCnt > m_flCount)
		{
			CGib::ThrowGib ("models/gib1.mdl", pev, 64.0f );
			CGib::ThrowGib ("models/gib2.mdl", pev, 64.0f );
			CGib::ThrowGib ("models/gib3.mdl", pev, 64.0f );
			m_flCount = m_flCnt + 1.0f;
		}
		m_flCnt = m_flCnt + 1.0f;

		if (m_flCnt == 12.0f)
		{
			MonsterDeathUse( m_hEnemy, this, USE_TOGGLE, 0.0f );
			ResetSequenceInfo();
		}

	}
	else
	{
		m_flCnt = 0;
		SetThink( BodyExplode2 );
	}

	// animation needs
	MonsterThink();

	pev->nextthink = gpGlobals->time + 0.1f;
}

void CArmagon :: BodyExplode2( void )
{
	if (m_fSequenceFinished)
	{
		SetThink( NULL );
		return;
	}

	if (pev->health == -100)
	{
		EMIT_SOUND( edict(), CHAN_AUTO, "misc/longexpl.wav", 1.0, ATTN_ARMAGON );

		pev->health = -200;

		CGib::ThrowGib ("models/gib1.mdl", pev, 64.0f );
		CGib::ThrowGib ("models/gib2.mdl", pev, 64.0f );
		CGib::ThrowGib ("models/gib3.mdl", pev, 64.0f );
		CGib::ThrowGib ("models/gib1.mdl", pev, 64.0f );
		CGib::ThrowGib ("models/gib2.mdl", pev, 64.0f );
		CGib::ThrowGib ("models/gib3.mdl", pev, 64.0f );
		CGib::ThrowGib ("models/gib1.mdl", pev, 64.0f );
		CGib::ThrowGib ("models/gib2.mdl", pev, 64.0f );
		CGib::ThrowGib ("models/gib3.mdl", pev, 64.0f );

		Q_RadiusDamage( this, this, 60.0f, this );

		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_EXPLOSION );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z + 64.0f );
		MESSAGE_END();
	}

	// animation needs
	MonsterThink();

	pev->nextthink = gpGlobals->time + 0.1f;
}

BOOL CArmagon :: MonsterCheckAttack( void )
{
	Vector spot1, spot2;
	CBaseEntity *pTarg;

	if( m_hEnemy == NULL )
		return FALSE;

	m_fLeftY = 0;

	pTarg = m_hEnemy;
	
	// see if any entities are in the way of the shot
	spot1 = EyePosition();
	spot2 = pTarg->EyePosition();

	TraceResult tr;
	UTIL_TraceLine( spot1, spot2, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr );

	if (tr.fInOpen && tr.fInWater)
		return FALSE;	// sight line crossed contents

	if (tr.pHit != pTarg->edict() && !m_fCharmed)
		return FALSE;	// don't have a clear shot
			
	
	// missile attack
	if (gpGlobals->time < m_flAttackFinished)
		return FALSE;

	float flAngle = pev->angles.y + m_flBodyYaw;
	float flDelta = pev->ideal_yaw - flAngle;
	float flDist = (spot2 - spot1).Length();

	if(( fabs( flDelta ) > 10 && ( flDist > 200 )) || !FBitSet( pTarg->pev->flags, FL_CLIENT|FL_MONSTER ))
		return FALSE;

	if( flDist < 400 )
	{
		// melee attack
		MonsterMeleeAttack();
		return TRUE;
	}

	// missile attack
	m_fLeftY = 1;

	return FALSE;
}

//=========================================================
// Spawn
//=========================================================
void CArmagon :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ))
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/armagon.mdl");
	UTIL_SetSize( pev, Vector( -48, -48, -24 ), Vector( 48, 48, 84 ));

	SetBoneController( 0, 0.0f );

	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;

	if( g_iSkillLevel == SKILL_EASY )
	{
		pev->yaw_speed = 5;
		pev->health = 2000;
		m_flLaunchFactor = 0.9f;
	}
	else if( g_iSkillLevel == SKILL_MEDIUM )
	{
		pev->yaw_speed = 9;
		pev->health = 2500;
		m_flLaunchFactor = 0.85f;
	}
	else
	{
		pev->yaw_speed = 12;
		pev->health = 3500;
		m_flLaunchFactor = 0.75f;
	}

	WalkMonsterInit ();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CArmagon :: Precache()
{
	PRECACHE_MODEL( "models/armagon.mdl" );

	PRECACHE_SOUND_ARRAY( pIdleSounds );

	PRECACHE_SOUND( "armagon/footfall.wav" );
	PRECACHE_SOUND( "armagon/servo.wav" );
	PRECACHE_SOUND( "armagon/death.wav" );
	PRECACHE_SOUND( "armagon/pain.wav" );
	PRECACHE_SOUND( "armagon/repel.wav" );
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CArmagon :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case ARMAGON_STAND_ATTACK:
		StandAttack();
		BaseThink();
		break;
	case ARMAGON_STEP_FALL:
		EMIT_SOUND( edict(), CHAN_BODY, "armagon/footfall.wav", 1.0, ATTN_ARMAGON );
		UTIL_ScreenShake( pev->origin + Vector( 0, 0, -24 ), 4.0f, 4.0f, 0.5f, 600.0f );
		break;
	case ARMAGON_STEP_SERVO:
		EMIT_SOUND( edict(), CHAN_WEAPON, "armagon/servo.wav", SERVO_VOLUME, ATTN_ARMAGON );
		break;
	case ARMAGON_RUN_REGROUP:
		MonsterRegroup();
		break;
	case ARMAGON_FIRE_LASER:
		LaunchLaser( -40.0f );
		LaunchLaser(  40.0f );
		break;
	case ARMAGON_FIRE_ROCKET:
		LaunchRocket( -40.0f );
		LaunchRocket(  40.0f );
		break;
	case ARMAGON_FINISH_ATTACK:
		SetActivity( ACT_ARM );
		AttackFinished( 0.3 );
		break;
	case ARMAGON_CHARGE_ONE:
		m_flCount = 1.0f;
		break;
	case ARMAGON_CHARGE_TWO:
		m_flCount = 2.0f;
		break;
	case ARMAGON_END_REGROUP:
		AttackFinished( 1.0 );
		MonsterRun();
		break;
	case ARMAGON_BODY_EXPLODE:
		StopAnimation();
		SetThink( BodyExplode1 );
		CFuncMultiExploder::MultiExplosion( pev->origin + Vector( 0, 0, 80 ), 20.0f, 10.0f, 3, 0.1f, 0.5f );
		pev->nextthink = gpGlobals->time + 0.1f;
		break;
	case ARMAGON_BODY_HIDE:
		pev->body = 1;
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}