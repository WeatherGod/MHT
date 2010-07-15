
/*********************************************************************
 * FILE: bassign.H                                                   *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    5 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for brute-force assignment code.  Used only in test    *
 *   programs.                                                       *
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

#include "bassign.h"

static const int NO_SUCH_THING = 0x7FFFFFFF;
static const int MAX_INTEGER = 0x7FFFFFFF;
static const double SOLVED = 0.;

typedef
  VECTOR_OF< VECTOR_OF< ROW_COL_COST > >
RCC_ARRAY_2D;

static void setupProblem( ROW_COL_COST *rcc, int numRCCs, int, int );
static double solveProblem();
static void search( int level, double costSoFar );
static double storeSolution( VECTOR_OF< void * > &tag, int *numTags );

static int g_minRow;
static int g_maxRow;
static int g_minCol;
static int g_maxCol;

static int g_numRows;
static int g_numCols;

static RCC_ARRAY_2D g_rccForRow;
static VECTOR_OF< int > g_numRCCsForRow;

static RCC_ARRAY_2D g_rccForCol;
static VECTOR_OF< int > g_numRCCsForCol;

static VECTOR_OF< int > g_mateForRow;
static VECTOR_OF< int > g_mateForCol;

static VECTOR_OF< void * > g_solution;
static int g_solutionSize;

static VECTOR_OF< void * > g_bestSolution;
static int g_bestSolutionSize;
static double g_bestSolutionCost;

double BFindBestAssignment( ROW_COL_COST *rcc, int numRCCs,
                            int numRows, int numCols,
                            VECTOR_OF< void * > &tag, int *numTags )
{
  BGN

  if( numRCCs == 0 )
  {
    *numTags = 0;
    return 0.;
  }

  setupProblem( rcc, numRCCs, numRows, numCols );

  if( solveProblem() == UNSOLVABLE )
    return UNSOLVABLE;
  else
    return storeSolution( tag, numTags );
}

static void setupProblem( ROW_COL_COST *rcc, int numRCCs,
                          int numRows, int numCols )
{
  BGN

  int row, col;
  int i;

  g_minRow = MAX_INTEGER;
  g_maxRow = -MAX_INTEGER;
  g_minCol = MAX_INTEGER;
  g_maxCol = -MAX_INTEGER;
  g_numRCCsForRow.resize( numRows );
  g_numRCCsForCol.resize( numCols );

  g_numRCCsForRow.clear();
  g_numRCCsForCol.clear();
  for( i = 0; i < numRCCs; i++ )
  {
    row = rcc[ i ].row;
    col = rcc[ i ].col;

    if( row >= 0 )
      g_numRCCsForRow[ row ]++;
    if( col >= 0 )
      g_numRCCsForCol[ col ]++;
  }

  g_rccForRow.resize( numRows );
  for( i = 0; i < numRows; i++ )
    g_rccForRow[ i ].resize( g_numRCCsForRow[ i ] );
  g_rccForCol.resize( numCols );
  for( i = 0; i < numCols; i++ )
    g_rccForCol[ i ].resize( g_numRCCsForCol[ i ] );

  g_numRCCsForRow.clear();
  g_numRCCsForCol.clear();
  for( i = 0; i < numRCCs; i++ )
  {
    row = rcc[ i ].row;
    col = rcc[ i ].col;

    if( row >= 0 )
      g_rccForRow[ row ][ g_numRCCsForRow[ row ]++ ] = rcc[ i ];
    if( col >= 0 )
      g_rccForCol[ col ][ g_numRCCsForCol[ col ]++ ] = rcc[ i ];

    if( row < g_minRow )
      g_minRow = row;
    if( row > g_maxRow )
      g_maxRow = row;

    if( col < g_minCol )
      g_minCol = col;
    if( col > g_maxCol )
      g_maxCol = col;
  }

  g_numRows = g_maxRow - g_minRow + 1;
  g_numCols = g_maxCol - g_minCol + 1;
}

static double solveProblem()
{
  BGN

  int i;

  g_mateForRow.resize( g_minRow, g_maxRow );
  g_mateForCol.resize( g_minCol, g_maxCol );
  g_solution.resize( g_numRows + g_numCols - 1 );
  g_solutionSize = 0;
  g_bestSolution.resize( g_numRows + g_numCols - 1 );
  g_bestSolutionSize = 0;
  g_bestSolutionCost = INFINITY;

  for( i = g_minRow; i <= g_maxRow; i++ )
    g_mateForRow[ i ] = NO_SUCH_THING;

  for( i = g_minCol; i <= g_maxCol; i++ )
    g_mateForCol[ i ] = NO_SUCH_THING;

  search( 0, 0. );

  if( g_bestSolutionCost == INFINITY )
    return UNSOLVABLE;
  else
    return SOLVED;
}

static void search( int level, double costSoFar )
{
  BGN

  int row, col;
  int i;

  if( level <= g_maxRow )
  {
    TRACE

    row = level;

    if( row < g_minRow ||
        g_mateForRow[ row ] != NO_SUCH_THING ||
        g_numRCCsForRow[ row ] == 0 )
      search( level + 1, costSoFar );
    else
      for( i = 0; i < g_numRCCsForRow[ row ]; i++ )
      {
        col = g_rccForRow[ row ][ i ].col;
        if( g_mateForCol[ col ] == NO_SUCH_THING )
        {
          g_mateForRow[ row ] = col;
          if( col >= 0 )
            g_mateForCol[ col ] = row;
          g_solution[ g_solutionSize++ ] = g_rccForRow[ row ][ i ].tag;

          search( level + 1, costSoFar + g_rccForRow[ row ][ i ].cost );

          g_solutionSize--;
          g_mateForCol[ col ] = NO_SUCH_THING;
          g_mateForRow[ row ] = NO_SUCH_THING;
        }
      }
  }
  else if( level <= g_maxRow + 1 + g_maxCol )
  {
    TRACE

    col = level - g_maxRow - 1;

    if( col < g_minCol ||
        g_mateForCol[ col ] != NO_SUCH_THING ||
        g_numRCCsForCol[ col ] == 0 )
      search( level + 1, costSoFar );
    else
      for( i = 0; i < g_numRCCsForCol[ col ]; i++ )
      {
        row = g_rccForCol[ col ][ i ].row;
        if( g_mateForRow[ row ] == NO_SUCH_THING )
        {
          g_mateForCol[ col ] = row;
          if( row >= 0 )
            g_mateForRow[ row ] = col;
          g_solution[ g_solutionSize++ ] = g_rccForCol[ col ][ i ].tag;

          search( level + 1, costSoFar + g_rccForCol[ col ][ i ].cost );

          g_solutionSize--;
          g_mateForRow[ row ] = NO_SUCH_THING;
          g_mateForCol[ col ] = NO_SUCH_THING;
        }
      }
  }
  else if( costSoFar < g_bestSolutionCost )
  {
    TRACE

    for( i = 0; i < g_solutionSize; i++ )
      g_bestSolution[ i ] = g_solution[ i ];
    g_bestSolutionSize = g_solutionSize;
    g_bestSolutionCost = costSoFar;
  }
}

static double storeSolution( VECTOR_OF< void * > &tag, int *numTags )
{
  BGN

  int i;

  tag.resize( g_bestSolutionSize - 1 );
  for( i = 0; i < g_bestSolutionSize; i++ )
    tag[ i ] = g_bestSolution[ i ];
  *numTags = g_bestSolutionSize;

  return g_bestSolutionCost;
}

