#include "cmdlib.h"
#include "mem.h"

static qboolean mem_initialized = false;

typedef struct chunkheader_s
{
	// next and previous memheaders in chain belonging to pool
	struct chunkheader_s *next;
	struct chunkheader_s *prev;

	// size of the memory after the header (excluding header and sentinel2)
	size_t size;

	// immediately followed by data
} chunkheader_t;

static chunkheader_t *chunks;
static size_t total_allocated;
static size_t total_active, total_peakactive;

void Q_InitMem( void )
{
	if( mem_initialized )
		return;

	chunks = NULL;
	mem_initialized = true;
	total_allocated = 0;
	total_active = 0;
	total_peakactive = 0;
}

void Q_PrintMem( void )
{
	if( !mem_initialized )
		return;

	printf( "Active memory: %f MB (%u bytes)\n", total_active  / 1048576.0, (unsigned int)total_active );
	printf( "Peak active memory: %f MB (%u bytes)\n", total_peakactive  / 1048576.0, (unsigned int)total_peakactive );
	printf( "Total memory allocated: %f MB (%u bytes)\n", total_allocated  / 1048576.0, (unsigned int)total_allocated );
}

void Q_ShutdownMem( void )
{
	size_t total;

	if( !mem_initialized )
		return;

	total = 0;
	while( chunks ) {
		total += chunks->size;
		qfree( (void *)((unsigned char *)chunks + sizeof(chunkheader_t)) );
	}

//	printf( "Leaked memory: %f MB (%i bytes)\n", total  / 1048576.0, total );
//	printf( "Total memory allocated: %f MB (%i bytes)\n", total_allocated  / 1048576.0, total_allocated );

	mem_initialized = false;
}

void *qmalloc( size_t size )
{
	void *data;
	chunkheader_t *chunk;

	if( size <= 0 )
		return NULL;

	if( mem_initialized )
		data = malloc( sizeof(chunkheader_t) + size );
	else
		data = malloc( size );

	if( !data )
		Error( "Failed on allocating %d bytes\n", size );

	if( !mem_initialized )
		return data;

	chunk = ( chunkheader_t * )data;
	chunk->size = size;
	chunk->next = chunks;
	chunk->prev = NULL;
	chunks = chunk;
	if (chunk->next)
		chunk->next->prev = chunk;

	total_allocated += size;
	total_active += size;
	if( total_active > total_peakactive )
		total_peakactive = total_active;

	return ( void * )(( unsigned char * )data + sizeof(chunkheader_t) );
}

void qfree( void *data )
{
	chunkheader_t *chunk;

	if( !data )
		return;
	if( !mem_initialized ) {
		free( data );
		return;
	}

	chunk = ( chunkheader_t * )(( unsigned char * )data - sizeof(chunkheader_t) );
	if( chunk->prev )
		chunk->prev->next = chunk->next;
	else
		chunks = chunk->next;
	if( chunk->next )
		chunk->next->prev = chunk->prev;

	total_active -= chunk->size;

	free( chunk );
}
