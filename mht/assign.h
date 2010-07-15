
/*********************************************************************
 * FILE: assign.H                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    5 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Declarations for assignment problems and routines to solve      *
 *   them.                                                           *
 *                                                                   *
 *              Representation of Assignment Problems                *
 *                                                                   *
 *   An assignment problem is represented as an array of possible    *
 *   assignments.  Each possible assignment is stored in a           *
 *   ROW_COL_COST structure.  This name is historical.  Originally,  *
 *   I was representing assignment problems in the traditional way   *
 *   of using an assignment matrix -- hence the references to        *
 *   "rows" and "columns".  But matrices proved too limited, and     *
 *   made for messy code, so I switched to bipartite graphs.  Now,   *
 *   "row" refers to nodes on one side of the graph, "col" refers    *
 *   to nodes on the other, and a ROW_COL_COST structure stores      *
 *   the cost of assigning one "row" to one "col" (the cost of an    *
 *   arc between the row and the column).                            *
 *                                                                   *
 *   Rows and columns are identified with positive integers.  If a   *
 *   row may be left unassigned, then the problem representation     *
 *   should include a ROW_COL_COST with that row number, a column    *
 *   number of -1, and the cost of leaving the row unassigned.       *
 *   Similarly, if a column may be left unassigned, then there       *
 *   should be a ROW_COL_COST with that column number, a row number  *
 *   of -1, and the cost of leaving the column unassigned.           *
 *                                                                   *
 *   There may be more than one ROW_COL_COST for the same pair of    *
 *   rows and columns, giving different costs of assigning them in   *
 *   different ways.  Of course, if FindBestAssignment() (see below) *
 *   is used to find the best solution to the problem, then it will  *
 *   ignore all but the lowest-cost possible assignment between each *
 *   row-column pair.  However, the higher-cost assignments can      *
 *   become important when using ASSIGNMENT_PQUEUE objects to find   *
 *   the second-best solution, third-best, and so on.                *
 *                                                                   *
 *   Some row and column numbers might not appear in any             *
 *   ROW_COL_COST's.  These will be ignored.                         *
 *                                                                   *
 *   Each ROW_COL_COST structure contains an additional field that   *
 *   is used to identify it.  A solution to an assignment problem    *
 *   (see below) is given as a VECTOR of these "tag" fields,         *
 *   identifying a set of assignments used.  The tag field is        *
 *   declared as a void pointer.  By default, it points to the       *
 *   ROW_COL_COST structure itself, but it may be set to point to    *
 *   any object with which that particular assignment is associated. *
 *   If it is set to 0, it will never appear in the solution VECTOR, *
 *   even if the assignment is used.                                 *
 *                                                                   *
 *   The ROW_COL_COST structures must be sorted first by row, then   *
 *   by column, then by cost.  The routine SortAssignmentProblem()   *
 *   is provided for this purpose (see below).                       *
 *                                                                   *
 *   In addition to the array of ROW_COL_COST structures, and the    *
 *   number of structures in the array, most routines that take      *
 *   assignment problems as input also want to be given the number   *
 *   of rows and the number of columns.  These are used to optimize  *
 *   memory usage, and need not be exact.  As long as the given      *
 *   number of rows is greater than the largest row number appearing *
 *   in any ROW_COL_COST, and the number of columns is greater than  *
 *   the largest column number, then the routine will work.          *
 *                                                                   *
 *   To summarize, an assingment problem is given to a routine by    *
 *   means of four variables:                                        *
 *                                                                   *
 *     ROW_COL_COST *rcc -- A sorted array of possible assignments.  *
 *     int numRCCs -- The number of entries in rcc.                  *
 *     int numRows -- Greater than the largest row number in rcc.    *
 *     int numCols -- Greater than the largest column number in rcc. *
 *                                                                   *
 *                     SortAssignmentProblem()                       *
 *                                                                   *
 *   This takes a list of ROW_COL_COST's and calls qsort() to sort   *
 *   it properly.                                                    *
 *                                                                   *
 *                     FindBestAssignment()                          *
 *                                                                   *
 *   This takes an assignment problem, represented in the way        *
 *   described above, and finds the set of assignments that has the  *
 *   lowest total cost and assigns each row to one column, and each  *
 *   column to one row (some rows and columns might be left          *
 *   unassigned, if there are ROW_COL_COST's that assign them        *
 *   to -1).  The solution is returned in a VECTOR_OF< void * > as   *
 *   a list of ROW_COL_COST tags.                                    *
 *                                                                   *
 *   The return value of FindBestAssignment() gives the total cost   *
 *   of the assignment found.  If there are no solutions, then       *
 *   FindBestAssignment() returns UNSOLVABLE.                        *
 *                                                                   *
 * EXAMPLE:                                                          *
 *                                                                   *
 *   Code for an easy assignment problem --                          *
 *                                                                   *
 *     VECTOR_OF< void * > solution;                                 *
 *     int solutionSize;                                             *
 *     ROW_COL_COST rcc[ 7 ];                                        *
 *     int i;                                                        *
 *                                                                   *
 *     rcc[0].set( 0, 0, 10, "Row 0 assigned to col 0, high-cost" ); *
 *     rcc[1].set( 0, 0, 1, "Row 0 assigned to col 0, low-cost" );   *
 *     rcc[2].set( 1, 0, 100, "Row 1 assigned to col 0" );           *
 *     rcc[3].set( 1, 2, 1, "Row 1 assigned to col 2" );             *
 *     rcc[4].set( 2, 2, 10, "Row 2 assigned to col 2" );            *
 *     rcc[5].set( 2, -1, 10, "Row 2 unassigned, high-cost" );       *
 *     rcc[6].set( 2, -1, 1, "Row 2 unassigned, low-cost" );         *
 *                                                                   *
 *     SortAssignmentProblem( rcc, 7 );                              *
 *     FindBestAssignment( rcc, 7, 3, 3, solution, &solutionSize );  *
 *                                                                   *
 *     for( i = 0; i < solutionSize; i++ )                           *
 *       cout << (char *)solution[ i ] << endl;                      *
 *                                                                   *
 *   The assignment problem coded here can be drawn as a graph like  *
 *   this --                                                         *
 *                                                                   *
 *      ROW                     COL                                  *
 *                                                                   *
 *                  10                                               *
 *           -----------------                                       *
 *     +---+/                 \+---+                                 *
 *     | 0 |-------------------| 0 |                                 *
 *     +---+         1        /+---+                                 *
 *                       -----                                       *
 *                 -----/                                            *
 *           -----/ 100                                              *
 *     +---+/                  +---+                                 *
 *     | 1 |                   | 1 |                                 *
 *     +---+\                  +---+                                 *
 *           -----\  1                                               *
 *                 -----\                                            *
 *                       -----                                       *
 *     +---+                  \+---+                                 *
 *     | 2 |-------------------| 2 |                                 *
 *     +---+\       10         +---+                                 *
 *       |   -----\                                                  *
 *        \        -----\                                            *
 *         |        10   ----- unassigned                            *
 *          \                                                        *
 *           ----------------- unassigned                            *
 *                   1                                               *
 *                                                                   *
 *   Each possible assignment is tagged with a string.  When the     *
 *   code is executed, the strings associated with the best set of   *
 *   assignments will be printed out.  So the result should be --    *
 *                                                                   *
 *     Row 0 assigned to col 0, low-cost                             *
 *     Row 1 assigned to col 2                                       *
 *     Row 2 unassigned, low-cost                                    *
 *                                                                   *
 *   Note that, since no ROW_COL_COST structure mentions column 1,   *
 *   it is ignored.                                                  *
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

#ifndef ASSIGN_H
#define ASSIGN_H
#include <math.h>
#include "except.h"
#include "vector.h"

#ifdef DECLARE_ASSIGN
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

GLOBAL long G_numAssignCalls;
GLOBAL long G_totalAssignSizes;
GLOBAL long G_maxAssignSize;
static const double UNSOLVABLE = INFINITY;

/*-------------------------------------------------------------------*
 | ROW_COL_COST -- structure for representing a possible assignment
 *-------------------------------------------------------------------*/

struct ROW_COL_COST
{
  int row;
  int col;
  double cost;
  void *tag;

  ROW_COL_COST():
    row( 0 ),
    col( 0 ),
    cost( 0. ),
    tag( 0 )
  {
  }

  ROW_COL_COST( int rowArg, int colArg, double costArg ):
    row( rowArg ),
    col( colArg ),
    cost( costArg ),
    tag( this )
  {
  }

  ROW_COL_COST( int rowArg, int colArg, double costArg, void *tagArg ):
    row( rowArg ),
    col( colArg ),
    cost( costArg ),
    tag( tagArg )
  {
  }

  void set( int rowArg, int colArg, double costArg )
  {
    row = rowArg;
    col = colArg;
    cost = costArg;
    tag = this;
  }

  void set( int rowArg, int colArg, double costArg, void *tagArg )
  {
    row = rowArg;
    col = colArg;
    cost = costArg;
    tag = tagArg;
  }
};

/*-------------------------------------------------------------------*
 | Routine prototypes
 *-------------------------------------------------------------------*/

void SortAssignmentProblem( ROW_COL_COST *rcc, int numRCCs );
double FindBestAssignment( ROW_COL_COST *rcc, int numRCCs,
                           int numRows, int numCols,
                           VECTOR_OF< void * > &tag, int *numTags );

#undef GLOBAL
#endif

