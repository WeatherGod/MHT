
/*********************************************************************
 * FILE: mht.H                                                       *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   11 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Base classes for fundamental multiple hypothesis tracking       *
 *   algorithm.                                                      *
 *                                                                   *
 *   The purpose of this package is to separate the problem of       *
 *   tracking a single target from the problem of tracking multiple  *
 *   targets.  The application which uses this package need only     *
 *   define the way that individual track trees grow.  The MHT       *
 *   package, then, prunes the trees in such a way that, in the end, *
 *   each measurement report is assigned to at most one track.       *
 *                                                                   *
 *   To make an application that employs the mht algorithm, you      *
 *   define one or more subclasses of the base classes REPORT,       *
 *   T_HYPO, and MHT.  Each of these classes has a set of virtual    *
 *   functions that should be defined by the application             *
 *   programmer for his or her specific application.                 *
 *                                                                   *
 *   The basic meanings of these three base classes are as follows:  *
 *                                                                   *
 *     REPORT subclasses contain descriptions of reported            *
 *       measurements (e.g. blips from radar, or edgels in an        *
 *       image).                                                     *
 *                                                                   *
 *     T_HYPO ("track hypothesis") subclasses are the nodes on the   *
 *       track trees.  Each node represents a possible               *
 *       interpretation of the measurements reported at a given      *
 *       time.  For example, one type of T_HYPO might represent the  *
 *       possibility that a given REPORT was a report of the present *
 *       target, and contain an estimate of the target's position.   *
 *       Another type might represent the possibility that the       *
 *       target dissappeared.  Etc.  A path through the track tree   *
 *       represents a sequence of events in the life of the target.  *
 *                                                                   *
 *     The MHT subclass (there's usually only one per program)       *
 *       manages the track trees and reports, and prunes each tree   *
 *       down to a single path.                                      *
 *                                                                   *
 *   The main loop of an application should call the scan() member   *
 *   function of an MHT object.  This function goes through the      *
 *   following steps:                                                *
 *                                                                   *
 *     1. Call the virtual function measureAndValidate(), which must *
 *        be provided by the application, to gather a list of        *
 *        REPORTs, start new track trees, and grow new leaves for    *
 *        all the existing track trees.                              *
 *                                                                   *
 *        For example, in a radar-tracking application,              *
 *        measureAndValidate() would gather the measurements from    *
 *        the radar, put them in REPORT structures, and then         *
 *        validate them to the leaves of the track trees.            *
 *                                                                   *
 *     2. Prune the track trees according to a variety of pruning    *
 *        strategies.                                                *
 *                                                                   *
 *     3. For each track tree,                                       *
 *          While the root has only one child (because of pruning),  *
 *            if the root's m_mustVerify member (see below) is 1     *
 *              call the root's virtual verify() routine             *
 *            if the root's m_endsTrack member (see below) is 1      *
 *              remove the track tree                                *
 *            else                                                   *
 *              remove the root from the tree, making it's only      *
 *              child into the new root                              *
 *                                                                   *
 *   The verify() routines called in step 3, which are members of    *
 *   T_HYPO objects, should be used to define the application's      *
 *   behavior when a target's state has been determined.  For        *
 *   example, it might display a symbol on the screen representing   *
 *   the target's location, or make a separate record for later      *
 *   use.                                                            *
 *                                                                   *
 *   Each of the base classes is described in detail below.          *
 *                                                                   *
 *                           REPORT                                  *
 *                                                                   *
 *   A REPORT contains a description of a reported measurement.      *
 *   The code in the MHT package doesn't need to know any of the     *
 *   details of a REPORT.                                            *
 *                                                                   *
 *   Note: the REPORT base class cannot be copied because it has a   *
 *   "links to" member, whos copy constructor just generates an      *
 *   error.  Descendants of REPORT should have copy constructors     *
 *   which simply initialize their REPORT base with the default      *
 *   (no argument) constructer.  For example:                        *
 *                                                                   *
 *      class FOO: public REPORT                                     *
 *      {                                                            *
 *          .                                                        *
 *          .                                                        *
 *          .                                                        *
 *                                                                   *
 *        public:                                                    *
 *                                                                   *
 *          FOO( const FOO &src ):                                   *
 *            REPORT(),                                              *
 *            ...                                                    *
 *          .                                                        *
 *          .                                                        *
 *          .                                                        *
 *      };                                                           *
 *                                                                   *
 *                           T_HYPO                                  *
 *                                                                   *
 *   There are three main application-specific functions of a        *
 *   T_HYPO:                                                         *
 *                                                                   *
 *     1. When the application makes a T_HYPO, it must assign it a   *
 *        log-likelihood.  This is usually done in the constructor.  *
 *                                                                   *
 *     2. In each iteration, the application must give children to   *
 *        each T_HYPO that is a leaf of a track tree.  This is done  *
 *        by the measureAndValidate() member of the MHT object,      *
 *        which calls member functions of the T_HYPO leaves.         *
 *                                                                   *
 *     3. When a T_HYPO has been determined to be true (becomes the  *
 *        root of its tree), the application has an opportunity to   *
 *        respond (e.g. by displaying something on the screen).      *
 *        This is done by the T_HYPO's verify() member.              *
 *                                                                   *
 *   The constructors for T_HYPO are protected (so that T_HYPO may   *
 *   only be used as a base class).  They are:                       *
 *                                                                   *
 *     T_HYPO()                                                      *
 *                                                                   *
 *       This makes a T_HYPO that is independent of any REPORT.  For *
 *       example, one representing the possibility that a target     *
 *       is still in existance but wasn't detected.                  *
 *                                                                   *
 *     T_HYPO( REPORT *report )                                      *
 *                                                                   *
 *       This makes a T_HYPO that is based on the given REPORT.  For *
 *       example, one representing the possibility that the reported *
 *       measurement resulted from this target.                      *
 *                                                                   *
 *   The following member functions for T_HYPO's are public:         *
 *                                                                   *
 *     void installChild( T_HYPO *c )                                *
 *                                                                   *
 *       Install c as a child of this T_HYPO on its tree.            *
 *                                                                   *
 *     int endsTrack()                                               *
 *                                                                   *
 *       Returns the value of m_endsTrack (see below).               *
 *                                                                   *
 *     int mustVerify()                                              *
 *                                                                   *
 *       Returns the value of m_mustVerify (see below).              *
 *                                                                   *
 *     int hasReport()                                               *
 *                                                                   *
 *       Returns 1 if the T_HYPO constructor was called with a       *
 *       REPORT for an argument, 0 if it was called with no          *
 *       arguments.                                                  *
 *                                                                   *
 *     double getLogLikelihood()                                     *
 *                                                                   *
 *       Returns the value of m_logLikelihood (see below).           *
 *                                                                   *
 *     T_TREE *getTree()                                             *
 *                                                                   *
 *       Returns a pointer to the track tree that the T_HYPO is on.  *
 *       This probably shouldn't be called by any application code.  *
 *       The T_TREE structure has no user-serviceable parts inside.  *
 *                                                                   *
 *     int getTrackStamp()                                           *
 *                                                                   *
 *       Returns an integer which uniquely identifies the track tree *
 *       that this T_HYPO is on.                                     *
 *                                                                   *
 *     int getGroupId()                                              *
 *                                                                   *
 *       Returns an integer which uniquely identifies the group that *
 *       this T_HYPO's track tree is in (see implementation notes    *
 *       below).                                                     *
 *                                                                   *
 *     int getTimeStamp()                                            *
 *                                                                   *
 *       Returns the number of calls that were made to MHT::scan()   *
 *       before this T_HYPO was born.                                *
 *                                                                   *
 *     REPORT *getReport()                                           *
 *                                                                   *
 *       Returns the report that this T_HYPO was constructed with.   *
 *       This can only be used if the T_HYPO was constructed with a  *
 *       report (i.e. only if hasReport() returns 1).                *
 *                                                                   *
 *   The following virtual functions should be defined by the        *
 *   application:                                                    *
 *                                                                   *
 *     virtual void verify()                                         *
 *                                                                   *
 *       Called when this T_HYPO has been determined to be true.     *
 *                                                                   *
 *       Important: the verify() function is ONLY called if          *
 *       m_mustVerify is set to 1.  If m_mustVerify is set to 0, it  *
 *       will NEVER be called for that T_HYPO.  (See below).         *
 *                                                                   *
 *   The following protected member fields of a T_HYPO subclass      *
 *   should be set by the constructor:                               *
 *                                                                   *
 *     char m_endsTrack                                              *
 *                                                                   *
 *       This is a flag which indicates whether the T_HYPO indicates *
 *       that the tree is no longer of use.  For example, a type of  *
 *       T_HYPO that represents a target dissappearing should set    *
 *       this flag to 1.                                             *
 *                                                                   *
 *       Note: a T_HYPO that has m_endsTrack set to 1 still needs to *
 *       grow new children in every iteration.  Make a dummy T_HYPO  *
 *       subclass for this, which just takes the likelihood of its   *
 *       parent.  The dummy T_HYPOs should have m_endsTrack = 1.     *
 *                                                                   *
 *     char m_mustVerify                                             *
 *                                                                   *
 *       This flag indicates whether the T_HYPO's verify() member    *
 *       function actually does something.                           *
 *                                                                   *
 *       Important: the verify() function is ONLY called if          *
 *       m_mustVerify is set to 1.  If m_mustVerify is set to 0, it  *
 *       will NEVER be called for that T_HYPO.                       *
 *                                                                   *
 *     double m_logLikelihood                                        *
 *                                                                   *
 *       This is the log of the likelihood that the path from the    *
 *       original root of the tree to this T_HYPO is the closest to  *
 *       the truth.                                                  *
 *                                                                   *
 *                               MHT                                 *
 *                                                                   *
 *   The constructor for the MHT base class is protected, so it can  *
 *   only be constructed from subclasses.  It takes arguments that   *
 *   define the amount of each type of pruning:                      *
 *                                                                   *
 *     MHT( int maxDepth, double minGHypoRatio, int maxGHypos )      *
 *                                                                   *
 *       maxDepth is used for N-scanback pruning.  It defines the    *
 *       maximum depth that a track tree can reach before it will    *
 *       be pruned down to a single choice from the root.  Once so   *
 *       pruned, at least one node will be verified and removed      *
 *       from the tree in step 3 of scan() (see above).              *
 *                                                                   *
 *       minGHypoRatio is used for ratio pruning.  It defines the    *
 *       minimum ratio between the likelihood of the worst           *
 *       group hypothesis to that of the best (worst/best). Any      *
 *       group hypothesis with a likelihood lower than               *
 *       minGHypoRatio*likelihood-of-best-group-hypothesis will      *
 *       be discarded.                                               *
 *                                                                   *
 *       maxGHypos is used for k-best pruning.  It defines the       *
 *       maximum number of group hypotheses that will ever be        *
 *       considered for one group.                                   *
 *                                                                   *
 *       (See IMPLEMENTATION NOTES, below, for a discussion of       *
 *       groups and group hypotheses)                                *
 *                                                                   *
 *   There are two protected members which provide information       *
 *   for the application's use.  THESE MUST NOT BE DIRECTLY ALTERED  *
 *   BY THE APPLICATION:                                             *
 *                                                                   *
 *     iDLIST_OF< REPORT > m_newReportList                           *
 *                                                                   *
 *       This is a doubly-linked list (see "list.H") of the          *
 *       REPORTS that have been installed since the end of the last  *
 *       call to scan().  (See installReport(), below.)              *
 *                                                                   *
 *     ptrDLIST_OF< T_HYPO > m_activeTHypoList                       *
 *                                                                   *
 *       This is a doubly-linked list (see "list.H") of all the      *
 *       T_HYPO's that are currently leaves of track trees.          *
 *                                                                   *
 *   The following member functions are protected:                   *
 *                                                                   *
 *     void installReport( REPORT *report )                          *
 *                                                                   *
 *       Hand a REPORT to the MHT object.  This will place the       *
 *       REPORT on m_newReportList, for use later on.                *
 *                                                                   *
 *       The MHT object will delete the REPORT when it's done with   *
 *       it (when all the T_HYPO's that refer to it have been either *
 *       pruned or verified).                                        *
 *                                                                   *
 *     void installTree( T_HYPO *rootNode, int timeOffset = 0 )      *
 *                                                                   *
 *       Start a new track tree.  rootNode is the root of the tree.  *
 *       timeOffset is an amount to add to the currentTime (the      *
 *       counter of the number of calls to scan(), see below) to     *
 *       make the time stamp of the root.  This can be useful if     *
 *       the track tree will begin with a depth greater than 1, and  *
 *       the leaves should have time stamps equal to the current     *
 *       time.  In that case, timeOffset should be a negative number *
 *       equal in magnitude to the depth of the tree.                *
 *                                                                   *
 *       Note: due to the implementation of trees (see "tree.H"), it *
 *       isn't kosher to give a T_HYPO children before it is placed  *
 *       in a tree.  Thus, the only way to start a track tree with   *
 *       more than one node in it is to                              *
 *                                                                   *
 *         1. Construct the root T_HYPO.                             *
 *         2. Call installTree(), making the T_HYPO the root of a    *
 *            tree.                                                  *
 *         3. Make the other T_HYPO's for the tree, and place them   *
 *            on it by calling the installChild() member of the      *
 *            appropriate nodes.                                     *
 *                                                                   *
 *   The following member functions are public:                      *
 *                                                                   *
 *     int isInUse()                                                 *
 *                                                                   *
 *       Returns 1 if there are still active track trees in the MHT. *
 *       0 if not.                                                   *
 *                                                                   *
 *     int getCurrentTime()                                          *
 *                                                                   *
 *       Returns a count of the number of calls that have been made  *
 *       to scan() since the MHT was created.                        *
 *                                                                   *
 *     int scan()                                                    *
 *                                                                   *
 *       See the beginning of these comments for a description.      *
 *                                                                   *
 *       The return value is either 1, if there are some active      *
 *       track trees, or 0 if there were no REPORTs for the scan and *
 *       all of the track trees have been removed.                   *
 *                                                                   *
 *   The following virtual function must be defined in the           *
 *   application:                                                    *
 *                                                                   *
 *     virtual void measureAndValidate()                             *
 *                                                                   *
 *       This should do two things.                                  *
 *                                                                   *
 *       First, it should gather all the measurements for one scan   *
 *       and use installReport() to put REPORTs for them onto        *
 *       m_newReportList.                                            *
 *                                                                   *
 *       Second, it should go through the T_HYPOs in                 *
 *       m_activeTHypoList, giving each of them one or more          *
 *       children.  It is essential that EVERY T_HYPO in the list    *
 *       is given at least one child -- even T_HYPOs whose           *
 *       m_endsTrack members are set to 1.                           *
 *                                                                   *
 * IMPLEMENTATION NOTES:                                             *
 *                                                                   *
 *   Everything of importance starts with MHT::scan().  The basic    *
 *   behavior of scan() is described above, but I'll go into a       *
 *   little more detail here, to explain what the various classes    *
 *   defined in this file are for.                                   *
 *                                                                   *
 *   After calling measureAndValidate(), scan() arranges the track   *
 *   trees into GROUPs.  Each GROUP is a list of trees that share    *
 *   measurements.  Trees in different GROUPs are completely         *
 *   independent of each other.                                      *
 *                                                                   *
 *   The GROUPs are constructed incrementally.  First, each new tree *
 *   is placed in a new GROUP by itself.  Then, any two GROUPs that  *
 *   contain trees that share measurements are merged.  Finally,     *
 *   every GROUP that can be divided up (because some of the trees   *
 *   in them are no longer sharing any measurements) are split.      *
 *                                                                   *
 *   In addition to a list of track trees, each GROUP has a list of  *
 *   G_HYPO's (group hypotheses).  Each G_HYPO is a list of leaf     *
 *   T_HYPO's, one from each of the trees in the group.              *
 *   The leaves in one G_HYPO are compatible with each other -- the  *
 *   paths from the roots of the trees to those leaves don't share   *
 *   any measurements.  Each G_HYPO has a likelihood, computed as    *
 *   the product of the likelihoods of the T_HYPO's it postulates.   *
 *                                                                   *
 *   Since the GROUPs are computed incrementally, their G_HYPOs can  *
 *   be carried over from the previous call to scan().  But, after   *
 *   measureAndValidate() is called, the G_HYPOs become obsolete;    *
 *   they no longer point to leaves of the trees, since all the      *
 *   nodes that were previously leaves have been given children by   *
 *   measureAndValidate().                                           *
 *                                                                   *
 *   For each GROUP, now, we generate a new set of G_HYPOs.  To do   *
 *   this, we start by constructing assignment problems from all the *
 *   old G_HYPOs.  Each assignment problem covers the possible ways  *
 *   of assigning the new REPORTs to the T_HYPOs in the old G_HYPO.  *
 *   Each of these possible assignments corresponds to one of the    *
 *   children of the T_HYPO.  Then, an ASSIGNMENT_PQUEUE (see        *
 *   apqueue.H) is used to come up with solutions to these           *
 *   assignment problems in decreasing order of likelihood.  A new   *
 *   G_HYPO is made to represent each of these solutions.            *
 *                                                                   *
 *   As soon as the first new G_HYPO is made, N-scanback pruning is  *
 *   applied.  This goes through each of the trees in the group,     *
 *   and, if the tree's depth is greater than m_maxDepth, prunes     *
 *   away the children of the root that don't lead to a leaf in the  *
 *   G_HYPO.  The root, then, is left with only one child (since     *
 *   the G_HYPO contains only one leaf from each tree), and will be  *
 *   removed in step 3 of the basic scan() steps described under     *
 *   CONTENTS, above.                                                *
 *                                                                   *
 *   After N-scanback pruning has been applied, some of the old      *
 *   G_HYPOs are no longer valid, since the nodes that they use have *
 *   been pruned away.  These G_HYPOs, and their corresponding       *
 *   assignment problems, are deleted.                               *
 *                                                                   *
 *   Finally, the remaining new G_HYPOs are generated.  Generation   *
 *   of G_HYPOs terminates when either m_minGHypoRatio or            *
 *   m_maxGHypos is reached (see the description of the MHT          *
 *   constructor, above).                                            *
 *                                                                   *
 *   Any leaf that doesn't make it into a G_HYPO is now removed.     *
 *   Any internal node that no longer has any children (because they *
 *   were removed) is now removed.                                   *
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

#ifndef MHT_H
#define MHT_H

#include <cmath>
#include <string.h>		// for strncopy()
#include <iostream>		// for std::cout, std::endl
#include <assert.h>

#include "except.h"
#include "list.h"
#include "tree.h"
#include "links.h"
#include "vector.h"
//#include "mem.h"


#ifdef DECLARE_MHT
  #define GLOBAL
  #define INITVAL( v ) = (v)
#else
  #define GLOBAL extern
  #define INITVAL( v )
#endif

static const double DOUBLE_NOT_READY = -INFINITY;

/*-------------------------------------------------------------------*
 | Stuff declared in this file
 *-------------------------------------------------------------------*/

GLOBAL long G_numCallsToScan;
GLOBAL double G_timeSpentInScan;
GLOBAL double G_numCallsToPruneAndHypothesize;
GLOBAL double G_timeSpentInPruneAndHypothesize;
GLOBAL double G_timeSpentInApqueue;
GLOBAL long G_numApqueueProblems;
GLOBAL long G_totalApqueueProblemSizes;
GLOBAL long G_maxApqueueProblemSize;
GLOBAL double G_totalApqueueProblemCoverage;
GLOBAL double G_minApqueueProblemCoverage INITVAL( INFINITY );
GLOBAL double G_maxApqueueProblemCoverage;

class REPORT;
class T_HYPO;
class G_HYPO;
class T_TREE;
class GROUP;
class MHT;

/*-------------------------------------------------------------------*
 | REPORT -- base class for measurement reports
 *-------------------------------------------------------------------*/

class REPORT: public DLISTnode
{
  friend class T_HYPO;
  friend class G_HYPO;
  friend class MHT;

  protected: MEMBERS_FOR_DLISTnode( REPORT )

  private:

    int m_rowNum;                    // row number used to represent
                                     //   this REPORT in assignment
                                     //   problems
    int m_groupId;                   // unique id of GROUP that contains
                                     //   trees which use this REPORT
                                     //   (once the GROUPs have been
                                     //   merged and split properly,
                                     //   only one GROUP will contain
                                     //   trees referring to any given
                                     //   REPORT)
    LINKS_TO< T_HYPO > m_tHypoLinks; // links to T_HYPOs that use this
                                     //   REPORT

  public:

    REPORT():
      DLISTnode(),
      m_rowNum( 0 ),
      m_groupId( 0 ),
      m_tHypoLinks()
    {
    }

    virtual ~REPORT() {}

    void setGroupId( int groupId ) { m_groupId = groupId; }
    void setAllGroupIds( int groupId );
    void checkGroupIds();
    int isInUse() { return ! m_tHypoLinks.isEmpty(); }

  private:

    void setRowNum( int rowNum ) { m_rowNum = rowNum; }
    int getRowNum() { return m_rowNum; }

  public:

    virtual void print() { std::cout << "R:" << (void *)this; }
    virtual void describe( int spaces = 0 );
};

/*-------------------------------------------------------------------*
 | T_HYPO -- base class for track hypotheses
 *-------------------------------------------------------------------*/

class T_HYPO: public TREEnode
{
  friend class REPORT;
  friend class G_HYPO;
  friend class T_TREE;
  friend class MHT;

  protected: MEMBERS_FOR_TREEnode( T_HYPO )

  private:

    T_TREE *m_tree;                  // tree that this T_HYPO is on
    int m_timeStamp;                 // number of calls to MHT::scan()
                                     //   before this T_HYPO was made
    LINKS_TO< REPORT > m_reportLink; // link to one REPORT
    LINKS_TO< G_HYPO > m_gHypoLinks; // links to the G_HYPOs that
                                     //   postulate this T_HYPO
    char m_flag;                     // used in splitting GROUPs

  protected:

    char m_endsTrack;
    char m_mustVerify;
    double m_logLikelihood;

  protected:

    T_HYPO():
      TREEnode(),
      m_tree( 0 ),
      m_timeStamp( 0 ),
      m_reportLink(),
      m_gHypoLinks(),
      m_flag( 0 ),
      m_logLikelihood( DOUBLE_NOT_READY ),
      m_endsTrack( 0 ),
      m_mustVerify( 0 )
    {
    }

    T_HYPO( REPORT *report ):
      TREEnode(),
      m_tree( 0 ),
      m_timeStamp( 0 ),
      m_reportLink(),
      m_gHypoLinks(),
      m_flag( 0 ),
      m_logLikelihood( DOUBLE_NOT_READY ),
      m_endsTrack( 0 ),
      m_mustVerify( 0 )
    {
      MAKE_LINK( this, m_reportLink,
                 report, m_tHypoLinks );
    }

    virtual ~T_HYPO() {}

  public:

    void installChild( T_HYPO *child )
    {
      PTR_INTO_iTREE_OF< T_HYPO > p = this;

      p.insertFirstChild( child );
      child->setStamps( m_tree, m_timeStamp + 1 );
    }

    int endsTrack() { return m_endsTrack; }
    int mustVerify() { return m_mustVerify; }
    int hasReport() { return m_reportLink.hasOneMember(); }

    double getLogLikelihood()
    {
      #ifdef TSTBUG
        assert( m_logLikelihood != DOUBLE_NOT_READY );
      //    THROW_ERR( "THypo wasn't given a logLikelihood" );
      #endif

      return m_logLikelihood;
    }

    T_TREE *getTree() { return m_tree; }

    inline int getTrackStamp();
    inline int getGroupId();

    int getTimeStamp() { return m_timeStamp; }
    REPORT *getReport() { return m_reportLink.getHead(); }

    virtual void verify()
      { assert(false);}//THROW_ERR( "Call to T_HYPO::verify()" ) }

  private:

    int isInUse() { return ! isLeaf() || ! m_gHypoLinks.isEmpty(); }

    int getRowNum()
      { return hasReport() ? getReport()->getRowNum() : -1; }
    int getNumChildren()
      { return TREEnode::getNumChildren(); }

    void setStamps( T_TREE *tree, int timeStamp )
      { m_tree = tree; m_timeStamp = timeStamp; }

    void setFlag() { m_flag = 1; }
    void resetFlag() { m_flag = 0; }
    int flagIsSet() { return m_flag; }

  public:

    virtual void print() { std::cout << "T:" << (void *)this; }
    virtual void describe( int spaces = 0, int depth = 0 );
    virtual void describeTree( int spaces = 0, int depth = 0 );
};

/*-------------------------------------------------------------------*
 | G_HYPO -- group hypothesis
 *-------------------------------------------------------------------*/

class G_HYPO: public DLISTnode
{
  friend class REPORT;
  friend class T_HYPO;
  friend class GROUP;
  friend class MHT;

  protected: MEMBERS_FOR_DLISTnode( G_HYPO )

  private:

    double m_logLikelihood;          // log of likelihood
    int m_numTHyposUsedInProblem;    // this is the length of the
                                     //   m_tHypoLinks list at the time
                                     //   that an assignment problem is
                                     //   made for this G_HYPO.  If
                                     //   N-scanback pruning removes any
                                     //   T_HYPO that this G_HYPO
                                     //   postulates, it will also be
                                     //   automatically removed from
                                     //   m_tHypoLinks (see links.H).
                                     //   So we can tell that N-scanback
                                     //   pruning made the G_HYPO
                                     //   invalid by checking to see if
                                     //   m_numTHyposUsedInProblem is
                                     //   more than the length of
                                     //   m_tHypoLinks
    LINKS_TO< T_HYPO > m_tHypoLinks; // links to T_HYPOs postulated

  public:

    G_HYPO():
      DLISTnode(),
      m_logLikelihood( 0. ),
      m_numTHyposUsedInProblem( 0 ),
      m_tHypoLinks()
    {
    }

    G_HYPO( VECTOR_OF< void * > &solution, int solutionSize );

    int isInUse() { return ! m_tHypoLinks.isEmpty(); }

    double getLogLikelihood() const { return m_logLikelihood; }
    int wasReduced()
      { return m_numTHyposUsedInProblem > m_tHypoLinks.getLength(); }

    void setNumtHypos()
    {
        m_numTHyposUsedInProblem = m_tHypoLinks.getLength();
     }

    void addTHypo( T_HYPO *tHypo )
    {
      m_logLikelihood += tHypo->getLogLikelihood();

      MAKE_LINK( this, m_tHypoLinks,
                 tHypo, m_gHypoLinks );
    }

    int getGroupId() { return (*m_tHypoLinks).getGroupId(); }
    int getNumTHypos() { return m_tHypoLinks.getLength(); }
    double getLogLikelihood() { return m_logLikelihood; }

    void makeProblem();
    void nScanBackPrune( int maxDepth );
    void recomputeLogLikelihood();

    int mustSplit();
    G_HYPO *split( int groupId );
    void merge( G_HYPO *src );

    void setFlags();
    void resetFlags();
    int allFlagsAreSet();

  public:

    virtual void print() { std::cout << "G:" << (void *)this; }
    virtual void describe( int spaces = 0 );
};

/*-------------------------------------------------------------------*
 | T_TREE -- track tree
 *-------------------------------------------------------------------*/

class T_TREE: public DLISTnode
{
  friend class MHT;

  protected: MEMBERS_FOR_DLISTnode( T_TREE )

  private:

    iTREE_OF< T_HYPO > m_tree;
    int m_id;
    int m_groupId;

  private:

    T_TREE( T_HYPO *root, int id, int time ):
      DLISTnode(),
      m_tree(),
      m_id( id ),
      m_groupId( 0 )
    {
      m_tree.insertRoot( root );
      root->setStamps( this, time );
    }

  public:

    iTREE_OF< T_HYPO > *getTree() { return &m_tree; }
    int getId() { return m_id; }
    int getGroupId() { return m_groupId; }
    void setGroupId( int groupId ) { m_groupId = groupId; }
};

/*-------------------------------------------------------------------*
 | GROUP -- group of trees that share measurements
 *-------------------------------------------------------------------*/

class GROUP: public DLISTnode
{
  protected: MEMBERS_FOR_DLISTnode( GROUP )

  private:

    iDLIST_OF< G_HYPO > m_gHypoList;
    G_HYPO *m_bestGHypo;

    /* NOTE: the actual list of trees is implicit in the list of
       G_HYPOs, since each G_HYPO postulates one T_HYPO from each
       tree in the GROUP.  To find the trees in this group (which I
       never actually have to do), go through the m_tHypoLinks
       member of one of the G_HYPOs, and call the getTree() function
       for each T_HYPO on it. */

  public:

    GROUP(): DLISTnode(), m_gHypoList() {}

    GROUP( T_TREE *tree ):
      DLISTnode(),
      m_gHypoList()
    {
      m_gHypoList.append( new G_HYPO() );
      (*m_gHypoList).addTHypo( tree->getTree()->getRoot() );
    }

    int isInUse()
      { return ! m_gHypoList.isEmpty() && (*m_gHypoList).isInUse(); }

    void merge( GROUP *src,
                double logMinGHypoRatio,
                int maxGHypos );
    void splitIfYouMust();
    void removeRepeats();
    void clear(int depth);
    void pruneAndHypothesize( int maxDepth,
                              double logMinGHypoRatio,
                              int maxGHypos );

    int getGroupId() { return (*m_gHypoList).getGroupId(); }
    int getNumGHypos() { return m_gHypoList.getLength(); }

    void check();
    virtual void print() { std::cout << "C:" << (void *)this; }
    virtual void describe( int spaces = 0 );
};

/*-------------------------------------------------------------------*
 | MHT -- multiple hypothesis tracker
 *-------------------------------------------------------------------*/

class MHT
{
  private:

    int m_dbgEndA;
    int m_dbgStartB;
    int m_dbgEndB;
    int m_dbgStartC;
    int m_dbgEndC;

  protected:

    int m_lastTrackIdUsed;
    int m_currentTime;

    int m_maxDepth;
    double m_logMinGHypoRatio;
    int m_maxGHypos;

    iDLIST_OF< T_TREE > m_tTreeList;
    PTR_INTO_iDLIST_OF< T_TREE > m_nextNewTTree;
    iDLIST_OF< GROUP > m_groupList;
    iDLIST_OF< REPORT > m_oldReportList;

    int m_dbgStartA;
    iDLIST_OF< REPORT > m_newReportList;
    ptrDLIST_OF< T_HYPO > m_activeTHypoList;

  protected:

    MHT( int maxDepth, double minGHypoRatio, int maxGHypos ):
      m_lastTrackIdUsed( 0 ),
      m_currentTime( 0 ),
      m_maxDepth( maxDepth ),
      m_logMinGHypoRatio( log( minGHypoRatio ) ),
      m_maxGHypos( maxGHypos ),
      m_tTreeList(),
      m_nextNewTTree( m_tTreeList ),
      m_groupList(),
      m_oldReportList(),
      m_newReportList(),
      m_activeTHypoList(),
      m_dbgStartA( 0x7FFFFFFF ),
      m_dbgEndA( 0x7FFFFFFF ),
      m_dbgStartB( 0x7FFFFFFF ),
      m_dbgEndB( 0x7FFFFFFF ),
      m_dbgStartC( 0x7FFFFFFF ),
      m_dbgEndC( 0x7FFFFFFF )
    {
    }

    virtual ~MHT() {}

    void installReport( REPORT *report )
      { m_newReportList.append( report ); }

    void installTree( T_HYPO *rootNode, int timeOffset = 0 )
    {
      T_TREE *tree = new T_TREE( rootNode,
                                 m_lastTrackIdUsed++,
                                 m_currentTime + timeOffset );

      m_activeTHypoList.append( *rootNode );
      m_tTreeList.append( tree );
      if( ! m_nextNewTTree.isValid() )
        m_nextNewTTree.set( m_tTreeList, START_AT_TAIL );
    }

  protected:

    virtual void measureAndValidate()
      { assert(false);}//THROW_ERR( "Call to MHT::measureAndValidate()" ) }

  public:

    int isInUse() { return ! m_tTreeList.isEmpty(); }
    int getCurrentTime() { return m_currentTime; }

    int scan();
    void clear();

  private:

    void importNewReports();

    void makeNewGroups();
    void findGroupLabels();
    void splitGroups();
    void mergeGroups();

    void pruneAndHypothesize();
    void removeUnusedTHypos();
    void verifyTTreeRoots();
    void verifyLastTTreeRoots();

    void removeUnusedTTrees();
    void removeUnusedReports();
    void removeUnusedGroups();

    void updateActiveTHypoList();

    void checkGroups();
    void doDbgA();
    void doDbgB();
    void doDbgC();

  public:

    virtual void print() { std::cout << "M:" << (void *)this; }
    virtual void describe( int spaces = 0 );

    virtual void printStats( int spaces = 0 );

    void setDbgA( int start = 0x7FFFFFFF, int end = 0x7FFFFFFF )
      { m_dbgStartA = start; m_dbgEndA = end; }
    void setDbgB( int start = 0x7FFFFFFF, int end = 0x7FFFFFFF )
      { m_dbgStartB = start; m_dbgEndB = end; }
    void setDbgC( int start = 0x7FFFFFFF, int end = 0x7FFFFFFF )
      { m_dbgStartC = start; m_dbgEndC = end; }
};

/*-------------------------------------------------------------------*
 | Inline member functions that can't be written inside the class
 | definitions because they refer to stuff declared later that the
 | classes they come from (results of chicken-and-egg problems).
 *-------------------------------------------------------------------*/

inline int T_HYPO::getTrackStamp()
{
  #ifdef TSTBUG
    assert( m_tree != 0 );
    //  THROW_ERR( "Trying to get track stamp from "
    //             "uninitialized tHypo" )
  #endif

  return m_tree->getId();
}

inline int T_HYPO::getGroupId()
{
  #ifdef TSTBUG
    assert( m_tree != 0 );
    //  THROW_ERR( "Trying to get group id from "
    //             "uninitialized tHypo" )
  #endif

  return m_tree->getGroupId();
}

#endif

