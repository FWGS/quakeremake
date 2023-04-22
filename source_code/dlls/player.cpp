	/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== player.cpp ========================================================

  functions dealing with the player

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "client.h"
#include "player.h"
#include "weapons.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "hltv.h"
#include "items.h"
#include "skill.h"

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL ULONG		g_ulModelIndexEyes;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL int		g_iSkillLevel;
extern DLL_GLOBAL float		g_flWeaponCheat;

BOOL gInitHUD = TRUE;

extern void CopyToBodyQue(entvars_t* pev);
extern void respawn(entvars_t *pev, BOOL fCopyCorpse);
extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer );

// Global Savedata for player
TYPEDESCRIPTION	CBasePlayer::m_playerSaveData[] = 
{
	DEFINE_FIELD( CBasePlayer, m_afButtonLast, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_afButtonPressed, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_afButtonReleased, FIELD_INTEGER ),

	DEFINE_FIELD( CBasePlayer, m_afPhysicsFlags, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iWeapon, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_flFlySound, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flNextAttack, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flSwimTime, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_flInvincibleTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flInvincibleFinished, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flInvincibleSound, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flInvisibleTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flInvisibleFinished, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flInvisibleSound, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flSuperDamageTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flSuperDamageFinished, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flSuperDamageSound, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flRadSuitTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flRadSuitFinished, FIELD_TIME ),
#ifdef HIPNOTIC
	DEFINE_FIELD( CBasePlayer, m_flWetSuitTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flWetSuitFinished, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flEmpathyShieldTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flEmpathyShieldFinished, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flEmpathyShieldSound, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_fEarthQuake, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBasePlayer, m_flEarthQuakeTime, FIELD_TIME ),
#endif /* HIPNOTIC */

	DEFINE_FIELD( CBasePlayer, m_flCheckHealthTime, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_bitsHUDDamage, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_flFallVelocity, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, m_fInitHUD, FIELD_BOOLEAN ),

	DEFINE_FIELD( CBasePlayer, m_iFOV, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iHideHUD, FIELD_INTEGER ),
};	


int gmsgShake = 0;
int gmsgFade = 0;
int gmsgResetHUD = 0;
int gmsgInitHUD = 0;
int gmsgDamage = 0;
int gmsgHudText = 0;
int gmsgDeathMsg = 0;
int gmsgScoreInfo = 0;
int gmsgTeamInfo = 0;
int gmsgTeamScore = 0;
int gmsgGameMode = 0;
int gmsgSayText = 0;
int gmsgTextMsg = 0;
int gmsgSetFOV = 0;
int gmsgHideHUD = 0;
int gmsgTempEntity = 0;
int gmsgLevelName = 0;
int gmsgStats = 0;		// generic stats message
int gmsgItems = 0;
int gmsgFoundSecret = 0;
int gmsgKilledMonster = 0;

void LinkUserMessages( void )
{
	// Already taken care of?
	if ( gmsgDamage )
	{
		return;
	}

	gmsgDamage = REG_USER_MSG( "Damage", 8 );
	gmsgHudText = REG_USER_MSG( "HudText", -1 );
	gmsgSayText = REG_USER_MSG( "SayText", -1 );
	gmsgTextMsg = REG_USER_MSG( "TextMsg", -1 );
	gmsgResetHUD = REG_USER_MSG("ResetHUD", 1);		// called every respawn
	gmsgInitHUD = REG_USER_MSG("InitHUD", 0 );		// called every time a new player joins the server
	gmsgDeathMsg = REG_USER_MSG( "DeathMsg", -1 );
	gmsgScoreInfo = REG_USER_MSG( "ScoreInfo", 9 );
	gmsgTeamInfo = REG_USER_MSG( "TeamInfo", -1 );  // sets the name of a player's team
	gmsgTeamScore = REG_USER_MSG( "TeamScore", -1 );  // sets the score of a team on the scoreboard
	gmsgGameMode = REG_USER_MSG( "GameMode", 1 );
	gmsgHideHUD = REG_USER_MSG( "HideHUD", 1 );
	gmsgSetFOV = REG_USER_MSG( "SetFOV", 1 );
	gmsgShake = REG_USER_MSG("ScreenShake", sizeof(ScreenShake));
	gmsgFade = REG_USER_MSG("ScreenFade", sizeof(ScreenFade));
	gmsgTempEntity = REG_USER_MSG("TempEntity", -1);
	gmsgLevelName = REG_USER_MSG("LevelName", -1);
	gmsgStats = REG_USER_MSG( "Stats", 3 );
	gmsgItems = REG_USER_MSG( "Items", 4 );
	gmsgFoundSecret = REG_USER_MSG( "FoundSecret", 0 );
	gmsgKilledMonster = REG_USER_MSG( "KillMonster", 0 );
}

LINK_ENTITY_TO_CLASS( player, CBasePlayer );

void CBasePlayer :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	if (pev->health < 0)
		return;

	if ( FClassnameIs( pAttacker->pev, "teledeath" ) )
	{
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/teledth1.wav", 1, ATTN_NORM );
		return;
	}

	PainSound ();
}

void CBasePlayer :: DeathBubbles( float flCount )
{
	CBubbleSource *pBubbles = GetClassPtr( (CBubbleSource *)NULL);
	if( !pBubbles ) return;

	pBubbles->pev->classname = MAKE_STRING( "air_bubbles" );
	UTIL_SetOrigin( pBubbles->pev, pev->origin );
	pBubbles->pev->air_finished = flCount;
	pBubbles->pev->button = 1; // tell entity what is the player bubbles
	pBubbles->Spawn();
}

#ifdef HIPNOTIC
int CBasePlayer :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if( g_fDischarged && m_flWetSuitFinished )
		return 0;

	return CQuakeMonster :: TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

#endif /* HIPNOTIC */
void CBasePlayer :: PainSound( void )
{
	// water pain sounds
	if (pev->watertype == CONTENT_WATER && pev->waterlevel == 3)
	{
		DeathBubbles( 1 );

		if ( RANDOM_FLOAT(0,1) > 0.5 )
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/drown1.wav", 1, ATTN_NORM );
		else
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/drown2.wav", 1, ATTN_NORM );
		return;
	}

	// slime pain sounds
	if (pev->watertype == CONTENT_SLIME)
	{
		if ( RANDOM_FLOAT(0,1) > 0.5 )
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/lburn1.wav", 1, ATTN_NORM );
		else
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/lburn2.wav", 1, ATTN_NORM );
		return;
	}

	// lava pain sounds
	if (pev->watertype == CONTENT_LAVA)
	{
		if ( RANDOM_FLOAT(0,1) > 0.5 )
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/lburn1.wav", 1, ATTN_NORM );
		else
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/lburn2.wav", 1, ATTN_NORM );
		return;
	}

	// don't make multiple pain sounds right after each other
	if (pev->pain_finished > gpGlobals->time)
	{
		m_bAxHitMe = 0;
		return;
	}

	pev->pain_finished = gpGlobals->time + 0.5;

	// ax pain sound
	if (m_bAxHitMe == 1)
	{
		m_bAxHitMe = 0;
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/axhit1.wav", 1, ATTN_NORM );
		return;
	}

	// play random sound
	switch ( RANDOM_LONG(0,5) )
	{
	case 0: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/pain1.wav", 1, ATTN_NORM );	break;
	case 1: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/pain2.wav", 1, ATTN_NORM );	break;
	case 2: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/pain3.wav", 1, ATTN_NORM );	break;
	case 3: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/pain4.wav", 1, ATTN_NORM );	break;
	case 4: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/pain5.wav", 1, ATTN_NORM );	break;
	case 5: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/pain6.wav", 1, ATTN_NORM );	break;
	}
}

void CBasePlayer :: DeathSound( void )
{
	// water death sounds
	if (pev->waterlevel == 3)
	{
		DeathBubbles( 20 );
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/h2odeath.wav", 1, ATTN_NONE);
		return;
	}

	// play random sound
	switch ( RANDOM_LONG(0,4) )
	{
 	case 0: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death1.wav", 1, ATTN_NORM );	break;
 	case 1: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death2.wav", 1, ATTN_NORM );	break;
 	case 2: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death3.wav", 1, ATTN_NORM );	break;
 	case 3: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death4.wav", 1, ATTN_NORM );	break;
 	case 4: EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/death5.wav", 1, ATTN_NORM );	break;
	}
}

//=========================================================
// PackDeadPlayerItems - call this when a player dies to
// pack up the appropriate weapons and ammo items, and to
// destroy anything that shouldn't be packed.
//
// This is pretty brute force :(
//=========================================================
void CBasePlayer::PackDeadPlayerItems( void )
{
	int iWeaponRules;
	int iAmmoRules;

	// get the game rules 
	iWeaponRules = g_pGameRules->DeadPlayerWeapons( this );
 	iAmmoRules = g_pGameRules->DeadPlayerAmmo( this );

	if ( iWeaponRules == GR_PLR_DROP_GUN_NO && iAmmoRules == GR_PLR_DROP_AMMO_NO )
	{
		// nothing to pack. Remove the weapons and return. Don't call create on the box!
		RemoveAllItems();
		return;
	}

	CWeaponBox *pBox = CWeaponBox::DropBackpack( this, m_iWeapon );

	RemoveAllItems();// now strip off everything that wasn't handled by the code above.
}

void CBasePlayer::RemoveAllItems( void )
{
	pev->viewmodel = 0;
	pev->weaponmodel = 0;
	m_iItems = 0;
}

/*
 * GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
 *
 * ENTITY_METHOD(PlayerDie)
 */
entvars_t *g_pevLastInflictor;  // Set in combat.cpp.  Used to pass the damage inflictor for death messages.
			  // Better solution:  Add as parameter to all Killed() functions.

void CBasePlayer::Killed( entvars_t *pevAttacker, int iGib )
{
	ClientObituary( pev, pevAttacker );

	SetAnimation( PLAYER_DIE );
	
	m_iRespawnFrames = 0;
	m_iDeaths += 1;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes


	m_flInvincibleFinished = 0;
	m_flInvisibleFinished = 0;
	m_flSuperDamageFinished = 0;
	m_flRadSuitFinished = 0;
#ifdef HIPNOTIC
	m_flWetSuitFinished = 0;
	m_flEmpathyShieldFinished = 0;
#endif /* HIPNOTIC */

	pev->deadflag		= DEAD_DYING;
	pev->movetype		= MOVETYPE_TOSS;
	pev->solid		= SOLID_NOT;

	UTIL_SetSize( pev, Vector( -8, -8, -8 ), Vector( 8, 8, 8 ));

	ClearBits( pev->flags, FL_ONGROUND );
	if (pev->velocity.z < 10)
		pev->velocity.z += RANDOM_FLOAT(0,300);

	// send "health" update message to zero
	m_iClientHealth = 0;
	MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
		WRITE_BYTE( STAT_HEALTH );
		WRITE_SHORT( m_iClientHealth );
	MESSAGE_END();

	// reset FOV
	pev->fov = m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
		WRITE_BYTE(0);
	MESSAGE_END();

	if ( ShouldGibMonster( iGib ))
	{
		pev->solid = SOLID_NOT;
		CGib::ThrowHead ("models/h_player.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib2.mdl", pev);
		CGib::ThrowGib ("models/gib3.mdl", pev);
		pev->effects |= EF_NODRAW;

		if ( FClassnameIs( pevAttacker, "teledeath" ) )
		{
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/teledth1.wav", 1, ATTN_NORM );
			return;
		}

		if ( FClassnameIs( pevAttacker, "teledeath2" ) )
		{
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "player/teledth1.wav", 1, ATTN_NORM );
			return;
		}

		if( RANDOM_LONG( 0, 1 ))
			EMIT_SOUND( edict(), CHAN_VOICE, "player/gib.wav", 1.0, ATTN_NORM );
		else EMIT_SOUND( edict(), CHAN_VOICE, "player/udeath.wav", 1.0, ATTN_NORM );

		return;
	}

	DeathSound();
	
	pev->angles.x = 0;
	pev->angles.z = 0;

	SetThink(PlayerDeathThink);
	pev->nextthink = gpGlobals->time + 0.1;
}


// Set the activity based on an event or current state
void CBasePlayer::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;
	float speed;
	char szAnim[64];

	speed = pev->velocity.Length2D();

	if (pev->flags & FL_FROZEN)
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	switch (playerAnim) 
	{
	case PLAYER_JUMP:
		m_IdealActivity = ACT_HOP;
		break;
	
	case PLAYER_SUPERJUMP:
		m_IdealActivity = ACT_LEAP;
		break;
	
	case PLAYER_DIE:
		m_IdealActivity = ACT_DIESIMPLE;
		break;

	case PLAYER_ATTACK1:	
		switch( m_Activity )
		{
		case ACT_HOVER:
		case ACT_SWIM:
		case ACT_HOP:
		case ACT_LEAP:
		case ACT_DIESIMPLE:
			m_IdealActivity = m_Activity;
			break;
		default:
			m_IdealActivity = ACT_RANGE_ATTACK1;
			break;
		}
		break;
	case PLAYER_IDLE:
	case PLAYER_WALK:
		if ( !FBitSet( pev->flags, FL_ONGROUND ) && (m_Activity == ACT_HOP || m_Activity == ACT_LEAP) )	// Still jumping
		{
			m_IdealActivity = m_Activity;
		}
		else if ( pev->waterlevel > 1 )
		{
			if ( speed == 0 )
				m_IdealActivity = ACT_HOVER;
			else
				m_IdealActivity = ACT_SWIM;
		}
		else
		{
			m_IdealActivity = ACT_WALK;
		}
		break;
	}

	switch (m_IdealActivity)
	{
	case ACT_HOVER:
	case ACT_LEAP:
	case ACT_SWIM:
	case ACT_HOP:
	case ACT_DIESIMPLE:
	default:
		if ( m_Activity == m_IdealActivity)
			return;
		m_Activity = m_IdealActivity;

		animDesired = LookupActivity( m_Activity );
		// Already using the desired animation?
		if (pev->sequence == animDesired)
			return;

		pev->gaitsequence = 0;
		pev->sequence		= animDesired;
		pev->frame			= 0;
		ResetSequenceInfo( );
		return;

	case ACT_RANGE_ATTACK1:
		strcpy( szAnim, "ref_shoot_" );
		strcat( szAnim, m_szAnimExtention );
		animDesired = LookupSequence( szAnim );
		if (animDesired == -1)
			animDesired = 0;

		if ( pev->sequence != animDesired || !m_fSequenceLoops )
		{
			pev->frame = 0;
		}

		if (!m_fSequenceLoops)
		{
			pev->effects |= EF_NOINTERP;
		}

		m_Activity = m_IdealActivity;

		pev->sequence		= animDesired;
		ResetSequenceInfo( );
		break;

	case ACT_WALK:
		if (m_Activity != ACT_RANGE_ATTACK1 || m_fSequenceFinished)
		{
			strcpy( szAnim, "ref_aim_" );
			strcat( szAnim, m_szAnimExtention );
			animDesired = LookupSequence( szAnim );
			if (animDesired == -1)
				animDesired = 0;
			m_Activity = ACT_WALK;
		}
		else
		{
			animDesired = pev->sequence;
		}
	}

	if ( speed > 220 )
	{
		pev->gaitsequence	= LookupActivity( ACT_RUN );
	}
	else if (speed > 0)
	{
		pev->gaitsequence	= LookupActivity( ACT_WALK );
	}
	else
	{
		// pev->gaitsequence	= LookupActivity( ACT_WALK );
		pev->gaitsequence	= LookupSequence( "deep_idle" );
	}


	// Already using the desired animation?
	if (pev->sequence == animDesired)
		return;

	//ALERT( at_console, "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	pev->sequence		= animDesired;
	pev->frame			= 0;
	ResetSequenceInfo( );
}

/*
===========
WaterMove
============
*/
void CBasePlayer::WaterMove( void )
{
	int air;

	if (pev->movetype == MOVETYPE_NOCLIP)
		return;

	if (pev->health < 0)
		return;

	// waterlevel 0 - not in water
	// waterlevel 1 - feet in water
	// waterlevel 2 - waist in water
	// waterlevel 3 - head in water

	if (pev->waterlevel != 3) 
	{
		// not underwater
		
		// play 'up for air' sound
		if (pev->air_finished < gpGlobals->time)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/gasp2.wav", 1, ATTN_NORM);
		else if (pev->air_finished < gpGlobals->time + 9)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/gasp1.wav", 1, ATTN_NORM);

		pev->air_finished = gpGlobals->time + 12;
		pev->dmg = 2;
	}
	else if (pev->air_finished < gpGlobals->time)
	{	
		// fully under water
		// stop restoring damage while underwater

		if (pev->pain_finished < gpGlobals->time)
		{
			// take drowning damage
			pev->dmg += 2;
			if (pev->dmg > 15)
				pev->dmg = 10;
			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), pev->dmg, DMG_DROWN);
			pev->pain_finished = gpGlobals->time + 1;
		} 
	}

	if (!pev->waterlevel)
	{
		if (FBitSet(pev->flags, FL_INWATER))
		{       
			// play leave water sound
			EMIT_SOUND(ENT(pev), CHAN_BODY, "misc/outwater.wav", 0.8, ATTN_NORM);
			ClearBits(pev->flags, FL_INWATER);
		}
		return;
	}

	if (pev->watertype == CONTENT_WATER)
	{
		// make bubbles
		air = (int)(pev->air_finished - gpGlobals->time);
		if (!RANDOM_LONG(0,0x1f) && RANDOM_LONG(0,11) >= air)
		{
			switch (RANDOM_LONG(0,3))
			{
			case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/swim1.wav", 0.8, ATTN_NORM); break;
			case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/swim2.wav", 0.8, ATTN_NORM); break;
			case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/swim3.wav", 0.8, ATTN_NORM); break;
			case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/swim4.wav", 0.8, ATTN_NORM); break;
			}
		}
	}

	if (pev->watertype == CONTENT_LAVA)		// do damage
	{
		if (pev->dmgtime < gpGlobals->time)
		{
			if (m_flRadSuitFinished > gpGlobals->time)
				pev->dmgtime = gpGlobals->time + 1;
			else
				pev->dmgtime = gpGlobals->time + 0.2;

			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 10 * pev->waterlevel, DMG_BURN);
		}
	}
	else if (pev->watertype == CONTENT_SLIME)		// do damage
	{
		if (pev->dmgtime < gpGlobals->time && m_flRadSuitFinished < gpGlobals->time)
		{
			pev->dmgtime = gpGlobals->time + 1;
			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 4 * pev->waterlevel, DMG_ACID);
		}
	}
	
	if (!FBitSet(pev->flags, FL_INWATER))
	{
		// player enter water sound
		if (pev->watertype == CONTENT_LAVA)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "player/inlava.wav", 1, ATTN_NORM);
		if (pev->watertype == CONTENT_WATER)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "player/inh2o.wav", 1, ATTN_NORM);
		if (pev->watertype == CONTENT_SLIME)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "player/slimbrn2.wav", 1, ATTN_NORM);

		SetBits(pev->flags, FL_INWATER);
		pev->dmgtime = 0;
	}

	if (! (pev->flags & FL_WATERJUMP) )
		pev->velocity = pev->velocity - 0.8 * pev->waterlevel * gpGlobals->frametime * pev->velocity;
}

void CBasePlayer::PlayerDeathThink(void)
{
	float flForward;

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		flForward = pev->velocity.Length() - 20;
		if (flForward <= 0)
			pev->velocity = g_vecZero;
		else    
			pev->velocity = flForward * pev->velocity.Normalize();
	}

	if ( HasWeapons() )
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}


	if (pev->modelindex && (!m_fSequenceFinished) && (pev->deadflag == DEAD_DYING))
	{
		StudioFrameAdvance( );

		m_iRespawnFrames++;				// Note, these aren't necessarily real "frames", so behavior is dependent on # of client movement commands
		if ( m_iRespawnFrames < 120 )   // Animations should be no longer than this
			return;
	}

	// once we're done animating our death and we're on the ground, we want to set movetype to None so our dead body won't do collisions and stuff anymore
	// this prevents a bug where the dead body would go to a player's head if he walked over it while the dead player was clicking their button to respawn
	if ( pev->movetype != MOVETYPE_NONE && FBitSet(pev->flags, FL_ONGROUND) )
		pev->movetype = MOVETYPE_NONE;

	if (pev->deadflag == DEAD_DYING)
		pev->deadflag = DEAD_DEAD;
	
	StopAnimation();

	pev->effects |= EF_NOINTERP;
	pev->framerate = 0.0;

	BOOL fAnyButtonDown = (pev->button & ~IN_SCORE );
	
	// wait for all buttons released
	if (pev->deadflag == DEAD_DEAD)
	{
		if (fAnyButtonDown)
			return;

		if ( g_pGameRules->FPlayerCanRespawn( this ) )
		{
			m_fDeadTime = gpGlobals->time;
			pev->deadflag = DEAD_RESPAWNABLE;
		}
		
		return;
	}

// if the player has been dead for one second longer than allowed by forcerespawn, 
// forcerespawn isn't on. Send the player off to an intermission camera until they 
// choose to respawn.
	if ( g_pGameRules->IsMultiplayer() && ( gpGlobals->time > (m_fDeadTime + 6) ) && !(m_afPhysicsFlags & PFLAG_OBSERVER) )
	{
		// go to dead camera. 
		StartDeathCam();
	}
	
// wait for any button down,  or mp_forcerespawn is set and the respawn time is up
	if (!fAnyButtonDown && !( g_pGameRules->IsMultiplayer() && forcerespawn.value > 0 && (gpGlobals->time > (m_fDeadTime + 5))) )
		return;

	pev->button = 0;
	m_iRespawnFrames = 0;

	//ALERT(at_console, "Respawn\n");

	respawn(pev, !(m_afPhysicsFlags & PFLAG_OBSERVER) );// don't copy a corpse if we're in deathcam.
	pev->nextthink = -1;
}

//=========================================================
// StartDeathCam - find an intermission spot and send the
// player off into observer mode
//=========================================================
void CBasePlayer::StartDeathCam( void )
{
	edict_t *pSpot, *pNewSpot;
	int iRand;

	if ( pev->view_ofs == g_vecZero )
	{
		// don't accept subsequent attempts to StartDeathCam()
		return;
	}

	pSpot = FIND_ENTITY_BY_CLASSNAME( NULL, "info_intermission");	

	if ( !FNullEnt( pSpot ) )
	{
		// at least one intermission spot in the world.
		iRand = RANDOM_LONG( 0, 3 );

		while ( iRand > 0 )
		{
			pNewSpot = FIND_ENTITY_BY_CLASSNAME( pSpot, "info_intermission");
			
			if ( pNewSpot )
			{
				pSpot = pNewSpot;
			}

			iRand--;
		}

		CopyToBodyQue( pev );
		StartObserver( pSpot->v.origin, pSpot->v.v_angle );
	}
	else
	{
		// no intermission spot. Push them up in the air, looking down at their corpse
		TraceResult tr;
		CopyToBodyQue( pev );
		UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, 128 ), ignore_monsters, edict(), &tr );
		StartObserver( tr.vecEndPos, UTIL_VecToAngles( tr.vecEndPos - pev->origin  ) );
		return;
	}
}

void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle )
{
	m_afPhysicsFlags |= PFLAG_OBSERVER;

	pev->view_ofs = g_vecZero;
	pev->angles = pev->v_angle = vecViewAngle;
	pev->fixangle = TRUE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_NONE;
	pev->modelindex = 0;
	UTIL_SetOrigin( pev, vecPosition );
}

void CBasePlayer::Jump( void )
{
	Vector		vecWallCheckDir;// direction we're tracing a line to find a wall when walljumping
	Vector		vecAdjustedVelocity;
	Vector		vecSpot;
	TraceResult	tr;
	
	if (FBitSet(pev->flags, FL_WATERJUMP))
		return;
	
	if (pev->waterlevel >= 2)
	{
		if (pev->watertype == CONTENT_WATER)
			pev->velocity.z = 100;
		else if (pev->watertype == CONTENT_SLIME)
			pev->velocity.z = 80;
		else
			pev->velocity.z = 50;

		// play swiming sound
		if (m_flSwimTime < gpGlobals->time)
		{
			m_flSwimTime = gpGlobals->time + 1;
			if (RANDOM_LONG(0, 1))
				EMIT_SOUND(ENT(pev), CHAN_BODY, "misc/water1.wav", 1, ATTN_NORM);
			else
				EMIT_SOUND(ENT(pev), CHAN_BODY, "misc/water2.wav", 1, ATTN_NORM);
		}

		return;
	}

	// jump velocity is sqrt( height * gravity * 2)

	// If this isn't the first frame pressing the jump button, break out.
	if ( !FBitSet( m_afButtonPressed, IN_JUMP ) )
		return;         // don't pogo stick

	if ( !(pev->flags & FL_ONGROUND) || !pev->groundentity )
	{
		return;
	}

	// many features in this function use v_forward, so makevectors now.
	UTIL_MakeVectors (pev->angles);
	
	SetAnimation( PLAYER_JUMP );

	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/plyrjmp8.wav", 1, ATTN_NORM);
}

void CBasePlayer::AddPoints( int score, BOOL bAllowNegativeScore )
{
	// Positive score always adds
	if ( score < 0 )
	{
		if ( !bAllowNegativeScore )
		{
			if ( pev->frags < 0 )		// Can't go more negative
				return;
			
			if ( -score > pev->frags )	// Will this go negative?
			{
				score = -pev->frags;		// Sum will be 0
			}
		}
	}

	pev->frags += score;

	MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( ENTINDEX(edict()) );
		WRITE_SHORT( pev->frags );
		WRITE_SHORT( m_iDeaths );
		WRITE_SHORT( 0 );
		WRITE_SHORT( g_pGameRules->GetTeamIndex( m_szTeamName ) + 1 );
	MESSAGE_END();
}


void CBasePlayer::AddPointsToTeam( int score, BOOL bAllowNegativeScore )
{
	int index = entindex();

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

		if ( pPlayer && i != index )
		{
			if ( g_pGameRules->PlayerRelationship( this, pPlayer ) == GR_TEAMMATE )
			{
				pPlayer->AddPoints( score, bAllowNegativeScore );
			}
		}
	}
}

void CBasePlayer::PreThink(void)
{
	int buttonsChanged = (m_afButtonLast ^ pev->button);	// These buttons have changed this frame
	
	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed =  buttonsChanged & pev->button;		// The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & (~pev->button);	// The ones not down are "released"

	g_pGameRules->PlayerThink( this );

	if ( g_fGameOver )
		return;         // intermission or finale

	UTIL_MakeVectors(pev->v_angle);             // is this still used?
	
	ItemPreFrame( );
	WaterMove();

#ifdef HIPNOTIC
	// Kill player on Edge of Oblivion
	if(( pev->origin.z < -1300.0f ) && FStrEq( STRING( gpWorld->pev->model ), "maps/hipdm1.bsp" ) && ( pev->health > 0.0f ) )
	{
		m_iDeathType = MAKE_STRING ("falling");

		if (m_flInvincibleFinished >= gpGlobals->time)
		{
			m_flInvincibleFinished = 0;
			m_iItems &= ~IT_INVULNERABILITY;
			m_flInvincibleTime = 0;
		}
		TakeDamage( pev, gpWorld->pev, pev->health + 1000.0f, DMG_FALL);
	}

#endif /* HIPNOTIC */
	// mega_health received too many health for player! decrease it every second
	if( pev->health > pev->max_health && (m_flCheckHealthTime <= gpGlobals->time))
	{
		m_flCheckHealthTime = gpGlobals->time + 1;
		pev->health -= 1;
	}

#ifdef HIPNOTIC
	if( m_flWetSuitFinished > gpGlobals->time )
	{
		if( pev->waterlevel == 2 )
		{
			pev->velocity *= 1.25f;
		}
		if( pev->waterlevel == 3 )
		{
			pev->velocity *= 1.5f;
		}

		if( pev->waterlevel >= 2 )
		{
			// play scuba sound
			if( m_flSwimTime < gpGlobals->time )
			{
				m_flSwimTime = gpGlobals->time + 7.0f;
				EMIT_SOUND (ENT(pev), CHAN_BODY, "misc/wetsuit.wav", 1, ATTN_NORM);
			}
			else
			{
				if (fabs(m_flSwimTime - gpGlobals->time - 6.0f) < 0.04f)
				{
					DeathBubbles(1);
				}
				else if (fabs(m_flSwimTime - gpGlobals->time - 5.5f) < 0.04f)
				{
					DeathBubbles(1);
				}
				else if (fabs(m_flSwimTime - gpGlobals->time - 5.0f ) < 0.04f)
				{
					DeathBubbles(1);
				}
			}
		}
	}

#endif /* HIPNOTIC */
	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();

	if (pev->deadflag >= DEAD_DYING)
	{
		PlayerDeathThink();
		return;
	}

	if (pev->button & IN_JUMP)
	{
		Jump();
	}

	if ( !FBitSet ( pev->flags, FL_ONGROUND ) )
	{
		m_flFallVelocity = -pev->velocity.z;
	}
}

/*
================
CheckPowerups

Check for turning off powerups

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
================
*/
void CBasePlayer::CheckPowerups( void )
{
	if (pev->health <= 0)
		return;

	// invisibility
	if (m_flInvisibleFinished)
	{
		// sound and screen flash when items starts to run out
		if (m_flInvisibleSound < gpGlobals->time)
		{
			EMIT_SOUND(ENT(pev), CHAN_AUTO, "items/inv3.wav", 0.5, ATTN_IDLE);
			m_flInvisibleSound = gpGlobals->time + (RANDOM_FLOAT( 0, 3 ) + 1);
		}

		if (m_flInvisibleFinished < gpGlobals->time + 3)
		{
			if (m_flInvisibleTime == 1)
			{
				CLIENT_PRINTF( edict(), print_console, "Ring of Shadows magic is fading\n");
				EMIT_SOUND(ENT(pev), CHAN_AUTO, "items/inv2.wav", 1, ATTN_NORM);
				m_flInvisibleTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}
			
			if (m_flInvisibleTime < gpGlobals->time)
			{
				m_flInvisibleTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}
		}

		// use the eyes
		pev->modelindex = g_ulModelIndexEyes;
		pev->flags |= FL_NOTARGET;

		if (m_flInvisibleFinished < gpGlobals->time)
		{
			// just stopped
			pev->flags &= ~FL_NOTARGET;
			m_iItems &= ~IT_INVISIBILITY;
			m_flInvisibleFinished = 0;
			m_flInvisibleTime = 0;
		}
	}
	else
		pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes

	// invincibility
	if (m_flInvincibleFinished)
	{
		// sound and screen flash when items starts to run out
		if (m_flInvincibleFinished < gpGlobals->time + 3)
		{
			if (m_flInvincibleTime == 1)
			{
				CLIENT_PRINTF( edict(), print_console, "Protection is almost burned out\n");
				EMIT_SOUND( edict(), CHAN_AUTO, "items/protect2.wav", 1, ATTN_NORM);
				m_flInvincibleTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}
			
			if (m_flInvincibleTime < gpGlobals->time)
			{
				m_flInvincibleTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}
		}
		
		if (m_flInvincibleFinished < gpGlobals->time)
		{	
			// just stopped
			m_iItems &= ~IT_INVULNERABILITY;
			m_flInvincibleTime = 0;
			m_flInvincibleFinished = 0;
		}

		if (m_flInvincibleFinished > gpGlobals->time)
		{
			if( m_iItems & IT_QUAD )
			{
				pev->renderfx = kRenderFxGlowShell;
				pev->rendercolor = Vector( 255, 125, 255 );	// RGB
				pev->renderamt = 100;	// Shell size
			}
			else
			{
				pev->renderfx = kRenderFxGlowShell;
				pev->rendercolor = Vector( 255, 128, 0 );	// RGB
				pev->renderamt = 100;	// Shell size
			}
			pev->effects |= EF_BRIGHTLIGHT;
		}
		else
		{
			pev->renderfx = kRenderFxNone;
			pev->rendermode = kRenderNormal;
			pev->renderamt = 255;
			pev->effects &= ~EF_BRIGHTLIGHT;
		}
	}

	// super damage
	if (m_flSuperDamageFinished)
	{
		// sound and screen flash when items starts to run out
		if (m_flSuperDamageFinished < gpGlobals->time + 3)
		{
			if (m_flSuperDamageTime == 1)
			{
				CLIENT_PRINTF( edict(), print_console, "Quad Damage is wearing off\n");
				EMIT_SOUND( edict(), CHAN_AUTO, "items/damage2.wav", 1, ATTN_NORM);
				m_flSuperDamageTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}	  
			
			if (m_flSuperDamageTime < gpGlobals->time)
			{
				m_flSuperDamageTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}
		}

		if (m_flSuperDamageFinished < gpGlobals->time)
		{	
			// just stopped
			m_iItems &= ~IT_QUAD;
			m_flSuperDamageFinished = 0;
			m_flSuperDamageTime = 0;
		}

		if (m_flSuperDamageFinished > gpGlobals->time)
		{
			if(!( m_iItems & IT_INVULNERABILITY ))
			{
				pev->renderfx = kRenderFxGlowShell;
				pev->rendercolor = Vector( 0, 127, 255 );	// RGB
				pev->renderamt = 100;	// Shell size
			}
			pev->effects |= EF_BRIGHTLIGHT;
		}
		else
		{
			pev->renderfx = kRenderFxNone;
			pev->rendermode = kRenderNormal;
			pev->renderamt = 255;
			pev->effects &= ~EF_BRIGHTLIGHT;
		}
	}	

	// suit	
	if (m_flRadSuitFinished)
	{
		pev->air_finished = gpGlobals->time + 12;		// don't drown

		// sound and screen flash when items starts to run out
		if (m_flRadSuitFinished < gpGlobals->time + 3)
		{
			if (m_flRadSuitTime == 1)
			{
				CLIENT_PRINTF( edict(), print_console, "Air supply in Biosuit expiring\n");
				EMIT_SOUND(ENT(pev), CHAN_AUTO, "items/suit2.wav", 1, ATTN_NORM);
				m_flRadSuitTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}
			
			if (m_flRadSuitTime < gpGlobals->time)
			{
				m_flRadSuitTime = gpGlobals->time + 1;
	         			BONUS_FLASH( edict() );
			}
		}

		if (m_flRadSuitFinished < gpGlobals->time)
		{	
			// just stopped
			m_iItems &= ~IT_SUIT;
			m_flRadSuitTime = 0;
			m_flRadSuitFinished = 0;
		}
	}	
#ifdef HIPNOTIC

	// wetsuit	
	if (m_flWetSuitFinished)
	{
		pev->air_finished = gpGlobals->time + 12;		// don't drown

		// sound and screen flash when items starts to run out
		if (m_flWetSuitFinished < gpGlobals->time + 3)
		{
			if (m_flWetSuitTime == 1)
			{
				CLIENT_PRINTF( edict(), print_console, "Air supply in Wetsuit is running out\n");
				EMIT_SOUND(ENT(pev), CHAN_AUTO, "items/suit2.wav", 1, ATTN_NORM);
				m_flWetSuitTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}
			
			if (m_flWetSuitTime < gpGlobals->time)
			{
				m_flWetSuitTime = gpGlobals->time + 1;
	         			BONUS_FLASH( edict() );
			}
		}

		if (m_flWetSuitFinished < gpGlobals->time)
		{	
			// just stopped
			m_iItems &= ~IT_WETSUIT;
			m_flWetSuitTime = 0;
			m_flWetSuitFinished = 0;
		}
	}

	// Empathy Shields	
	if (m_flEmpathyShieldFinished)
	{
		// sound and screen flash when items starts to run out
		if (m_flEmpathyShieldFinished < gpGlobals->time + 3)
		{
			if (m_flEmpathyShieldTime == 1)
			{
				CLIENT_PRINTF( edict(), print_console, "Empathy Shields are running out\n");
				EMIT_SOUND(ENT(pev), CHAN_AUTO, "items/suit2.wav", 1, ATTN_NORM);
				m_flEmpathyShieldTime = gpGlobals->time + 1;
				BONUS_FLASH( edict() );
			}
			
			if (m_flEmpathyShieldTime < gpGlobals->time)
			{
				m_flEmpathyShieldTime = gpGlobals->time + 1;
	         			BONUS_FLASH( edict() );
			}
		}

		if (m_flEmpathyShieldFinished < gpGlobals->time)
		{	
			// just stopped
			m_iItems &= ~IT_EMPATHY_SHIELDS;
			m_flEmpathyShieldTime = 0;
			m_flEmpathyShieldFinished = 0;
			pev->effects &= ~EF_BRIGHTLIGHT;
		}
		else
			pev->effects |= EF_BRIGHTLIGHT;
	}
#endif /* HIPNOTIC */
}

void CBasePlayer::PostThink()
{
	if ( g_fGameOver )
		return; // intermission or finale

	if (!IsAlive()) return;

	// do weapon stuff
	ItemPostFrame( );

	// check to see if player landed hard enough to make a sound
	// falling farther than half of the maximum safe distance, but not as far a max safe distance will
	// play a bootscrape sound, and no damage will be inflicted. Fallling a distance shorter than half
	// of maximum safe distance will make no sound. Falling farther than max safe distance will play a 
	// fallpain sound, and damage will be inflicted based on how far the player fell
	if ( (FBitSet(pev->flags, FL_ONGROUND)) && (pev->health > 0) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
	{
		// ALERT ( at_console, "%f\n", m_flFallVelocity );

		if (pev->watertype == CONTENT_WATER)
		{
			EMIT_SOUND(ENT(pev), CHAN_BODY, "player/h2ojump.wav", 1, ATTN_NORM);
		}
		else if ( m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
		{
			// after this point, we start doing damage
			float flFallDamage = g_pGameRules->FlPlayerFallDamage( this );

			EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/land2.wav", 1, ATTN_NORM);

			if ( flFallDamage > 0 )
			{
				TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), flFallDamage, DMG_FALL ); 
				pev->punchangle.x = 0;
			}
		}

		if ( IsAlive() )
		{
			SetAnimation( PLAYER_WALK );
		}
	}

#ifdef HIPNOTIC
	// earthquake
	if( m_flEarthQuakeTime > gpGlobals->time )
	{
		if (!m_fEarthQuake)
		{
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "misc/quake.wav", 1, ATTN_NONE);
			m_fEarthQuake = TRUE;
		}
		if (FBitSet(pev->flags, FL_ONGROUND))
		{
			pev->velocity += UTIL_RandomBloodVector() * 50.0f;
			pev->velocity.z += RANDOM_FLOAT( 0.0f, 150.0f );
		}
	}
	else
	{
		if (m_fEarthQuake)
		{
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "misc/quakeend.wav", 1, ATTN_NONE);
			m_fEarthQuake = FALSE;
		}
	}

	if( m_flWetSuitFinished > gpGlobals->time )
	{
		if( pev->waterlevel == 2 )
		{
			pev->velocity *= 0.8f;
		}
		if( pev->waterlevel == 3 )
		{
			pev->velocity *= 0.66f;
		}
	}
 
#endif /* HIPNOTIC */
	if (FBitSet(pev->flags, FL_ONGROUND))
	{		
		m_flFallVelocity = 0;
	}

	// select the proper animation for the player character	
	if ( IsAlive() )
	{
		if (!pev->velocity.x && !pev->velocity.y)
			SetAnimation( PLAYER_IDLE );
		else if ((pev->velocity.x || pev->velocity.y) && (FBitSet(pev->flags, FL_ONGROUND)))
			SetAnimation( PLAYER_WALK );
		else if (pev->waterlevel > 1)
			SetAnimation( PLAYER_WALK );
	}

	StudioFrameAdvance( );
	CheckPowerups();

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = pev->button;
}


// checks if the spot is clear of players
BOOL IsSpawnPointValid( CBaseEntity *pPlayer, CBaseEntity *pSpot )
{
	CBaseEntity *ent = NULL;

	if ( !pSpot->IsTriggered( pPlayer ) )
	{
		return FALSE;
	}

	while ( (ent = UTIL_FindEntityInSphere( ent, pSpot->pev->origin, 128 )) != NULL )
	{
		// if ent is a client, don't spawn on 'em
		if ( ent->IsPlayer() && ent != pPlayer )
			return FALSE;
	}

	return TRUE;
}


DLL_GLOBAL CBaseEntity	*g_pLastSpawn;

/*
============
EntSelectSpawnPoint

Returns the entity to spawn at

USES AND SETS GLOBAL g_pLastSpawn
============
*/
edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer )
{
	CBaseEntity *pSpot;
	edict_t		*player;

	player = pPlayer->edict();

	pSpot = UTIL_FindEntityByClassname( NULL, "testplayerstart");
	if ( !FNullEnt(pSpot) )
		goto ReturnSpot;

// choose a info_player_deathmatch point
	if (g_pGameRules->IsCoOp())
	{
		pSpot = UTIL_FindEntityByClassname( g_pLastSpawn, "info_player_coop");
		if ( !FNullEnt(pSpot) )
			goto ReturnSpot;
		pSpot = UTIL_FindEntityByClassname( g_pLastSpawn, "info_player_start");
		if ( !FNullEnt(pSpot) ) 
			goto ReturnSpot;
	}
	else if ( g_pGameRules->IsDeathmatch() )
	{
		pSpot = g_pLastSpawn;
		// Randomize the start spot
		for ( int i = RANDOM_LONG(1,5); i > 0; i-- )
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
		if ( FNullEnt( pSpot ) )  // skip over the null point
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );

		CBaseEntity *pFirstSpot = pSpot;

		do 
		{
			if ( pSpot )
			{
				// check if pSpot is valid
				if ( IsSpawnPointValid( pPlayer, pSpot ) )
				{
					if ( pSpot->pev->origin == Vector( 0, 0, 0 ) )
					{
						pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
						continue;
					}

					// if so, go to pSpot
					goto ReturnSpot;
				}
			}
			// increment pSpot
			pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
		} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

		// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
		if ( !FNullEnt( pSpot ) )
		{
			CBaseEntity *ent = NULL;
			while ( (ent = UTIL_FindEntityInSphere( ent, pSpot->pev->origin, 128 )) != NULL )
			{
				// if ent is a client, kill em (unless they are ourselves)
				if ( ent->IsPlayer() && !(ent->edict() == player) )
					ent->TakeDamage( VARS(INDEXENT(0)), VARS(INDEXENT(0)), 300, DMG_GENERIC );
			}
			goto ReturnSpot;
		}
	}

	if (gpWorld->serverflags)
	{	
		// return with a rune to start
		pSpot = UTIL_FindEntityByClassname(NULL, "info_player_start2");
		if ( !FNullEnt(pSpot) )
			goto ReturnSpot;
	}

	pSpot = UTIL_FindEntityByClassname(NULL, "info_player_start");
	if ( !FNullEnt(pSpot) )
		goto ReturnSpot;

ReturnSpot:
	if ( FNullEnt( pSpot ) )
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}

	g_pLastSpawn = pSpot;
	return pSpot->edict();
}

void CBasePlayer::Spawn( void )
{
	pev->classname		= MAKE_STRING("player");
	pev->health		= 100;
	pev->armorvalue		= 0;
	pev->takedamage		= DAMAGE_AIM;
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_WALK;
	pev->max_health		= pev->health;
	pev->flags		   &= FL_PROXY;	// keep proxy flag sey by engine
	pev->flags		   |= FL_CLIENT;
	pev->air_finished		= gpGlobals->time + 12;
	pev->dmg			= 2;				// initial water damage
	pev->effects		= 0;
	pev->deadflag		= DEAD_NO;
	pev->dmg_take		= 0;
	pev->dmg_save		= 0;
	pev->friction		= 1.0;
	pev->gravity		= 1.0;
	m_bitsHUDDamage		= -1;
	m_afPhysicsFlags		= 0;

	pev->fov = m_iFOV		= 0;// init field of view.
	m_iClientFOV		= -1; // make sure fov reset is sent

	m_flCheckHealthTime = 0;	

	m_flNextAttack = UTIL_WeaponTimeBase();

	// dont let uninitialized value here hurt the player
	m_flFallVelocity = 0;

	g_pGameRules->SetDefaultPlayerTeam( this );
	g_pGameRules->GetPlayerSpawnSpot( this );

	SET_MODEL(ENT(pev), "models/player.mdl");
	pev->sequence = LookupActivity( ACT_IDLE );

	UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

	pev->view_ofs = VEC_VIEW;
	Precache();

	m_fNoPlayerSound = FALSE;// normal sound behavior.

	m_fInitHUD = TRUE;
	m_iClientHideHUD = -1;
	m_iClientArmor = -1;

	m_flNextChatTime = gpGlobals->time;

	if (g_pGameRules->IsMultiplayer())
	{
		UTIL_MakeVectors(pev->angles);
		CTeleFog::CreateFog( pev->origin + gpGlobals->v_forward*20);
	}

	CTeleFrag::CreateTDeath( pev->origin, this );
	g_pGameRules->PlayerSpawn( this );
}

void CBasePlayer :: Precache( void )
{
	g_ulModelIndexPlayer = PRECACHE_MODEL( "models/player.mdl" );

	g_ulModelIndexEyes = PRECACHE_MODEL( "models/eyes.mdl" );

	// in the event that the player JUST spawned, and the level node graph
	// was loaded, fix all of the node graph pointers before the game starts.
	
	// SOUNDS / MODELS ARE PRECACHED in ClientPrecache() (game specific)
	// because they need to precache before any clients have connected

	// init geiger counter vars during spawn and each time
	// we cross a level transition
	m_bitsHUDDamage = -1;

	m_iClientArmor = -1;
	m_iClientItems = -1;
	m_iClientWeapon = -1;
	m_iClientCurrentAmmo = -1;
	m_iClientAmmoShells = -1;
	m_iClientAmmoNails = -1;
	m_iClientAmmoRockets = -1;
	m_iClientAmmoCells = -1;

	// Make sure any necessary user messages have been registered
	LinkUserMessages();

	m_iUpdateTime = 5;  // won't update for 1/2 a second

	if ( gInitHUD )
		m_fInitHUD = TRUE;
}

int CBasePlayer::Save( CSave &save )
{
	if ( !CQuakeMonster::Save(save) )
		return 0;

	return save.WriteFields( "PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData) );
}

int CBasePlayer::Restore( CRestore &restore )
{
	if ( !CQuakeMonster::Restore(restore) )
		return 0;

	int status = restore.ReadFields( "PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData) );

	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;

	pev->v_angle.z = 0;	// Clear out roll
	pev->angles = pev->v_angle;

	pev->fixangle = TRUE;           // turn this way immediately

	UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

	return status;
}

//==============================================
// HasWeapons - do I have any weapons at all?
//==============================================
BOOL CBasePlayer::HasWeapons( void )
{
	return m_iItems != 0;
}

const char *CBasePlayer::TeamID( void )
{
	if ( pev == NULL )		// Not fully connected yet
		return "";

	// return their team name
	return m_szTeamName;
}

//==============================================
CBaseEntity *FindEntityForward( CBaseEntity *pMe )
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		return pHit;
	}
	return NULL;
}

/*
===============
ForceClientDllUpdate

When recording a demo, we need to have the server tell us the entire client state
so that the client side .dll can behave correctly.
Reset stuff so that the state is transmitted.
===============
*/
void CBasePlayer :: ForceClientDllUpdate( void )
{
	m_iClientHealth  = -1;
	m_iClientArmor = -1;
	m_iClientItems = -1;
	m_iClientWeapon = -1;
	m_iClientCurrentAmmo = -1;
	m_iClientAmmoShells = -1;
	m_iClientAmmoNails = -1;
	m_iClientAmmoRockets = -1;
	m_iClientAmmoCells = -1;

	m_fInitHUD = TRUE;		// Force HUD gmsgResetHUD message

	// Now force all the necessary messages
	//  to be sent.
	UpdateClientData();
}

void CBasePlayer :: SetChangeParms( void )
{
	// remove items
#ifndef HIPNOTIC
	int remove_items = (IT_KEY1|IT_KEY2|IT_INVISIBILITY|IT_INVULNERABILITY|IT_SUIT|IT_QUAD); 
#else /* HIPNOTIC */
	int remove_items = (IT_KEY1|IT_KEY2|IT_INVISIBILITY|IT_INVULNERABILITY|IT_SUIT|IT_QUAD|IT_WETSUIT|IT_EMPATHY_SHIELDS); 
#endif /* HIPNOTIC */
	m_iItems &= ~remove_items;
	
	// cap super health
	if (pev->health > 100)
		pev->health = 100;

	if (pev->health < 50)
		pev->health = 50;

#ifdef HIPNOTIC
	pev->gravity = 1.0f;

#endif /* HIPNOTIC */
	g_levelParams[PARM_ITEMS] = m_iItems;
	g_levelParams[PARM_HEALTH] = pev->health;
	g_levelParams[PARM_ARMORVALUE] = pev->armorvalue;

	if (ammo_shells < 25)
		g_levelParams[PARM_SHELLS] = 25;
	else g_levelParams[PARM_SHELLS] = ammo_shells;

	g_levelParams[PARM_NAILS] = ammo_nails;
	g_levelParams[PARM_ROCKETS] = ammo_rockets;
	g_levelParams[PARM_CELLS] = ammo_cells;
	g_levelParams[PARM_CURWEAPON] = m_iWeapon;
	g_levelParams[PARM_ARMORTYPE] = pev->armortype * 100;
	g_levelParams[PARM_SERVERFLAGS] = gpWorld->serverflags;
	ALERT( at_aiconsole, "SetChangeParms: items %p\n", m_iItems );

	g_changelevel = TRUE;
}

void CBasePlayer :: SetNewParms( void )
{
	g_levelParams[PARM_ITEMS] = (IT_SHOTGUN|IT_AXE);
	g_levelParams[PARM_HEALTH] = 100;
	g_levelParams[PARM_ARMORVALUE] = 0;
	g_levelParams[PARM_SHELLS] = 25;
	g_levelParams[PARM_NAILS] = 0;
	g_levelParams[PARM_ROCKETS] = 0;
	g_levelParams[PARM_CELLS] = 0;
	g_levelParams[PARM_CURWEAPON] = 1;
	g_levelParams[PARM_ARMORTYPE] = 0;
}

void CBasePlayer :: DecodeLevelParms( void )
{
	if (!g_changelevel) return;

#ifndef HIPNOTIC
	if (gpWorld->serverflags)
	{
		if( FStrEq( STRING( gpWorld->pev->model ), "maps/start.bsp" ))
			SetNewParms (); // take away all stuff on starting new episode
	}
#else /* HIPNOTIC */
	if( FStrEq( STRING( gpWorld->pev->model ), "maps/start.bsp" ))
		SetNewParms (); // take away all stuff on starting new episode

	if( FStrEq( STRING( gpWorld->pev->model ), "maps/hip1m1" ))
		SetNewParms (); // take away all stuff on starting new episode

	if( FStrEq( STRING( gpWorld->pev->model ), "maps/hip2m1" ))
		SetNewParms (); // take away all stuff on starting new episode

	if( FStrEq( STRING( gpWorld->pev->model ), "maps/hip3m1" ))
		SetNewParms (); // take away all stuff on starting new episode
#endif /* HIPNOTIC */
	
	m_iItems = g_levelParams[PARM_ITEMS];
	pev->health = g_levelParams[PARM_HEALTH];
	pev->armorvalue = g_levelParams[PARM_ARMORVALUE];
	ammo_shells = g_levelParams[PARM_SHELLS];
	ammo_nails = g_levelParams[PARM_NAILS];
	ammo_rockets = g_levelParams[PARM_ROCKETS];
	ammo_cells = g_levelParams[PARM_CELLS];
	m_iWeapon = g_levelParams[PARM_CURWEAPON];
	pev->armortype = g_levelParams[PARM_ARMORTYPE] * 0.01;
	m_iItems |= (gpWorld->serverflags << 28); // add runes
	ALERT( at_aiconsole, "DecodeLevelParms: items %p\n", m_iItems );
		
	memset( g_levelParams, 0, sizeof (g_levelParams));

	g_changelevel = FALSE;
}

/*
============
ImpulseCommands
============
*/
void CBasePlayer::ImpulseCommands( )
{
	TraceResult	tr;// UNDONE: kill me! This is temporary for PreAlpha CDs
		
	int iImpulse = (int)pev->impulse;

	if (iImpulse >= 1 && iImpulse <= 8)
	{
		W_ChangeWeapon( iImpulse );
	}

#ifdef HIPNOTIC
	if (iImpulse >= 225 && iImpulse <= 226)
	{
		W_ChangeWeapon( iImpulse );
	}

#endif /* HIPNOTIC */
	switch (iImpulse)
	{
	case 10:
		W_CycleWeaponCommand();
		break;
	case 12:
		W_CycleWeaponReverseCommand();
		break;
	default:
		// check all of the cheat impulse commands now
		CheatImpulseCommands( iImpulse );
		break;
	}
	
	pev->impulse = 0;
}

//=========================================================
//=========================================================
void CBasePlayer::CheatImpulseCommands( int iImpulse )
{
	if ( g_flWeaponCheat == 0.0 )
	{
		return;
	}

	CBaseEntity *pEntity;
	TraceResult tr;

	switch ( iImpulse )
	{
	case 9:
		ammo_nails = 200;
		ammo_shells = 100;
		ammo_rockets = 100;
#ifndef HIPNOTIC
		ammo_cells = 200;
#else /* HIPNOTIC */
		ammo_cells = 100;
#endif /* HIPNOTIC */
		m_iItems |= IT_NAILGUN|IT_SUPER_NAILGUN|IT_SUPER_SHOTGUN|IT_ROCKET_LAUNCHER|IT_GRENADE_LAUNCHER|IT_LIGHTNING|IT_KEY1|IT_KEY2;
#ifdef HIPNOTIC
		m_iItems |= IT_LASER_CANNON|IT_MJOLNIR|IT_PROXIMITY_GUN;
#endif /* HIPNOTIC */
		m_iWeapon = IT_ROCKET_LAUNCHER;
		CheckAmmo();
		W_SetCurrentAmmo();
		break;
	case 11:
		gpWorld->serverflags = gpWorld->serverflags * 2 + 1;
		m_iItems |= (gpWorld->serverflags << 28); // store runes as high bits
		break;
	case 103:
		// What the hell are you doing?
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			CQuakeMonster *pMonster = pEntity->GetMonster();
			if ( pMonster )
				pMonster->ReportAIState();
		}
		break;
	case 106:
		// Give me the classname and targetname of this entity.
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			ALERT ( at_console, "Classname: %s", STRING( pEntity->pev->classname ) );
			
			if ( !FStringNull ( pEntity->pev->targetname ) )
			{
				ALERT ( at_console, " - Targetname: %s\n", STRING( pEntity->pev->targetname ) );
			}
			else
			{
				ALERT ( at_console, " - TargetName: No Targetname\n" );
			}

			ALERT ( at_console, "Model: %s\n", STRING( pEntity->pev->model ) );
			if ( pEntity->pev->globalname )
				ALERT ( at_console, "Globalname: %s\n", STRING( pEntity->pev->globalname ) );
		}
		break;

	case 107:
		{
			TraceResult tr;

			edict_t		*pWorld = g_engfuncs.pfnPEntityOfEntIndex( 0 );

			Vector start = pev->origin + pev->view_ofs;
			Vector end = start + gpGlobals->v_forward * 1024;
			UTIL_TraceLine( start, end, ignore_monsters, edict(), &tr );
			if ( tr.pHit )
				pWorld = tr.pHit;
			const char *pTextureName = TRACE_TEXTURE( pWorld, start, end );
			if ( pTextureName )
				ALERT( at_console, "Texture: %s\n", pTextureName );
		}
#ifdef HIPNOTIC
		break;
	case 200:
		m_iItems |= IT_WETSUIT;
		m_flWetSuitTime = 1;
		m_flWetSuitFinished = gpGlobals->time + 30;
		break;
	case 201:
		m_iItems |= IT_EMPATHY_SHIELDS;
		m_flEmpathyShieldTime = 1;
		m_flEmpathyShieldFinished = gpGlobals->time + 30;
#endif /* HIPNOTIC */
		break;
	case 255:
		m_iItems |= IT_QUAD;
		m_flSuperDamageTime = 1;
		m_flSuperDamageFinished = gpGlobals->time + 30;
		break;
	}
}

void CBasePlayer :: SendWeaponAnim( int iAnim )
{
	pev->weaponanim = iAnim;

	MESSAGE_BEGIN( MSG_ONE, SVC_WEAPONANIM, NULL, pev );
		WRITE_BYTE( iAnim );	// sequence number
		WRITE_BYTE( 1 );		// weaponmodel bodygroup.
	MESSAGE_END();
} 

/*
============
ItemPreFrame

Called every frame by the player PreThink
============
*/
void CBasePlayer::ItemPreFrame()
{
	if ( gpGlobals->time < m_flNextAttack )
		return;
}


/*
============
ItemPostFrame

Called every frame by the player PostThink
============
*/
void CBasePlayer::ItemPostFrame()
{
	if( gpGlobals->time < m_flNextAttack )
		return;

	ImpulseCommands();

	if( pev->button & IN_ATTACK )
	{
		SuperDamageSound();
		W_Attack();

		m_bPlayedIdleAnim = FALSE;
	}
	else
	{
		// no fire buttons down
		if ( !m_bPlayedIdleAnim )
		{
			m_bPlayedIdleAnim = TRUE;
			SendWeaponAnim( 0 );
		}
	}
}

/*
=========================================================
	UpdateClientData

resends any changed player HUD info to the client.
Called every frame by PlayerPreThink
Also called at start of demo recording and playback by
ForceClientDllUpdate to ensure the demo gets messages
reflecting all of the HUD state info.
=========================================================
*/
void CBasePlayer :: UpdateClientData( void )
{
	if (m_fInitHUD)
	{
		m_fInitHUD = FALSE;
		gInitHUD = FALSE;

		MESSAGE_BEGIN( MSG_ONE, gmsgResetHUD, NULL, pev );
			WRITE_BYTE( 0 );
		MESSAGE_END();

		// send levelname to client
		MESSAGE_BEGIN( MSG_ONE, gmsgLevelName, NULL, pev );
			WRITE_STRING( STRING( gpWorld->levelname ));
		MESSAGE_END();

		// send total secrets count
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_TOTALSECRETS );
			WRITE_SHORT( gpWorld->total_secrets );
		MESSAGE_END();

		// send found secrets count (save\restore)
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_SECRETS );
			WRITE_SHORT( gpWorld->found_secrets );
		MESSAGE_END();

		// send total monsters count
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_TOTALMONSTERS );
			WRITE_SHORT( gpWorld->total_monsters );
		MESSAGE_END();

		// send killed monsters count (save\restore)
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_MONSTERS );
			WRITE_SHORT( gpWorld->killed_monsters );
		MESSAGE_END();

		if ( !m_fGameHUDInitialized )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgInitHUD, NULL, pev );
			MESSAGE_END();

			g_pGameRules->InitHUD( this );
			m_fGameHUDInitialized = TRUE;
		}

		// re-initalize ammo
		W_SetCurrentAmmo ();
	}

	if ( m_iHideHUD != m_iClientHideHUD )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgHideHUD, NULL, pev );
			WRITE_BYTE( m_iHideHUD );
		MESSAGE_END();

		m_iClientHideHUD = m_iHideHUD;
	}

	if ( m_iFOV != m_iClientFOV )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
			WRITE_BYTE( m_iFOV );
		MESSAGE_END();

		// cache FOV change at end of function, so weapon updates can see that FOV has changed
	}

	if (pev->health != m_iClientHealth)
	{
		int iHealth = max( pev->health, 0 );  // make sure that no negative health values are sent

		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_HEALTH );
			WRITE_SHORT( iHealth );
		MESSAGE_END();

		m_iClientHealth = pev->health;
	}

	if (m_iItems != m_iClientItems)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgItems, NULL, pev );
			WRITE_LONG( m_iItems );
		MESSAGE_END();
		m_iClientItems = m_iItems;
	}

	if (m_iWeapon != m_iClientWeapon)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_ACTIVEWEAPON );
			WRITE_SHORT( m_iWeapon );
		MESSAGE_END();
		m_iClientWeapon = m_iWeapon;
	}

	int iCurrentAmmo = 0;

	if (m_pCurrentAmmo) 
		iCurrentAmmo = *m_pCurrentAmmo;

	if (iCurrentAmmo != m_iClientCurrentAmmo)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_AMMO );
			WRITE_SHORT( iCurrentAmmo );
		MESSAGE_END();
		m_iClientCurrentAmmo = iCurrentAmmo;
	}

	if (ammo_shells != m_iClientAmmoShells)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_SHELLS );
			WRITE_SHORT( ammo_shells );
		MESSAGE_END();
		m_iClientAmmoShells = ammo_shells;
	}

	if (ammo_nails != m_iClientAmmoNails)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_NAILS );
			WRITE_SHORT( ammo_nails );
		MESSAGE_END();
		m_iClientAmmoNails = ammo_nails;
	}

	if (ammo_rockets != m_iClientAmmoRockets)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_ROCKETS );
			WRITE_SHORT( ammo_rockets );
		MESSAGE_END();
		m_iClientAmmoRockets = ammo_rockets;
	}

	if (ammo_cells != m_iClientAmmoCells)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_CELLS );
			WRITE_SHORT( ammo_cells );
		MESSAGE_END();
		m_iClientAmmoCells = ammo_cells;
	}

	if (pev->armorvalue != m_iClientArmor)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStats, NULL, pev );
			WRITE_BYTE( STAT_ARMOR );
			WRITE_SHORT( (int)pev->armorvalue );
		MESSAGE_END();
		m_iClientArmor = pev->armorvalue;
	}

	if (pev->dmg_take || pev->dmg_save)
	{
		// Comes from inside me if not set
		Vector damageOrigin = pev->origin;
		// send "damage" message
		// causes screen to flash, and pain compass to show direction of damage
		edict_t *other = pev->dmg_inflictor;
		if ( other )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(other);
			if ( pEntity )
				damageOrigin = pEntity->Center();
		}

		MESSAGE_BEGIN( MSG_ONE, gmsgDamage, NULL, pev );
			WRITE_BYTE( pev->dmg_save );
			WRITE_BYTE( pev->dmg_take );
			WRITE_COORD( damageOrigin.x );
			WRITE_COORD( damageOrigin.y );
			WRITE_COORD( damageOrigin.z );
		MESSAGE_END();
	
		pev->dmg_take = 0;
		pev->dmg_save = 0;
	}

	// Cache and client weapon change
	m_iClientFOV = m_iFOV;
}

void CBasePlayer :: EnableControl(BOOL fControl)
{
	if (!fControl)
		pev->flags |= FL_FROZEN;
	else
		pev->flags &= ~FL_FROZEN;
}

//=========================================================
// Multiplayer intermission spots.
//=========================================================
class CInfoIntermission : public CPointEntity
{
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
};

// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
void CInfoIntermission :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "mangle"))	// a quake alias
	{
		UTIL_StringToVector( (float *)pev->angles, pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CInfoIntermission::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->v_angle = g_vecZero;
	UTIL_SetOrigin( pev, pev->origin );
}

LINK_ENTITY_TO_CLASS( info_intermission, CInfoIntermission );
