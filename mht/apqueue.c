/*********************************************************************
 * FILE: apqueue.C                                                   *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    6 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for virtual priority queues of solutions to            *
 *   assignment problems.  See apqueue.H for details.                *
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

#include <math.h>

#include "apqueue.h"
/*DBG BEGIN */
#define FIELD_WIDTH 15
static void printProblem( ROW_COL_COST *g_rcc, int g_numRCCs,
                          int g_numRows, int g_numCols,
                          VECTOR_OF< double > &rowLow,
                          VECTOR_OF< double > &colLow )
{
  int row, col;
  int i, k;

  i = 0;
  for( row = -1; row < g_numRows; row++ )
  {
    printf( " " );

    for( col = -1; col < g_numCols; col++ )
    {
      k = 0;
      if( i < g_numRCCs &&
          g_rcc[ i ].row == row &&
          g_rcc[ i ].col == col )
      {
        printf( " (%5.2f", g_rcc[ i ].cost ); k += 7;
        i++;
        while( i < g_numRCCs &&
               g_rcc[ i ].row == row &&
               g_rcc[ i ].col == col )
        {
          printf( ",%5.2f", g_rcc[ i ].cost ); k += 6;
          i++;
        }
        printf( ")" ); k += 1;
      }
      else
      {
        printf( " ."); k += 2;
      }
      while( k < FIELD_WIDTH )
      {
        printf( " " ); k += 1;
      }
    }

    if( row >= 0 )
      printf( " -> %5.2f", rowLow[ row ] );
    printf( "\n" );
  }

  for( col = -1; col < g_numCols; col++ )
  {
    k = 0;
    if( col >= 0 )
    {
      printf( " %5.2f", colLow[ col ] );
      k += 6;
    }
    while( k < FIELD_WIDTH )
    {
      printf( " " ); k += 1;
    }
  }
  printf( "\n" );
  printf( "  number of rcc's = %d\n", g_numRCCs );
}
/*DBG END */

/*-------------------------------------------------------------------*
 | Constants
 *-------------------------------------------------------------------*/

static const int IGNORE_THIS = 0x7FFFFFFF;
static const double PRECISION = 1000000.;

/*-------------------------------------------------------------------*
 | apqSOLUTION::setup() -- common part of all constructors for
 |                         apqSOLUTION's
 |
 | This takes the base solution and the assignment problem as input,
 | and computes a lower bound on the solution cost.
 *-------------------------------------------------------------------*/

void apqSOLUTION::setup( void **baseSolutionTag,
                         ROW_COL_COST *rcc,
                         double parentCost )
{
  BGN

#ifdef NEVER
  static VECTOR_OF< double > lowestCostForRow;
  static VECTOR_OF< char > rowIsInProblem;
  int numRowsInProblem;
  static VECTOR_OF< double > lowestCostForCol;
  static VECTOR_OF< char > colIsInProblem;
  int numColsInProblem;
  double rowTotal;
  double colTotal;
  int row;
  int col;
  int i;
#endif

  memcpy( m_rcc, rcc, m_numRCCs * sizeof( *m_rcc ) );

  if( m_baseSolutionSize > 0 )
  {
    m_baseSolutionTag = new void*[ m_baseSolutionSize ];
    memcpy( m_baseSolutionTag, baseSolutionTag,
            m_baseSolutionSize * sizeof( *m_baseSolutionTag ) );
  }

#ifdef NEVER
  lowestCostForRow.resize( m_numRows );
  rowIsInProblem.resize( m_numRows );
  rowIsInProblem.clear();
  numRowsInProblem = 0;

  lowestCostForCol.resize( m_numCols );
  colIsInProblem.resize( m_numCols );
  colIsInProblem.clear();
  numColsInProblem = 0;

  for( i = 0; i < m_numRCCs; i++ )
  {
    row = m_rcc[ i ].row;
    if( row >= 0 )
      if( ! rowIsInProblem[ row ] ||
          m_rcc[ i ].cost < lowestCostForRow[ row ] )
      {
        rowIsInProblem[ row ] = 1;
        lowestCostForRow[ row ] = m_rcc[ i ].cost;
      }

    col = m_rcc[ i ].col;
    if( col >= 0 )
      if( ! colIsInProblem[ col ] ||
          m_rcc[ i ].cost < lowestCostForCol[ col ] )
      {
        colIsInProblem[ col ] = 1;
        lowestCostForCol[ col ] = m_rcc[ i ].cost;
      }
  }

  rowTotal = 0;
  for( row = 0; row < m_numRows; row++ )
    if( rowIsInProblem[ row ] )
    {
      numRowsInProblem++;
      rowTotal += lowestCostForRow[ row ];
    }

  colTotal = 0;
  for( col = 0; col < m_numCols; col++ )
    if( colIsInProblem[ col ] )
    {
      numColsInProblem++;
      colTotal += lowestCostForCol[ col ];
    }

  if( numRowsInProblem > numColsInProblem ||
      (numRowsInProblem == numColsInProblem && rowTotal > colTotal ) )
    m_cost = m_baseCost + rowTotal;
  else
    m_cost = m_baseCost + colTotal;

  if( m_cost < parentCost )
#endif
    m_cost = parentCost;

#ifdef NEVER
  m_cost = floor( m_cost * PRECISION ) / PRECISION;
#endif
}

/*-------------------------------------------------------------------*
 | apqSOLUTION::solve() -- actually solve the problem
 *-------------------------------------------------------------------*/

void apqSOLUTION::solve( void )
{
  BGN

  static VECTOR_OF< void * > solutionTag;
  int solutionSize;
  int i;

  if( m_solutionTag != 0 )
    return;

  for ( i=0; i<m_numRCCs;i++)
     {
       int row = m_rcc[i].row;
       int col = m_rcc[i].col;

#ifdef SDBG
       printf("before BestAssign  m_rcc=%d row=%d col=%d tag=%d\n",&(m_rcc[i]),row,col,m_rcc[i].tag);
#endif

     }

   m_cost = FindBestAssignment( m_rcc, m_numRCCs, m_numRows, m_numCols,
                               solutionTag, &solutionSize );
  if( m_cost == UNSOLVABLE )
    return;
  m_cost += m_baseCost;

  m_solutionSize = m_baseSolutionSize + solutionSize;
  m_solutionTag = new void*[ m_solutionSize ];

  if( m_baseSolutionSize > 0 )
  {
    memcpy( m_solutionTag, m_baseSolutionTag,
            m_baseSolutionSize * sizeof( *m_solutionTag ) );
    delete [] m_baseSolutionTag;
    m_baseSolutionTag = 0;
  }

  for( i = 0; i < solutionSize; i++ )
    m_solutionTag[ m_baseSolutionSize + i ] = solutionTag[ i ];
}

/*-------------------------------------------------------------------*
 | apqSOLUTION::partition() -- partition an assignment problem
 |
 | See apqueue.H for a description of the overall algorithm.
 |
 | Note: this routine is destructive.  After it is executed, the
 | apqSOLUTION has nothing left in its list of possible assignments.
 *-------------------------------------------------------------------*/

void apqSOLUTION::partition()
{
  BGN

  void *doomedRCCtag;
  ROW_COL_COST doomedRCC;
  char rowIsNotEmpty;
  char colIsNotEmpty;
  apqSOLUTION *solution;
  int i, k;

#ifdef SDBG
    for( i = 0; i < m_numRCCs; i++ )
       printf("m_rcc=%d row=%d col=%d tag=%d\n",&m_rcc[i],m_rcc[i].row,m_rcc[i].col,m_rcc[i].tag);
#endif

  /* loop through all the assignments in the solution (not counting the
     ones in the base solution) */

  while( m_baseSolutionSize < m_solutionSize )
  {
    /* find the ROW_COL_COST structure for this assignment -- this
       structure is "doomed" to be removed from the assignment
       problem */
    doomedRCCtag = m_solutionTag[ m_baseSolutionSize ];

#ifdef SDBG
    printf("m_baseSolnSize=%d doomedRCCtag=%d m_numRCCs=%d\n",m_baseSolutionSize,doomedRCCtag,m_numRCCs);
#endif

    for( i = 0; i < m_numRCCs; i++ )
      if( m_rcc[ i ].tag == doomedRCCtag )
        break;
/*
    #ifdef TSTBUG */
      if( i >= m_numRCCs )
        THROW_ERR( "ASSIGNMENT_PQUEUE looking for non-existant tag" )
/*    #endif*/

#ifdef SDBG
    printf("doomedRCC was found at i=%d\n",i);
#endif

    doomedRCC = m_rcc[ i ];
    /* a row of -1, or a column of -1, should be ignored, since these
       aren't really nodes */
    if( doomedRCC.row < 0 )
      doomedRCC.row = IGNORE_THIS;
    if( doomedRCC.col < 0 )
      doomedRCC.col = IGNORE_THIS;

    /* remove the doomed ROW_COL_COST from the assignment problem,
       maintaining the list of ROW_COL_COST's in order */
    m_numRCCs--;
    if( i < m_numRCCs )
      memmove( &m_rcc[ i ], &m_rcc[ i + 1 ],
               (m_numRCCs - i) * sizeof( *m_rcc ) );

    /* What we have now is the problem called P' in the header comments
       for apqueue.H, after <r,c,s> has been removed. */

    /* find out if the row and column can possibly be assigned to
       other things */
    rowIsNotEmpty = (doomedRCC.row == IGNORE_THIS);
    colIsNotEmpty = (doomedRCC.col == IGNORE_THIS);
    for( i = 0; i < m_numRCCs; i++ )
    {
      if( m_rcc[ i ].row == doomedRCC.row )
      {
        rowIsNotEmpty = 1;
        if( colIsNotEmpty )
          break;
      }
      if( m_rcc[ i ].col == doomedRCC.col )
      {
        colIsNotEmpty = 1;
        if( rowIsNotEmpty )
          break;
      }
    }

    /* if they can, then proceed find a lower limit on the cost of
       the best solution to this problem, make a problem/solution pair,
       and place it on the list */
    if( rowIsNotEmpty && colIsNotEmpty )
    {
      /* the constructor both finds a lower limit on the cost of the
         best solution and makes the problem/solution pair */
      solution = new apqSOLUTION( m_problemTag,
                                  m_baseCost,
                                  m_cost,
                                  m_solutionTag,
                                  m_baseSolutionSize,
                                  m_rcc,
                                  m_numRCCs,
                                  m_numRows,
                                  m_numCols );

      /* the pair is invalid if no solution is possible */
      if( solution->isValid() )
        append( solution );
      else
        delete solution;
    }

    /* remove all the possible assignments for this row and column */
    if( (rowIsNotEmpty && doomedRCC.row != IGNORE_THIS) ||
        (colIsNotEmpty && doomedRCC.col != IGNORE_THIS) )
    {
      for( i = 0;
           i < m_numRCCs &&
           m_rcc[ i ].row != doomedRCC.row &&
           m_rcc[ i ].col != doomedRCC.col;
           i++ )
        ;

      k = i;
      while( i < m_numRCCs )
      {
        if( m_rcc[ i ].row != doomedRCC.row &&
            m_rcc[ i ].col != doomedRCC.col )
          m_rcc[ k++ ] = m_rcc[ i ];
        i++;
      }

      m_numRCCs = k;
    }

    /* add the "doomed" assignment to the base solution */
    m_baseCost += doomedRCC.cost;
    m_baseSolutionSize++;

#ifdef SDBG
    for( i = 0; i < m_numRCCs; i++ )
       printf("m_rcc=%d row=%d col=%d tag=%d\n",&m_rcc[i],m_rcc[i].row,m_rcc[i].col,m_rcc[i].tag);
#endif

  }
}

/*-------------------------------------------------------------------*
 | apqSOLUTION::getSolutionTags() -- get the solution from a
 |                                   problem/solution pair
 *-------------------------------------------------------------------*/

void apqSOLUTION::
getSolutionTags( VECTOR_OF< void * > &solutionTag, int *solutionSize )
{
  BGN

  int i;

  solutionTag.resize( m_solutionSize );
  for( i = 0; i < m_solutionSize; i++ )
    solutionTag[ i ] = m_solutionTag[ i ];
  *solutionSize = m_solutionSize;

}

/*-------------------------------------------------------------------*
 | apqSOLUTION::print() -- for debugging
 *-------------------------------------------------------------------*/

void apqSOLUTION::print()
{
  BGN

  int i, j;
  ROW_COL_COST *rcc;

  if( m_cost == UNSOLVABLE )
    cout << "#";
  else
    cout << m_cost;

  for( i = 0; i < m_solutionSize; i++ )
  {
    rcc = (ROW_COL_COST *)m_solutionTag[ i ];
    if( i < m_baseSolutionSize )
      cout << " [" << rcc->row << ","
                   << rcc->col << ","
                   << rcc->cost << "]";
    else
      cout << " (" << rcc->row << ","
                   << rcc->col << ","
                   << rcc->cost << ")";
  }

  for( i = 0; i < m_numRCCs; i++ )
  {
    for( j = 0; j < m_solutionSize; j++ )
      if( m_solutionTag[ j ] == m_rcc[ i ].tag )
        break;
    if( j >= m_solutionSize )
      cout << "  " << m_rcc[ i ].row << ","
                   << m_rcc[ i ].col << ","
                   << m_rcc[ i ].cost << " ";
  }
}

/*-------------------------------------------------------------------*
 | ASSIGNMENT_PQUEUE::addProblem() -- add a problem to the virtual
 |                                    priority queue
 *-------------------------------------------------------------------*/

void ASSIGNMENT_PQUEUE::addProblem( void *problemTag,
                                    ROW_COL_COST *rcc,
                                    int numRCCs,
                                    int numRows,
                                    int numCols )
{
  BGN

  apqSOLUTION *solution = new apqSOLUTION( problemTag,
                                           rcc,
                                           numRCCs,
                                           numRows,
                                           numCols );
  if( solution->isValid() )
    m_solutionList.prepend( solution );
  else
    delete solution;
}

/*-------------------------------------------------------------------*
 | ASSIGNMENT_PQUEUE::removeProblem() -- remove a problem from the
 |                                       virtual priority queue
 *-------------------------------------------------------------------*/

void ASSIGNMENT_PQUEUE::removeProblem( void *problemTag )
{
  BGN

  PTR_INTO_iDLIST_OF< apqSOLUTION > ptr;

  LOOP_DLIST( ptr, m_solutionList )
    if( (*ptr).getProblemTag() == problemTag )
      ptr.remove();
}

/*-------------------------------------------------------------------*
 | ASSIGNMENT_PQUEUE::getNextSolution() -- get the next-lowest-cost
 |                                         solution
 *-------------------------------------------------------------------*/

void *ASSIGNMENT_PQUEUE::
getNextSolution( VECTOR_OF< void * > &solutionTag, int *solutionSize )
{
  BGN

  void *problemTag;

  findBestSolution();
  if( m_bestSolution == 0 )
    return 0;

  problemTag = m_bestSolution->getProblemTag();
  m_bestSolution->getSolutionTags( solutionTag, solutionSize );

  m_bestSolution->partition();
  delete m_bestSolution;
  m_bestSolution = 0;

  return problemTag;
}

/*-------------------------------------------------------------------*
 | ASSIGNMENT_PQUEUE::findBestSolution() -- find the best solution on
 |                                          the list of
 |                                          problem/solution pairs
 *-------------------------------------------------------------------*/

void ASSIGNMENT_PQUEUE::findBestSolution()
{
  BGN

  PTR_INTO_iDLIST_OF< apqSOLUTION > ptr;
  double estimatedCost;

  m_bestSolution = 0;

  while( m_bestSolution == 0 )
  {
    if( m_solutionList.isEmpty() )
      return;

    m_bestSolution = m_solutionList.getHead();
    LOOP_DLIST( ptr, m_solutionList )
    {
      if( (*ptr).getCost() < m_bestSolution->getCost() )
        m_bestSolution = ptr.get();
      else if( (*ptr).getCost() == m_bestSolution->getCost() )
        if( (*ptr).isSolved() == m_bestSolution->isSolved() )
        {
          if( (*ptr).getNumRCCs() < m_bestSolution->getNumRCCs() )
            m_bestSolution = ptr.get();
        }
        else if( (*ptr).isSolved() )
          m_bestSolution = ptr.get();
    }

    if( ! m_bestSolution->isSolved() )
    {
      estimatedCost = m_bestSolution->getCost();
      m_bestSolution->solve();

/*
      #ifdef TSTBUG
*/
        if( m_bestSolution->getCost()- estimatedCost<-0.001 )
          THROW_ERR( "Estimated cost (" << estimatedCost << ")"
                     " higher than actual (" <<
                     m_bestSolution->getCost() << ")" );
/*
      #endif
*/

      if( m_bestSolution->getCost() == UNSOLVABLE )
      {
        delete m_bestSolution;
        m_bestSolution = 0;
      }
      else if( m_bestSolution->getCost() > estimatedCost )
        m_bestSolution = 0;
    }
  }
}

