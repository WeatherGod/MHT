
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

    PTR_INTO_LINKS_TO< G_HYPO > gHypoPtr;
    int i, k;

    Indent( spaces - 1);
    for( i = 0; i < depth; i++ )
    {
        std::cout << " |";
    }
    std::cout << " **T_HYPO ";
    print();
    std::cout << std::endl;

    Indent( spaces - 1 );
    for( i = 0; i <= depth; i++ )
    {
        std::cout << " |";
    }
    std::cout << "   track = " << getTrackStamp();
    std::cout << ", time = " << m_timeStamp;
    std::cout << ", group id = " << getGroupId();
    std::cout << std::endl;

    Indent( spaces - 1 );
    for( i = 0; i <= depth; i++ )
    {
        std::cout << " |";
    }
    if( m_reportLink.isEmpty() )
    {
        std::cout << "   NO REPORT";
    }
    else
    {
        std::cout << "   report = ";
        (*m_reportLink).print();
    }
    std::cout << ", logLikelihood = " << m_logLikelihood;
    if( ! m_mustVerify )
    {
        std::cout << ", NEED NOT VERIFY";
    }
    if( m_endsTrack )
    {
        std::cout << ", ENDS TRACK";
    }
    std::cout << std::endl;

    Indent( spaces - 1 );
    for( i = 0; i <= depth; i++ )
    {
        std::cout << " |";
    }
    std::cout << "   gHypo's:";
    k = 0;

    LOOP_LINKS( gHypoPtr, m_gHypoLinks )
    {
        if( k++ >= 3 )
        {
            std::cout << std::endl;
            Indent( spaces - 1 );
            for( i = 0; i <= depth; i++ )
            {
                std::cout << " |";
            }
            std::cout << "           ";
            k = 0;
        }

        std::cout << " ";
        (*gHypoPtr).print();
    }
    std::cout << std::endl;
}

/*-------------------------------------------------------------------*
 | T_HYPO::describeTree() -- recursive diagnostic
 *-------------------------------------------------------------------*/

void T_HYPO::describeTree( int spaces, int depth )
{

    PTR_INTO_iTREE_OF< T_HYPO > childPtr;

    describe( spaces, depth );

    LOOP_TREEchildren( childPtr, this )
    {
        (*childPtr).describeTree( spaces, depth + 1 );
    }
}

