
/*********************************************************************
 * FILE: apqueue.H                                                   *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    6 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Declarations for virtual priority queues of solutions to        *
 *   assignment problems.  The objects defined in this file provide  *
 *   a mechanism for getting successively higher-cost solutions to   *
 *   an assignment problem.                                          *
 *                                                                   *
 *   The virtual priority queues are coded as ASSIGNMENT_PQUEUE      *
 *   objects.  Any number of assignment problems may be put into one *
 *   of these objects.  You can think of the object as then holding  *
 *   a list of all possible solutions to all the problems put in it  *
 *   (although it doesn't really enumerate them).  When you ask the  *
 *   ASSIGNMENT_PQUEUE for a solution, it goes through the list and  *
 *   finds the lowest-cost solution of them all.  It then removes    *
 *   that solution and removes it from the list, so the next time,   *
 *   it will give you the next-lowest-cost solution.  Etc.           *
 *                                                                   *
 *   ASSIGNMENT_PQUEUE's have the following member functions:        *
 *                                                                   *
 *     ASSIGNMENT_PQUEUE()                                           *
 *       The constructor takes no arguments.                         *
 *                                                                   *
 *     void addProblem( void *problemTag,                            *
 *                      ROW_COL_COST *rcc,                           *
 *                      int numRCCs,                                 *
 *                      int numRows,                                 *
 *                      int numCols )                                *
 *       This takes an assignment problem, represented in the way    *
 *       described in assign.H, and (virtually) puts all its         *
 *       possible solutions on the list.  The 'problemTag' is a      *
 *       void pointer that will be used to identify this problem.    *
 *       It should be set to point to an object associated with the  *
 *       problem.                                                    *
 *                                                                   *
 *     void removeProblem( void *problemTag )                        *
 *       Remove all the solutions to the given problem from the      *
 *       list.  The problem is identified with the void pointer that *
 *       was given when it was added to the ASSIGNMENT_PQUEUE.       *
 *                                                                   *
 *     void removeAllProblems()                                      *
 *       Empty the list.                                             *
 *                                                                   *
 *     int isEmpty()                                                 *
 *       Returns 1 if there are no more solutions to the problems    *
 *       that have been added to the ASSIGNMENT_PQUEUE.              *
 *       0 otherwise.                                                *
 *                                                                   *
 *     double getNextSolutionCost()                                  *
 *       Returns the cost of the next solution that will be returned *
 *       by getNextSolution() (see below).  This does not remove the *
 *       solution from the list.                                     *
 *                                                                   *
 *     void *getNextSolution( VECTOR_OF< void * > &solutionTag,      *
 *                            int *solutionSize )                    *
 *       Get the next-lowest-cost solution and remove it from the    *
 *       list.  The return value is the 'problemTag', which          *
 *       identifies which problem the solution came from.  The       *
 *       solution itself is returned in the way described in         *
 *       assign.H (the way FindBestAssignment() returns it).         *
 *                                                                   *
 * IMPLEMENTATION NOTES:                                             *
 *                                                                   *
 *   I'm using Murty's algorithm for ranked assignments, described   *
 *   in                                                              *
 *                                                                   *
 *     K. G. Murty                                                   *
 *     An algorithm for ranking all the assignments in order of      *
 *     increasing cost                                               *
 *     Operations Research, 16:682-687                               *
 *     1968                                                          *
 *                                                                   *
 *   He describes it there in terms of assignment matrices.  I found *
 *   it easier to implement in terms of bipartite graphs.            *
 *                                                                   *
 *   The algorithm works on a list of problem/solution pairs.  Each  *
 *   problem is a subgraph of one of the original problems put into  *
 *   the queue, and is paired with its optimal solution.  When the   *
 *   next solution is requested, we find the pair on the list that   *
 *   has the best solution.  We remove that pair and replace it with *
 *   pairs for problems that are subgraphs of the problem we just    *
 *   solved.  This replacement, called "partitioning", is done in    *
 *   such a way that the union of all the possible solutions to the  *
 *   new problems is exactly the set of possible solutions to the    *
 *   original problem minus the solution we just returned.           *
 *                                                                   *
 *   In psuedocode:                                                  *
 *                                                                   *
 *     To add a problem, P0, to the ASSIGNMENT_PQUEUE (addProblem()) *
 *       * Find the best solution, S0, to P0                         *
 *       * Add <P0,S0> to the list of problem/solution pairs         *
 *                                                                   *
 *     To get the next solution (getNextSolution())                  *
 *       * Find the pair, <P,S>, that has the lowest-cost solution   *
 *       * Remove <P,S> from the list of problem/solution pairs      *
 *       * PARTITION P: For each arc, <r,c,s>, in S, which connects  *
 *         row node r to column node c with cost s ...               *
 *         * Let P' = P                                              *
 *         * Remove <r,c,s> from P'                                  *
 *         * Look for the best solution, S', to P'                   *
 *         * If S' exists ...                                        *
 *           * Add <P',S'> to the list of problem/solution pairs     *
 *         * From P, remove all arcs that include r, and all arcs    *
 *           that include c, except <r,c,s> itself                   *
 *       * Return S                                                  *
 *                                                                   *
 *   Arcs are represented by ROW_COL_COST structures (see assign.H). *
 *                                                                   *
 *   The problem/solution pairs are stored in apqSOLUTION            *
 *   objects.  The solutions in these objects are divided into two   *
 *   parts.  The division is based on the observation that, in most  *
 *   of these problems, part of the solution is forced by the way    *
 *   that problems are partitioned.  The last step in partitioning   *
 *   removes all possible assignments for r and c except for         *
 *   <r,c,s>.  So we don't really have to look at r and c anymore.   *
 *   Instead, the assignment <r,c,s> is put into the "base" part of  *
 *   the solution, and r and c are removed entirely from the graph.  *
 *   The rest of the solution is computed by a call to               *
 *   FindBestAssignment().                                           *
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

#ifndef APQUEUE_H
#define APQUEUE_H

#include "except.h"
#include "list.h"
#include "assign.h"
#include <assert.h>

/*-------------------------------------------------------------------*
 | Declarations of stuff found in this file.
 *-------------------------------------------------------------------*/

class apqSOLUTION;
class ASSIGNMENT_PQUEUE;

/*-------------------------------------------------------------------*
 | apqSOLUTION -- problem/solution pair (see header comments above)
 *-------------------------------------------------------------------*/

class apqSOLUTION: public DLISTnode
{
  friend class ASSIGNMENT_PQUEUE;

  private:

    void *m_problemTag;              // id of original problem
    double m_baseCost;               // cost of "base" part of solution
    double m_cost;                   // total cost of solution
    int m_solutionSize;              // total solution size
    void **m_solutionTag;            // completed solution
    int m_baseSolutionSize;          // the first m_baseSolutionSize
                                     //   assignments in m_solutionTag
                                     //   form the "base" solution
    void **m_baseSolutionTag;        // storage for the "base" solution
                                     //   before the problem is really
                                     //   solved
    ROW_COL_COST *m_rcc;             // possible assignments
    int m_numRCCs;                   // number of possible assignments
    int m_numRows;                   // number of rows
    int m_numCols;                   // number of columns

  private:

    apqSOLUTION( void *problemTag,
                 double baseCost,
                 double parentCost,
                 void **baseSolutionTag,
                 int baseSolutionSize,
                 ROW_COL_COST *rcc,
                 int numRCCs,
                 int numRows,
                 int numCols ):
      m_problemTag( problemTag ),
      m_baseCost( baseCost ),
      m_cost( 0 ),
      m_solutionSize( 0 ),
      m_solutionTag( 0 ),
      m_baseSolutionSize( baseSolutionSize ),
      m_baseSolutionTag( 0 ),
      m_rcc( new ROW_COL_COST[ numRCCs ] ),
      m_numRCCs( numRCCs ),
      m_numRows( numRows ),
      m_numCols( numCols )
    {
      setup( baseSolutionTag, rcc, parentCost );
    }

    apqSOLUTION( void *problemTag,
                 ROW_COL_COST *rcc,
                 int numRCCs,
                 int numRows,
                 int numCols ):
      m_problemTag( problemTag ),
      m_baseCost( 0 ),
      m_cost( 0 ),
      m_solutionSize( 0 ),
      m_solutionTag( 0 ),
      m_baseSolutionSize( 0 ),
      m_baseSolutionTag( 0 ),
      m_rcc( new ROW_COL_COST[ numRCCs ] ),
      m_numRCCs( numRCCs ),
      m_numRows( numRows ),
      m_numCols( numCols )
    {
      setup( 0, rcc );
    }

    ~apqSOLUTION()
    {
      
      delete [] m_solutionTag;
      delete [] m_baseSolutionTag;
      delete [] m_rcc;
    }

    int isSolved() const { return m_solutionTag != 0; }

    void setup( void **baseSolutionTag,
                ROW_COL_COST *rcc,
                double parentCost = -INFINITY );
    void solve( void );
    void partition();

    int isValid() { return m_cost != UNSOLVABLE; }

    void *getProblemTag() { return m_problemTag; }
    double getCost() { return m_cost; }
    void getSolutionTags( VECTOR_OF< void * > &solutionTag,
                          int *solutionSize );
    int getNumRCCs() { return m_numRCCs; }

    void print();

  protected:

    MEMBERS_FOR_DLISTnode( apqSOLUTION )
};

/*-------------------------------------------------------------------*
 | ASSIGNMENT_PQUEUE -- virtual priority queue of solutions to
 |                      assignment problems
 *-------------------------------------------------------------------*/

class ASSIGNMENT_PQUEUE
{
  private:

    iDLIST_OF< apqSOLUTION > m_solutionList;
    apqSOLUTION *m_bestSolution;

  public:

    ASSIGNMENT_PQUEUE(): m_solutionList(), m_bestSolution( 0 ) {}

    void addProblem( void *problemTag,
                     ROW_COL_COST *rcc,
                     int numRCCs,
                     int numRows,
                     int numCols );

    void removeProblem( void *problemTag );

    void removeAllProblems()
    {

      m_solutionList.removeAll();
    }

    int isEmpty()
    {
      findBestSolution();
      return m_solutionList.isEmpty() ||
             m_bestSolution->getCost() == INFINITY;
    }

    double getNextSolutionCost()
    {

      findBestSolution();

      #ifdef TSTBUG
        // NOTE: Assertion boolean was negated from original for the if-statement
        assert( m_bestSolution != 0 );
        //  THROW_ERR("Assignment priority queue is empty")
      #endif

      return m_bestSolution->getCost();
    }

    void *getNextSolution( VECTOR_OF< void * > &solutionTag,
                           int *solutionSize );

  private:

    void findBestSolution();
};

#endif

