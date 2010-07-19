
/*********************************************************************
 * FILE: mem.C                                                       *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   31 MAY 93 -- (mlm) created                                      *
 *   21 FEB 95 -- (mlm) changed various declarations from int to     *
 *                long or size_t, for portability, added CANT_RECUR  *
 *                to new() and delete()                              *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for memory management.  See mem.H for details.         *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *   For magic numbers, I use long values that are easy to recognize *
 *   in hex (0xDEADBEEF, etc.).  This should make them easy to spot  *
 *   when looking through memory dumps with a debugger.              *
 *                                                                   *
 * ----------------------------------------------------------------- *
 *                                                                   *
 *             Copyright (c) 1993, NEC Research Institute            *
 *                       All Rights Reserved.                        *
 *                                                                   *
 *   Permission to use, copy, and modify this software and its       *
 *   documentation is hereby granted only under the following terms  *
 *   and conditions.  Both the above copyright notice and this       *
 *   permission notice must appear in all copies of the software,    *
 *   derivative works or modified versions, and any portions         *
 *   thereof, and both notices must appear in supporting             *
 *   documentation.                                                  *
 *                                                                   *
 *   Correspondence should be directed to NEC at:                    *
 *                                                                   *
 *                     Ingemar J. Cox                                *
 *                                                                   *
 *                     NEC Research Institute                        *
 *                     4 Independence Way                            *
 *                     Princeton                                     *
 *                     NJ 08540                                      *
 *                                                                   *
 *                     phone:  609 951 2722                          *
 *                     fax:  609 951 2482                            *
 *                     email:  ingemar@research.nj.nec.com (Inet)    *
 *                                                                   *
 *********************************************************************/

#define DECLARE_MEM

#include <cstdlib>
#include <malloc.h>		// for malloc()
#include <new>			// for std::set_new_handler()

#include "mem.h"

/*-------------------------------------------------------------------*
 | Data structures
 *-------------------------------------------------------------------*/

/* pointer to new() error handler routine */
typedef void (*HANDLER_FOR_NEW)();

/* header for each group of chunks */
struct GROUP_HEADER
{
    struct GROUP_HEADER *next;
    int poolNum;
    int numChunks;
    size_t chunkSize;
};

/* header for each chunk */
struct CHUNK_HEADER
{
    union
    {
        void *next;
        void **pool;
    }
    link;

#ifdef DEBUG
    size_t size;
    void *prevAlloced;
    void *nextAlloced;
    long magicNumber;
#endif
};
static const long HEADER_ALLOCED = 0xCafeBabeL;
static const long HEADER_FREE =    0xDeadBeefL;

/* footer for each chunk (only if DEBUG is defined) */
#ifdef DEBUG
struct CHUNK_FOOTER
{
    long magicNumber;
};
static const long FOOTER_ALLOCED = 0xAddedFeeL;
static const long FOOTER_FREE =    0xFadedBedL;
#endif

/*-------------------------------------------------------------------*
 | Global variables
 *-------------------------------------------------------------------*/

/* array of memory pools */
static const int CHUNK_SLACK_LOG2 = 5;
static const int CHUNK_SLACK = (1 << CHUNK_SLACK_LOG2);
static const int NUM_POOLS = 257;
static const size_t MAX_CHUNK_SIZE = CHUNK_SLACK * (NUM_POOLS - 1);
static void *poolArray[ NUM_POOLS ];

/* predefined emergency memory */
static const int EMERGENCY_BUF_LONGS = 2048;
static long emergencyBuf[ EMERGENCY_BUF_LONGS ] =
{ EMERGENCY_BUF_LONGS };

/* heads of linked lists */
static GROUP_HEADER *g_firstGroup;
#ifdef DEBUG
static CHUNK_HEADER g_allocedList;
#endif

/*-------------------------------------------------------------------*
 | Static routine declarations
 *-------------------------------------------------------------------*/

/* routines for doing simple allocation and deallocation */
static inline int isSimpleMem( void *mem );
static inline void *simpleAlloc( size_t originalSize );
static inline void simpleDealloc( void *mem );

/* routines for doing allocation and deallocation with memory pools */
static inline void *chunkAlloc( size_t size );
static inline void chunkDealloc( void *mem );
static void *chunkGroupAlloc( int poolNum );
static void chunkCleanup();
static inline CHUNK_HEADER &chunkHeaderFor( void *mem );
#ifdef DEBUG
static inline CHUNK_FOOTER &chunkFooterFor( void *mem );
static inline void putOnAllocedList( void *mem );
static inline void takeOffAllocedList( void *mem );
#endif

/* routines for doing allocation and deallocation in emergencies */
static inline int isEmergencyMem( void *mem );
static void *emergencyAlloc( size_t size );
static inline void emergencyDealloc( void *mem );

/* routines for calling malloc() and free() */
static void *alloc( size_t size );
static inline void dealloc( void *mem );
static inline HANDLER_FOR_NEW getHandlerForNew();

/*-------------------------------------------------------------------*
 | operator new()
 *-------------------------------------------------------------------*/

void *operator new( size_t size )
{
    //CANT_RECUR


#ifdef TSTBUG
    assert( size > 0 );
    //    THROW_ERR( "Illegal size given to new() -- " << size )
#endif

    void *mem;

    G_numAllocations++;

    if( G_errHasHappened )
    {
        mem = emergencyAlloc( size );
    }
    else if( size > MAX_CHUNK_SIZE )
    {
        mem = simpleAlloc( size );
    }
    else
    {
        mem = chunkAlloc( size );
    }

    return mem;
}

/*-------------------------------------------------------------------*
 | operator delete()
 *-------------------------------------------------------------------*/

void operator delete( void *mem )
{
    //CANT_RECUR


    if( mem != 0 )
    {
        G_numDeallocations++;

        if( isEmergencyMem( mem ) )
        {
            emergencyDealloc( mem );
        }
        else if( isSimpleMem( mem ) )
        {
            simpleDealloc( mem );
        }
        else
        {
            chunkDealloc( mem );
        }
    }
}

/*-------------------------------------------------------------------*
 | CollectGarbage() -- free unused groups of memory chunks
 *-------------------------------------------------------------------*/

void CollectGarbage()
{


    chunkCleanup();
}

/*-------------------------------------------------------------------*
 | CheckMem() -- check magic numbers in headers and footers of
 |               memory chunks
 *-------------------------------------------------------------------*/

#ifdef DEBUG

void CheckMem()
{
    static char isBusy = 0;
    void *mem;
    void *nextMem;
    int i;

    if( isBusy )
    {
        return;
    }
    isBusy = 1;

    assert( poolArray[ 0 ] == 0 );
    //  THROW_ERR( "Pool for zero size chunks exists" )

    for( i = 1; i < NUM_POOLS; i++ )
        for( mem = poolArray[ i ]; mem != 0; mem = nextMem )
        {
            nextMem = chunkHeaderFor( mem ).link.next;

            assert( chunkHeaderFor( mem ).magicNumber == HEADER_FREE &&
            chunkFooterFor( mem ).magicNumber == FOOTER_FREE );
            //  THROW_ERR( "Corrupted heap at " << mem )
        }

    for( mem = g_allocedList.nextAlloced; mem != 0; mem = nextMem )
    {
        nextMem = chunkHeaderFor( mem ).nextAlloced;

        assert( chunkHeaderFor( mem ).magicNumber == HEADER_ALLOCED &&
        chunkFooterFor( mem ).magicNumber == FOOTER_ALLOCED );
        //  THROW_ERR( "Corrupted heap at " << mem )
    }

    isBusy = 0;
}

void CheckMem( void *mem )
{
    static char isBusy = 0;

    if( isBusy )
    {
        return;
    }
    isBusy = 1;

    long head;
    long foot;

    if( mem != 0 )
    {
        head = chunkHeaderFor( mem ).magicNumber;
        foot = chunkFooterFor( mem ).magicNumber;

        assert( (head == HEADER_ALLOCED || head == HEADER_FREE) &&
        (head != HEADER_ALLOCED || foot == FOOTER_ALLOCED) &&
        (head != HEADER_FREE || foot == FOOTER_FREE) );
        //  THROW_ERR( "Corrupted heap at " << mem );
    }

    isBusy = 0;
}

#endif

/*-------------------------------------------------------------------*
 | isSimpleMem() -- determine if memory was allocated with
 |                  simpleAlloc()
 *-------------------------------------------------------------------*/

static inline int isSimpleMem( void *mem )
{


    return chunkHeaderFor( mem ).link.pool == 0;
}

/*-------------------------------------------------------------------*
 | simpleAlloc() -- allocate memory without using memory pools
 *-------------------------------------------------------------------*/

static inline void *simpleAlloc( size_t originalSize )
{

    size_t size;
    void *mem;

#ifdef DEBUG
    originalSize += 4 - (originalSize % 4);
    size = sizeof( CHUNK_HEADER ) +
    originalSize +
    sizeof( CHUNK_FOOTER );
#else
    size = sizeof( CHUNK_HEADER ) + originalSize;
#endif

    mem = alloc( size );
    mem = (CHUNK_HEADER *)mem + 1;

    chunkHeaderFor( mem ).link.pool = 0;

#ifdef DEBUG
    chunkHeaderFor( mem ).size = originalSize;
    chunkHeaderFor( mem ).magicNumber = HEADER_ALLOCED;
    chunkFooterFor( mem ).magicNumber = FOOTER_ALLOCED;
    putOnAllocedList( mem );
#endif

    return mem;
}

/*-------------------------------------------------------------------*
 | simpleDealloc() -- deallocate memory that was allocated with
 |                    simpleAlloc()
 *-------------------------------------------------------------------*/

static inline void simpleDealloc( void *mem )
{


#ifdef DEBUG
    if( chunkHeaderFor( mem ).magicNumber != HEADER_ALLOCED ||
    chunkFooterFor( mem ).magicNumber != FOOTER_ALLOCED )
    {
        assert( chunkHeaderFor( mem ).magicNumber != HEADER_FREE ||
        chunkFooterFor( mem ).magicNumber != FOOTER_FREE );
        //  THROW_ERR( "Memory deallocated twice -- " << mem )

        assert(false);
        //THROW_ERR( "Corrupted heap or deallocating bad address at "
        //           << mem )
    }

    chunkHeaderFor( mem ).magicNumber = HEADER_FREE;
    chunkFooterFor( mem ).magicNumber = FOOTER_FREE;
    takeOffAllocedList( mem );
#endif

    dealloc( (CHUNK_HEADER *)mem - 1 );
}

/*-------------------------------------------------------------------*
 | chunkAlloc() -- allocate a chunk of memory using memory pools
 *-------------------------------------------------------------------*/

static inline void *chunkAlloc( size_t size )
{


    int poolNum;
    register void *mem;
    register void **pool;

    poolNum = size >> CHUNK_SLACK_LOG2;
    if( (size & (CHUNK_SLACK - 1)) != 0 )
    {
        poolNum++;
    }
    pool = &poolArray[ poolNum ];

    if( (mem = *pool) == 0 )
    {
        mem = chunkGroupAlloc( poolNum );
    }

#ifdef DEBUG
    assert( chunkHeaderFor( mem ).magicNumber == HEADER_FREE &&
    chunkFooterFor( mem ).magicNumber == FOOTER_FREE );
    //  THROW_ERR( "Corrupted heap at " << mem );
#endif

    *pool = chunkHeaderFor( mem ).link.next;

    chunkHeaderFor( mem ).link.pool = pool;

#ifdef DEBUG
    chunkHeaderFor( mem ).magicNumber = HEADER_ALLOCED;
    chunkFooterFor( mem ).magicNumber = FOOTER_ALLOCED;
    putOnAllocedList( mem );
#endif

    return mem;
}

/*-------------------------------------------------------------------*
 | chunkDealloc() -- deallocate a chunk of memory that was allocated
 |                   by chunkAlloc()
 *-------------------------------------------------------------------*/

static inline void chunkDealloc( void *mem )
{


#ifdef DEBUG
    if( chunkHeaderFor( mem ).magicNumber != HEADER_ALLOCED ||
    chunkFooterFor( mem ).magicNumber != FOOTER_ALLOCED )
    {
        assert( chunkHeaderFor( mem ).magicNumber != HEADER_FREE ||
        chunkFooterFor( mem ).magicNumber != FOOTER_FREE );
        //  THROW_ERR( "Memory deallocated twice" )

        assert(false);
        //THROW_ERR( "Corrupted heap or deallocating bad address at "
        //           << mem )
    }
#endif

    register void **pool;

    pool = chunkHeaderFor( mem ).link.pool;
    chunkHeaderFor( mem ).link.next = *pool;
    *pool = mem;

#ifdef DEBUG
    chunkHeaderFor( mem ).magicNumber = HEADER_FREE;
    chunkFooterFor( mem ).magicNumber = FOOTER_FREE;
    takeOffAllocedList( mem );
#endif
}

/*-------------------------------------------------------------------*
 | chunkGroupAlloc() -- allocate a group of memory chunks to fill up
 |                      a memory pool
 *-------------------------------------------------------------------*/

static void *chunkGroupAlloc( int poolNum )
{


    size_t originalSize;
    size_t chunkSize;
    size_t groupSize;
    int numChunksInGroup;
    GROUP_HEADER *groupHeader;
    void *firstMem;
    void *mem;
    int i;

    originalSize = poolNum << CHUNK_SLACK_LOG2;

#ifdef DEBUG
    chunkSize =
    sizeof( CHUNK_HEADER ) + originalSize + sizeof( CHUNK_FOOTER );
    numChunksInGroup = (sizeof( CHUNK_HEADER ) +
    MAX_CHUNK_SIZE +
    sizeof( CHUNK_FOOTER )) /
    chunkSize;
#else
    chunkSize = sizeof( CHUNK_HEADER ) + originalSize;
    numChunksInGroup = (sizeof( CHUNK_HEADER ) + MAX_CHUNK_SIZE) /
    chunkSize;
#endif

    groupSize = sizeof( GROUP_HEADER ) + numChunksInGroup * chunkSize;
    groupHeader = (GROUP_HEADER *)alloc( groupSize );
    firstMem = (CHUNK_HEADER *)(groupHeader + 1) + 1;

    groupHeader->next = g_firstGroup;
    groupHeader->poolNum = poolNum;
    groupHeader->numChunks = numChunksInGroup;
    groupHeader->chunkSize = chunkSize;
    g_firstGroup = groupHeader;

    mem = firstMem;
    for( i = 0; i < numChunksInGroup - 1; i++ )
    {
        chunkHeaderFor( mem ).link.next = (char *)mem + chunkSize;

#ifdef DEBUG
        chunkHeaderFor( mem ).size = originalSize;
        chunkHeaderFor( mem ).magicNumber = HEADER_FREE;
        chunkFooterFor( mem ).magicNumber = FOOTER_FREE;
#endif

        mem = (char *)mem + chunkSize;
    }

    chunkHeaderFor( mem ).link.next = 0;

#ifdef DEBUG
    chunkHeaderFor( mem ).size = originalSize;
    chunkHeaderFor( mem ).magicNumber = HEADER_FREE;
    chunkFooterFor( mem ).magicNumber = FOOTER_FREE;
#endif

    return firstMem;
}

/*-------------------------------------------------------------------*
 | chunkCleanup() -- deallocate groups of chunks that aren't being
 |                   used
 *-------------------------------------------------------------------*/

static void chunkCleanup()
{


    GROUP_HEADER *prevGroup;
    GROUP_HEADER *group;
    GROUP_HEADER *nextGroup;
    void *prevMem;
    void *mem;
    void *nextMem;
    void **pool;
    int flag;
    int i;

    /* find the groups that should be free()'ed */
    for( group = g_firstGroup; group != 0; group = nextGroup )
    {
        nextGroup = group->next;

        pool = &poolArray[ group->poolNum ];

        flag = 1;
        mem = (CHUNK_HEADER *)(group + 1) + 1;
        for( i = 0; i < group->numChunks; i++ )
        {
            if( chunkHeaderFor( mem ).link.pool == pool )
            {
                flag = 0;
                break;
            }
            mem = (char *)mem + group->chunkSize;
        }

        mem = (CHUNK_HEADER *)(group + 1) + 1;
        for( i = 0; i < group->numChunks; i++ )
        {
            if( flag == 1 || chunkHeaderFor( mem ).link.pool != pool )
            {
                *(char *)mem = flag;
            }
            mem = (char *)mem + group->chunkSize;
        }
    }

    /* remove all memory chunks that are in doomed groups from the
       array of memory pools */
    for( i = 1; i < NUM_POOLS; i++ )
    {
        pool = &poolArray[ i ];

        prevMem = 0;
        for( mem = *pool; mem != 0; mem = nextMem )
        {
            nextMem = chunkHeaderFor( mem ).link.next;

            if( *(char *)mem == 1 )
            {
                if( prevMem == 0 )
                {
                    *pool = nextMem;
                }
                else
                {
                    chunkHeaderFor( prevMem ).link.next = nextMem;
                }
            }
            else
            {
                prevMem = mem;
            }
        }
    }

    /* free() doomed groups */
    prevGroup = 0;
    for( group = g_firstGroup; group != 0; group = nextGroup )
    {
        nextGroup = group->next;

        pool = &poolArray[ group->poolNum ];

        mem = (CHUNK_HEADER *)(group + 1) + 1;
        if( chunkHeaderFor( mem ).link.pool != pool &&
        *(char *)mem == 1 )
        {
            if( prevGroup == 0 )
            {
                g_firstGroup = nextGroup;
            }
            else
            {
                prevGroup->next = nextGroup;
            }

#ifdef DEBUG
            size_t groupSize = sizeof( GROUP_HEADER ) +
            group->numChunks * group->chunkSize;
            memset( group, 0, groupSize );
#endif

            dealloc( group );
        }
        else
        {
            prevGroup = group;
        }
    }
}

/*-------------------------------------------------------------------*
 | chunkHeaderFor() -- get the header for a given chunk
 *-------------------------------------------------------------------*/

static inline CHUNK_HEADER &chunkHeaderFor( void *mem )
{


    return ((CHUNK_HEADER *)mem)[ -1 ];
}

/*-------------------------------------------------------------------*
 | chunkFooterFor() -- get the footer for a given chunk (only if
 |                     DEBUG is defined)
 *-------------------------------------------------------------------*/

#ifdef DEBUG

static inline CHUNK_FOOTER &chunkFooterFor( void *mem )
{
    return
    *((CHUNK_FOOTER *)((char *)mem + chunkHeaderFor( mem ).size));
}

#endif

/*-------------------------------------------------------------------*
 | putOnAllocedList() -- put a chunk on the linked list of chunks
 |                       that have been allocated (only if DEBUG
 |                       is defined)
 *-------------------------------------------------------------------*/

#ifdef DEBUG

static inline void putOnAllocedList( void *mem )
{
    CHUNK_HEADER *header = &chunkHeaderFor( mem );

    header->prevAlloced = &g_allocedList + 1;
    header->nextAlloced = g_allocedList.nextAlloced;

    if( header->prevAlloced != 0 )
    {
        chunkHeaderFor( header->prevAlloced ).nextAlloced = mem;
    }
    if( header->nextAlloced != 0 )
    {
        chunkHeaderFor( header->nextAlloced ).prevAlloced = mem;
    }
}

#endif

/*-------------------------------------------------------------------*
 | takeOffAllocedList() -- take a memory chunk off the list of chunks
 |                         that are allocated
 *-------------------------------------------------------------------*/

#ifdef DEBUG

static inline void takeOffAllocedList( void *mem )
{
    CHUNK_HEADER *header = &chunkHeaderFor( mem );

    if( header->prevAlloced != 0 )
        chunkHeaderFor( header->prevAlloced ).nextAlloced =
        header->nextAlloced;
    if( header->nextAlloced != 0 )
        chunkHeaderFor( header->nextAlloced ).prevAlloced =
        header->prevAlloced;
}

#endif

/*-------------------------------------------------------------------*
 | isEmergencyMem() -- determine if a piece of memory was allocated
 |                     by emergencyAlloc()
 *-------------------------------------------------------------------*/

static inline int isEmergencyMem( void *mem )
{


    return (mem >= (void *)emergencyBuf &&
    mem < (void *)(emergencyBuf + EMERGENCY_BUF_LONGS));
}

/*-------------------------------------------------------------------*
 | emergencyAlloc() -- allocate memory from the emergency buffer
 *-------------------------------------------------------------------*/

static void *emergencyAlloc( size_t size )
{


    long i, j;

    size = (size_t)(.5 + (double)size / sizeof( *emergencyBuf ));
    size++;

    for( i = 0; i < EMERGENCY_BUF_LONGS; i += labs( emergencyBuf[ i ] ) )
        if( emergencyBuf[ i ] == 0 )
        {
            break;
        }
    if( i > EMERGENCY_BUF_LONGS || emergencyBuf[ i ] == 0 )
    {
        fprintf( stderr, "\nEmergency buffer is corrupted\n" );
        //DumpCore();
        assert(false);
    }

    for( i = 0; i < EMERGENCY_BUF_LONGS; i += labs( emergencyBuf[ i ] ) )
        if( emergencyBuf[ i ] > 0 )
        {
            j = i + emergencyBuf[ i ];
            while( j < EMERGENCY_BUF_LONGS && emergencyBuf[ j ] > 0 )
            {
                emergencyBuf[ i ] += emergencyBuf[ j ];
                j += emergencyBuf[ j ];
            }

            if( emergencyBuf[ i ] >= size )
            {
                break;
            }
        }

    if( i == EMERGENCY_BUF_LONGS )
    {
        fprintf( stderr, "\nOut of emergency buffer space "
        "(looking for %d bytes)\n",
        size * 2 );
        //DumpCore();
        assert(false);
    }

    if( size < emergencyBuf[ i ] )
    {
        emergencyBuf[ i + size ] = emergencyBuf[ i ] - size;
    }
    emergencyBuf[ i ] = -size;

    return (void *)&emergencyBuf[ i + 1 ];
}

/*-------------------------------------------------------------------*
 | emergencyDealloc() -- deallocate memory that was allocated with
 |                       emergencyAlloc()
 *-------------------------------------------------------------------*/

static inline void emergencyDealloc( void *mem )
{


    if( ((long *)mem)[ -1 ] < 0 )
    {
        ((long *)mem)[ -1 ] *= -1;
    }
}

/*-------------------------------------------------------------------*
 | alloc() -- call malloc(), and try to free up memory if it fails
 *-------------------------------------------------------------------*/

static void *alloc( size_t originalSize )
{


    void *mem;
    size_t size;
    HANDLER_FOR_NEW handlerForNew;

    size = originalSize + sizeof( size_t );
    mem = malloc( size );

    if( mem == 0 )
    {
        chunkCleanup();
        mem = malloc( size );

        while( mem == 0 && (handlerForNew = getHandlerForNew()) != 0 )
        {
            (*handlerForNew)();
            chunkCleanup();
            mem = malloc( size );
        }

        assert( mem != 0 );
        //  THROW_ERR( "Can't allocate " << size << " bytes of memory" )
    }

    G_allocatedMemory += size;
    if( G_allocatedMemory > G_peakAllocatedMemory )
    {
        G_peakAllocatedMemory = G_allocatedMemory;
    }

    *((size_t *)mem) = size;

    return (size_t *)mem + 1;
}

/*-------------------------------------------------------------------*
 | dealloc() -- call free()
 *-------------------------------------------------------------------*/

static inline void dealloc( void *mem )
{


    size_t size;

    mem = (size_t *)mem - 1;

    size = *((size_t *)mem);

#ifdef TSTBUG
    assert( size >= sizeof( size_t ) );
    //  THROW_ERR( "Corrupted heap at " << mem )
    *((size_t *)mem) = 0;
#endif

    free( mem );

    G_allocatedMemory -= size;
}

/*-------------------------------------------------------------------*
 | getHandlerForNew() -- get the routine that was registered with
 |                       set_new_handler()
 *-------------------------------------------------------------------*/

static inline HANDLER_FOR_NEW getHandlerForNew()
{


    HANDLER_FOR_NEW handlerForNew;

    handlerForNew = std::set_new_handler( 0 );
    std::set_new_handler( handlerForNew );

    return handlerForNew;
}
