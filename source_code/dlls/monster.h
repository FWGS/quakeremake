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
#ifndef MONSTER_H
#define MONSTER_H

typedef enum
{
	ATTACK_NONE = 0,
	ATTACK_STRAIGHT,
	ATTACK_SLIDING,
	ATTACK_MELEE,
	ATTACK_MISSILE
} ATTACKSTATE;

typedef enum
{
	STATE_IDLE = 0,
	STATE_WALK,
	STATE_RUN,
	STATE_ATTACK,
	STATE_PAIN,
	STATE_DEAD
} AISTATE;

// distance ranges
typedef enum
{
	RANGE_MELEE = 0,
	RANGE_NEAR,
	RANGE_MID,
	RANGE_FAR
} RANGETYPE;

//
// generic Monster
//
class CQuakeMonster : public CBaseAnimating
{
public:
	float		m_flSearchTime;
	float		m_flPauseTime;

	AISTATE		m_iAIState;
	ATTACKSTATE	m_iAttackState;

	Activity		m_Activity;	// what the monster is doing (animation)
	Activity		m_IdealActivity;	// monster should switch to this activity

	float		m_flMonsterSpeed;
	float		m_flMoveDistance;	// member laste move distance. Used for strafe
	BOOL		m_fLeftY;

	float		m_flSightTime;
	EHANDLE		m_hSightEntity;
	int		m_iRefireCount;

	float		m_flEnemyYaw;
	RANGETYPE		m_iEnemyRange;
	BOOL		m_fEnemyInFront;
	BOOL		m_fEnemyVisible;

	virtual CQuakeMonster *GetMonster( void ) { return this; }

	// overloaded monster functions (same as th_callbacks in quake)
	virtual void	MonsterIdle( void ) {}
	virtual void	MonsterWalk( void ) {}
	virtual void	MonsterRun( void ) {}
	virtual void	MonsterMeleeAttack( void ) {}
	virtual void	MonsterMissileAttack( void ) {}
	virtual void	MonsterPain( CBaseEntity *pAttacker, float flDamage );
	virtual void	MonsterKilled( entvars_t *pevAttacker, int iGib ) {}
	virtual void	MonsterSight( void );
	virtual void	MonsterAttack( void );
	virtual void	CornerReached( void ) {}	// called while path_corner is reached

	virtual BOOL	MonsterCheckAnyAttack( void );
	virtual BOOL	MonsterCheckAttack( void );
	virtual BOOL	MonsterHasMeleeAttack( void ) { return FALSE; }
	virtual BOOL	MonsterHasMissileAttack( void ) { return FALSE; }
	virtual BOOL	MonsterHasPain( void ) { return TRUE; }	// tarbaby feels no pain

	void EXPORT	MonsterThink( void );
	void EXPORT	MonsterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT	MonsterDeathUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void		TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	int		TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	virtual void	Killed( entvars_t *pevAttacker, int iGib );
		
	// common utility functions
	void		SetEyePosition( void );
	void		SetActivity ( Activity NewActivity );
	inline void	StopAnimation( void ) { pev->framerate = 0; }
	void		AttackFinished( float flFinishTime );
	RANGETYPE		TargetRange( CBaseEntity *pTarg );
	BOOL		TargetVisible( CBaseEntity *pTarg );
	BOOL		InFront( CBaseEntity *pTarg );
	BOOL		FindTarget( void );
	BOOL		FacingIdeal( void );
	void EXPORT	FoundTarget( void );
	void		HuntTarget ( void );
	BOOL		ShouldGibMonster( int iGib );
	BOOL		CheckRefire( void );
	void		ReportAIState( void ); // debug

	// MoveExecute functions
	BOOL		CloseEnough( float flDist );
	BOOL		WalkMove( float flYaw, float flDist );
	void		MoveToGoal( float flDist );

	virtual void	AI_Forward( float flDist );
	virtual void	AI_Backward( float flDist );
	virtual void	AI_Pain( float flDist );
	virtual void	AI_PainForward( float flDist );
	virtual void	AI_Walk( float flDist );
	virtual void	AI_Run( float flDist );
	virtual void	AI_Idle( void );
	virtual void	AI_Turn( void );
	virtual void	AI_Run_Melee( void );
	virtual void	AI_Run_Missile( void );
	virtual void	AI_Run_Slide( void );
	virtual void	AI_Charge( float flDist );
	virtual void	AI_Charge_Side( void );
	virtual void	AI_Face( void );
	virtual void	AI_Melee( void );
	virtual void	AI_Melee_Side( void );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	// monsters init
	void EXPORT	FlyMonsterInitThink( void );
	void		FlyMonsterInit( void );

	void EXPORT	WalkMonsterInitThink( void );
	void		WalkMonsterInit( void );

	void EXPORT	SwimMonsterInitThink( void );
	void		SwimMonsterInit( void );
};

//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:
	void Spawn( const char *szGibModel );
	void EXPORT BounceGibTouch ( CBaseEntity *pOther );
	void EXPORT WaitTillLand( void );
	Vector VelocityForDamage( float flDamage );

	static void ThrowHead( const char *szGibName, entvars_t *pevVictim );
	static void ThrowGib( const char *szGibName, entvars_t *pevVictim );

	int		m_bloodColor;
	int		m_cBloodDecals;
	float	m_lifeTime;
};

class CBubbleSource : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	Think( void );
};

// misc helper entity for trigger_teleport
class CTeleFog : public CBaseEntity
{
	void Spawn( void );
	int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
	void Think( void );
public:
	static void CreateFog( const Vector pos );
};

// misc helper entity for trigger_teleport
class CTeleFrag : public CBaseEntity
{
	int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
	void Touch( CBaseEntity *pOther );
public:
	static void CreateTDeath( const Vector pos, const CBaseEntity *pOwner );
};

#endif