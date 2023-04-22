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
//
// Train.cpp
//
// implementation of CHudAmmo class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "ref_params.h"

#define STAT_MINUS		10 // num frame for '-' stats digit

DECLARE_MESSAGE( m_sbar, Stats )
DECLARE_MESSAGE( m_sbar, Items )
DECLARE_MESSAGE( m_sbar, LevelName )
DECLARE_MESSAGE( m_sbar, FoundSecret )
DECLARE_MESSAGE( m_sbar, KillMonster )
DECLARE_MESSAGE( m_sbar, Finale )

int CHudSBar::Init(void)
{
	HOOK_MESSAGE( Stats );
	HOOK_MESSAGE( Items );
	HOOK_MESSAGE( LevelName );
	HOOK_MESSAGE( FoundSecret );
	HOOK_MESSAGE( KillMonster );
	HOOK_MESSAGE( Finale );

	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);

	return 1;
};

int CHudSBar::VidInit(void)
{
	int i;

	memset( gHUD.item_gettime, 0, sizeof( gHUD.item_gettime ));

	for( i = 0; i < 10; i++ )
	{
		sb_nums[0][i] = gHUD.GetSpriteIndex (UTIL_VarArgs("num_%i",i));
		sb_nums[1][i] = gHUD.GetSpriteIndex (UTIL_VarArgs("anum_%i",i));
	}

	sb_nums[0][10] = gHUD.GetSpriteIndex ("number_minus");
	sb_nums[1][10] = gHUD.GetSpriteIndex ("anumber_minus");

	sb_colon = gHUD.GetSpriteIndex ("num_colon");
	sb_slash = gHUD.GetSpriteIndex ("num_slash");

	sb_weapons[0][0] = gHUD.GetSpriteIndex ("inv_shotgun");
	sb_weapons[0][1] = gHUD.GetSpriteIndex ("inv_sshotgun");
	sb_weapons[0][2] = gHUD.GetSpriteIndex ("inv_nailgun");
	sb_weapons[0][3] = gHUD.GetSpriteIndex ("inv_snailgun");
	sb_weapons[0][4] = gHUD.GetSpriteIndex ("inv_rlaunch");
	sb_weapons[0][5] = gHUD.GetSpriteIndex ("inv_srlaunch");
	sb_weapons[0][6] = gHUD.GetSpriteIndex ("inv_lightng");

	sb_weapons[1][0] = gHUD.GetSpriteIndex ("inv2_shotgun");
	sb_weapons[1][1] = gHUD.GetSpriteIndex ("inv2_sshotgun");
	sb_weapons[1][2] = gHUD.GetSpriteIndex ("inv2_nailgun");
	sb_weapons[1][3] = gHUD.GetSpriteIndex ("inv2_snailgun");
	sb_weapons[1][4] = gHUD.GetSpriteIndex ("inv2_rlaunch");
	sb_weapons[1][5] = gHUD.GetSpriteIndex ("inv2_srlaunch");
	sb_weapons[1][6] = gHUD.GetSpriteIndex ("inv2_lightng");

	for( i = 0; i < 5; i++ )
	{
		sb_weapons[2+i][0] = gHUD.GetSpriteIndex (UTIL_VarArgs("inva%i_shotgun",i+1));
		sb_weapons[2+i][1] = gHUD.GetSpriteIndex (UTIL_VarArgs("inva%i_sshotgun",i+1));
		sb_weapons[2+i][2] = gHUD.GetSpriteIndex (UTIL_VarArgs("inva%i_nailgun",i+1));
		sb_weapons[2+i][3] = gHUD.GetSpriteIndex (UTIL_VarArgs("inva%i_snailgun",i+1));
		sb_weapons[2+i][4] = gHUD.GetSpriteIndex (UTIL_VarArgs("inva%i_rlaunch",i+1));
		sb_weapons[2+i][5] = gHUD.GetSpriteIndex (UTIL_VarArgs("inva%i_srlaunch",i+1));
		sb_weapons[2+i][6] = gHUD.GetSpriteIndex (UTIL_VarArgs("inva%i_lightng",i+1));
	}

	sb_ammo[0] = gHUD.GetSpriteIndex ("sb_shells");
	sb_ammo[1] = gHUD.GetSpriteIndex ("sb_nails");
	sb_ammo[2] = gHUD.GetSpriteIndex ("sb_rocket");
	sb_ammo[3] = gHUD.GetSpriteIndex ("sb_cells");

	sb_armor[0] = gHUD.GetSpriteIndex ("sb_armor1");
	sb_armor[1] = gHUD.GetSpriteIndex ("sb_armor2");
	sb_armor[2] = gHUD.GetSpriteIndex ("sb_armor3");

	sb_items[0] = gHUD.GetSpriteIndex ("sb_key1");
	sb_items[1] = gHUD.GetSpriteIndex ("sb_key2");
	sb_items[2] = gHUD.GetSpriteIndex ("sb_invis");
	sb_items[3] = gHUD.GetSpriteIndex ("sb_invuln");
	sb_items[4] = gHUD.GetSpriteIndex ("sb_suit");
	sb_items[5] = gHUD.GetSpriteIndex ("sb_quad");

	sb_sigil[0] = gHUD.GetSpriteIndex ("sb_sigil1");
	sb_sigil[1] = gHUD.GetSpriteIndex ("sb_sigil2");
	sb_sigil[2] = gHUD.GetSpriteIndex ("sb_sigil3");
	sb_sigil[3] = gHUD.GetSpriteIndex ("sb_sigil4");

	sb_faces[4][0] = gHUD.GetSpriteIndex ("face1");
	sb_faces[4][1] = gHUD.GetSpriteIndex ("face_p1");
	sb_faces[3][0] = gHUD.GetSpriteIndex ("face2");
	sb_faces[3][1] = gHUD.GetSpriteIndex ("face_p2");
	sb_faces[2][0] = gHUD.GetSpriteIndex ("face3");
	sb_faces[2][1] = gHUD.GetSpriteIndex ("face_p3");
	sb_faces[1][0] = gHUD.GetSpriteIndex ("face4");
	sb_faces[1][1] = gHUD.GetSpriteIndex ("face_p4");
	sb_faces[0][0] = gHUD.GetSpriteIndex ("face5");
	sb_faces[0][1] = gHUD.GetSpriteIndex ("face_p5");

	sb_face_invis = gHUD.GetSpriteIndex ("face_invis");
	sb_face_invuln = gHUD.GetSpriteIndex ("face_invul2");
	sb_face_invis_invuln = gHUD.GetSpriteIndex ("face_inv2");
	sb_face_quad = gHUD.GetSpriteIndex ("face_quad");

	sb_sbar = gHUD.GetSpriteIndex ("sbar");
	sb_ibar = gHUD.GetSpriteIndex ("ibar");
	sb_scorebar = gHUD.GetSpriteIndex ("scorebar");

	sb_completed = gHUD.GetSpriteIndex ("complete");
	sb_finale = gHUD.GetSpriteIndex ("finale");
	sb_inter = gHUD.GetSpriteIndex ("intermission");

	m_iFlags |= HUD_INTERMISSION;	// g-cont. allow episode finales

	return 1;
};

void CHudSBar::DrawPic( int x, int y, int pic )
{
	if (!gHUD.m_iIntermission)
	{
		x += ((ScreenWidth - 320)>>1);
		y += (ScreenHeight - SBAR_HEIGHT);
	}

	SPR_Set( gHUD.GetSprite( pic ), 255, 255, 255 );
	SPR_Draw( 0,  x, y, &gHUD.GetSpriteRect( pic ));
}

void CHudSBar::DrawTransPic( int x, int y, int pic )
{
	if (!gHUD.m_iIntermission)
	{
		x += ((ScreenWidth - 320)>>1);
		y += (ScreenHeight - SBAR_HEIGHT);
	}

	SPR_Set( gHUD.GetSprite( pic ), 255, 255, 255 );
	SPR_DrawHoles( 0, x, y, &gHUD.GetSpriteRect( pic ));
}

void CHudSBar::DrawNum( int x, int y, int num, int digits, int color )
{
	char	str[12];
	char	*ptr;
	int	l, frame;

	l = UTIL_IntegerToString( num, str );
	ptr = str;

	if( l > digits ) ptr += (l - digits);
	if( l < digits ) x += (digits - l) * 24;

	while( *ptr )
	{
		if( *ptr == '-' )
			frame = STAT_MINUS;
		else frame = *ptr -'0';

		DrawTransPic( x, y, sb_nums[color][frame] );
		x += 24;
		ptr++;
	}
}

void CHudSBar::DrawString( int x, int y, char *str )
{
	gEngfuncs.pfnDrawSetTextColor( 0.5, 0.5, 0.5 );
	DrawConsoleString( x + (( ScreenWidth - 320 ) >> 1), y + ScreenHeight - SBAR_HEIGHT, str );
}

void CHudSBar::DrawCharacter( int x, int y, int num )
{
	char str[3];

	str[0] = (char)num;
	str[1] = '\0';

	DrawString( x, y, str );
}

void CHudSBar::DrawFace( float flTime )
{
	if(( gHUD.items & (IT_INVISIBILITY | IT_INVULNERABILITY)) == (IT_INVISIBILITY|IT_INVULNERABILITY))
	{
		DrawPic( 112, 0, sb_face_invis_invuln );
		return;
	}

	if( gHUD.items & IT_QUAD )
	{
		DrawPic( 112, 0, sb_face_quad );
		return;
	}

	if( gHUD.items & IT_INVISIBILITY)
	{
		DrawPic (112, 0, sb_face_invis );
		return;
	}

	if( gHUD.items & IT_INVULNERABILITY )
	{
		DrawPic( 112, 0, sb_face_invuln );
		return;
	}

	int f, anim;

	if (gHUD.stats[STAT_HEALTH] >= 100)
		f = 4;
	else f = gHUD.stats[STAT_HEALTH] / 20;

	if (flTime <= gHUD.faceanimtime)
		anim = 1;
	else
		anim = 0;

	DrawPic( 112, 0, sb_faces[f][anim] );
}

void CHudSBar::DrawScoreBoard( float flTime )
{
	char str[80];
	int l, minutes, seconds, tens, units;

	sprintf( str, "Monsters:%3i /%3i", gHUD.stats[STAT_MONSTERS], gHUD.stats[STAT_TOTALMONSTERS] );
	DrawString( 8, 1, str );

	sprintf( str, "Secrets :%3i /%3i", gHUD.stats[STAT_SECRETS], gHUD.stats[STAT_TOTALSECRETS] );
	l = ConsoleStringLen( str );
	DrawString( 8, 10, str );

	// time
	minutes = flTime / 60;
	seconds = flTime - 60 * minutes;
	tens = seconds / 10;
	units = seconds - 10 * tens;
	sprintf( str, "Time :%3i:%i%i", minutes, tens, units );
	DrawString( 184, 1, str );

	// draw level name
	DrawString( 16 + l, 10, levelname );
}

void CHudSBar::DrawIntermission( float flTime )
{
	int dig, num;

	DrawPic((ScreenWidth*0.5f)-94, (ScreenHeight*0.5f) - 96, sb_completed );
	DrawTransPic((ScreenWidth*0.5f)-160, (ScreenHeight*0.5f) - 64, sb_inter );

	// time
	dig = completed_time / 60;
	DrawNum(ScreenWidth*0.5f, (ScreenHeight*0.5f) - 56, dig, 3, 0 );
	num = completed_time - dig * 60;
	DrawTransPic((ScreenWidth*0.5f)+74 ,(ScreenHeight*0.5f)- 56, sb_colon );
	DrawTransPic((ScreenWidth*0.5f)+86 ,(ScreenHeight*0.5f)- 56, sb_nums[0][num/10] );
	DrawTransPic((ScreenWidth*0.5f)+106,(ScreenHeight*0.5f)- 56, sb_nums[0][num%10] );

	DrawNum((ScreenWidth*0.5f), (ScreenHeight*0.5f)- 16, gHUD.stats[STAT_SECRETS], 3, 0 );
	DrawTransPic((ScreenWidth*0.5f)+72,(ScreenHeight*0.5f)- 16, sb_slash );
	DrawNum((ScreenWidth*0.5f)+80, (ScreenHeight*0.5f)- 16, gHUD.stats[STAT_TOTALSECRETS], 3, 0 );

	DrawNum((ScreenWidth*0.5f), (ScreenHeight*0.5f)+ 24, gHUD.stats[STAT_MONSTERS], 3, 0 );
	DrawTransPic((ScreenWidth*0.5f)+72,(ScreenHeight*0.5f)+ 24, sb_slash );
	DrawNum((ScreenWidth*0.5f)+80, (ScreenHeight*0.5f)+ 24, gHUD.stats[STAT_TOTALMONSTERS], 3, 0 );
}

void CHudSBar::DrawFinale( float flTime )
{
	DrawTransPic((ScreenWidth-288)*0.5f, 16, sb_finale );
}

void CHudSBar::DrawInventory( float flTime )
{
	char	num[6];
	float	time;
	int	i, flashon;

	DrawPic( 0, -24, sb_ibar );

	// weapons
	for( i = 0; i < 7; i++ )
	{
		if( gHUD.items & ( IT_SHOTGUN << i ))
		{
			time = gHUD.item_gettime[i];
			flashon = (int)((flTime - time) * 10);
			if( flashon >= 10 )
			{
				if( gHUD.stats[STAT_ACTIVEWEAPON] == (IT_SHOTGUN << i))
					flashon = 1;
				else flashon = 0;
			}
			else flashon = (flashon % 5) + 2;

			DrawPic( i * 24, -16, sb_weapons[flashon][i] );
		}
	}

	// ammo counts
	for( i = 0; i < 4; i++ )
	{
		sprintf( num, "%3i", gHUD.stats[STAT_SHELLS+i] );

		if (num[0] != ' ')
			DrawCharacter((6 * i + 1) * 8 - 2, -26, num[0] );
		if (num[1] != ' ')
			DrawCharacter((6 * i + 2) * 8 - 2, -26, num[1] );
		if (num[2] != ' ')
			DrawCharacter((6 * i + 3) * 8 - 2, -26, num[2] );
	}

	flashon = 0;

	// items
	for( i = 0; i < 6; i++ )
          {
		if (gHUD.items & (1<<(17 + i)))
		{
			time = gHUD.item_gettime[17+i];

			if( time && time > flTime - 2 && flashon )
         			{
         				// flash frame
			}
			else
			{
				//MED 01/04/97 changed keys
				DrawPic( 192 + i * 16, -16, sb_items[i] );
			}
		}
	}

	// sigils
	for( i = 0; i < 4; i++ )
	{
		if (gHUD.items & (1<<( 28 + i )))
		{
			time = gHUD.item_gettime[28+i];
			if( time && time > flTime - 2 && flashon )
         			{
         				// flash frame
			}
			else
			{
				DrawPic( 320 - 32 + i * 8, -16, sb_sigil[i] );
			}
		}
	}
}

int CHudSBar::Draw(float fTime)
{
	if (!gHUD.m_iIntermission)
	{
		completed_time = fTime;
	}
	else if (gHUD.m_iIntermission == 1)
	{
		DrawIntermission( fTime );
		return 1;
	}
	else if (gHUD.m_iIntermission == 2)
	{
		DrawFinale( fTime );
		return 1;
	}

	if( gHUD.m_iHideHUDDisplay & HIDEHUD_HUD )
		return 1;

	if (gHUD.sb_lines > 24)
	{
		DrawInventory( fTime );
	}

	if ((gEngfuncs.GetMaxClients() == 1 ) && (gHUD.showscores || gHUD.stats[STAT_HEALTH] <= 0))
	{
		if (gHUD.sb_lines)
			DrawPic( 0, 0, sb_scorebar );
		DrawScoreBoard ( fTime );
	}
	else if (gHUD.sb_lines)
	{
		DrawPic( 0, 0, sb_sbar );
	}

	if ((gEngfuncs.GetMaxClients() == 1 ) && (gHUD.showscores || gHUD.stats[STAT_HEALTH] <= 0))
		return 1;

	DrawFace( fTime );

	// health
	DrawNum( 136, 0, gHUD.stats[STAT_HEALTH], 3, gHUD.stats[STAT_HEALTH] <= 25 );

	// armor
	if (gHUD.items & IT_INVULNERABILITY)
	{
		DrawNum( 24, 0, 666, 3, 1 );
		DrawPic( 0, 0, sb_items[3] );
	}
	else
	{
		DrawNum( 24, 0, gHUD.stats[STAT_ARMOR], 3, gHUD.stats[STAT_ARMOR] <= 25 );

		if (gHUD.items & IT_ARMOR3)
			DrawPic( 0, 0, sb_armor[2] );
		else if (gHUD.items & IT_ARMOR2)
			DrawPic( 0, 0, sb_armor[1] );
		else if (gHUD.items & IT_ARMOR1)
			DrawPic( 0, 0, sb_armor[0] );
	}

	// ammo icon
	if (gHUD.items & IT_SHELLS)
		DrawPic( 224, 0, sb_ammo[0] );
	else if (gHUD.items & IT_NAILS)
		DrawPic( 224, 0, sb_ammo[1] );
	else if (gHUD.items & IT_ROCKETS)
		DrawPic( 224, 0, sb_ammo[2] );
	else if (gHUD.items & IT_CELLS)
		DrawPic( 224, 0, sb_ammo[3] );

	// g-cont. hide ammo count on axe
	if (gHUD.stats[STAT_ACTIVEWEAPON] != IT_AXE)
		DrawNum( 248, 0, gHUD.stats[STAT_AMMO], 3, gHUD.stats[STAT_AMMO] <= 10 );

	return 1;
}

int CHudSBar::MsgFunc_Stats(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int statnum = READ_BYTE();

	if( statnum < 0 || statnum >= MAX_STATS )
	{
		gEngfuncs.Con_Printf( "gmsgStats: bad stat %i\n", statnum );
		return 0;
	}

	// update selected stat
	gHUD.stats[statnum] = (unsigned int)READ_SHORT();

	return 1;
}

int CHudSBar::MsgFunc_Items(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	unsigned int newItems = (unsigned int)READ_LONG();

	if (gHUD.items != newItems)
	{
		// set flash times
		for( int i = 0; i < 32; i++ )
			if(( newItems & ( 1<<i )) && !( gHUD.items & ( 1<<i )))
				gHUD.item_gettime[i] = gEngfuncs.GetClientTime();
		gHUD.items = newItems;
	}

	return 1;
}

int CHudSBar::MsgFunc_LevelName( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	strncpy( levelname, READ_STRING(), sizeof( levelname ) - 1 );

	return 1;
}

int CHudSBar::MsgFunc_FoundSecret( const char *pszName, int iSize, void *pbuf )
{
	gHUD.stats[STAT_SECRETS]++;
	return 1;
}

int CHudSBar::MsgFunc_KillMonster( const char *pszName, int iSize, void *pbuf )
{
	gHUD.stats[STAT_MONSTERS]++;
	return 1;
}

int CHudSBar::MsgFunc_Finale( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if (gpViewParams)
		gpViewParams->intermission = 2;

	return 1;
}