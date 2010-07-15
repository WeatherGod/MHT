
/*********************************************************************
 * FILE: mem.H                                                       *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   31 MAY 93 -- (mlm) created                                      *
 *   21 FEB 95 -- (mlm) changed various declarations from int to     *
 *                long, for portability                              *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Definitions for memory management.  This file, in conjunction   *
 *   with mem.C, redefines operator new() and operator delete() to   *
 *   make them more efficient for many common types of programs.     *
 *   Also, when DEBUG is defined, this version of new() and delete() *
 *   provide extensive bug checking, and can usually trap problems   *
 *   like out-of-bounds subscripts.                                  *
 *                                                                   *
 *                operator new() and operator delete()               *
 *                                                                   *
 *   Many programs allocate and deallocate miriads of small objects  *
 *   that have similar sizes.  For example, a program that maintains *
 *   a tree of nodes, inserting and deleting them frequently, will   *
 *   generally allocate and deallocate objects exactly the size of   *
 *   these nodes (i.e. it will allocate and deallocate the nodes     *
 *   themselves, and few objects of other sizes).  In such cases, we *
 *   can greatly speed things up, and reduce memory fragmentation,   *
 *   by keeping old, deallocated chunks of memory around for reuse.  *
 *                                                                   *
 *   When a program calls the version of new() provided in mem.C,    *
 *   the memory it asks for will be allocated in one of three ways.  *
 *                                                                   *
 *     1. If the amount of memory is large (more than a couple K),   *
 *        then it will simply be allocated using malloc().  When     *
 *        this memory is deallocated, it will be free()'ed.          *
 *                                                                   *
 *     2. If it is small, then it is allocated from one of a         *
 *        predefined set of "memory pools".  Each memory pool is a   *
 *        linked list of memory chunks that are the same size, and   *
 *        not in use.  When a chunk of memory is allocated from a    *
 *        pool, it is removed from the list.  When that memory is    *
 *        deallocated, it is returned to the pool.                   *
 *                                                                   *
 *        The memory pool that new() chooses is the one containing   *
 *        the smallest chunks larger than the size requested.  This  *
 *        means that the memory returned might be a few bytes        *
 *        larger than expected.                                      *
 *                                                                   *
 *        If the pool is empty, then one call to malloc() will be    *
 *        made to allocate a group of chunks of the appropriate      *
 *        size.  The number of chunks depends on their size (the     *
 *        size of each group is held roughly constant).  This        *
 *        reduces the number of calls made to malloc(), which is     *
 *        slow, and also reduces the amount of memory fragmentation. *
 *                                                                   *
 *        Memory allocated for a pool is never freed, except by      *
 *        calls to CollectGarbage() (see below).                     *
 *                                                                   *
 *     3. If an error has occurred, then the memory will be          *
 *        allocated from a predefined, "emergency" buffer.  This     *
 *        should only ever happen during the execution of a          *
 *        THROW_ERR() macro.                                         *
 *                                                                   *
 *   If malloc() is called, and it can't allocate any memory, then   *
 *   a call is made to CollectGarbage() (see below), and malloc()    *
 *   is tried again.  If it still can't allocate any memory, then    *
 *   a call is made to the routine set by set_new_handler(),         *
 *   followed by another call to CollectGarbage().  If malloc()      *
 *   STILL can't allocate any memory, then the new_handler routine   *
 *   and CollectGarbage() are called AGAIN.  The process is          *
 *   repeated until either malloc() succeeds, or the new_handler     *
 *   has been set to 0.  (set_new_handler() is a standard C++        *
 *   library routine.)                                               *
 *                                                                   *
 *   If the new_handler and CollectGarbage() are unable to get       *
 *   malloc() to work, then an error is generated using THROW_ERR(). *
 *   This means that this version of new() NEVER RETURNS NULL.       *
 *                                                                   *
 *   If DEBUG was defined when mem.C was compiled, then every chunk  *
 *   of memory allocated by methods 1 or 2 will have a magic number  *
 *   placed before and after it.  Since these magic numbers lie      *
 *   outside the memory that was requested, a correctly running      *
 *   program will never change them.  If they ever change, then it   *
 *   means there is a bug.                                           *
 *                                                                   *
 *   The magic numbers are checked by every call to new() and        *
 *   delete().  They may also be checked by CheckMem() (see below).  *
 *                                                                   *
 *                       CollectGarbage()                            *
 *                                                                   *
 *   This routine goes through all the groups of chunks that have    *
 *   been allocated, and deletes the ones that don't contain any     *
 *   chunks which are currently in use.  That is, it checks every    *
 *   chunk in the group to see if it is on one of the memory-pool    *
 *   linked lists.  If EVERY chunk in the group is on such a list,   *
 *   then the ENTIRE group is free()'ed.  If even ONE chunk is       *
 *   currently in use, then the ENTIRE group remains allocated.      *
 *                                                                   *
 *   This should be called at the end of high-level routines which   *
 *   aren't used frequently, and which thrash memory alot.  It can   *
 *   significantly reduce memory usage, but it also slows things     *
 *   down.                                                           *
 *                                                                   *
 *                         CheckMem()                                *
 *                                                                   *
 *   This routine is overloaded.  It can take either no arguments,   *
 *   or a void pointer.                                              *
 *                                                                   *
 *   If DEBUG is not defined, then neither version of this routine   *
 *   does anything.                                                  *
 *                                                                   *
 *   Otherwise, the version with no arguments goes through every     *
 *   currently allocated chunk of memory, and every chunk sitting in *
 *   a memory pool, checking to see that their magic numbers are     *
 *   still valid.  If anything has trashed the beginning or end of   *
 *   any chunk, then an error will be generated using THROW_ERR().   *
 *                                                                   *
 *   The version that takes a void pointer checks ONLY the chunk of  *
 *   memory that it points to.  If that chunk of memory has been     *
 *   trashed, or it wasn't allocated with new(), then an error will  *
 *   be generated using THROW_ERR().                                 *
 *                                                                   *
 *                       Global Variables                            *
 *                                                                   *
 *   These versions of new() and delete() maintain a set of global   *
 *   variables that describe memory usage.  They are described by    *
 *   the comments beside their definitions below.                    *
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

#ifndef MEM_H
#define MEM_H

#include <stdio.h>

#include "except.h"

#ifdef DECLARE_MEM
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

/*-------------------------------------------------------------------*
 | Global variables
 *-------------------------------------------------------------------*/

GLOBAL long G_allocatedMemory;       // Total number of bytes currently
                                     //   allocated and in use, in
                                     //   memory pools, or used for
                                     //   internal bookkeeping
GLOBAL long G_peakAllocatedMemory;   // Largest value of
                                     //   G_allocatedMemory since the
                                     //   beginning of the program run
GLOBAL long G_numAllocations;        // Number of times operator new()
                                     //   has been called
GLOBAL long G_numDeallocations;      // Number of times
                                     //   operator delete() has been
                                     //   called with a non-zero
                                     //   argument

/*-------------------------------------------------------------------*
 | Global routines
 *-------------------------------------------------------------------*/

GLOBAL void CollectGarbage();

#ifdef DEBUG
  GLOBAL void CheckMem();
  GLOBAL void CheckMem( void *mem );
#else
  static inline void CheckMem() {}
  static inline void CheckMem( void * ) {}
#endif

#undef GLOBAL
#endif

