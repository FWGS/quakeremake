// bsp2hl.c

#include "bsp5.h"

qboolean convert_bsp = false;

/*
==================
BspInfo_PrintInfo
==================
*/
static void Bsp_Convert( const char *filename )
{
	convert_bsp = true;

	LoadBSPFile( filename_bsp );
	WriteBSPFile( filename_bsp, false );
}

/*
==================
Bsp2HL_Main
==================
*/
int Bsp2HL_Main( int argc, char **argv )
{
	if( argc < 2 ) {
		Error ("%s",
"usage: hmap2 -tohl bspfile\n"
"Prints information about a .bsp file\n"
		);
	}

	// init memory
	Q_InitMem ();

	Bsp_Convert( argv[argc-1] );

	// free allocated memory
	Q_ShutdownMem ();

	return 0;
}