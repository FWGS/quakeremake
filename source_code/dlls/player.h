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
#ifndef PLAYER_H
#define PLAYER_H


#include "pm_materials.h"


#define PLAYER_MAX_SAFE_FALL_SPEED		580.0	// approx 20 feet
#define PLAYER_FALL_PUNCH_THRESHHOLD		350.0	// won't punch player's screen/make scrape noise unless player falling at least this fast.

//
// Player PHYSICS FLAGS bits
//
#define PFLAG_OBSERVER			( 1<<0 )	// player is locked in stationary cam mode. Spectators can move, observers can't.

#define TEAM_NAME_LENGTH			16
#define CHAT_INTERVAL			1.0f

typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
} PLAYER_ANIM;

#include "monster.h"

class CBasePlayer : public CQuakeMonster
{
public:
	int		m_afButtonLast;
	int		m_afButtonPressed;
	int		m_afButtonReleased;


	float		m_flFallVelocity;
	
	unsigned int	m_afPhysicsFlags;	// physics flags - set when 'normal' physics should be revisited or overriden
	float		m_fNextSuicideTime; // the time after which the player can next use the suicide command

	float		m_flCheckHealthTime;	// used when  mega_health receive too many health for player!
	int		m_bitsHUDDamage;		// Damage bits for the current fame. These get sent to 
						// the hude via the DAMAGE message
	BOOL		m_fInitHUD;		// True when deferred HUD restart msg needs to be sent
	BOOL		m_fGameHUDInitialized;

	float		m_fDeadTime;	// the time at which the player died  (used in PlayerDeathThink())

	BOOL		m_fNoPlayerSound;	// a debugging feature. Player makes no sound if this is true. 

	int		m_iUpdateTime;	// stores the number of frame ticks before sending HUD update messages
	int		m_iClientHealth;	// the health currently known by the client.  If this changes, send a new
	int		m_iClientArmor;	// the Battery currently known by the client.  If this changes, send a new
	int		m_iHideHUD;	// the players hud weapon info is to be hidden
	int		m_iClientHideHUD;
	int		m_iFOV;		// field of view
	int		m_iClientFOV;	// client's known FOV

	int		m_iDeaths;
	float		m_iRespawnFrames;	// used in PlayerDeathThink() to make sure players can always respawn

	float		m_flFlySound;	// for "trigger_push"
	float		m_flSwimTime;

	float		m_flInvincibleTime;
	float		m_flInvincibleFinished;
	float		m_flInvincibleSound;

	float		m_flInvisibleTime;
	float		m_flInvisibleFinished;
	float		m_flInvisibleSound;

	float		m_flSuperDamageTime;
	float		m_flSuperDamageFinished;
	float		m_flSuperDamageSound;

	float		m_flRadSuitTime;
	float		m_flRadSuitFinished;

	char		m_szTeamName[TEAM_NAME_LENGTH];

	virtual void Spawn( void );
	void MonsterPain( CBaseEntity *pAttacker, float flDamage );
	void PainSound( void );

//	virtual void Think( void );
	virtual void Jump( void );
	virtual void PreThink( void );
	virtual void PostThink( void );
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual BOOL IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL IsPlayer( void ) { return TRUE; }
	virtual BOOL IsNetClient( void ) { return TRUE; }		// Bots should return FALSE for this, they can't receive NET messages
							// Spectators should return TRUE for this
	virtual const char *TeamID( void );

	int BloodColor( void ) { return BLOOD_COLOR_RED; }

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	void PackDeadPlayerItems( void );
	void RemoveAllItems( void );
	BOOL SwitchWeapon( CBasePlayerItem *pWeapon );

	// JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	virtual void UpdateClientData( void );
	
	static TYPEDESCRIPTION m_playerSaveData[];

	virtual void Precache( void );

	void DeathSound ( void );
	void DeathBubbles( float flCount );

	void SetAnimation( PLAYER_ANIM playerAnim );
	void SetWeaponAnimType( const char *szExtention );
	char m_szAnimExtention[32];

	// custom player functions
	virtual void ImpulseCommands( void );
	void CheatImpulseCommands( int iImpulse );

	void StartDeathCam( void );
	void StartObserver( Vector vecPosition, Vector vecViewAngle );

	void AddPoints( int score, BOOL bAllowNegativeScore );
	void AddPointsToTeam( int score, BOOL bAllowNegativeScore );
	void ItemPreFrame( void );
	void ItemPostFrame( void );
	void EnableControl(BOOL fControl);

	void WaterMove( void );
	void EXPORT PlayerDeathThink( void );

	void CheckPowerups( void );
	void ForceClientDllUpdate( void );  // Forces all client .dll specific data to be resent to client.

	void DeathMessage( entvars_t *pevKiller );

	// changelevel
	void	SetNewParms( void );
	void	SetChangeParms( void );
	void	DecodeLevelParms( void );

	// Weapon selection
	int	W_BestWeapon( void );
	void	W_SetCurrentAmmo( int sendanim = 1 );
	BOOL	W_CheckNoAmmo( void );
	void	W_ChangeWeapon( int iWeaponNumber );
	void	W_CycleWeaponCommand( void );
	void	W_CycleWeaponReverseCommand( void );
	BOOL	HasWeapons( void );

	// Weapon functionality
	static void FireBullets( entvars_t *pev, int iShots, Vector vecDir, Vector vecSpread );
	static void LightningDamage( Vector p1, Vector p2, CBaseEntity *pAttacker, float flDamage, Vector vecDir );

	void	SuperDamageSound( void );
	void	SendWeaponAnim( int iAnim );

	// Weapons
	void	W_Attack( void );
	void	W_FireAxe( void );
	void	W_FireShotgun( void );
	void	W_FireSuperShotgun( void );
	void	W_FireRocket( void );
	void	W_FireLightning( void );
	void	W_FireGrenade( void );
	void	W_FireSuperSpikes( void );
	void	W_FireSpikes( void );

	// Ammunition
	void	CheckAmmo( void );
	int	*m_pCurrentAmmo;	// Always points to one of the four ammo counts below

	// Backpacks
	void	DropBackpack( void );

	// Weapons
	int	m_iClientWeapon;	// The last status of the m_iWeapon sent to the client.
	int	m_iClientItems;	// The last status of the m_iItems sent to the client.
	int	m_iClientCurrentAmmo;
	int	m_iWeaponSwitch;
	int	m_iBackpackSwitch;
	int	m_iAutoWepSwitch;

	int	m_iClientAmmoShells;
	int	m_iClientAmmoNails;
	int	m_iClientAmmoRockets;
	int	m_iClientAmmoCells;

	// Weapon Data
	float	m_flAxeFire;
	float	m_flLightningSoundTime;
	float	m_flLightningTime;
	int	m_iNailOffset;
	float	m_flNextChatTime;
	float	m_flNextAttack;	
	BOOL	m_bPlayedIdleAnim;
};

extern int	gmsgHudText;
extern int	gmsgHideHUD;
extern int	gmsgTempEntity;	// quake missing sfx
extern int	gmsgFoundSecret;
extern int	gmsgKilledMonster;

extern BOOL gInitHUD;

#endif // PLAYER_H
