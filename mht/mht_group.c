
/*********************************************************************
 * FILE: mht_group.C                                                 *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   12 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Member functions for GROUP and G_HYPO objects.  See mht.H.      *
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

#include <stdlib.h>

#include "mht.h"
#include "apqueue.h"
#include "pqueue.h"
#include "timer.h"

/*-------------------------------------------------------------------*
 | g_apqueue -- assignment priority queue for generating new G_HYPOs
 *-------------------------------------------------------------------*/

static ASSIGNMENT_PQUEUE g_apqueue;

/*-------------------------------------------------------------------*
 | Stuff used by GROUP::merge()
 |
 | Merge is more complicated than it might seem.
 |
 | Here's the way a simple algorithm would work:
 |
 |   For each G_HYPO, A, in GROUP 0
 |     For each G_HYPO, B, in GROUP 1
 |       Make a new G_HYPO that consists of the list of T_HYPOs from
 |       A concatenated with the T_HYPOs from B.  The likelihood of
 |       This new G_HYPO is just the product of the likelihood of A
 |       and the likelihood of B.
 |
 | The problem with this is that it can produce too many new G_HYPOs.
 |
 | To ensure that we never have a GROUP that violates the pruning
 | parameters, we have to generate the new G_HYPOs in decreasing order
 | of likelihood, stopping when either the likelihood ratio has gone
 | below m_minGHypoRatio or the total number of G_HYPOs has reached
 | m_maxGHypos.
 |
 | Here's the algorithm I use:
 |
 | 1. Make a list of the G_HYPOs in GROUP 0, sorted in decreasing
 |    order of likelihood
 |
 | 2. Make a list of the G_HYPOs in GROUP 1, sorted in decreasing
 |    order of likelihood
 |
 | 3. Initialize a priority queue (see pqueue.H) with a "G_HYPO_PAIR"
 |    structure, which contains two indices, one into each of the
 |    two arrays of pointers to G_HYPOs.  This initial G_HYPO_PAIR
 |    has both indices set to 0.
 |
 |    A G_HYPO_PAIR indicates two G_HYPOs that can be combined to
 |    make a new G_HYPO for the merged GROUP.  Since the two lists
 |    are sorted by likelihood, and the likelihood of the resulting
 |    G_HYPO will be the product of the likelihoods of the two
 |    G_HYPOs indicated, the initial G_HYPO_PAIR indicates the most
 |    likely pair.
 |
 | 4. While the priority queue is not empty, the likelihood ratio
 |    hasn't gone below m_minGHypoRatio, and the total number of new
 |    G_HYPOs made hasn't reached m_maxGHypos
 |
 |    4.1 Get the next G_HYPO_PAIR off the priority queue.  This will
 |        indicate the most likely G_HYPO remaining to be made.
 |
 |    4.2 Make the G_HYPO that the G_HYPO_PAIR indicates.
 |
 |        Now, the next-most-likely G_HYPO after this one will be
 |        made by keeping one of the two G_HYPOs we just combined,
 |        and replacing the other with its successor in the array
 |        that it came from.  So ...
 |
 |    4.3 If the G_HYPO_PAIR's index into array 0 (from GROUP 0)
 |        is less than the number of elements in array 0,
 |        4.3.1 Make a new G_HYPO_PAIR, in which the index for array
 |              0 is incremented by 1, and put it on the priority
 |              queue
 |    4.4 If the G_HYPO_PAIR's index into array 1 (from GROUP 1)
 |        is less than the number of elements in array 1,
 |        4.4.1 Make a new G_HYPO_PAIR, in which the index for array
 |              1 is incremented by 1, and put it on the priority
 |              queue
 |
 | To reduce the number of G_HYPO_PAIRs put onto the priority queue,
 | and prevent the same pair from being generated more than once,
 | I use an array of flags to say whether a given pair has been used
 | or not.  The array has one flag for each possible pair.  The flag
 | is set to 1 as soon as the corresponding pair has been placed on
 | the priority queue.  If the flag is already 1, the pair is not
 | placed on the queue again.
 *-------------------------------------------------------------------*/

/* g_gHypoArray0 and g_gHypoArray1 are the two arrays of pointers to
   G_HYPOs being combined for GROUP::merge().

   NOTE: The compiler I'm using has a problem with globals
   that are referred to by objects that are placed in container
   classes.  Since G_HYPO_PAIR refers to g_gHypoArray0 and
   g_gHypoArray1, and G_HYPO_PAIR is used in the PQUEUE_OF<> container
   class, the problem happens here.  The symptoms are:

   1. If the global is declared static, then two separate instances
      of it are allocated -- one that can be accessed by G_HYPO_PAIR,
      and one that can be accessed by the rest of the program.

   2. If the global is not static, then its constructor and destructor
      are both called twice.

   I've worked around this by using SAFE_GLOBAL().  See safeglobal.H.
*/

#ifdef COMPILER_IS_DEBUGGED
  static VECTOR_OF< G_HYPO * > g_gHypoArray0;
  static VECTOR_OF< G_HYPO * > g_gHypoArray1;
#else
  #define GLOBAL
  SAFE_GLOBAL( VECTOR_OF< G_HYPO * >, g_gHypoArray0 )
  SAFE_GLOBAL( VECTOR_OF< G_HYPO * >, g_gHypoArray1 )
  #undef GLOBAL
#endif
#define KBEST
#define NSCAN_BACK_PRUNING

/* flags for pairs of G_HYPOs */
static VECTOR_OF< char > g_gHypoPairUsed;

/* structure indicating a pair of G_HYPOs */
struct G_HYPO_PAIR
{
  int i0, i1;

  G_HYPO_PAIR(): i0( 0 ), i1( 0 ) {}

  G_HYPO_PAIR( int i0Arg, int i1Arg ):
    i0( i0Arg ),
    i1( i1Arg )
  {
  }

  G_HYPO *getGHypo0() const { return g_gHypoArray0[ i0 ]; }
  G_HYPO *getGHypo1() const { return g_gHypoArray1[ i1 ]; }

  double getLogLikelihood() const
  {
    return getGHypo0()->getLogLikelihood() +
           getGHypo1()->getLogLikelihood();
  }

  int operator<( const G_HYPO_PAIR &that ) const
  {
    return getLogLikelihood() < that.getLogLikelihood();
  }

  int operator>( const G_HYPO_PAIR &that ) const
  {
    return getLogLikelihood() > that.getLogLikelihood();
  }
};

/*-------------------------------------------------------------------*
 | setupArray0() and setupArray1() -- routines to fill in
 |                                    g_gHypoArray0 and g_gHypoArray1
 |
 | These routines perform steps 1 and 2 of the algorithm described
 | above.
 |
 | NOTE: compareGHypoPtrs(), which I'd really rather put after the
 | two setupArray routines, has to go first because of some weirdness
 | with the way the SGI compiler handles templates.
 *-------------------------------------------------------------------*/

static int compareGHypoPtrs( const void *addr0, const void *addr1 )
{
  

  #define gHypo0 (*(G_HYPO **)addr0)
  #define gHypo1 (*(G_HYPO **)addr1)

  if( gHypo0->getLogLikelihood() < gHypo1->getLogLikelihood() )
    return 1;
  if( gHypo0->getLogLikelihood() > gHypo1->getLogLikelihood() )
    return -1;
  return 0;

  #undef gHypo0
  #undef gHypo1
}

static void setupArray0( iDLIST_OF< G_HYPO > &gHypoList, int numGHypos )
{
  

  int compareGHypoPtrs( const void *addr0, const void *addr1 );

  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr;
  int i;

  g_gHypoArray0.resize( numGHypos );

  i = 0;
  LOOP_DLIST( gHypoPtr, gHypoList )
  {
    (*gHypoPtr).recomputeLogLikelihood(); // make sure logLikelihood
                                          //   is up to date
    g_gHypoArray0[ i++ ] = gHypoPtr.get();
  }

  qsort( &g_gHypoArray0[ 0 ],
         numGHypos,
         sizeof( G_HYPO * ),
         compareGHypoPtrs );
}

static void setupArray1( iDLIST_OF< G_HYPO > &gHypoList, int numGHypos )
{
  

  int compareGHypoPtrs( const void *addr0, const void *addr1 );

  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr;
  int i;

  g_gHypoArray1.resize( numGHypos );

  i = 0;
  LOOP_DLIST( gHypoPtr, gHypoList )
  {
    (*gHypoPtr).recomputeLogLikelihood(); // make sure logLikelihood
                                          //   is up to date
    g_gHypoArray1[ i++ ] = gHypoPtr.get();
  }

  qsort( &g_gHypoArray1[ 0 ],
         numGHypos,
         sizeof( G_HYPO * ),
         compareGHypoPtrs );
}

/*-------------------------------------------------------------------*
 | GROUP::merge() -- merge two GROUPs
 |
 | See above for algorithm.
 *-------------------------------------------------------------------*/

void GROUP::merge( GROUP *src,
                   double logMinGHypoRatio,
                   int maxGHypos )
{
  

  #define G_HYPO_PAIR_USED( g0, g1 ) \
    g_gHypoPairUsed[ ((g0) * maxGHypos) + g1 ]

  static PQUEUE_OF< G_HYPO_PAIR > pqueue;
  G_HYPO_PAIR gHypoPair( 0, 0 );
  G_HYPO *gHypo;
  double bestLogLikelihood;
  int numGHypos0;
  int numGHypos1;
  iDLIST_OF< G_HYPO > newGHypoList;
  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr;
  int i;

  numGHypos1 = src->m_gHypoList.getLength();
  numGHypos0 = m_gHypoList.getLength();
  //CheckMem();

  if( numGHypos1 == 1 )
  {

    LOOP_DLIST( gHypoPtr, m_gHypoList )
    {
      (*gHypoPtr).merge( src->m_gHypoList.getHead() );
       //CheckMem();
    }
    src->m_gHypoList.removeAll();

    return;
  }
  setupArray0( m_gHypoList, numGHypos0 );
  setupArray1( src->m_gHypoList, numGHypos1 );
  pqueue.clear();
  pqueue.resize( 2 * maxGHypos );
  g_gHypoPairUsed.resize( 0, maxGHypos * maxGHypos );
  g_gHypoPairUsed.clear();
  bestLogLikelihood = gHypoPair.getLogLikelihood();
  i = 0;
  while( i < maxGHypos &&
         gHypoPair.getLogLikelihood() - bestLogLikelihood >=
           logMinGHypoRatio )
  {
    gHypo = new G_HYPO;
    gHypo->merge( gHypoPair.getGHypo0() );
    gHypo->merge( gHypoPair.getGHypo1() );
    newGHypoList.append( gHypo );
    //CheckMem();
    if( gHypoPair.i0 + 1 < numGHypos0 &&
        ! G_HYPO_PAIR_USED( gHypoPair.i0 + 1, gHypoPair.i1 ) )
    {
      pqueue.put( G_HYPO_PAIR( gHypoPair.i0 + 1, gHypoPair.i1 ) );
      G_HYPO_PAIR_USED( gHypoPair.i0 + 1, gHypoPair.i1 ) = 1;
    }
    if( gHypoPair.i1 + 1 < numGHypos1 &&
        ! G_HYPO_PAIR_USED( gHypoPair.i0, gHypoPair.i1 + 1 ) )
    {
      pqueue.put( G_HYPO_PAIR( gHypoPair.i0, gHypoPair.i1 + 1 ) );
      G_HYPO_PAIR_USED( gHypoPair.i0, gHypoPair.i1 + 1 ) = 1;
    }
    if( ! pqueue.isEmpty() )
      gHypoPair = pqueue.get();
    else
      break;

    i++;
  }

/* 
  LOOP_LINKS( gHypoPtr, m_gHypoList )
  {
    (*gHypoPtr).describe( 2 );
  }
*/

  m_gHypoList.removeAll();
  //CheckMem();

  src->m_gHypoList.removeAll();
  //CheckMem();
  m_gHypoList.splice( newGHypoList );

  #undef G_HYPO_PAIR_USED
}

/*-------------------------------------------------------------------*
 | GROUP::splitIfYouMust() -- split the GROUP into two parts if it
 |                            needs to be split
 |
 | A GROUP can be split if it contains trees that have been assigned
 | different group id's.  This routine checks to see if that's true,
 | and splits the GROUP in two if it is.
 |
 | When the GROUP is split, it keeps all the trees with one group id,
 | and gives all the other trees to the new GROUP, which is placed
 | onto the list of GROUPs directly after this one.  In other words,
 | if we have a list of GROUPs
 |
 |   A B C D
 |
 | and GROUP B is split, then the list will contain
 |
 |   A B B1 C D
 |
 | where B and B1 are the two GROUPs resulting from the split.
 |
 | All the trees in the new B have the same group id.  The trees in
 | B1 might have different group id's, so splitIfYouMust() should be
 | called for B1 next.  This happens automatically if splitIfYouMust()
 | is being called in a loop:
 |
 |   LOOP_DLIST( groupPtr, m_groupList )
 |   {
 |     (*groupPtr).splitIfYouMust();
 |   }
 |
 | When groupPtr points to B, B is split into the new B and B1.
 | groupPtr still points to B, so in the next iteration of the loop
 | it will be moved to B's successor, which is now B1, and
 | splitIfYouMust() will be called for B1.
 |
 | The splitting itself simply requires splitting each G_HYPO
 | according to the group id that is to be kept in this GROUP.  This
 | is accomplished by G_HYPO::split(), which keeps all the trees
 | that have the given group id, and puts all the others into a new
 | G_HYPO, which it returns.
 |
 | Once the GROUP has been split, some G_HYPOs might end up identical
 | (since the only difference between them, originally, was in the
 | T_HYPO they chose for a tree that's no longer in the GROUP).  These
 | are removed by removeRepeats().
 *-------------------------------------------------------------------*/

void GROUP::splitIfYouMust()
{
  

  GROUP *newGroup;
  int groupId;
  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr;

  if( m_gHypoList.isEmpty() || ! (*m_gHypoList).mustSplit() )
    return;

  newGroup = new GROUP();
  groupId = (*m_gHypoList).getGroupId();

  LOOP_DLIST( gHypoPtr, m_gHypoList )
  {
    newGroup->m_gHypoList.append( (*gHypoPtr).split( groupId ) );
  }

  removeRepeats();
  newGroup->removeRepeats();

  /* put the new group onto the list after this one */
  append( newGroup );
}

/*-------------------------------------------------------------------*
 | GROUP::removeRepeats() -- remove identical G_HYPOs from the GROUP
 |
 | Each G_HYPO in the list is compared with all the G_HYPOs that come
 | after it.  All the ones that are identical to it are removed.
 | This means there will be no identical pairs of G_HYPOs left after
 | the routine is done.
 |
 | Here's the algorithm:
 |
 | 1. For each G_HYPO, A, on the list
 |    1.1 Flag all the T_HYPOs that are referred to in A
 |    1.2 For each G_HYPO, B, on the list after A
 |        1.2.1 Check to see if all the T_HYPOs referred to in B
 |              have been flagged.  If they all have, then A and B
 |              refer to exactly the same T_HYPOs, and are identical,
 |              so ...
 |              1.2.1.1 Remove B
 |    1.3 Unflag all the T_HYPOs that are referred to in A
 *-------------------------------------------------------------------*/

void GROUP::removeRepeats()
{
  

  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr0;
  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr1;

  LOOP_DLIST( gHypoPtr0, m_gHypoList )
  {
    (*gHypoPtr0).setFlags();

    for( gHypoPtr1 = gHypoPtr0, ++gHypoPtr1;
         gHypoPtr1.isValid();
         ++gHypoPtr1 )
      if( (*gHypoPtr1).allFlagsAreSet() )
        gHypoPtr1.remove();

    (*gHypoPtr0).resetFlags();
  }
}

/*-------------------------------------------------------------------*
 | GROUP::pruneAndHypothesize() -- prune the track trees and make new
 |                                 G_HYPOs
 |
 | The algorithm for this is described in mht.H.
 *-------------------------------------------------------------------*/

void GROUP::pruneAndHypothesize( int maxDepth,
                                 double logMinGHypoRatio,
                                 int maxGHypos )
{
  

  Timer timer0;
  Timer timer1;
  G_numCallsToPruneAndHypothesize++;

  VECTOR_OF< void * > solution;
  int solutionSize;
  double bestCost;

  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr;
  G_HYPO *newGHypo;

  iDLIST_OF< G_HYPO > newGHypoList;

  int numNewGHypos;

  /* make an assignment problem for each G_HYPO, and put it on an
     assignment priority queue (see apqueue.H) */

  g_apqueue.removeAllProblems();

  LOOP_DLIST( gHypoPtr, m_gHypoList )
  {
    (*gHypoPtr).makeProblem();
  }

  /* if there are no possible assignments, do nothing (this GROUP is
     empty */
  if( g_apqueue.isEmpty() )
  {
    G_timeSpentInPruneAndHypothesize += timer0.elapsedTime();
    return;
  }

  /* get the best solution to an assignment problem */

  timer1.reset_clock();
  bestCost = g_apqueue.getNextSolutionCost();
  g_apqueue.getNextSolution( solution, &solutionSize );


  /* make a new G_HYPO based on the solution to the assignment
     problem */
  
  m_bestGHypo = new G_HYPO( solution, solutionSize );
  
  newGHypoList.append( m_bestGHypo );
  numNewGHypos = 1;

  /* make the rest of the new G_HYPOs */
  while( numNewGHypos < maxGHypos &&
         ! g_apqueue.isEmpty() &&
         bestCost - g_apqueue.getNextSolutionCost() >=
           logMinGHypoRatio )
  {
    g_apqueue.getNextSolution( solution, &solutionSize );

    newGHypo = new G_HYPO( solution, solutionSize );
    newGHypoList.append( newGHypo );
    numNewGHypos++;
  }
  G_timeSpentInApqueue += timer1.elapsedTime();

  LOOP_DLIST( gHypoPtr, newGHypoList )
  {
     (*gHypoPtr).setNumtHypos();
  }

  /* apply N-scanback pruning, and remove any G_HYPO that loses a
     T_HYPO in the process */
  m_bestGHypo->nScanBackPrune( maxDepth );

  LOOP_DLIST( gHypoPtr, newGHypoList )
    if( (*gHypoPtr).wasReduced() )
    {
      g_apqueue.removeProblem( gHypoPtr );
      gHypoPtr.remove();
    }


  /* replace the old G_HYPOs with the new ones */
  m_gHypoList.removeAll();
  m_gHypoList.splice( newGHypoList );

  G_timeSpentInPruneAndHypothesize += timer0.elapsedTime();
}

void GROUP::clear( int maxDepth)
{
  

  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr;

  LOOP_DLIST( gHypoPtr, m_gHypoList )
     (*gHypoPtr).setNumtHypos();

  /* apply N-scanback pruning, and remove any G_HYPO that loses a
     T_HYPO in the process */
  m_bestGHypo->nScanBackPrune( maxDepth );

  LOOP_DLIST( gHypoPtr, m_gHypoList )
    if( (*gHypoPtr).wasReduced() )
    {
      g_apqueue.removeProblem( gHypoPtr );
      gHypoPtr.remove();
    }
}

/*-------------------------------------------------------------------*
 | GROUP::check() -- check that the group is ok (diagnostic tool)
 *-------------------------------------------------------------------*/

void GROUP::check()
{
  

  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr;
  int numTHypos;
  int groupId;

  numTHypos = (*m_gHypoList).getNumTHypos();
  assert( numTHypos != 0 );
  //  THROW_ERR( "Group with 0 trees" )
  groupId = (*m_gHypoList).getGroupId();
  LOOP_DLIST( gHypoPtr, m_gHypoList )
  {
    assert( (*gHypoPtr).getNumTHypos() == numTHypos );
    //  THROW_ERR( "Group with different numbers of tHypos" )
    assert( (*gHypoPtr).getGroupId() == groupId );
    //  THROW_ERR( "More than one group id in same group" )
  }
}

/*-------------------------------------------------------------------*
 | GROUP::describe() -- diagnostic tool
 *-------------------------------------------------------------------*/

void GROUP::describe( int spaces )
{
  

  PTR_INTO_iDLIST_OF< G_HYPO > gHypoPtr;

  Indent( spaces ); std::cout << "CLUSTER "; print(); std::cout << std::endl;

  LOOP_LINKS( gHypoPtr, m_gHypoList )
  {
    (*gHypoPtr).describe( spaces + 2 );
  }
}

/*-------------------------------------------------------------------*
 | G_HYPO::G_HYPO() -- constructor for making G_HYPOs based on
 |                     solutions to assignment problems
 |
 | See G_HYPO::makeProblem() for an explanation.
 *-------------------------------------------------------------------*/

G_HYPO::G_HYPO( VECTOR_OF< void * > &solution, int solutionSize ):
  m_logLikelihood( 0 ),
  m_numTHyposUsedInProblem( 0 ),
  m_tHypoLinks()
{
  

  int i;
  for( i = 0; i < solutionSize; i++ )
    addTHypo( (T_HYPO *)solution[ i ] );
}

/*-------------------------------------------------------------------*
 | G_HYPO::makeProblem() -- make an assignment problem for a G_HYPO
 |
 | I think the best way to describe what's happening in this routine
 | is with an example and some diagrams.
 |
 |                      track tree 0         track tree 1
 |   +--------+              |                    |
 |   | G_HYPO |============--+------------------  |
 |   +--------+            \ |                  \ |
 |                         +---+                +---+
 |                         | A |                | B |
 |                         +---+                +---+
 |                         / | \                / | \
 |                        /  |  \              /  |  \
 |                   +---+ +---+ +---+    +---+ +---+ +---+
 |                   | C | | D | | E |    | F | | G | | H |
 |                   +---+ +---+ +---+    +---+ +---+ +---+
 |   +----------+    /      |             /      |
 |   | REPORT 0 |====-------+-------------       |
 |   +----------+           |                    |
 |                          |                    |
 |   +----------+          /                    /
 |   | REPORT 1 |==========---------------------
 |   +----------+
 |
 | A through H are T_HYPOs in two track trees (the whole track trees
 | aren't shown).  C, D, and E are new leaves of track tree 0.  F, G,
 | and H are new leaves of track tree 1.  A and B were leaves in the
 | previous iteration, before measureAndValidate() was called.
 |
 | C represents the possibility that REPORT 0 should be assigned to
 | track 0.  D represents the possibility that REPORT 1 should be
 | assigned to track 0.  E represents a way that no REPORT might be
 | assigned to track 0.  Similarly, F, G, and H represent the
 | possibilities that REPORT 0, REPORT 1, or no REPORT should be
 | assigned to track 1, respectively.
 |
 | The G_HYPO is old, left over from the previous scan when A and
 | B were leaves. It postulates that the true state of track 0 is the
 | one represented by A, and the true state of track 1 is the one
 | represented by B.  We're calling G_HYPO::makeProblem() as a step
 | in the construction of the new G_HYPOs that will be children of
 | this one.  Each of these children will postulate a legal combination
 | of one of C, D, or E, with one of F, G, or H.
 |
 | Since each REPORT may be assigned to only one track, and each
 | track may have no more than one REPORT assigned to it, what we
 | have here is a classical assignment problem (see apqueue.H and
 | assign.H).  Drawing the assignment problem as a bipartite graph,
 | we get:
 |
 |      "rows"              "columns"
 |
 |               unassigned
 |                    \
 |                     \E
 |   +----------+   C   \
 |   | REPORT 0 |-------- track tree 0
 |   +----------+       /
 |               \     /D
 |                \   /
 |                 \ /
 |                  X
 |                 / \
 |                /   \
 |               /     \F
 |   +----------+       \
 |   | REPORT 1 |-------- track tree 1
 |   +----------+   G   /
 |                     /H
 |                    /
 |               unassigned
 |
 | Each of the "row" nodes in the graph represents a report.  Each of
 | the "column" nodes represents a track tree.  Each of the arcs,
 | which specify possible assignments, corresponds to a leaf T_HYPO in
 | one of the trees.  The cost of an arc is the negated log
 | likelihood of the T_HYPO it corresponds to (negated because the
 | assignment code minimizes the cost, and we want to maximize the
 | likelihood).
 |
 | A solution to this problem is a list of arcs.  Since each arc
 | corresponds to a T_HYPO, this is the same as a list of T_HYPOs.
 | And a list of T_HYPOs is a G_HYPO.  The log likelihood of the
 | G_HYPO is just the negated cost of the solution.
 |
 | What makeProblem() does is to construct the assignment problem
 | for an old G_HYPO and put it on the ASSIGNMENT_PQUEUE.  The
 | tag fields of the ROW_COL_COST structures (see assign.H)
 | are set to point to the T_HYPOs that they correspond to.  So,
 | to make a G_HYPO from a solution to one of these problems, we
 | need only make a list of the T_HYPOs that the solution points to.
 |
 | The row and column numbers used in the problem are arbitrary.
 | But it is more memory efficient if their ranges are kept to a
 | minimum.  Each REPORT is assigned a unique row number when it is
 | imported by MHT::importNewReports().  Each track tree is given
 | a column number on the fly (just an integer that's incremented
 | through the loop).
 *-------------------------------------------------------------------*/

void G_HYPO::makeProblem()
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;
  PTR_INTO_iTREE_OF< T_HYPO > childPtr;
  VECTOR_OF< ROW_COL_COST > rcc;
  int numRCCs;
  int rccNum;
  int colNum;
  int maxRow;

  /* record the number of T_HYPOs in the G_HYPO (which is the same
     as the number of track trees), for use later in determining
     whether the G_HYPO has become invalid as a result of pruning */
  m_numTHyposUsedInProblem = m_tHypoLinks.getLength();

  /* count the number of ROW_COL_COST structures that there will be
     in the assignment problem */
  numRCCs = 0;
  maxRow = 0;
  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    #ifdef TSTBUG
      assert( ! (*tHypoPtr).isLeaf() );
      //  THROW_ERR( "THypo has no children for next iteration" )
    #endif

    numRCCs += (*tHypoPtr).getNumChildren();
  }

  /* make room for the ROW_COL_COST structures */
  rcc.resize( numRCCs );

  /* make ROW_COL_COST structures out of the T_HYPOs */
  rccNum = 0;
  colNum = 0;
  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    LOOP_TREEchildren( childPtr, tHypoPtr.get() )
    {
      rcc[ rccNum++ ].set( (*childPtr).getRowNum(), colNum,
                           -(*childPtr).getLogLikelihood(),
                            childPtr.get() );
      if( (*childPtr).getRowNum() > maxRow )
        maxRow = (*childPtr).getRowNum();
    }
    colNum++;
  }

  /* sort the ROW_COL_COST structures and put the problem on the
     ASSIGNMENT_PQUEUE */
  SortAssignmentProblem( &rcc[ 0 ], numRCCs );
  g_apqueue.addProblem( this, &rcc[ 0 ], rccNum, maxRow + 1, colNum );

  G_numApqueueProblems++;
  G_totalApqueueProblemSizes += numRCCs;
  if( numRCCs > G_maxApqueueProblemSize )
    G_maxApqueueProblemSize = numRCCs;

  double problemCoverage = (double)numRCCs /
                           ((double)(maxRow + 2) * (colNum + 1) - 1.);

  G_totalApqueueProblemCoverage += problemCoverage;
  if( problemCoverage < G_minApqueueProblemCoverage )
    G_minApqueueProblemCoverage = problemCoverage;
  if( problemCoverage > G_maxApqueueProblemCoverage )
    G_maxApqueueProblemCoverage = problemCoverage;
}

/*-------------------------------------------------------------------*
 | G_HYPO::nScanBackPrune() -- perform N-scanback pruning
 |
 | This is called for the most likely G_HYPO in each group.  The
 | algorithm is:
 |
 |   For each T_HYPO in the G_HYPO
 |     Trace the path from the T_HYPO to the root of the track tree
 |     that it's on.
 |     If the length of that path is greater than maxDepth
 |       Find the node in the path that is a child of the root.
 |       Prune away all the other children of the root, along with
 |       their descendants.
 *-------------------------------------------------------------------*/

void G_HYPO::nScanBackPrune( int maxDepth )
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;
  PTR_INTO_iTREE_OF< T_HYPO > parentPtr;
  PTR_INTO_iTREE_OF< T_HYPO > siblingPtr;
  PTR_INTO_iTREE_OF< T_HYPO > tHypoToSave;
  int depth;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    if( (*tHypoPtr).isRoot() )
      continue;

    depth = 1;
    
    tHypoToSave = tHypoPtr.get();

    for( parentPtr = tHypoPtr.get(), parentPtr.gotoParent();
         ! parentPtr.isAtRoot();
         tHypoToSave = parentPtr, parentPtr.gotoParent() )
      depth++;
    if( depth < maxDepth )
      continue;
    
    LOOP_TREEchildren( siblingPtr, parentPtr )
      if( siblingPtr != tHypoToSave )
        siblingPtr.removeSubtree();
  }
}

/*-------------------------------------------------------------------*
 | G_HYPO::recomputeLogLikelihood() -- make sure the log likelihood
 |                                     of the G_HYPO is up to date
 *-------------------------------------------------------------------*/

void G_HYPO::recomputeLogLikelihood()
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;

  m_logLikelihood = 0;
  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    m_logLikelihood += (*tHypoPtr).getLogLikelihood();
  }
}

/*-------------------------------------------------------------------*
 | G_HYPO::mustSplit() -- determine if the G_HYPO should be split
 |
 | A G_HYPO should be split into G_HYPOs for different GROUPs if it
 | refers to track trees that have been assigned different group id's.
 *-------------------------------------------------------------------*/

int G_HYPO::mustSplit()
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;
  int groupId;

  if( m_tHypoLinks.isEmpty() )
    return 0;

  groupId = (*m_tHypoLinks).getGroupId();

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
    if( (*tHypoPtr).getGroupId() != groupId )
      return 1;
  return 0;
}

/*-------------------------------------------------------------------*
 | G_HYPO::split() -- split a G_HYPO in two
 |
 | The G_HYPO keeps all the track trees that have been assigned the
 | given group id, and puts all the others into a new G_HYPO, which
 | it returns.
 *-------------------------------------------------------------------*/

G_HYPO *G_HYPO::split( int groupId )
{
  

  G_HYPO *newGHypo = new G_HYPO();
  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;
  T_HYPO *tHypo;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    tHypo = tHypoPtr.get();

    if( tHypo->getGroupId() != groupId )
    {
      tHypoPtr.remove();
      m_logLikelihood -= tHypo->getLogLikelihood();
      newGHypo->addTHypo( tHypo );
    }
  }

  return newGHypo;
}

/*-------------------------------------------------------------------*
 | G_HYPO::merge() -- merge the G_HYPO with another G_HYPO
 *-------------------------------------------------------------------*/

void G_HYPO::merge( G_HYPO *src )
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;

  LOOP_LINKS( tHypoPtr, src->m_tHypoLinks )
  {
    addTHypo( tHypoPtr.get() );
  }
}

/*-------------------------------------------------------------------*
 | G_HYPO::setFlags() -- set the flags of all the T_HYPOs in the
 |                       G_HYPO
 *-------------------------------------------------------------------*/

void G_HYPO::setFlags()
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    (*tHypoPtr).setFlag();
  }
}

/*-------------------------------------------------------------------*
 | G_HYPO::resetFlags() -- reset the flags of all the T_HYPOs in the
 |                         G_HYPO
 *-------------------------------------------------------------------*/

void G_HYPO::resetFlags()
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    (*tHypoPtr).resetFlag();
  }
}

/*-------------------------------------------------------------------*
 | G_HYPO::allFlagsAreSet() -- see if the T_HYPOs in this G_HYPO have
 |                             all had their flags set
 *-------------------------------------------------------------------*/

int G_HYPO::allFlagsAreSet()
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    if( ! (*tHypoPtr).flagIsSet() )
      return 0;
  }

  return 1;
}

/*-------------------------------------------------------------------*
 | G_HYPO::describe() -- diagnostics
 *-------------------------------------------------------------------*/

void G_HYPO::describe( int spaces )
{
  

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;
  int k;

  Indent( spaces ); std::cout << "G_HYPO "; print(); std::cout << std::endl;

  Indent( spaces ); std::cout << "|";
  std::cout << " numTHyposUsed = " << m_numTHyposUsedInProblem;
  std::cout << ", logLikelihood = " << m_logLikelihood;
  std::cout << std::endl;

  Indent( spaces ); std::cout << "| tHypo's:";
  k = 0;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    if( k++ >= 3 )
    {
      std::cout << std::endl;
      Indent( spaces ); std::cout << "|         ";
      k = 0;
    }

    std::cout << " "; (*tHypoPtr).print();
  }
  std::cout << std::endl;
}

