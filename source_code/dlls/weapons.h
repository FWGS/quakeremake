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
#ifndef WEAPONS_H
#define WEAPONS_H

extern DLL_GLOBAL short g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL short g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL short g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
extern void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern void Q_RadiusDamage( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, CBaseEntity *pIgnore );
extern void EjectBrass ( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype );

extern float Q_CanDamage(CBaseEntity *pTarget, CBaseEntity *pInflictor);
extern void SpawnMeatSpray( Vector vecOrigin, Vector vecVelocity );

extern BOOL IsSkySurface( CBaseEntity *pEnt, const Vector &point, const Vector &vecDir );

typedef struct 
{
	CBaseEntity	*pEntity;
	float		amount;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;

#ifndef HIPNOTIC
class CRocket : public CBaseEntity
#else /* HIPNOTIC */
class CRocket : public CBaseDelay
#endif /* HIPNOTIC */
{
public:
	void Spawn( void );
	void Explode( void );

	// Rocket funcs
	static CRocket *CreateRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
	void EXPORT RocketTouch( CBaseEntity *pOther );

	// Grenade funcs
	static CRocket *CreateGrenade( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
#ifdef HIPNOTIC
	static CRocket *CreateProxGrenade( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
#endif /* HIPNOTIC */
	void EXPORT GrenadeTouch( CBaseEntity *pOther );
	void EXPORT GrenadeExplode( void );
#ifdef HIPNOTIC
	void EXPORT ProximityThink( void );
	void EXPORT ProximityTouch( CBaseEntity *pOther );
	void ProximityGrenadeExplode( void );
#endif /* HIPNOTIC */

	int	m_iTrail;
};

class CNail : public CBaseEntity
{
public:
	void Spawn( void );
	static  CNail *CreateNail( Vector vecOrigin, Vector vecDir, CBaseEntity *pOwner );
	static  CNail *CreateSuperNail( Vector vecOrigin, Vector vecDir, CBaseEntity *pOwner );
	static  CNail *CreateKnightSpike( Vector vecOrigin, Vector vecDir, CBaseEntity *pOwner );
#ifdef HIPNOTIC
	static  CNail *CreateHipLaser( Vector vecOrigin, Vector vecDir, CBaseEntity *pOwner );
#endif /* HIPNOTIC */
	void EXPORT NailTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
#ifdef HIPNOTIC
	void EXPORT LaserTouch( CBaseEntity *pOther );
	void EXPORT LaserThink( void );
#endif /* HIPNOTIC */
};

#ifdef HIPNOTIC
#define SF_TRAP_SILENT	16	// special flag for trap_shooter

#endif /* HIPNOTIC */
class CLaser : public CBaseEntity
{
public:
	void Spawn( void );
	static  CLaser *LaunchLaser( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
	void EXPORT LaserTouch( CBaseEntity *pOther );
};

class CZombieMissile : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );
	void EXPORT MeatTouch( CBaseEntity *pOther );
public:
	static CZombieMissile *CreateMissile( Vector vecOrigin, Vector vecOffset, Vector vecAngles, CBaseEntity *pOwner );
	static CZombieMissile *CreateSpray( Vector vecOrigin, Vector vecVelocity );
};

class CShalMissile : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );
	void EXPORT ShalTouch( CBaseEntity *pOther );
	void EXPORT ShalHome( void );
public:
	static CShalMissile *CreateMissile( Vector vecOrigin, Vector vecVelocity );
#ifdef HIPNOTIC
};

class CMjolnirLightning : public CBaseDelay
{
	void Spawn( void );
	void Think( void );
	BOOL TargetVisible( CBaseEntity *pTarg );
public:
	static CMjolnirLightning *CreateLightning( CBaseEntity *pPrev, CBaseEntity *pOwner, const Vector &vecAngles, float flDist );
#endif /* HIPNOTIC */
};

#endif // WEAPONS_H
