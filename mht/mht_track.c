
/*********************************************************************
 * FILE: mht_track.C                                                 *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *   13 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Member functions for T_TREE and T_HYPO objects.  See mht.H.     *
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

#include "mht.h"

/*-------------------------------------------------------------------*
 | T_HYPO::describe() -- diagnostic
 *-------------------------------------------------------------------*/

void T_HYPO::describe( int spaces, int depth )
{
  BGN

  PTR_INTO_LINKS_TO< G_HYPO > gHypoPtr;
  int i, k;

  Indent( spaces - 1);
  for( i = 0; i < depth; i++ )
    cout << " |";
  cout << " **T_HYPO "; print(); cout << endl;

  Indent( spaces - 1 );
  for( i = 0; i <= depth; i++ )
    cout << " |";
  cout << "   track = " << getTrackStamp();
  cout << ", time = " << m_timeStamp;
  cout << ", group id = " << getGroupId();
  cout << endl;

  Indent( spaces - 1 );
  for( i = 0; i <= depth; i++ )
    cout << " |";
  if( m_reportLink.isEmpty() )
    cout << "   NO REPORT";
  else
  {
    cout << "   report = "; (*m_reportLink).print();
  }
  cout << ", logLikelihood = " << m_logLikelihood;
  if( ! m_mustVerify )
    cout << ", NEED NOT VERIFY";
  if( m_endsTrack )
    cout << ", ENDS TRACK";
  cout << endl;

  Indent( spaces - 1 );
  for( i = 0; i <= depth; i++ )
    cout << " |";
  cout << "   gHypo's:";
  k = 0;

  LOOP_LINKS( gHypoPtr, m_gHypoLinks )
  {
    if( k++ >= 3 )
    {
      cout << endl;
      Indent( spaces - 1 );
      for( i = 0; i <= depth; i++ )
        cout << " |";
      cout << "           ";
      k = 0;
    }

    cout << " "; (*gHypoPtr).print();
  }
  cout << endl;
}

/*-------------------------------------------------------------------*
 | T_HYPO::describeTree() -- recursive diagnostic
 *-------------------------------------------------------------------*/

void T_HYPO::describeTree( int spaces, int depth )
{
  BGN

  PTR_INTO_iTREE_OF< T_HYPO > childPtr;

  describe( spaces, depth );

  LOOP_TREEchildren( childPtr, this )
  {
    (*childPtr).describeTree( spaces, depth + 1 );
  }
}

