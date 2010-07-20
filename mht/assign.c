
/*********************************************************************
 * FILE: assign.C                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    5 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for solving assignment problems.  See assign.H for     *
 *   details about usage.                                            *
 *                                                                   *
 * IMPLEMENTATION NOTES:                                             *
 *                                                                   *
 *   This uses the Hungarian method, basically as described in       *
 *                                                                   *
 *     Cooper and Steinberg                                          *
 *     Methods and Applications of Linear Programming                *
 *     1974, W. B. Saunders Company, Philadelphia, PA                *
 *     pp 235-257                                                    *
 *                                                                   *
 *   I haven't implemented most of the optimizations they suggest    *
 *   (they didn't make big enough differences to warrant the added   *
 *   complexity).                                                    *
 *                                                                   *
 *   Problems are solved by FindBestAssignment().  This begins by    *
 *   calling setupProblem(), which initializes a bunch of static     *
 *   globals to describe the problem in a way that can be solved by  *
 *   the Hungarian method.  There are two changes that               *
 *   setupProblem() must make to the original representation of the  *
 *   problem:                                                        *
 *                                                                   *
 *     1. Remove duplicate ROW_COL_COST's.  All but the lowest-cost  *
 *        rcc for any given row/column pair must be discarded.  They *
 *        are simply not copied into the global variables.           *
 *                                                                   *
 *     2. Make the problem square and handle the possibility of not  *
 *        assigning some rows and columns.  This is done by adding   *
 *        to the bipartite-graph a mirror-image of itself.  A new    *
 *        row is added for each column, and a new column is added    *
 *        each row.  These new rows are called "anti-columns" and    *
 *        the new columns are called "anti-rows".  If a column is to *
 *        be left unassigned, it will actually be assigned to its    *
 *        anti-column.  The same is done with rows.                  *
 *                                                                   *
 *        Anti-rows and anti-columns are identified by negative      *
 *        numbers.                                                   *
 *                                                                   *
 *        If row r can be left unassigned, then a possible           *
 *        assignment is made between row r and its anti-column.  The *
 *        cost of the assignment comes from the ROW_COL_COST that    *
 *        indicated that row r can be left unassigned.  The same is  *
 *        done with columns that can be left unassigned.             *
 *                                                                   *
 *        If a ROW_COL_COST gives a possible assignment between row  *
 *        r and column c, then there will be a 0 cost possible       *
 *        assignment between the corresponding anti-row and anti-    *
 *        column.  This ensures that if row r and column c are       *
 *        assigned to each other in the best solution, then the      *
 *        corresponding anti-row and anti-column can also be         *
 *        assigned to each other.                                    *
 *                                                                   *
 *        Once this mirror-image "anti-graph" is added, we have a    *
 *        square assignment problem of size                          *
 *        (numRows + numCols) * (numRows + numCols), and we'll be    *
 *        looking for solutions that assign each row to exactly      *
 *        one column, and each column to exactly one row.            *
 *                                                                   *
 *   Conceptually, the Hungarian method operates on progressively    *
 *   larger subgraphs of the original problem, trying to find a      *
 *   complete matching on each one.  The search for the matching     *
 *   proceeds regardless of cost; information about the costs of the *
 *   arcs is used only in choosing the set of arcs to be included    *
 *   in the subgraph.                                                *
 *                                                                   *
 *   In this code, the subgraph is represented by the array          *
 *   g_possMatesForRow.  For each row, g_possMatesForRow gives a     *
 *   list of columns that the row may be assigned to.  Arcs          *
 *   (ROW_COL_COST's) that are used in the subgraph are referred to  *
 *   in the comments as "enabled".                                   *
 *                                                                   *
 *   The routine jumpStartProblem() builds the initial subgraph, and *
 *   tries to solve the problem in a trivial way (by simply          *
 *   making the lowest-cost assignments when it can).  If this       *
 *   solution is successful, nothing else need be done.              *
 *                                                                   *
 *   The routine solveProblem() contains the main loop.  While the   *
 *   number of assignments that have been made is lower than the     *
 *   number that are needed for a complete matching, it tries to     *
 *   make new assignments by calling augment().  This routine tries  *
 *   to find a way to make more assignments than have already been   *
 *   made, using only the current subgraph.  If it fails, then       *
 *   solveProblem() changes the current subgraph, using information  *
 *   from the way that augment() failed.  Usually, arcs are added    *
 *   by this procedure.  Sometimes, a few might be removed.          *
 *                                                                   *
 *   When the number of assignments that have been made are the same *
 *   as the number that are needed, an optimal solution has been     *
 *   found.                                                          *
 *                                                                   *
 *   If solveProblem() can't improve the subgraph (a test that is    *
 *   burried in the middle of the loop), then the problem cannot be  *
 *   solved, and it returns UNSOLVABLE.                              *
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

#define DECLARE_ASSIGN

#include <stdlib.h>

#include "queue.h"
#include "assign.h"

/*-------------------------------------------------------------------*
 | Constants
 *-------------------------------------------------------------------*/

static const int NO_SUCH_THING = 0x7FFFFFFF;
static const int MAX_INTEGER = 0x7FFFFFFF;
static const int END_OF_PATIENCE = 10000;
static const double SOLVED = 0.;
static const int CANT_AUGMENT = 0;
static const int AUGMENTED = 1;

/*-------------------------------------------------------------------*
 | Static routines
 *-------------------------------------------------------------------*/

static void setupProblem( ROW_COL_COST *rcc, int numRCCs,
                          int numRows, int numCols );
static void jumpStartProblem();
static double solveProblem();
static int augment();
static double storeSolution( VECTOR_OF< void * > &tag, int *numTags );

static inline int anti( int rowOrCol )
{
    return -rowOrCol - 1;
}

/*-------------------------------------------------------------------*
 | Static global variables
 *-------------------------------------------------------------------*/

static int g_minRow, g_maxRow;        // range of row numbers that
//   appear in rcc's
static int g_minCol, g_maxCol;        // range of column numbers that
//   appear in rcc's

static VECTOR_OF< ROW_COL_COST > g_rcc; // list of rcc's in problem
static int g_numRCCs;                 // number of rcc's in problem

static int g_numAssignmentsNeeded;    // number of assignments required
//   for a complete solution
static int g_numAssignmentsMade;      // number of assignments that have
//   been made so far

static VECTOR_OF< double > g_minCostInRow; // minimum cost of possible
//   assignments for rows
static VECTOR_OF< double > g_minCostInCol; // minimum cost of possible
//   assignments for columns

static VECTOR_OF< VECTOR_OF< int > > g_possMateForRow; // This is a
//   two dimensional array.
//   g_possMateForRow[r][i] is
//   the column number of the
//   i'th column that may be
//   assigned to row r (counting
//   only those rcc's that are
//   currently enabled)
static VECTOR_OF< int > g_numPossMatesForRow; // number of entries in
//   g_possMateForRow for each
//   row

static VECTOR_OF< int > g_mateForRow; // current best column to assign
//   each row to
static VECTOR_OF< int > g_mateForCol; // current best row to assign
//   each column to

static QUEUE_OF< int > g_rowToSearch( 0 ); // next row to search for
//   possible augmentation
static VECTOR_OF< int > g_rowPredecessor; // g_rowPredecessor[r] is the
//   number of the row that led
//   to row r being searched for
//   an augmentation

static VECTOR_OF< char > g_rowIsEssential; // labels for essential rows
static VECTOR_OF< char > g_colIsEssential; // labels for essential
//   columns

/*-------------------------------------------------------------------*
 | SortAssignmentProblem() -- use qsort to sort an array of
 |                            ROW_COL_COST's
 *-------------------------------------------------------------------*/

static int compareRCCs( const void *addr0, const void *addr1 );

void SortAssignmentProblem( ROW_COL_COST *rcc, int numRCCs )
{


    qsort( (void *)rcc, numRCCs, sizeof( *rcc ), compareRCCs );
}

static int compareRCCs( const void *addr0, const void *addr1 )
{


#define rcc0 (*(ROW_COL_COST *)addr0)
#define rcc1 (*(ROW_COL_COST *)addr1)

    register int val;

    if( (val = rcc0.row - rcc1.row) != 0 )
    {
        return val;
    }
    if( (val = rcc0.col - rcc1.col) != 0 )
    {
        return val;
    }
    if( rcc0.cost < rcc1.cost )
    {
        return -1;
    }
    if( rcc0.cost > rcc1.cost )
    {
        return 1;
    }
    return 0;

#undef rcc0
#undef rcc1
}

/*-------------------------------------------------------------------*
 | FindBestAssignment() -- find the lowest-cost solution to an
 |                         assignment problem
 *-------------------------------------------------------------------*/

double FindBestAssignment( ROW_COL_COST *rcc, int numRCCs,
                           int numRows, int numCols,
                           VECTOR_OF< void * > &tag, int *numTags )
{


    G_numAssignCalls++;
    G_totalAssignSizes += numRCCs;
    if( numRCCs > G_maxAssignSize )
    {
        G_maxAssignSize = numRCCs;
    }

    if( numRCCs == 0 )
    {
        *numTags = 0;
        return 0.;
    }

    setupProblem( rcc, numRCCs, numRows, numCols );
    jumpStartProblem();

    if( solveProblem() == UNSOLVABLE )
    {
        return UNSOLVABLE;
    }
    else
    {
        return storeSolution( tag, numTags );
    }
}

/*-------------------------------------------------------------------*
 | setupProblem() -- initialize global variables
 |
 | See header comments for more details.
 *-------------------------------------------------------------------*/

static void setupProblem( ROW_COL_COST *rcc, int numRCCs,
                          int numRows, int numCols )
{


    register int row, col;
    register int i;

    /* fill in all the globals with starting values, and resizes all the
       arrays so that they'll be big enough for the given problem */
    g_minRow = MAX_INTEGER;
    g_maxRow = -MAX_INTEGER;
    g_minCol = MAX_INTEGER;
    g_maxCol = -MAX_INTEGER;
    g_rcc.resize( 2 * numRCCs );
    g_numRCCs = 0;
    g_numAssignmentsNeeded = 0;
    g_numAssignmentsMade = 0;
    g_possMateForRow.resize( -numCols, numRows );
    g_numPossMatesForRow.resize( -numCols, numRows );
    g_numPossMatesForRow.clear();
    g_mateForRow.resize( -numCols, numRows );
    g_mateForCol.resize( -numRows, numCols );
    g_minCostInRow.resize( numRows );
    g_minCostInCol.resize( numCols );
    g_rowToSearch.clear();
    g_rowToSearch.resize( numRows + numCols );
    g_rowPredecessor.resize( -numCols, numRows );
    g_rowIsEssential.resize( -numCols, numRows );
    g_colIsEssential.resize( -numRows, numCols );

    /* convert the problem to a form that can be solved by
       the Hungarian method */
    for( i = 0; i < numRCCs; i++ )
    {
        /* the following test causes us to skip all but the lowest-cost
           possible assignment between any given row and column (this
           relies on the fact that the ROW_COL_COST's are sorted by
           row, column, and then cost) */
        if( i > 0 &&
                rcc[ i ].col == rcc[ i - 1 ].col &&
                rcc[ i ].row == rcc[ i - 1 ].row )
        {
            continue;
        }

        /* if this RCC is saying that a column may be left unassigned,
             make an RCC saying that the column may be assigned to its
             anti-column
           else if it says that a row may be left unassigned,
             make an RCC saying that the row may be assigned to its
             anti-row
           else, it is saying that a row may be assigned to a column
             make an RCC saying that the row may be assigned to the column,
             and an RCC saying that the corresponding anti-row may be
             assigned to the corresponding anti-column */
        if( rcc[ i ].row < 0 )
            g_rcc[ g_numRCCs++ ].set( anti( rcc[ i ].col ),
                                      rcc[ i ].col,
                                      rcc[ i ].cost,
                                      &rcc[ i ] );
        else if( rcc[ i ].col < 0 )
            g_rcc[ g_numRCCs++ ].set( rcc[ i ].row,
                                      anti( rcc[ i ].row ),
                                      rcc[ i ].cost,
                                      &rcc[ i ] );
        else
        {
            g_rcc[ g_numRCCs++ ].set( rcc[ i ].row,
                                      rcc[ i ].col,
                                      rcc[ i ].cost,
                                      &rcc[ i ] );
            g_rcc[ g_numRCCs++ ].set( anti( rcc[ i ].col ),
                                      anti( rcc[ i ].row ),
                                      0.,
                                      0 );
        }
    }

    /* find the true minimum and maximum row and column numbers
       (including anti-rows and anti-columns), and count the maximum
       number of possible mates for each row */
    for( i = 0; i < g_numRCCs; i++ )
    {
        row = g_rcc[ i ].row;
        col = g_rcc[ i ].col;

        if( row < g_minRow )
        {
            g_minRow = row;
        }
        if( row > g_maxRow )
        {
            g_maxRow = row;
        }
        if( col < g_minCol )
        {
            g_minCol = col;
        }
        if( col > g_maxCol )
        {
            g_maxCol = col;
        }

        if( g_numPossMatesForRow[ g_rcc[ i ].row ]++ == 0 )
        {
            g_numAssignmentsNeeded++;
        }
    }

    /* make g_possMateForRow big enough for this problem */
    for( i = g_minRow; i <= g_maxRow; i++ )
    {
        g_possMateForRow[ i ].resize( g_numPossMatesForRow[ i ] );
    }

    /* initialize the lists of assignments to nothing */
    for( i = g_minRow; i <= g_maxRow; i++ )
    {
        g_mateForRow[ i ] = NO_SUCH_THING;
    }
    for( i = g_minCol; i <= g_maxCol; i++ )
    {
        g_mateForCol[ i ] = NO_SUCH_THING;
    }
}

/*-------------------------------------------------------------------*
 | jumpStartProblem() -- make initial subgraph, and try to solve the
 |                       problem the easy way
 |
 | See header comments for more details.
 *-------------------------------------------------------------------*/

static void jumpStartProblem()
{


    register int row, col;
    register int i;

    /* initialize the arrays of minimum costs */
    for( i = 0; i <= g_maxRow; i++ )
    {
        g_minCostInRow[ i ] = INFINITY;
    }
    for( i = 0; i <= g_maxCol; i++ )
    {
        g_minCostInCol[ i ] = INFINITY;
    }

    /* find the cost of the cheapest possible assignment for each row */
    for( i = 0; i < g_numRCCs; i++ )
    {
        row = g_rcc[ i ].row;

        if( row >= 0 && g_rcc[ i ].cost < g_minCostInRow[ row ] )
        {
            g_minCostInRow[ row ] = g_rcc[ i ].cost;
        }
    }

    /* subtract the cost of the cheapest assignment in each row from all
       the possible assignments in that row (so the cheapest one will go
       to 0), and find the cost of the cheapest resulting possible
       assignment for each column */
    for( i = 0; i < g_numRCCs; i++ )
    {
        row = g_rcc[ i ].row;
        col = g_rcc[ i ].col;

        if( row >= 0 )
        {
            g_rcc[ i ].cost -= g_minCostInRow[ row ];
        }

        if( col >= 0 && g_rcc[ i ].cost < g_minCostInCol[ col ] )
        {
            g_minCostInCol[ col ] = g_rcc[ i ].cost;
        }
    }

    /* subtract the cost of the cheapest assignment in each column from
       all the possible assignments in that column, if the resulting
       cost is 0, enable that assignment, and try to use it in our
       trivial solution */
    g_numPossMatesForRow.clear();
    for( i = 0; i < g_numRCCs; i++ )
    {
        row = g_rcc[ i ].row;
        col = g_rcc[ i ].col;

        if( col >= 0 )
        {
            g_rcc[ i ].cost -= g_minCostInCol[ col ];
        }

        if( g_rcc[ i ].cost == 0 )
        {
            g_possMateForRow[ row ][ g_numPossMatesForRow[ row ]++ ] = col;

            if( g_mateForRow[ row ] == NO_SUCH_THING &&
                    g_mateForCol[ col ] == NO_SUCH_THING )
            {
                g_mateForRow[ row ] = col;
                g_mateForCol[ col ] = row;
                g_numAssignmentsMade++;
            }
        }
    }
}

/*-------------------------------------------------------------------*
 | solveProblem() -- solve the problem the hard way
 |
 | See header comments for more details.
 *-------------------------------------------------------------------*/

static double solveProblem()
{


#ifdef DEBUG
    int numIterationsWithoutProgress = 0;
    int oldNumAssignmentsMade = g_numAssignmentsMade;
#endif

    double minCost;
    int result;
    register int row, col;
    register int i;

    /* main loop -- repeat until problem is solved */
    while( g_numAssignmentsMade < g_numAssignmentsNeeded )
    {
        /* try to increase the number of assignments made using the
           current subgraph */
        result = augment();

        /* if we couldn't increase the number of assignments, change the
           subgraph */
        if( result == CANT_AUGMENT )
        {
            /* if we've been trying to solve the problem a long time, and
               have changed the subgraph many times without increasing the
               number of assignments we can make in our matching, then
               there's probably something wrong */
#ifdef DEBUG
            if( g_numAssignmentsMade == oldNumAssignmentsMade )
            {
                assert( ++numIterationsWithoutProgress < END_OF_PATIENCE );
                //  THROW_ERR( "Giving up on Hungarian after " <<
                //             numIterationsWithoutProgress <<
                //             " iterations without progress" );
            }
            else
            {
                numIterationsWithoutProgress = 0;
            }
            oldNumAssignmentsMade = g_numAssignmentsMade;
#endif

            /* see Cooper & Steinberg to help understand the following */

            /* find essential rows */
            for( i = g_minRow; i <= g_maxRow; i++ )
                g_rowIsEssential[ i ] =
                    (g_mateForRow[ i ] != NO_SUCH_THING &&
                     ! g_colIsEssential[ g_mateForRow[ i ] ]);

            /* find minimum cost assignment of a non-essential row to a
               non-essential column */
            minCost = INFINITY;
            for( i = 0; i < g_numRCCs; i++ )
                if( ! g_rowIsEssential[ g_rcc[ i ].row ] &&
                        ! g_colIsEssential[ g_rcc[ i ].col ] &&
                        g_rcc[ i ].cost < minCost )
                {
                    minCost = g_rcc[ i ].cost;
                }

            /* if there is no minimum cost assignment between a non-essential
               row and column, the problem is unsolvable */
            if( minCost == INFINITY )
            {
                return UNSOLVABLE;
            }

#ifdef TSTBUG
            assert( minCost != 0 );
            //  THROW_ERR( "Big, hairy, Hungarian bug -- minCost == 0" )
#endif

            /* construct the new subgraph */
            g_numPossMatesForRow.clear();
            for( i = 0; i < g_numRCCs; i++ )
            {
                row = g_rcc[ i ].row;
                col = g_rcc[ i ].col;

                if( g_rowIsEssential[ row ] )
                {
                    if( g_colIsEssential[ col ] )
                    {
                        g_rcc[ i ].cost += minCost;
                    }
                }
                else
                {
                    if( ! g_colIsEssential[ col ] )
                    {
                        g_rcc[ i ].cost -= minCost;
                    }
                }

                if( g_rcc[ i ].cost == 0 )
                    g_possMateForRow[ row ][ g_numPossMatesForRow[ row ]++ ] =
                        col;
            }
        }
    }

    return SOLVED;
}

/*-------------------------------------------------------------------*
 | augment() -- try to figure out a way to get one more assignment in
 |              the current subgraph
 |
 | See header comments for more details.
 *-------------------------------------------------------------------*/

static int augment()
{


    int row, col;
    int mateForCol;
    int oldMateForRow;
    int i;

    /* see Cooper & Steinberg to help understand the following */

    g_colIsEssential.clear();
    g_rowToSearch.clear();

    /* make list of rows to search -- we'll begin by searching any
       row which isn't yet assigned */
    for( row = g_minRow; row <= g_maxRow; row++ )
    {
        g_rowPredecessor[ row ] = NO_SUCH_THING;
        if( g_mateForRow[ row ] == NO_SUCH_THING )
        {
            g_rowToSearch.put( row );
        }
    }

    /* keep searching until we've run out of rows to search */
    while( ! g_rowToSearch.isEmpty() )
    {
        /* get one of the rows to search */
        row = g_rowToSearch.get();

        /* go through each of the possible assignments for this
           row in the current subgraph */
        for( i = 0; i < g_numPossMatesForRow[ row ]; i++ )
        {
            col = g_possMateForRow[ row ][ i ];
            mateForCol = g_mateForCol[ col ];

            /* if the row can be assigned to this column (because the
               column doesn't already have a mate), then we can get an
               extra assignment */
            if( mateForCol == NO_SUCH_THING )
            {
                do
                {
                    oldMateForRow = g_mateForRow[ row ];
                    g_mateForRow[ row ] = col;
                    g_mateForCol[ col ] = row;

                    row = g_rowPredecessor[ row ];
                    col = oldMateForRow;
                }
                while( row != NO_SUCH_THING );

                g_numAssignmentsMade++;

                return AUGMENTED;
            }

            /* otherwise, search the row that this column is assigned to,
               trying to find a different column to assign it to */
            if( g_rowPredecessor[ mateForCol ] == NO_SUCH_THING )
            {
                g_rowPredecessor[ mateForCol ] = row;
                g_rowToSearch.put( mateForCol );

                g_colIsEssential[ col ] = 1;
            }
        }
    }

    return CANT_AUGMENT;
}


/*-------------------------------------------------------------------*
 | storeSolution() -- store the solution in a VECTOR
 *-------------------------------------------------------------------*/

static double storeSolution( VECTOR_OF< void * > &tag, int *numTags )
{


    ROW_COL_COST *usedRCC;
    double cost;
    int n;
    int i;

    tag.resize( g_numAssignmentsMade );

    cost = 0.;
    n = 0;
    for( i = 0; i < g_numRCCs; i++ )
        if( g_rcc[ i ].tag != 0 &&
                g_rcc[ i ].row == g_mateForCol[ g_rcc[ i ].col ] )
        {
            usedRCC = (ROW_COL_COST *)g_rcc[ i ].tag;
            tag[ n++ ] = usedRCC->tag;
            cost += usedRCC->cost;
        }

    *numTags = n;

    return cost;
}
