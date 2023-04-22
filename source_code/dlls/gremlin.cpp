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

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define GREM_IDLE_SOUND	1
#define GREM_CHECK_GROUND	2
#define GREM_WAIT_FOR_REST	3
#define GREM_DROP_BACKPACK	4
#define GREM_FLIP_TOUCH	5
#define GREM_GORGE_RIGHT	6
#define GREM_GORGE_LEFT	7
#define GREM_SPAWN_DONE	8
#define GREM_CHECK_ENEMY	9
#define GREM_ATTACK_CENTER	10
#define GREM_ATTACK_RIGHT	11
#define GREM_MUZZLEFLASH	12

class CGremlin : public CQuakeMonster
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL MonsterHasMeleeAttack( void ) { return TRUE; }
	BOOL MonsterHasMissileAttack( void ) { return TRUE; }
	void MonsterMeleeAttack( void );
	void MonsterMissileAttack( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void Killed( entvars_t *pevAttacker, int iGib );

	void MonsterAttack( void );
	BOOL MonsterCheckAttack( void );
	void MonsterKilled( entvars_t *pevAttacker, int iGib );
	void MonsterPain( CBaseEntity *pAttacker, float flDamage );
	void MonsterDamage( entvars_t *pevTarg, entvars_t* pevAttacker, float flDamage );
	int BloodColor( void ) { return BLOOD_COLOR_RED; }
	CBaseEntity *MonsterFindVictim( void );
	BOOL MonsterAttemptWeaponSteal( void );
	BOOL MonsterFindTarget( void );
	BOOL CheckJump( void );

	void EXPORT JumpTouch( CBaseEntity *pOther );
	void EXPORT FlipTouch( CBaseEntity *pOther );
	void MonsterLeap( void );
	void MonsterMelee( float side );
	BOOL MonsterWeaponAttack( void );
	void MonsterDropBackpack( void );
	void MonsterGorge( float side );
	void MonsterThrowHead( entvars_t *pevHead );
	void MonsterSplit( void );
	BOOL CheckNoAmmo( void );

	void MonsterSight( void );
	void MonsterIdle( void );
	void MonsterWalk( void );	
	void MonsterRun( void );
	void MonsterCustom( void );

	void W_SetCurrentAmmo( void );
	void FireShotGun( void );
	void FireSuperShotGun( void );
	void FireNailGun( float ox );
	void FireLaserGun( float ox );
	void FireGrenade( void );
	void FireRocket( void );
	void FireProximityGrenade( void );
	void FireLightningGun( void );

	void AI_Walk( float flDist );
	void AI_Run( float flDist );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	static const char *pPainSounds[];

	int	m_iStoleWeapon;
	int	m_bTriggerSpawned;
	float	m_flWait;
};

LINK_ENTITY_TO_CLASS( monster_gremlin, CGremlin );

TYPEDESCRIPTION CGremlin :: m_SaveData[] = 
{
	DEFINE_FIELD( CGremlin, m_iStoleWeapon, FIELD_BOOLEAN ),
	DEFINE_FIELD( CGremlin, m_flWait, FIELD_TIME ),
	DEFINE_FIELD( CGremlin, m_bTriggerSpawned, FIELD_BOOLEAN ),
}; IMPLEMENT_SAVERESTORE( CGremlin, CQuakeMonster );

const char *CGremlin :: pPainSounds[] = 
{
	"grem/pain1.wav",
	"grem/pain2.wav",
	"grem/pain3.wav",
};

void CGremlin :: MonsterSight( void )
{
	if( !m_iStoleWeapon )
		EMIT_SOUND( edict(), CHAN_VOICE, "grem/sight1.wav", 1.0, ATTN_NORM );
}

void CGremlin :: MonsterPain( CBaseEntity *pAttacker, float flDamage )
{
	if( m_iAIState == STATE_REBORN )
		return;	// monster in 'spawn' mode

	if (RANDOM_FLOAT( 0.0f, 1.0f ) < 0.8f)
	{
		m_bGorging = FALSE;
		m_hEnemy = pAttacker;
		FoundTarget();
	}

	if( m_pfnTouch == JumpTouch )
		return;

	if( pev->pain_finished > gpGlobals->time )
		return;

	EMIT_SOUND_ARRAY_DYN( CHAN_VOICE, pPainSounds, ATTN_NORM ); 
	pev->pain_finished = gpGlobals->time + 1;

	m_iAIState = STATE_PAIN;

	if (m_iStoleWeapon)
		SetActivity( ACT_SMALL_FLINCH );
	else SetActivity( ACT_BIG_FLINCH );

	AI_Pain( 4 );
}

BOOL CGremlin :: MonsterAttemptWeaponSteal( void )
{
	Vector		delta;
	CBaseEntity	*pVictim;
	int		iBestWeapon;
	float		amount;

	if (m_iStoleWeapon)
	{
		ALERT( at_aiconsole, "your stupid gremlin trying to steal a weapon again!\n" );
		return FALSE;
	}

	if (!FBitSet(m_hEnemy->pev->flags, FL_CLIENT))
		return FALSE;

	delta = (m_hEnemy->pev->origin - pev->origin);

	if (delta.Length() > 100.0f)
		return FALSE;

	// 50% chance
	if (RANDOM_FLOAT( 0.0f, 1.0f ) < 0.1f)
		return FALSE;

	//
	// we are within range so lets go for it
	//
	pVictim = m_hEnemy;
	iBestWeapon = pVictim->m_iWeapon;

	if (iBestWeapon == IT_AXE || iBestWeapon == IT_SHOTGUN || iBestWeapon == IT_MJOLNIR)
		return FALSE;

	// take that weapon from the entity
	pVictim->m_iItems &= ~iBestWeapon;

	// give it to our gremlin
	m_iItems |= iBestWeapon;
	m_iWeapon = iBestWeapon;

	// switch the player weapon
	CBasePlayer *pPlayer = (CBasePlayer *)pVictim;
	pPlayer->m_iWeapon = pPlayer->W_BestWeapon();
	pPlayer->W_SetCurrentAmmo();

	// take some ammo while we are at it
	m_iItems &= ~(IT_SHELLS | IT_NAILS | IT_ROCKETS | IT_CELLS);

	if (iBestWeapon == IT_SUPER_SHOTGUN)
	{
		amount = pVictim->ammo_shells;
		if (amount > 20) amount = 20;
		pVictim->ammo_shells -= amount;
		ammo_shells += amount;
		m_iItems |= IT_SHELLS;
		m_pCurrentAmmo = &ammo_shells;
		CLIENT_PRINTF( pVictim->edict(), print_console, "Gremlin stole your Super Shotgun\n" );
	}
	else if (iBestWeapon == IT_NAILGUN)
	{
		amount = pVictim->ammo_nails;
		if (amount > 40) amount = 40;
		pVictim->ammo_nails -= amount;
		ammo_nails += amount;
		m_iItems |= IT_NAILS;
		m_pCurrentAmmo = &ammo_nails;
		CLIENT_PRINTF( pVictim->edict(), print_console, "Gremlin stole your Nailgun\n" );
	}
	else if (iBestWeapon == IT_SUPER_NAILGUN)
	{
		amount = pVictim->ammo_nails;
		if (amount > 40) amount = 40;
		pVictim->ammo_nails -= amount;
		ammo_nails += amount;
		m_iItems |= IT_NAILS;
		m_pCurrentAmmo = &ammo_nails;
		CLIENT_PRINTF( pVictim->edict(), print_console, "Gremlin stole your Super Nailgun\n" );
	}
	else if (iBestWeapon == IT_GRENADE_LAUNCHER)
	{
		amount = pVictim->ammo_rockets;
		if (amount > 5) amount = 5;
		pVictim->ammo_rockets -= amount;
		ammo_rockets += amount;
		m_iItems |= IT_ROCKETS;
		m_pCurrentAmmo = &ammo_rockets;
		CLIENT_PRINTF( pVictim->edict(), print_console, "Gremlin stole your Grenade Launcher\n" );
	}
	else if (iBestWeapon == IT_ROCKET_LAUNCHER)
	{
		amount = pVictim->ammo_rockets;
		if (amount > 5) amount = 5;
		pVictim->ammo_rockets -= amount;
		ammo_rockets += amount;
		m_iItems |= IT_ROCKETS;
		m_pCurrentAmmo = &ammo_rockets;
		CLIENT_PRINTF( pVictim->edict(), print_console, "Gremlin stole your Rocket Launcher\n" );
	}
	else if (iBestWeapon == IT_LIGHTNING)
	{
		amount = pVictim->ammo_cells;
		if (amount > 40) amount = 40;
		pVictim->ammo_cells -= amount;
		ammo_cells += amount;
		m_iItems |= IT_CELLS;
		m_pCurrentAmmo = &ammo_cells;
		CLIENT_PRINTF( pVictim->edict(), print_console, "Gremlin stole your Lightning Gun\n" );
	}
	else if (iBestWeapon == IT_LASER_CANNON)
	{
		amount = pVictim->ammo_cells;
		if (amount > 40) amount = 40;
		pVictim->ammo_cells -= amount;
		ammo_cells += amount;
		m_iItems |= IT_CELLS;
		m_pCurrentAmmo = &ammo_cells;
		CLIENT_PRINTF( pVictim->edict(), print_console, "Gremlin stole your Laser Cannon\n" );
	}
	else if (iBestWeapon == IT_PROXIMITY_GUN)
	{
		amount = pVictim->ammo_rockets;
		if (amount > 5) amount = 5;
		pVictim->ammo_rockets -= amount;
		ammo_rockets += amount;
		m_iItems |= IT_ROCKETS;
		m_pCurrentAmmo = &ammo_rockets;
		CLIENT_PRINTF( pVictim->edict(), print_console, "Gremlin stole your Proximity Gun\n" );
	}

	W_SetCurrentAmmo ();

	// tag the gremlin as having stolen a weapon
	m_iStoleWeapon = TRUE;
	m_flAttackFinished = gpGlobals->time;

	// don't fire the first shot at the person we stole the weapon from all the time
	if (RANDOM_FLOAT( 0.0f, 1.0f ) > 0.65f)
		pev->enemy = pVictim->edict();
	else pev->enemy = edict();

//	m_iAttackState = ATTACK_STRAIGHT;

	// find a recipient
	pVictim = MonsterFindVictim();

	if (pVictim)
	{
		m_hEnemy = pVictim;
		FoundTarget();
		m_flAttackFinished = gpGlobals->time;
		m_flSearchTime = gpGlobals->time + 1.0f;
	}

	return TRUE;
}

// Weapon setup after weapon switch
void CGremlin :: W_SetCurrentAmmo( void )
{
	m_iItems &= ~(IT_SHELLS | IT_NAILS | IT_ROCKETS | IT_CELLS);

	int iszWeaponModel = 0;

	// Find out what weapon the player's using
	if (m_iWeapon == IT_AXE)
	{
		m_pCurrentAmmo = NULL;
		iszWeaponModel = MAKE_STRING("models/p_crowbar.mdl");
	}
	else if (m_iWeapon == IT_SHOTGUN)
	{
		m_pCurrentAmmo = &ammo_shells;
		iszWeaponModel = MAKE_STRING("models/p_shot.mdl");
		m_iItems |= IT_SHELLS;
	}
	else if (m_iWeapon == IT_SUPER_SHOTGUN)
	{
		m_pCurrentAmmo = &ammo_shells;
		iszWeaponModel = MAKE_STRING("models/p_shot2.mdl");
		m_iItems |= IT_SHELLS;
	}
	else if (m_iWeapon == IT_NAILGUN)
	{
		m_pCurrentAmmo = &ammo_nails;
		iszWeaponModel = MAKE_STRING("models/p_nail.mdl");
		m_iItems |= IT_NAILS;
	}
	else if (m_iWeapon == IT_SUPER_NAILGUN)
	{
		m_pCurrentAmmo = &ammo_nails;
		iszWeaponModel = MAKE_STRING("models/p_nail2.mdl");
		m_iItems |= IT_NAILS;
	}
	else if (m_iWeapon == IT_GRENADE_LAUNCHER)
	{
		m_pCurrentAmmo = &ammo_rockets;
		m_iItems |= IT_ROCKETS;
		iszWeaponModel = MAKE_STRING("models/p_rock.mdl");
	}
	else if (m_iWeapon == IT_ROCKET_LAUNCHER)
	{
		m_pCurrentAmmo = &ammo_rockets;
		m_iItems |= IT_ROCKETS;
		iszWeaponModel = MAKE_STRING("models/p_rock2.mdl");
	}
	else if (m_iWeapon == IT_LIGHTNING)
	{
		m_pCurrentAmmo = &ammo_cells;
		iszWeaponModel = MAKE_STRING("models/p_light.mdl");
		m_iItems |= IT_CELLS;
	}
	else if (m_iWeapon == IT_LASER_CANNON)
	{
		m_pCurrentAmmo = &ammo_cells;
		iszWeaponModel = MAKE_STRING("models/p_laser.mdl");
		m_iItems |= IT_CELLS;
	}
	else if (m_iWeapon == IT_MJOLNIR)
	{
		m_pCurrentAmmo = &ammo_cells;
		iszWeaponModel = MAKE_STRING("models/p_hammer.mdl");
		m_iItems |= IT_CELLS;
	}
	else if (m_iWeapon == IT_PROXIMITY_GUN)
	{
		m_pCurrentAmmo = &ammo_rockets;
		iszWeaponModel = MAKE_STRING("models/p_prox.mdl");
		m_iItems |= IT_ROCKETS;
	}
	else
	{
		m_pCurrentAmmo = NULL;
	}

	pev->weaponmodel = iszWeaponModel;
}

/*
===========
GremlinFindTarget

gremlin is currently not attacking anything, so try to find a target

============
*/
BOOL CGremlin :: MonsterFindTarget( void )
{
	CBaseEntity	*pHead, *pGorge;
	float		flDist, flResult;
	BOOL		bResult;

	if ((!m_iStoleWeapon) && gpGlobals->time > m_flWait)
	{
		edict_t *pEdict = INDEXENT( 1 );
		m_flWait = gpGlobals->time + 1.0;
		pGorge = NULL;
		flDist = 2000;

		for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
		{
			if ( pEdict->free || !pEdict->pvPrivateData ) // Not in use
				continue;

			pHead = CBaseEntity :: Instance( pEdict );
		
			if (pHead->pev->health < 1 && (pHead->pev->flags & (FL_CLIENT|FL_MONSTER)) )
			{
				flResult = fabs( pHead->pev->origin.z - pev->origin.z );

				if ((TargetVisible( pHead )) && ( flResult < 80.0f ) && ( !pHead->m_bGorging ) && ( g_flVisibleDistance < flDist))
				{
					flDist = g_flVisibleDistance;
					pGorge = pHead;
				}
			}
		}

		if ( (pGorge != NULL) && (flDist < RANDOM_FLOAT( 0.0f, 700.0f)) )
		{
			m_flSearchTime = gpGlobals->time + 4.0;
			m_hOldEnemy = m_hEnemy;
			m_bGorging = TRUE;
			m_hEnemy = pGorge;
			FoundTarget();

			return TRUE;
		}
	}
	else if (m_iStoleWeapon)
	{
		pHead = MonsterFindVictim();
		if (pHead != NULL)
		{
			m_hEnemy = pHead;
			FoundTarget();
			m_flAttackFinished = gpGlobals->time;
			m_flSearchTime = gpGlobals->time + 2.0;
			return TRUE;
		}
	}

	bResult = FindTarget();
	m_flSearchTime = gpGlobals->time + 2.0;

	return bResult;
}

/*
============
GremlinFindVictim

find a victim to shoot at
============
*/
CBaseEntity *CGremlin :: MonsterFindVictim( void )
{
	CBaseEntity	*pHead, *pSelected;
	float		flDist, flHeadDist;

	m_flSearchTime = gpGlobals->time + 1.0;
	// look in our immediate vicinity

	pSelected = NULL;
	flDist = 1000;
	pHead = NULL;

	while(( pHead = UTIL_FindEntityInSphere( pHead, pev->origin, 1000 )) != NULL )
	{
		if (!FBitSet( pHead->pev->flags, FL_NOTARGET ) && (( pHead->pev->flags, FL_MONSTER ) || ( pHead->pev->flags, FL_CLIENT )) )
		{
			if (TargetVisible( pHead ) && (pHead->pev->health > 0) && (pHead != this ))
			{
				flHeadDist = (pHead->pev->origin - pev->origin).Length();

				if (pHead->edict() == pev->enemy) // lastvictim!!
					flHeadDist *= 2.0f;
				if ( pHead->pev->flags & FL_CLIENT )
					flHeadDist /= 1.5f;
				if (FClassnameIs( pHead->pev, "monster_gremlin" ))
					flHeadDist *= 1.5f;

				if (flHeadDist < flDist)
				{
					pSelected = pHead;
					flDist = flHeadDist;
				}

			}
		}
	}

	if (pSelected)
		pev->enemy = pSelected->edict();

	return pSelected;
}

void CGremlin :: MonsterThrowHead( entvars_t *pevHead )
{
	if (FClassnameIs( pevHead, "monster_ogre"))
		CGib::ThrowHead ("models/h_ogre.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_knight"))
		CGib::ThrowHead ("models/h_knight.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_shambler"))
		CGib::ThrowHead ("models/h_shams.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_demon1"))
		CGib::ThrowHead ("models/h_demon.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_wizard"))
		CGib::ThrowHead ("models/h_wizard.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_zombie"))
		CGib::ThrowHead ("models/h_zombie.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_dog"))
		CGib::ThrowHead ("models/h_dog.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_hell_knight"))
		CGib::ThrowHead ("models/h_hellkn.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_enforcer"))
		CGib::ThrowHead ("models/h_mega.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_army"))
		CGib::ThrowHead ("models/h_guard.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_shalrath"))
		CGib::ThrowHead ("models/h_shal.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_gremlin"))
		CGib::ThrowHead ("models/h_grem.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_scourge"))
		CGib::ThrowHead ("models/h_scourg.mdl", pevHead);
	else if (FClassnameIs( pevHead, "monster_fish"))
		CGib::ThrowHead ("models/gib1.mdl", pevHead);
	else CGib::ThrowHead ("models/h_player.mdl", pevHead);
}

/*
============
Gremlin_FireGrenade

fire a grenade
============
*/
void CGremlin :: FireGrenade( void )
{
	*m_pCurrentAmmo -= 1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/grenade.wav", 1, ATTN_NORM );
	pev->effects |= EF_MUZZLEFLASH;

	Vector vecVelocity = (m_hEnemy->pev->origin - pev->origin).Normalize() * 600.0f;
	vecVelocity.z = 200;

	// Create the grenade
	CRocket *pRocket = CRocket::CreateGrenade( pev->origin, vecVelocity, this );
}

/*
============
Gremlin_FireRocket

fire a rocket
============
*/
void CGremlin :: FireRocket( void )
{
	*m_pCurrentAmmo -= 1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/sgun1.wav", 1, ATTN_NORM );

	Vector dir = (m_hEnemy->pev->origin - pev->origin).Normalize();
	pev->v_angle = UTIL_VecToAngles( dir );

	// Create the rocket
	UTIL_MakeVectors( pev->v_angle );

	Vector vecOrg = pev->origin + (gpGlobals->v_forward * 8) + Vector(0,0,16);

	Vector vecDir = dir + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_right + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_up;
	CRocket *pRocket = CRocket::CreateRocket( vecOrg, vecDir, this );

//	GremlinRecoil(dir,-1000);	// funny stuff
}

/*
============
Gremlin_FireNailGun

fire a nailgun
============
*/
void CGremlin :: FireNailGun( float ox )
{
	*m_pCurrentAmmo -= 1;
	pev->effects |= EF_MUZZLEFLASH;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/rocket1i.wav", 1, ATTN_NORM );

	Vector dir = (m_hEnemy->pev->origin - pev->origin).Normalize();
	pev->v_angle = UTIL_VecToAngles( dir );

	UTIL_MakeVectors( pev->v_angle );
	Vector vecDir = dir + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_right + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_up;

	CNail *pNail = CNail::CreateNail( pev->origin + Vector(0,0,16), vecDir.Normalize(), this );
}

/*
============
Gremlin_FireLaserGun

fire a laser cannon
============
*/
void CGremlin :: FireLaserGun( float ox )
{
	*m_pCurrentAmmo -= 1;
	pev->effects |= EF_MUZZLEFLASH;

//	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/rocket1i.wav", 1, ATTN_NORM );

	Vector dir = (m_hEnemy->pev->origin - pev->origin).Normalize();
	pev->v_angle = UTIL_VecToAngles( dir );

	UTIL_MakeVectors( pev->v_angle );
	Vector vecDir = dir + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_right + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_up;

	CNail *pNail = CNail::CreateHipLaser( pev->origin + Vector(0,0,16), vecDir.Normalize(), this );
}

/*
============
Gremlin_FireShotGun

fire a shotgun
============
*/
void CGremlin :: FireShotGun( void )
{
	*m_pCurrentAmmo -= 1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/guncock.wav", 1, ATTN_NORM );

	Vector dir = (m_hEnemy->pev->origin - pev->origin).Normalize();
	pev->v_angle = UTIL_VecToAngles( dir );

	UTIL_MakeVectors( pev->v_angle );
	Vector vecDir = dir + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_right + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_up;
	vecDir = vecDir.Normalize();

	pev->v_angle = UTIL_VecToAngles( vecDir );
	FireBullets( 6, vecDir, Vector(0.04, 0.04, 0) );
}

/*
============
Gremlin_FireSuperShotGun

fire a shotgun
============
*/
void CGremlin :: FireSuperShotGun( void )
{
	*m_pCurrentAmmo -= 2;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/shotgn2.wav", 1, ATTN_NORM );

	Vector dir = (m_hEnemy->pev->origin - pev->origin).Normalize();
	pev->v_angle = UTIL_VecToAngles( dir );

	UTIL_MakeVectors( pev->v_angle );
	Vector vecDir = dir + RANDOM_FLOAT( -0.3f, 0.3f ) * gpGlobals->v_right + RANDOM_FLOAT( -0.3f, 0.3f ) * gpGlobals->v_up;
	vecDir = vecDir.Normalize();

	pev->v_angle = UTIL_VecToAngles( vecDir );
	FireBullets( 14, vecDir, Vector(0.14, 0.08, 0) );
}

/*
============
Gremlin_FireLightningGun

fire lightning gun
============
*/
void CGremlin :: FireLightningGun( void )
{
	// Make lightning sound every 0.6 seconds
	if ( m_flLightningSoundTime <= gpGlobals->time )
	{
		EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/lhit.wav", 1, ATTN_NORM);
		m_flLightningSoundTime = gpGlobals->time + 0.6;
	}

	// explode if under water
	if (pev->waterlevel > 1)
	{
		float flCellsBurnt = *m_pCurrentAmmo;
		*m_pCurrentAmmo = 0;
		g_fDischarged = TRUE;
		Q_RadiusDamage( this, this, 35 * flCellsBurnt, NULL );
		g_fDischarged = FALSE;
		return;
	}

	AI_Face();

	pev->effects |= EF_MUZZLEFLASH;

	// eat ammo every 0.1 secs
	if ( m_flLightningTime <= gpGlobals->time )
	{
		m_flLightningTime = gpGlobals->time + 0.1;
		*m_pCurrentAmmo -= 2;
	}

	Vector dir = (m_hEnemy->pev->origin - pev->origin).Normalize();
	pev->v_angle = UTIL_VecToAngles( dir );

	UTIL_MakeVectors( pev->v_angle );
	Vector vecDir = dir + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_right + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_up;
	vecDir = vecDir.Normalize();

	// Lightning bolt effect
	TraceResult trace;
	Vector vecOrg = pev->origin + Vector(0,0,16);
	UTIL_TraceLine( vecOrg, pev->origin + vecDir * 600.0f, ignore_monsters, ENT(pev), &trace );

	if (trace.fAllSolid == FALSE)
	{
		MESSAGE_BEGIN( MSG_BROADCAST, gmsgTempEntity );
			WRITE_BYTE( TE_LIGHTNING2 );
			WRITE_ENTITY( entindex() );
			WRITE_COORD( vecOrg.x );
			WRITE_COORD( vecOrg.y );
			WRITE_COORD( vecOrg.z );
			WRITE_COORD( trace.vecEndPos.x );
			WRITE_COORD( trace.vecEndPos.y );
			WRITE_COORD( trace.vecEndPos.z );
		MESSAGE_END();
	}

	// Do damage
	CBasePlayer::LightningDamage(vecOrg, trace.vecEndPos + (vecDir * 4), this, 30, vecDir );
}

/*
================
GremlinFireProximityGrenade
================
*/
void CGremlin :: FireProximityGrenade( void )
{
	*m_pCurrentAmmo -= 1;

	EMIT_SOUND( edict(), CHAN_WEAPON, "hipweap/proxbomb.wav", 1, ATTN_NORM );
	gpWorld->num_prox_grenades++;

	Vector dir = (m_hEnemy->pev->origin - pev->origin).Normalize();
	pev->v_angle = UTIL_VecToAngles( dir );

	// Create the rocket
	UTIL_MakeVectors( pev->v_angle );

	Vector vecDir = dir + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_right + RANDOM_FLOAT( -0.1f, 0.1f ) * gpGlobals->v_up;
	vecDir = vecDir.Normalize();
	Vector vecVelocity = vecDir * 600.0f;
	vecVelocity.z = 200;

	// Create the grenade
	CRocket *pRocket = CRocket::CreateProxGrenade( pev->origin, vecVelocity, this );
}
      
BOOL CGremlin :: MonsterWeaponAttack( void )
{
	// make sure what ammo is set
	W_SetCurrentAmmo ();

	if (!CheckNoAmmo())
		return FALSE;

	m_iAttackState = ATTACK_MISSILE;
	m_flShowHostile = gpGlobals->time + 1.0f; // wake monsters up
	SetActivity( ACT_RANGE_ATTACK1 );

	if (m_iWeapon == IT_SHOTGUN)
	{
		FireShotGun();
		AttackFinished(1);
	}
	else if (m_iWeapon == IT_SUPER_SHOTGUN)
	{
		FireSuperShotGun();
		AttackFinished(1);
	}
	else if (m_iWeapon == IT_NAILGUN)
	{
		FireNailGun( 4 );
		AttackFinished(1);
	}
	else if (m_iWeapon == IT_SUPER_NAILGUN)
	{
		FireNailGun( 4 );
		AttackFinished(1);
	}
	else if (m_iWeapon == IT_GRENADE_LAUNCHER)
	{
		FireGrenade();
		AttackFinished(1);
	}
	else if (m_iWeapon == IT_ROCKET_LAUNCHER)
	{
		FireRocket();
		AttackFinished(1);
	}
	else if (m_iWeapon == IT_LIGHTNING)
	{
		FireLightningGun();
		EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/lstart.wav", 1, ATTN_NORM);
		m_flLightningSoundTime = gpGlobals->time + 0.2;
		AttackFinished(1);
	}
	else if (m_iWeapon == IT_LASER_CANNON)
	{
		FireLaserGun( 4 );
		AttackFinished(1);
	}
	else if (m_iWeapon == IT_PROXIMITY_GUN)
	{
		FireProximityGrenade();
		AttackFinished(1);
	}

	return TRUE;
}

void CGremlin :: MonsterDamage( entvars_t *pevTarg, entvars_t* pevAttacker, float flDamage )
{
	CBaseEntity *pTarg = CBaseEntity::Instance( pevTarg );

	// check for godmode or invincibility
	if (pTarg->pev->flags & FL_GODMODE)
		return;

	if( pTarg->IsPlayer( ))
	{
		CBasePlayer *pPlayer = (CBasePlayer *)pTarg;

		if (pPlayer->m_flInvincibleFinished >= gpGlobals->time)
		{
			if (pPlayer->m_flInvincibleSound < gpGlobals->time)
			{
				EMIT_SOUND( edict(), CHAN_ITEM, "items/protect3.wav", 1, ATTN_NORM );
				pPlayer->m_flInvincibleSound = gpGlobals->time + 2;
			}
			return;
		}
	}

	// team play damage avoidance
	if ( (gpGlobals->teamplay == 1) && (pev->team > 0) && (pTarg->pev->team == pevAttacker->team) )
		return;

	// do the damage
	pTarg->pev->health -= flDamage;
}

/*
============
GremlinCheckNoAmmo

attack with a weapon
============
*/
BOOL CGremlin :: CheckNoAmmo( void )
{
	if (m_pCurrentAmmo && *m_pCurrentAmmo > 0)
		return TRUE;

	m_iStoleWeapon = FALSE;
	pev->weaponmodel = 0;

	return FALSE;
}
   
void CGremlin :: AI_Walk( float flDist )
{
	m_flMoveDistance = flDist;
	
	// check for noticing a player
	if (MonsterFindTarget ())
		return;

	MoveToGoal( flDist );
}

void CGremlin :: AI_Run( float flDist )
{
	TraceResult tr;
	Vector	  d;
	float	  r;

	if (pev->watertype == CONTENT_LAVA)
	{
		// do damage
		TakeDamage( pev, gpWorld->pev, 2000.0f, DMG_BURN );
	}

	m_flMoveDistance = flDist;

	if (m_bGorging)
	{
		if (m_hEnemy == NULL)
		{
			m_bGorging = FALSE;
			return;
		}

		UTIL_TraceLine( pev->origin, m_hEnemy->pev->origin, ignore_monsters, edict(), &tr );

		if (tr.flFraction != 1.0)
		{
			m_bGorging = FALSE;
			return;
		}

		if (!TargetVisible(m_hEnemy))
		{
			m_bGorging = FALSE;
			return;
		}

		r = (m_hEnemy->pev->origin - pev->origin).Length();

		if (r < 130.0f)
		{
			AI_Face();

			if (r < 45.0f)
			{
				MonsterMeleeAttack();
				m_iAttackState = ATTACK_STRAIGHT;
			}
			else if (!WalkMove(pev->angles.y, flDist))
			{
				m_bGorging = FALSE;
			}
			return;
		}
		MoveToGoal( flDist );     // done in C code...
	}
	else
	{
		if (RANDOM_FLOAT(0.0f, 1.0f) > 0.97f)
		{
			if (MonsterFindTarget())
				return;
		}

		// get away from player if we stole a weapon
		if (m_iStoleWeapon && m_hEnemy != NULL)
		{
			if (m_hEnemy->pev->health < 0 && ( m_hEnemy->pev->flags & FL_CLIENT ))
			{
				SetActivity( ACT_COMBAT_IDLE );
				m_iAIState = STATE_CUSTOM;
				return;
			}

			// make sure what ammo is set
			W_SetCurrentAmmo ();

			if (!CheckNoAmmo())
			{
				if (m_bTriggerSpawned)
				{
					UTIL_Remove(m_hTriggerField);
					m_bTriggerSpawned = FALSE;
					m_pGoalEnt = m_hEnemy;
				}
				return;
			}

			r = (m_hEnemy->pev->origin - pev->origin).Length();
			d = (pev->origin - m_hEnemy->pev->origin).Normalize();

			if (!m_bTriggerSpawned)
			{
				if (r < 150.0f)
				{
					// create a fake goal entity that heading the charmer
					CBaseEntity *pTrig = GetClassPtr( (CPointEntity *)NULL );
					pTrig->pev->classname = MAKE_STRING( "info_notnull" );
					UTIL_SetSize( pTrig->pev, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ));
					m_hTriggerField = pTrig;
					m_bTriggerSpawned = TRUE;
				}
			}

			if (m_bTriggerSpawned)
			{
				if (r > 250.0f)
				{
					UTIL_Remove(m_hTriggerField);
					m_bTriggerSpawned = FALSE;
					m_pGoalEnt = m_hEnemy;
//					m_iAttackState = ATTACK_SLIDING;
				}
				else
				{
					if (r < 160.0f)
					{
						Vector ang, end;
						BOOL bDone;
						float c;

						ang = UTIL_VecToAngles( d );
						bDone = FALSE;
						c = 0.0f;

						while (!bDone)
						{
							UTIL_MakeVectors (ang);
							end = m_hEnemy->pev->origin + gpGlobals->v_forward * 350.0f;
							UTIL_TraceLine(m_hEnemy->pev->origin, end, dont_ignore_monsters, ENT(pev), &tr);

							if (tr.flFraction == 1.0f)
							{
								UTIL_TraceLine(pev->origin, end, dont_ignore_monsters, ENT(pev), &tr);
								if (tr.flFraction == 1.0f)
									bDone = TRUE;
							}
                                                            	
							ang.y = UTIL_AngleMod( ang.y + 36.0f );
							if( ++c == 10.0f ) bDone = TRUE;
						}
						UTIL_SetOrigin( m_hTriggerField->pev, end );
					}

					m_pGoalEnt = m_hTriggerField;
					pev->ideal_yaw = UTIL_VecToYaw(( m_pGoalEnt->pev->origin - pev->origin ).Normalize());
					CHANGE_YAW( ENT(pev) );
					MoveToGoal (flDist);      // done in C code...
					pev->nextthink = gpGlobals->time + 0.1;
					return;
				}
			}
		}

		CQuakeMonster :: AI_Run( flDist );
		pev->nextthink = gpGlobals->time + 0.1f;

		// HACKHACK: reset spawn pause to allow gorging
		if (m_flPauseTime == 99999999)
			m_flPauseTime = 0.0f;
	}
}

/*
===============
GremlinDropBackpack
===============
*/
void CGremlin :: MonsterDropBackpack( void )
{
	CWeaponBox :: DropBackpack( this, m_iWeapon );
	pev->weaponmodel = 0; // remove item from hands
}

void CGremlin :: MonsterSplit( void )
{
	CBaseEntity	*pHead;
	Vector		ang, pos;
	BOOL		proceed;
	BOOL		bDone;
	TraceResult	tr;
	float		c;	

	if (gpWorld->num_spawn_gremlins >= (gpWorld->total_gremlins * 2))
		return;	// too many gremlins!

	bDone = FALSE;
	ang = pev->angles;
	c = 0;

	while (!bDone)
	{
		UTIL_MakeVectors(ang);
		pos = pev->origin + (80.0f * gpGlobals->v_forward);
		pHead = NULL;
		proceed = TRUE;

		while(( pHead = UTIL_FindEntityInSphere( pHead, pos, 35 )) != NULL && proceed )
		{
			if ((pHead->pev->health > 0) && (pHead->pev->flags & (FL_MONSTER | FL_CLIENT)))
				proceed = FALSE;
		}

		UTIL_TraceLine( pev->origin, pos, dont_ignore_monsters, edict(), &tr );

		if (tr.flFraction == 1.0f && (proceed))
		{
			UTIL_TraceLine( pev->origin, pos - Vector( 40, 40, 0 ), dont_ignore_monsters, edict(), &tr );

			if (tr.flFraction == 1.0f)
			{
				UTIL_TraceLine( pev->origin, pos + Vector( 40, 40, 0 ), dont_ignore_monsters, edict(), &tr );

				if (tr.flFraction == 1.0f)
				{
					UTIL_TraceLine( pev->origin, pos + Vector( 0, 0, 64 ), dont_ignore_monsters, edict(), &tr );

					if (tr.flFraction == 1.0f)
					{
						UTIL_TraceLine( pev->origin, pos - Vector( 0, 0, 64 ), dont_ignore_monsters, edict(), &tr );

						if (tr.flFraction != 1.0f)
						{
							bDone = TRUE;
						}
					}
				}
			}
		}

		if (!bDone)
		{
			ang.y += 36.0f;

			// not enough room to spawn new gremlin
			if (++c == 10.0f) return;
		}
	}

	edict_t *pent = CREATE_NAMED_ENTITY( MAKE_STRING( "monster_gremlin" ));

	if( FNullEnt( pent ))
	{
		ALERT ( at_error, "can't spawn monster_gremlin!\n" );
		return;
	}

	if (pev->health < 100) pev->health = 100;
	entvars_t *pevCreate = VARS( pent );
	pevCreate->origin = pos;
	pevCreate->angles = pev->angles;
	pevCreate->health = pev->health * 0.5f;
	pev->health *= 0.5f;	

	CQuakeMonster *pNewGrem = (CQuakeMonster *)CBaseEntity :: Instance( pent );
	pNewGrem->m_iAIState = STATE_REBORN;

	DispatchSpawn( ENT( pevCreate ) );
	gpWorld->num_spawn_gremlins++;

	pNewGrem->m_iAIState = STATE_REBORN;
	pNewGrem->SetActivity( ACT_HOVER ); // play born animation

	// update total monsters count
	MESSAGE_BEGIN( MSG_ALL, gmsgStats );
		WRITE_BYTE( STAT_TOTALMONSTERS );
		WRITE_SHORT( gpWorld->total_monsters );
	MESSAGE_END();
}

void CGremlin :: MonsterGorge( float side )
{
	Vector	delta;
	float	ldmg;

	if (m_hEnemy == NULL)
	{
		m_hEnemy = NULL;
		m_bGorging = FALSE;

		SetActivity( ACT_IDLE_ANGRY );
		return;	// already reborned by another gremlin
	}

	delta = (m_hEnemy->pev->origin - pev->origin);

	EMIT_SOUND( edict(), CHAN_VOICE, "demon/dhit2.wav", 1.0, ATTN_NORM );	
	ldmg = 7.0f + RANDOM_FLOAT( 0.0f, 5.0f );

	MonsterDamage( m_hEnemy->pev, pev, ldmg );

	UTIL_MakeVectors (pev->angles);
	SpawnMeatSpray (pev->origin + gpGlobals->v_forward * 16.0f, side * gpGlobals->v_right);

	if (m_hEnemy->pev->health < -200.0)
	{
		if (!m_hEnemy->m_bGorging)
		{
			m_hEnemy->m_bGorging = TRUE;
			EMIT_SOUND( edict(), CHAN_VOICE, "player/udeath.wav", 1.0, ATTN_NORM );
			MonsterThrowHead( m_hEnemy->pev );
			ldmg = 150.0f + RANDOM_FLOAT( 0.0f, 100.0f );

			// remove corpse
			if( m_hEnemy != NULL )
			{
				// but don't trying to remove player
				if( m_hEnemy->IsPlayer( ))
					m_hEnemy->pev->effects |= EF_NODRAW;
				else UTIL_Remove( m_hEnemy );
			}
			TakeHealth( ldmg, DMG_GENERIC );
			MonsterSplit();
		}

		m_hEnemy = NULL;
		m_bGorging = FALSE;

		SetActivity( ACT_IDLE_ANGRY );
	}
}

/*
==============
CheckDemonJump

==============
*/
BOOL CGremlin :: CheckJump( void )
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

BOOL CGremlin :: MonsterCheckAttack( void )
{
	Vector spot1, spot2;
	CBaseEntity *pTarg;
	float chance;

	pTarg = m_hEnemy;

	if (gpGlobals->time < m_flAttackFinished)
		return FALSE;

	// see if any entities are in the way of the shot
	spot1 = pev->origin;
	spot2 = pTarg->pev->origin;

	if (((spot2 - spot1).Length() <= 90.0f) && !m_iStoleWeapon)
	{
		m_iAttackState = ATTACK_MELEE;
		pev->impulse = ATTACK_MELEE;
		return TRUE;
	}

	// missile attack
	chance = 0.03f + m_iStoleWeapon;

	if (RANDOM_FLOAT(0.0f, 1.0f) < chance)
	{
		m_iAttackState = ATTACK_MISSILE;
		pev->impulse = ATTACK_MISSILE;
		return TRUE;
	}
	return FALSE;
}

void CGremlin :: MonsterLeap( void )
{
	AI_Face();

	if( pev->flags & FL_ONGROUND )
	{	
		SetTouch( JumpTouch );
		UTIL_MakeVectors( pev->angles );

		pev->origin.z++;
		pev->velocity = gpGlobals->v_forward * 300 + Vector( 0, 0, 300 );
		pev->flags &= ~FL_ONGROUND;
	}
	else
	{
		MonsterRun();
	}
}

void CGremlin :: MonsterMelee( float side )
{
	if (m_hEnemy == NULL)
		return;

	AI_Face();

	Vector delta = m_hEnemy->pev->origin - pev->origin;

	if (delta.Length() > 100)
		return;

	if (!Q_CanDamage( m_hEnemy, this ))
		return;

	EMIT_SOUND( edict(), CHAN_VOICE, "grem/attack.wav", 1.0, ATTN_NORM );		
	float ldmg = 10.0f + RANDOM_FLOAT( 0.0f, 5.0f );
	m_hEnemy->TakeDamage (pev, pev, ldmg, DMG_SLASH);

	UTIL_MakeVectors( pev->angles );
	SpawnMeatSpray (pev->origin + gpGlobals->v_forward * 16, side * gpGlobals->v_right);
}

void CGremlin :: MonsterMissileAttack( void )
{
	m_iAIState = STATE_ATTACK;

	if( m_iStoleWeapon )
	{
		if( MonsterWeaponAttack( ))
		{
			return;
		}
		else if(( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.1f ) && (pev->flags & FL_ONGROUND))
		{
			SetActivity( ACT_LEAP );
			return;
		}
	}

	if( pev->flags & FL_ONGROUND )
		SetActivity( ACT_LEAP );
}

void CGremlin :: MonsterMeleeAttack( void )
{
	float	num;

	if (m_bGorging)
	{
		// now we disable default AI system
		m_iAIState = STATE_CUSTOM;
		m_iAttackState = ATTACK_NONE;
		pev->impulse = ATTACK_NONE;

		SetActivity( ACT_SPECIAL_ATTACK1 );
	}
	else
	{
		m_iAIState = STATE_ATTACK;
		m_iAttackState = ATTACK_MELEE;

		if (m_iStoleWeapon)
		{
			ALERT( at_aiconsole, "gremlin meleeing with stolen weapon\n" );
		}
		else if ((m_hEnemy->IsPlayer()) && RANDOM_FLOAT( 0.0f, 1.0f ) < 0.4f)
		{
			if (MonsterAttemptWeaponSteal())
				return;
		}
		num = RANDOM_FLOAT( 0.0f, 1.0f );
		if (num < 0.3f)
		{
			SetActivity( ACT_MELEE_ATTACK1 );
		}
		else if (num < 0.6f)
		{
			SetActivity( ACT_MELEE_ATTACK2 );
		}
		else
		{
			SetActivity( ACT_MELEE_ATTACK1 );
		}
	}
}

void CGremlin :: MonsterIdle( void )
{
	if( m_iAIState == STATE_REBORN )
		return;	// monster in 'spawn' mode

	m_iAIState = STATE_IDLE;
	SetActivity( ACT_IDLE );
	m_flMonsterSpeed = 0;
}

void CGremlin :: MonsterWalk( void )
{
	m_iAIState = STATE_WALK;
	SetActivity( ACT_WALK );
	m_flMonsterSpeed = 8;
}

void CGremlin :: MonsterRun( void )
{
	m_iAIState = STATE_RUN;
	if( m_iStoleWeapon )
		SetActivity( ACT_RUN_SCARED );
	else SetActivity( ACT_RUN );
	m_flMonsterSpeed = 16;
}

void CGremlin :: MonsterAttack( void )
{
	if( pev->impulse == ATTACK_MELEE )
	{
		AI_Charge( 15 );
	}
	else if( pev->impulse == ATTACK_MISSILE && m_iStoleWeapon )
	{
		// process weapon shots
		if (m_iWeapon == IT_NAILGUN || m_iWeapon == IT_SUPER_NAILGUN)
		{
			// shoot every frame until sequence is finished
			FireNailGun( 4 );
		}
		else if (m_iWeapon == IT_LASER_CANNON)
		{
			// shoot every frame until sequence is finished
			FireLaserGun( 4 );
		}
		else if (m_iWeapon == IT_LIGHTNING)
		{
		}
	}

	if( m_fSequenceFinished )
	{
		pev->impulse = ATTACK_NONE;	// reset shadow of attack state
		MonsterRun();
	}
}

void CGremlin :: MonsterCustom( void )
{
	if( m_iAIState == STATE_CUSTOM && m_fSequenceFinished )
	{
		SetActivity( ACT_SPECIAL_ATTACK1 );
	}
}

void CGremlin :: JumpTouch( CBaseEntity *pOther )
{
	if (pev->health <= 0)
		return;

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
	pev->nextthink = gpGlobals->time + 0.1;
}

void CGremlin :: FlipTouch( CBaseEntity *pOther )
{
	if (!ENT_IS_ON_FLOOR( edict() ))
	{
		if (pev->flags & FL_ONGROUND)
		{
			SetTouch( NULL );
			SetActivity( ACT_DIEBACKWARD );
			pev->nextthink = gpGlobals->time + 0.1f;
		}
		return;	// not on ground yet
	}

	SetTouch( NULL );
	pev->solid = SOLID_NOT;
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CGremlin :: MonsterKilled( entvars_t *pevAttacker, int iGib )
{
	Vector vec;
	float dot;

	// check for gib
	if (m_iItems & (IT_SUPER_SHOTGUN|IT_NAILGUN|IT_SUPER_NAILGUN|IT_GRENADE_LAUNCHER|IT_ROCKET_LAUNCHER|IT_LIGHTNING|IT_LASER_CANNON|IT_PROXIMITY_GUN))
	{
		MonsterDropBackpack();
		m_iStoleWeapon = FALSE;
	}

	UTIL_MakeVectors (pev->angles);
	vec = (pevAttacker->origin - pev->origin).Normalize();
	dot = DotProduct( vec, gpGlobals->v_forward );

	if( ShouldGibMonster( iGib ))
	{
		EMIT_SOUND( edict(), CHAN_VOICE, "player/udeath.wav", 1.0, ATTN_NORM );
		CGib::ThrowHead ("models/h_grem.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		CGib::ThrowGib ("models/gib1.mdl", pev);
		UTIL_Remove( this );
		return;
	}
	else if (dot > 0.7f && (RANDOM_FLOAT( 0.0f, 1.0f ) < 0.5f) && (pev->flags & FL_ONGROUND))
	{
		AI_Face();
		pev->origin.z++;
		pev->flags &= ~FL_ONGROUND;
		pev->velocity = Vector( 0, 0, 350 ) - (gpGlobals->v_forward * 200.0f);
		EMIT_SOUND( edict(), CHAN_VOICE, "grem/death.wav", 1.0, ATTN_NORM );
		SetActivity( ACT_DIEBACKWARD );
		return;
	}

	// regular death
	EMIT_SOUND( edict(), CHAN_VOICE, "grem/death.wav", 1.0, ATTN_NORM );
	SetActivity( ACT_DIEFORWARD );
	AI_Forward( 2 );
}

void CGremlin :: Killed( entvars_t *pevAttacker, int iGib )
{
	MonsterKilled( pevAttacker, iGib );
	m_iAIState = STATE_DEAD;
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	pev->solid = SOLID_NOT;

	gpWorld->killed_monsters++;

	if( m_fCharmed )
		pev->effects &= ~EF_DIMLIGHT;

	if( m_hEnemy == NULL )
		m_hEnemy = CBaseEntity::Instance( pevAttacker );
	MonsterDeathUse( m_hEnemy, this, USE_TOGGLE, 0.0f );

	// just an event to increase internal client counter
	MESSAGE_BEGIN( MSG_ALL, gmsgKilledMonster );
	MESSAGE_END();
}

//=========================================================
// Spawn
//=========================================================
void CGremlin :: Spawn( void )
{
	if( !g_pGameRules->FAllowMonsters( ))
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}

	Precache( );

	SET_MODEL(ENT(pev), "models/grem.mdl");
	UTIL_SetSize( pev, Vector( -16, -16, -24 ), Vector( 16, 16, 32 ));
	pev->solid	= SOLID_SLIDEBOX;
	pev->movetype	= MOVETYPE_STEP;
	pev->health	= 100;
	pev->max_health	= 101;
	pev->yaw_speed	= 40;

	if( m_iAIState != STATE_REBORN )
		gpWorld->total_gremlins++;

	WalkMonsterInit ();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CGremlin :: Precache()
{
	PRECACHE_MODEL( "models/grem.mdl" );
	PRECACHE_MODEL( "models/h_grem.mdl" );

	PRECACHE_SOUND ("grem/death.wav");
	PRECACHE_SOUND ("grem/attack.wav");
	PRECACHE_SOUND ("demon/djump.wav");
	PRECACHE_SOUND ("demon/dhit2.wav");
	PRECACHE_SOUND ("grem/idle.wav");
	PRECACHE_SOUND ("grem/sight1.wav");
	PRECACHE_SOUND_ARRAY( pPainSounds );
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CGremlin :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case GREM_IDLE_SOUND:
		if( RANDOM_FLOAT( 0.0f, 1.0f ) < 0.1f )
			EMIT_SOUND( edict(), CHAN_VOICE, "grem/idle.wav", 1.0, ATTN_IDLE );
		break;
	case GREM_CHECK_GROUND:
		MonsterLeap();
		break;
	case GREM_WAIT_FOR_REST:
		// if three seconds pass, assume gremlin is stuck and jump again
		pev->nextthink = gpGlobals->time + 3.0;
		break;
	case GREM_DROP_BACKPACK:
		MonsterDropBackpack();
		m_iStoleWeapon = FALSE;

		if ((m_hOldEnemy != NULL) && m_hOldEnemy->pev->health > 0.0f)
		{
			m_hEnemy = m_hOldEnemy;
			HuntTarget ();
		}
		else
		{
			if (m_hMoveTarget)
			{
				// g-cont. stay over defeated player a few seconds
				// then continue patrol (if present)
				m_pGoalEnt = m_hMoveTarget;
			}

			MonsterIdle();
		}
		break;
	case GREM_FLIP_TOUCH:
		SetTouch( FlipTouch );
		break;
	case GREM_GORGE_RIGHT:
		MonsterGorge( 200 );
		break;
	case GREM_GORGE_LEFT:
		MonsterGorge( -200 );
		break;
	case GREM_SPAWN_DONE:
		m_flPauseTime = 0; // activate new gremlins immediately
		MonsterRun();
		break;
	case GREM_CHECK_ENEMY:
		MonsterRun();
		if ((m_hOldEnemy != NULL) && m_hOldEnemy->pev->health > 0.0f)
		{
			m_hEnemy = m_hOldEnemy;
			HuntTarget ();
		}
		else
		{
			if (m_hMoveTarget)
			{
				// g-cont. stay over defeated player a few seconds
				// then continue patrol (if present)
				m_pGoalEnt = m_hMoveTarget;
			}

			MonsterIdle();
		}
		break;
	case GREM_ATTACK_CENTER:
		MonsterMelee( 0.0f );
		break;
	case GREM_ATTACK_RIGHT:
		MonsterMelee( 200.0f );
		break;
	case GREM_MUZZLEFLASH:
		pev->effects |= EF_MUZZLEFLASH;
		break;
	default:
		CQuakeMonster::HandleAnimEvent( pEvent );
		break;
	}
}