
/*********************************************************************
 * FILE: mht_report.C                                                *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   13 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Member functions for REPORT objects.  See mht.H.                *
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
 *                                                                   *
 *********************************************************************/

#include "mht.h"

/*-------------------------------------------------------------------*
 | REPORT::setAllGroupIds() -- set the group id's of all the track
 |                             trees that use this REPORT
 |
 | This loops through each T_HYPO that is linked to the REPORT and
 | gets the track tree that it's on.  It then looks at the group id
 | of the tree.  If the group id has not yet been set, it simply sets
 | it.  If it has already been set to a different id, then that means
 | that the tree also contains a T_HYPO linked to another REPORT, and
 | all the trees linked to that REPORT have to be in the same group
 | as the trees linked to this REPORT.  So we search back in the list
 | of REPORTs, looking for those that have the tree's current group
 | id, and call setAllGroupIds() on them, recursively.
 *-------------------------------------------------------------------*/

void REPORT::setAllGroupIds( int groupId )
{
  BGN

  PTR_INTO_iDLIST_OF< REPORT > reportPtr;
  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;
  T_TREE *tree;
  int treeGroupId;

  m_groupId = groupId;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    tree = (*tHypoPtr).getTree();
    treeGroupId = tree->getGroupId();

    if( treeGroupId != groupId )
    {
      tree->setGroupId( groupId );
      if( treeGroupId != -1 )
      {
        for( (reportPtr = this),--reportPtr; reportPtr.isValid(); --reportPtr )
          if( (*reportPtr).m_groupId == treeGroupId )
            (*reportPtr).setAllGroupIds( groupId );
      }
    }
  }
}

/*-------------------------------------------------------------------*
 | REPORT::checkGroupIds() -- diagnostic
 *-------------------------------------------------------------------*/

void REPORT::checkGroupIds()
{
  BGN

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;
  T_TREE *tree;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    tree = (*tHypoPtr).getTree();

    if( tree->getGroupId() != m_groupId )
      THROW_ERR( "Bad group id for report" )
  }
}

/*-------------------------------------------------------------------*
 | REPORT::describe() -- diagnostic
 *-------------------------------------------------------------------*/

void REPORT::describe( int spaces )
{
  BGN

  PTR_INTO_LINKS_TO< T_HYPO > tHypoPtr;
  int k;

  Indent( spaces ); cout << "REPORT "; print(); cout << endl;

  Indent( spaces ); cout << "| tHypo's:";
  k = 0;

  LOOP_LINKS( tHypoPtr, m_tHypoLinks )
  {
    if( k++ >= 3 )
    {
      cout << endl;
      Indent( spaces ); cout << "|         ";
      k = 0;
    }

    cout << " "; (*tHypoPtr).print();
  }
  cout << endl;
}

