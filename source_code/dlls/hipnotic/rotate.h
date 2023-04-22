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
#ifndef ROTATE_H
#define ROTATE_H

enum e_rotates
{
	OBJECT_ROTATE = 0,
	OBJECT_MOVEWALL,
	OBJECT_SETORIGIN,
};

enum e_rot_state
{
	STATE_ACTIVE = 0,
	STATE_INACTIVE,
	STATE_SPEEDINGUP,
	STATE_SLOWINGDOWN,
	STATE_CLOSED,
	STATE_OPEN,
	STATE_OPENING,
	STATE_CLOSING,
};

enum e_train_state
{
	STATE_WAIT = 0,
	STATE_MOVE,
	STATE_STOP,
	STATE_FIND,
	STATE_NEXT,
};

// func_rotate_door
#define SF_DOOR_STAYOPEN		1

// func_rotate_entity
#define SF_ROTATE_TOGGLE		1
#define SF_ROTATE_START_ON		2

// func_movewall
#define SF_MOVEWALL_VISIBLE		1
#define SF_MOVEWALL_TOUCH		2
#define SF_MOVEWALL_NONBLOCKING	4	// g-cont. non-solid but can touched

#define SF_PATH_ROTATION		1
#define SF_PATH_ANGLES		2
#define SF_PATH_STOP		4
#define SF_PATH_NO_ROTATE		8
#define SF_PATH_DAMAGE		16
#define SF_PATH_MOVETIME		32
#define SF_PATH_SET_DAMAGE		64

#define SetThink2( a ) m_pfnThink2 = static_cast <void (CFuncRotateTrain::*)(void)> (a)

#endif//ROTATE_H