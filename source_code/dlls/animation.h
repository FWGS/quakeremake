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
#ifndef ANIMATION_H
#define ANIMATION_H

#define ACTIVITY_NOT_AVAILABLE		-1

#define EVENT_SPECIFIC			0
#define EVENT_SCRIPTED			1000
#define EVENT_SHARED			2000
#define EVENT_CLIENT			5000

typedef struct
{
	int	event;
	char	*options;
} MonsterEvent_t;

int LookupActivity( void *pmodel, entvars_t *pev, int activity );
int LookupActivityHeaviest( void *pmodel, entvars_t *pev, int activity );
int LookupSequence( void *pmodel, const char *label );
void GetSequenceInfo( void *pmodel, entvars_t *pev, float *pflFrameRate, float *pflGroundSpeed );
int GetSequenceFlags( void *pmodel, entvars_t *pev );
int LookupAnimationEvents( void *pmodel, entvars_t *pev, float flStart, float flEnd );
float SetController( void *pmodel, entvars_t *pev, int iController, float flValue );
float SetBlending( void *pmodel, entvars_t *pev, int iBlender, float flValue );
void GetEyePosition( void *pmodel, float *vecEyePosition );
int FindTransition( void *pmodel, int iEndingAnim, int iGoalAnim, int *piDir );
void SetBodygroup( void *pmodel, entvars_t *pev, int iGroup, int iValue );
int GetBodygroup( void *pmodel, entvars_t *pev, int iGroup );

int GetAnimationEvent( void *pmodel, entvars_t *pev, MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index );
int ExtractBbox( void *pmodel, int sequence, float *mins, float *maxs );
float SequenceDuration( void *pmodel, int iSequence );

// From /engine/studio.h
#define STUDIO_LOOPING		0x0001


#endif	//ANIMATION_H
