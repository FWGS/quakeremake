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
//=========================================================
// sound.cpp 
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

/*QUAKED ambient_suck_wind (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
*/
class CAmbientSuckWind : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/suck1.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/suck1.wav", 1, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_suck_wind, CAmbientSuckWind );

/*QUAKED ambient_drone (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
*/
class CAmbientDrone : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/drone6.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/drone6.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_drone, CAmbientDrone );

/*QUAKED ambient_flouro_buzz (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
*/
class CAmbientFluoroBuzz : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/buzz1.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/buzz1.wav", 1, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_flouro_buzz, CAmbientFluoroBuzz );

/*QUAKED ambient_drip (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
*/
class CAmbientDrip : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/drip1.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/drip1.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_drip, CAmbientDrip );

/*QUAKED ambient_comp_hum (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
*/
class CAmbientCompHum : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/comp1.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/comp1.wav", 1, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_comp_hum, CAmbientCompHum );

/*QUAKED ambient_thunder (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
a random strike thunder sound
*/
class CAmbientThunder : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/thunder1.wav" );
	}

	void Spawn( void )
	{
		Precache();
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 5.0, 15.0 );
	}

	void Think( void )
	{
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/thunder1.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 10.0, 30.0 ); // next time is 10 - 30 seconds
	}
};

LINK_ENTITY_TO_CLASS( ambient_thunder, CAmbientThunder );

/*QUAKED ambient_light_buzz (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
*/
class CAmbientLightBuzz : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/fl_hum1.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/fl_hum1.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_light_buzz, CAmbientLightBuzz );

/*QUAKED ambient_swamp1 (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
*/
class CAmbientSwamp1 : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/swamp1.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/swamp1.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_swamp1, CAmbientSwamp1 );

/*QUAKED ambient_swamp2 (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
*/
class CAmbientSwamp2 : public CBaseEntity
{
public:
	void Precache( void )
	{
		PRECACHE_SOUND( "ambience/swamp2.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambience/swamp2.wav", 0.5, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

#ifndef HIPNOTIC
LINK_ENTITY_TO_CLASS( ambient_swamp2, CAmbientSwamp2 );
#else /* HIPNOTIC */
LINK_ENTITY_TO_CLASS( ambient_swamp2, CAmbientSwamp2 );

/*QUAKED ambient_humming (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
  "volume" how loud it should be (0.5 is default)
*/
class CAmbientHumming : public CBaseEntity
{
public:
	void Precache( void )
	{
		if( !m_flVolume ) m_flVolume = 0.5f;
		PRECACHE_SOUND( "ambient/humming.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambient/humming.wav", m_flVolume, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_humming, CAmbientHumming );

/*QUAKED ambient_rushing (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
  "volume" how loud it should be (0.5 is default)
*/
class CAmbientRushing : public CBaseEntity
{
public:
	void Precache( void )
	{
		if( !m_flVolume ) m_flVolume = 0.5f;
		PRECACHE_SOUND( "ambient/rushing.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambient/rushing.wav", m_flVolume, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_rushing, CAmbientRushing );

/*QUAKED ambient_running_water (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
  "volume" how loud it should be (0.5 is default)
*/
class CAmbientRunningWater : public CBaseEntity
{
public:
	void Precache( void )
	{
		if( !m_flVolume ) m_flVolume = 0.5f;
		PRECACHE_SOUND( "ambient/runwater.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambient/runwater.wav", m_flVolume, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_running_water, CAmbientRunningWater );

/*QUAKED ambient_fan_blowing (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
  "volume" how loud it should be (0.5 is default)
*/
class CAmbientFanBlowing : public CBaseEntity
{
public:
	void Precache( void )
	{
		if( !m_flVolume ) m_flVolume = 0.5f;
		PRECACHE_SOUND( "ambient/fanblow.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambient/fanblow.wav", m_flVolume, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_fan_blowing, CAmbientFanBlowing );

/*QUAKED ambient_waterfall (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
  "volume" how loud it should be (0.5 is default)
*/
class CAmbientWaterfall : public CBaseEntity
{
public:
	void Precache( void )
	{
		if( !m_flVolume ) m_flVolume = 0.5f;
		PRECACHE_SOUND( "ambient/waterfal.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambient/waterfal.wav", m_flVolume, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_waterfall, CAmbientWaterfall );

/*QUAKED ambient_riftpower (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
  "volume" how loud it should be (0.5 is default)
*/
class CAmbientRiftPower : public CBaseEntity
{
public:
	void Precache( void )
	{
		if( !m_flVolume ) m_flVolume = 0.5f;
		PRECACHE_SOUND( "ambient/riftpowr.wav" );
		UTIL_EmitAmbientSound( ENT(pev), pev->origin, "ambient/riftpowr.wav", m_flVolume, ATTN_STATIC, SND_SPAWNING, 100 );
	}
	void Spawn( void ) { Precache(); }
};

LINK_ENTITY_TO_CLASS( ambient_riftpower, CAmbientRiftPower );

class CPlaySoundBase : public CBaseToggle
{
public:
	void Precache( void );
	void EXPORT PlaySoundUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT PlaySoundThink( void );
};

void CPlaySoundBase :: Precache( void )
{
	PRECACHE_SOUND( (char *)STRING( pev->noise ));
}

void CPlaySoundBase :: PlaySoundUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( pev->spawnflags & 1 )
	{
		if( pev->button )
		{
			pev->button = 1;
			EMIT_SOUND(ENT(pev), pev->impulse, STRING(pev->noise), m_flVolume, pev->speed);
		}
		else
		{
			pev->button = 0;
			STOP_SOUND(ENT(pev), pev->impulse, (char*)STRING(pev->noise));
		}
	}
	else
	{
		EMIT_SOUND(ENT(pev), pev->impulse, STRING(pev->noise), m_flVolume, pev->speed);
	}
}

void CPlaySoundBase :: PlaySoundThink( void )
{
	float t = m_flWait * RANDOM_FLOAT( 0.0f, 1.0f );
	if( t < m_flDelay ) t = m_flDelay;

	pev->nextthink = gpGlobals->time + t;
	PlaySoundUse( this, this, USE_TOGGLE, 0.0f );
}

/*QUAKED play_sound_triggered (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) toggle
play a sound when it is used
"toggle" determines whether sound should be stopped when triggered again
"volume" how loud (1 default full volume)
"noise" sound to play
"impulse" channel on which to play sound (0-7) (0 automatic is default)
"speed" attenuation factor
   -1 - no attenuation
    1 - normal
    2 - idle
    3 - static
*/
/*QUAKED random_thunder_triggered (0.3 0.1 0.6) (-10 -10 -8) (10 10 8) toggle
"toggle" determines whether sound should be stopped when triggered again
"volume" how loud (1 default full volume)
"speed" attenuation factor
   -1 - no attenuation
    1 - normal
    2 - idle
    3 - static
*/
class CPlaySoundTriggered : public CPlaySoundBase
{
public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( play_sound_triggered, CPlaySoundTriggered );
LINK_ENTITY_TO_CLASS( random_thunder_triggered, CPlaySoundTriggered );

void CPlaySoundTriggered :: Spawn( void )
{
	if( m_flVolume == 0.0f )
		m_flVolume = 1.0f;
	if( pev->speed == 0 )
		pev->speed = 1.0f;
	if( pev->speed == -1 )
		pev->speed = 0;

	if( pev->spawnflags & 1 )
	{
		if( !pev->impulse )
			pev->impulse = CHAN_AUTO;
          }

	if( FClassnameIs( pev, "random_thunder_triggered" ))
	{
		pev->noise = MAKE_STRING( "ambience/thunder1.wav" );
		pev->impulse = CHAN_BODY;
	}

	Precache ();
	SetUse( &CPlaySoundBase::PlaySoundUse );
}

/*QUAKED play_sound (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
play a sound on a periodic basis
"volume" how loud (1 default full volume)
"noise" sound to play
"wait" random time between sounds (default 20)
"delay" minimum delay between sounds (default 2)
"impulse" channel on which to play sound (0-7) (0 automatic is default)
"speed" attenuation factor
   -1 - no attenuation
    1 - normal
    2 - idle
    3 - static
*/
/*QUAKED random_thunder (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
"wait" random time between strikes (default 20)
"delay" minimum delay between strikes (default 2)
"volume" how loud (1 default full volume)
"speed" attenuation factor
   -1 - no attenuation
    1 - normal
    2 - idle
    3 - static
*/
class CPlaySound : public CPlaySoundBase
{
public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( play_sound, CPlaySound );
LINK_ENTITY_TO_CLASS( random_thunder, CPlaySound );

void CPlaySound :: Spawn( void )
{
	if( m_flVolume == 0.0f )
		m_flVolume = 1.0f;
	if( pev->speed == 0 )
		pev->speed = 1.0f;
	if( pev->speed == -1 )
		pev->speed = 0;

	if( pev->spawnflags & 1 )
	{
		if( !pev->impulse )
			pev->impulse = CHAN_AUTO;
          }

	if( !m_flWait )
		m_flWait = 20.0f;

	if( !m_flDelay )
		m_flDelay = 2.0f;

	if( FClassnameIs( pev, "random_thunder" ))
	{
		pev->noise = MAKE_STRING( "ambience/thunder1.wav" );
		pev->impulse = CHAN_BODY;
	}

	Precache ();
	SetThink( PlaySoundThink );

	float t = m_flWait * RANDOM_FLOAT( 0.0f, 1.0f );
	if( t < m_flDelay ) t = m_flDelay;

	pev->nextthink = gpGlobals->time + t;
}
#endif /* HIPNOTIC */
