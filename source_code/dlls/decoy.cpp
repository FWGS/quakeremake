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

class CDecoy : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );

	void MonsterKilled( entvars_t *pevAttacker, int iGib );
	int BloodColor( void ) { return BLOOD_COLOR_RED; }
	BOOL MonsterCheckAttack( void ) { return FALSE; }
	BOOL MonsterHasPain( void ) { return FALSE; }
	void AI_Idle( void );
	void AI_Walk( float flDist );
	void AI_Run( float flDist );

	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );
	void UpdateStepSound( float fvol, float flDelay );
};

LINK_ENTITY_TO_CLASS( monster_decoy, CDecoy );

void CDecoy :: MonsterIdle( void )
{
	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CDecoy :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 12;
}

void CDecoy :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	SetActivity( ACT_RUN );
	m_flMonsterSpeed = 18;
}

void CDecoy :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	if( ShouldGibMonster( iGib ))
	{
		EMIT_SOUND( edict(), CHAN_VOICE, "player/udeath.wav", 1.0, ATTN_NORM );
		CGib::ThrowHead ("models/h_player.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib2.mdl", pev);
		CGib::ThrowGib ("models/gib3.mdl", pev);
		UTIL_Remove( this );
		return;
	}

	// regular death
	switch ( RANDOM_LONG(0,4) )
	{
 	case 0: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death1.wav", 1, ATTN_NORM );	break;
 	case 1: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death2.wav", 1, ATTN_NORM );	break;
 	case 2: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death3.wav", 1, ATTN_NORM );	break;
 	case 3: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death4.wav", 1, ATTN_NORM );	break;
 	case 4: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death5.wav", 1, ATTN_NORM );	break;
	}
}

void CDecoy :: UpdateStepSound( float fvol, float flDelay )
{
	if (pev->pain_finished >= gpGlobals->time)
		return;

	switch (RANDOM_LONG(1,7))
	{
	case 1:	EMIT_SOUND( ENT(pev), CHAN_BODY, "misc/foot1.wav", fvol, ATTN_NORM ); break;
	case 2:	EMIT_SOUND( ENT(pev), CHAN_BODY, "misc/foot2.wav", fvol, ATTN_NORM ); break;
	case 3:	EMIT_SOUND( ENT(pev), CHAN_BODY, "misc/foot3.wav", fvol, ATTN_NORM ); break;
	case 4:	EMIT_SOUND( ENT(pev), CHAN_BODY, "misc/foot4.wav", fvol, ATTN_NORM ); break;
	case 5:	EMIT_SOUND( ENT(pev), CHAN_BODY, "misc/foot5.wav", fvol, ATTN_NORM ); break;
	case 6:	EMIT_SOUND( ENT(pev), CHAN_BODY, "misc/foot6.wav", fvol, ATTN_NORM ); break;
	case 7:	EMIT_SOUND( ENT(pev), CHAN_BODY, "misc/foot7.wav", fvol, ATTN_NORM ); break;
	}

	pev->pain_finished = gpGlobals->time + flDelay;
}

void CDecoy :: AI_Idle( void )
{
	CHANGE_YAW( ENT(pev) );
	
	if (gpGlobals->time > m_flPauseTime)
	{
		MonsterWalk();
		return;
	}

	// change angle slightly
}

void CDecoy :: AI_Walk( float flDist )
{
	m_flMoveDistance = flDist;

	MoveToGoal( flDist );

	UpdateStepSound( 0.35f, 0.45f );
}

void CDecoy :: AI_Run( float flDist )
{
	m_flMoveDistance = flDist;

	MoveToGoal( flDist );

	UpdateStepSound( 0.5f, 0.3f );
}

//=========================================================
// Spawn
//=========================================================
void CDecoy :: Spawn( void )
{
	edict_t	*pl = INDEXENT( 1 );

	if( !g_pGameRules->FAllowMonsters( ))
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/player.mdl");
	UTIL_SetSize( pev, VEC_HULL_MIN, VEC_HULL_MAX );

	pev->view_ofs	= VEC_VIEW;
	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->colormap	= pl->v.colormap;	// copy colormap from realplayer
	pev->weaponmodel	= pl->v.weaponmodel;
	pev->health	= 3000000;

	WalkMonsterInit ();
	gpWorld->total_monsters--;
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CDecoy :: Precache()
{
	PRECACHE_MODEL( "models/player.mdl" );
	PRECACHE_MODEL( "models/h_player.mdl" );
	PRECACHE_MODEL( "models/freeman.mdl" );	// easter egg

	// get pointer to decoy
	g_pPlayerDecoy = this;
}