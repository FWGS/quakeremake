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
//  hud.h
//
// class CHud declaration
//
// CHud handles the message, calculation, and drawing the HUD
//


#define RGB_YELLOWISH 0x00FFFFFF //255,160,0
#define RGB_REDISH 0x00FF1010 //255,160,0
#define RGB_GREENISH 0x0000A000 //0,160,0

#include "wrect.h"
#include "cl_dll.h"

#define DHN_DRAWZERO 1
#define DHN_2DIGITS  2
#define DHN_3DIGITS  4
#define MIN_ALPHA	 100	
#define SBAR_HEIGHT  24

enum 
{ 
	MAX_PLAYERS = 64,
	MAX_TEAMS = 64,
	MAX_TEAM_NAME = 16,
};

typedef struct cvar_s cvar_t;

typedef struct
{
	int	destcolor[3];
	int	percent;		// 0-256
} cshift_t;

#define CSHIFT_CONTENTS	0
#define CSHIFT_DAMAGE	1
#define CSHIFT_BONUS	2
#define CSHIFT_POWERUP	3
#define NUM_CSHIFTS		4

#define HUD_ACTIVE		1
#define HUD_INTERMISSION	2

#define MAX_PLAYER_NAME_LENGTH	32

//
//-----------------------------------------------------
//
class CHudBase
{
public:
	int m_iFlags; // active, moving, 
	virtual ~CHudBase() {}
	virtual int Init( void ) {return 0;}
	virtual int VidInit( void ) {return 0;}
	virtual int Draw(float flTime) {return 0;}
	virtual void Think(void) {return;}
	virtual void Reset(void) {return;}
	virtual void InitHUDData( void ) {}		// called every time a server is connected to

};

struct HUDLIST
{
	CHudBase	*p;
	HUDLIST	*pNext;
};

//
//-----------------------------------------------------
//
class CHudSBar: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);

	int MsgFunc_Stats( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_Items( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_LevelName( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_FoundSecret( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_KillMonster( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_Finale( const char *pszName, int iSize, void *pbuf );

	// drawing funcs
	void DrawPic( int x, int y, int pic );
	void DrawTransPic( int x, int y, int pic );
	void DrawString( int x, int y, char *str );
	void DrawNum( int x, int y, int num, int digits, int color );
	void DrawCharacter( int x, int y, int num );

	void DrawFace( float flTime );
	void DrawScoreBoard( float flTime );
	void DrawInventory( float flTime );
	void DrawIntermission( float flTime );
	void DrawFinale( float flTime );
private:
	int	sb_ibar;
	int	sb_sbar;
	int	sb_scorebar;

	int	sb_nums[2][11];
	int	sb_colon, sb_slash;

	int	sb_weapons[7][8];   // 0 is active, 1 is owned, 2-5 are flashes
	int	sb_ammo[4];
	int	sb_sigil[4];
	int	sb_armor[3];
	int	sb_items[32];

	int	sb_faces[7][2];		// 0 is gibbed, 1 is dead, 2-6 are alive
					// 0 is static, 1 is temporary animation
	int	sb_face_invis;
	int	sb_face_quad;
	int	sb_face_invuln;
	int	sb_face_invis_invuln;

	// intermission stuff
	int	sb_completed;
	int	sb_finale;
	int	sb_inter;

	char	levelname[80];
	float	completed_time;
#ifdef HIPNOTIC

	// hipnotic items
	int	hsb_weapons[7][5];
	int	hsb_items[2];
	int	hipweapons[4];
#endif /* HIPNOTIC */
};

//
//-----------------------------------------------------
//
class CHudScoreboard: public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int DrawPlayers( int xoffset, float listslot, int nameoffset = 0, const char *team = NULL ); // returns the ypos where it finishes drawing
	int MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf );
	void DeathMsg( int killer, int victim );

	int m_iNumTeams;

	int m_iLastKilledBy;
	int m_fLastKillTime;
	int m_iPlayerNum;
	int m_iShowscoresHeld;

	void GetAllPlayersInfo( void );
};

struct extra_player_info_t 
{
	short frags;
	short deaths;
	short playerclass;
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
};

struct team_info_t 
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ping;
	short packetloss;
	short ownteam;
	short players;
	int already_drawn;
	int scores_overriden;
	int teamnumber;
};

extern hud_player_info_t	g_PlayerInfoList[MAX_PLAYERS+1];	// player info from the engine
extern extra_player_info_t	g_PlayerExtraInfo[MAX_PLAYERS+1];	// additional player info sent directly to the client dll
extern team_info_t		g_TeamInfo[MAX_TEAMS+1];

//
//-----------------------------------------------------
//
class CHudDeathNotice : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf );

private:
	int m_HUD_d_skull;  // sprite index of skull icon
};

//
//-----------------------------------------------------
//
class CHudSayText : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_SayText( const char *pszName, int iSize, void *pbuf );
	void SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex = -1 );
	void EnsureTextFitsInOneLineAndWrapIfHaveTo( int line );
private:
	struct cvar_s *m_HUD_saytext;
	struct cvar_s *m_HUD_saytext_time;
};

//
//-----------------------------------------------------
//
const int maxHUDMessages = 16;
struct message_parms_t
{
	client_textmessage_t *pMessage;
	float	time;
	int	x, y;
	int	totalWidth, totalHeight;
	int	width;
	int	lines;
	int	lineLength;
	int	length;
	int	r, g, b;
	int	text;
	int	fadeBlend;
	float	charTime;
	float	fadeTime;
};

//
//-----------------------------------------------------
//
class CHudTextMessage: public CHudBase
{
public:
	int Init( void );
	static char *LocaliseTextString( const char *msg, char *dst_buffer, int buffer_size );
	static char *BufferedLocaliseTextString( const char *msg );
	char *LookupString( const char *msg_name, int *msg_dest = NULL );
	int MsgFunc_TextMsg(const char *pszName, int iSize, void *pbuf);
};

//
//-----------------------------------------------------
//
class CHudMessage: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_HudText(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_GameTitle(const char *pszName, int iSize, void *pbuf);

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time );
	void MessageAdd(client_textmessage_t * newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );

private:
	client_textmessage_t	*m_pMessages[maxHUDMessages];
	float			m_startTime[maxHUDMessages];
	message_parms_t		m_parms;
	float			m_gameTitleTime;
	client_textmessage_t	*m_pGameTitle;

	int m_HUD_title_life;
	int m_HUD_title_half;
	int m_index;		// g-cont. custom message index
};

//
//-----------------------------------------------------
//
#define MAX_SPRITE_NAME_LENGTH	24

//
//-----------------------------------------------------
//



class CHud
{
private:
	HUDLIST		*m_pHudList;
	client_sprite_t	*m_pSpriteList;
	int		m_iSpriteCount;
	int		m_iSpriteCountAllRes;
	float		m_flMouseSensitivity;
public:

	float		m_flTime;	   // the current client time
	float		m_fOldTime;  // the time at which the HUD was last redrawn
	double		m_flTimeDelta; // the difference between flTime and fOldTime
	Vector		m_vecOrigin;
	Vector		m_vecAngles;
	int		m_iKeyBits;
	int		m_iHideHUDDisplay;
	int		m_iFOV;
	int		m_Teamplay;
	int		m_iRes;
	cvar_t		*m_pCvarStealMouse;
	cvar_t		*m_pCvarDraw;
	cvar_t		*m_pCvarCrosshair;
	
	int DrawHudString(int x, int y, int iMaxX, const char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, const char *szString, int r, int g, int b );
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
private:
	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	HSPRITE *m_rghSprites;	/*[HUD_SPRITE_COUNT]*/			// the sprites loaded from hud.txt
	wrect_t *m_rgrcRects;	/*[HUD_SPRITE_COUNT]*/
	char *m_rgszSpriteNames; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

	wrect_t nullrc;

	struct cvar_s *default_fov;
public:
	HSPRITE GetSprite( int index ) 
	{
		return (index < 0) ? 0 : m_rghSprites ? m_rghSprites[index] : 0;
	}

	wrect_t& GetSpriteRect( int index )
	{
		return m_rgrcRects ? m_rgrcRects[index] : nullrc;
	}

	int GetSpriteIndex( const char *SpriteName );	// gets a sprite index, for use in the m_rghSprites[] array

	CHudSBar		m_sbar;
	CHudMessage	m_Message;
	CHudScoreboard	m_Scoreboard;
	CHudDeathNotice	m_DeathNotice;
	CHudSayText	m_SayText;
	CHudTextMessage	m_TextMessage;

	void Init( void );
	void VidInit( void );
	void Think(void);
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	CHud() : m_iSpriteCount(0), m_pHudList(NULL) {}  
	~CHud();			// destructor, frees allocated memory

	// user messages
	int _cdecl MsgFunc_Damage(const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_ResetHUD(const char *pszName,  int iSize, void *pbuf);
	void _cdecl MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf );
	void _cdecl MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_HideHUD( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf);
	int  _cdecl MsgFunc_TempEntity( const char *pszName, int iSize, void *pbuf );

	inline bool ClientInvisible( void ) { return (items & IT_INVISIBILITY) ? true : false; }

	// Screen information
	SCREENINFO	m_scrinfo;

	int	m_iWeaponBits;
	int	m_fPlayerDead;
	int	m_iIntermission;

	int	sb_lines;			// Quake sbar height
	int	showscores;		// show scoreboard

	// information for local display
	int		stats[MAX_STATS];		// health, etc
	unsigned int	items;			// inventory bit flags
	float		item_gettime[MAX_STATS];	// cl.time of aquiring item, for blinking
	float		faceanimtime;		// use anim frame if cl.time < this

	cshift_t	cshifts[NUM_CSHIFTS];	// color shifts for damage, powerups
	cshift_t	prev_cshifts[NUM_CSHIFTS];	// and content types

	void AddHudElem(CHudBase *p);

	float GetSensitivity();

};

extern CHud gHUD;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;
