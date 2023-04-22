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

===== lights.cpp ========================================================

  spawn and think functions for editor-placed lights

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

extern DLL_GLOBAL BOOL		g_fXashEngine;

#define SF_LIGHT_START_OFF		1

/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) LIGHT_START_OFF
Non-displayed light.
Default light value is 300
Default style is 0
If targeted, it will toggle between on or off.
*/
class CLight : public CPointEntity
{
public:
	virtual void	KeyValue( KeyValueData* pkvd ); 
	virtual void	Spawn( void );
	void		Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	
	static TYPEDESCRIPTION m_SaveData[];

	int		m_iStyle;
};
LINK_ENTITY_TO_CLASS( light, CLight );

TYPEDESCRIPTION	CLight::m_SaveData[] = 
{
	DEFINE_FIELD( CLight, m_iStyle, FIELD_INTEGER ),
}; IMPLEMENT_SAVERESTORE( CLight, CPointEntity );

void CLight :: KeyValue( KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style"))
	{
		m_iStyle = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CPointEntity::KeyValue( pkvd );
	}
}

void CLight :: Spawn( void )
{
	if (FStringNull(pev->targetname))
	{       // inert light
		REMOVE_ENTITY(ENT(pev));
		return;
	}
	
	if (m_iStyle >= 32)
	{
		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
			LIGHT_STYLE(m_iStyle, "a");
		else
			LIGHT_STYLE(m_iStyle, "m");
	}
}


void CLight :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (m_iStyle >= 32)
	{
		if ( !ShouldToggle( useType, !FBitSet(pev->spawnflags, SF_LIGHT_START_OFF) ) )
			return;

		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
		{
			LIGHT_STYLE(m_iStyle, "m");
			ClearBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
		else
		{
			LIGHT_STYLE(m_iStyle, "a");
			SetBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
	}
}

/*QUAKED light_fluoro (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300
Default style is 0
If targeted, it will toggle between on or off.
Makes steady fluorescent humming sound
*/
class CLightFluoro : public CLight
{
public:
	void	Precache( void ); 
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( light_fluoro, CLightFluoro );

void CLightFluoro :: Precache( void )
{
	PRECACHE_SOUND( "ambience/fl_hum1.wav" );

	UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/fl_hum1.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
}

void CLightFluoro :: Spawn( void )
{
	Precache ();

	if (m_iStyle >= 32)
	{
		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
			LIGHT_STYLE(m_iStyle, "a");
		else
			LIGHT_STYLE(m_iStyle, "m");
	}
}

/*QUAKED light_fluorospark (0 1 0) (-8 -8 -8) (8 8 8)
Non-displayed light.
Default light value is 300
Default style is 10
Makes sparking, broken fluorescent sound
*/
class CLightFluoroSpark : public CLight
{
public:
	void	Precache( void ); 
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( light_fluorospark, CLightFluoroSpark );

void CLightFluoroSpark :: Precache( void )
{
	PRECACHE_SOUND( "ambience/buzz1.wav" );

	UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/buzz1.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
}

void CLightFluoroSpark :: Spawn( void )
{
	Precache ();

	if (!m_iStyle)
		m_iStyle = 10;
}

/*QUAKED light_globe (0 1 0) (-8 -8 -8) (8 8 8)
Sphere globe light.
Default light value is 300
Default style is 0
*/
class CLightGlobe : public CPointEntity
{
public:
	void	Precache( void ); 
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( light_globe, CLightGlobe );

void CLightGlobe :: Precache( void )
{
	PRECACHE_MODEL( "sprites/s_light.spr" );
}

void CLightGlobe :: Spawn( void )
{
	Precache ();

	if( g_fXashEngine )
	{
		// NOTE: this has effect only in Xash3D
		pev->effects = EF_FULLBRIGHT;
	}

	SET_MODEL( ENT(pev), "sprites/s_light.spr" );

	// set this to allow support HD-textures replacement
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;
}

/*QUAKED light_torch_small_walltorch (0 .5 0) (-10 -10 -20) (10 10 20)
Short wall torch
Default light value is 200
Default style is 0
*/
class CLightTorch : public CPointEntity
{
public:
	void	Precache( void ); 
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( light_torch_small_walltorch, CLightTorch );

void CLightTorch :: Precache( void )
{
	PRECACHE_MODEL( "models/flame1.mdl" );
	PRECACHE_SOUND( "ambience/fire1.wav" );

	UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/fire1.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
}

void CLightTorch :: Spawn( void )
{
	Precache ();
/*
	// g-cont. use STUDIO_NF_FULLBRIGHT here because body of torch must be shaded
	if( g_fXashEngine )
	{
		// NOTE: this has effect only in Xash3D
		pev->effects = EF_FULLBRIGHT;
	}
*/
	SET_MODEL( ENT(pev), "models/flame1.mdl" );

	// run animation
	pev->framerate = RANDOM_FLOAT( 0.45, 0.55 ); // DMC models have too fast sequence
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.0f, 0.2f );
}

/*QUAKED light_flame_large_yellow (0 1 0) (-10 -10 -12) (12 12 18)
Large yellow flame ball
*/
/*QUAKED light_flame_small_yellow (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Small yellow flame ball
*/
/*QUAKED light_flame_small_white (0 1 0) (-10 -10 -40) (10 10 40) START_OFF
Small white flame ball
*/
class CLightFlame : public CPointEntity
{
public:
	void	Precache( void ); 
	void	Spawn( void );
	void	Think( void );
};

LINK_ENTITY_TO_CLASS( light_flame_large_yellow, CLightFlame );
LINK_ENTITY_TO_CLASS( light_flame_small_yellow, CLightFlame );
LINK_ENTITY_TO_CLASS( light_flame_small_white, CLightFlame );

void CLightFlame :: Precache( void )
{
	if( FClassnameIs( pev, "light_flame_large_yellow" ))
		PRECACHE_MODEL( "models/flame2b.mdl" );
	else PRECACHE_MODEL( "models/flame2.mdl" );
	PRECACHE_SOUND( "ambience/fire1.wav" );

	UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/fire1.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
}

void CLightFlame :: Spawn( void )
{
	Precache ();

	if( g_fXashEngine )
	{
		// NOTE: this has effect only in Xash3D
		pev->effects = EF_FULLBRIGHT;
	}

	if( FClassnameIs( pev, "light_flame_large_yellow" ))
		SET_MODEL( ENT(pev), "models/flame2b.mdl" );
	else SET_MODEL( ENT(pev), "models/flame2.mdl" );

	// run animation
	pev->animtime = gpGlobals->time;
	pev->framerate = RANDOM_FLOAT( 0.45, 0.55 ); // DMC models have too fast sequence
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.0f, 0.2f );
}

void CLightFlame :: Think( void )
{
	TraceResult tr;

	// try to link flame with moving brush
	UTIL_TraceLine( pev->origin + Vector( 0, 0, 32 ), pev->origin + Vector( 0, 0, -32 ), ignore_monsters, ENT( pev ), &tr);

	// g-cont. e1m5 have a secret button with flame at the top.
	if( g_fXashEngine && tr.flFraction != 1.0 && !FNullEnt( tr.pHit ))
	{
		ALERT( at_aiconsole, "%s linked with %s (%s)\n", STRING( pev->classname ),
		STRING( VARS( tr.pHit )->classname ), STRING( VARS( tr.pHit )->targetname )); 
		pev->movetype = MOVETYPE_COMPOUND;	// set movewith type
		pev->aiment = tr.pHit;		// set parent
	}
}