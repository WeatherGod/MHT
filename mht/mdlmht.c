
/*********************************************************************
 * FILE: mdlmht.C                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    6 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *    Member functions for model-based multiple hypothesis testing.  *
 *    See "mdlmht.H" for details.                                    *
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

#include "mdlmht.h"

/*-------------------------------------------------------------------*
 | MDL_MHT::measureAndValidate() -- collect reports of measurements
 |                                  and grow track trees
 *-------------------------------------------------------------------*/

void MDL_MHT::measureAndValidate()
{


    PTR_INTO_ptrDLIST_OF< T_HYPO > tHypoPtr;
    MDL_T_HYPO *tHypo;
    PTR_INTO_iDLIST_OF< REPORT > reportPtr;
    MDL_REPORT *report;
    MDL_ROOT_T_HYPO *root;

    /* get reports of measurements */
    measure();

    /* loop through all the active track hypotheses (leaves of the track
       trees), making children for each one */
    LOOP_DLIST( tHypoPtr, m_activeTHypoList )
    {
        tHypo = (MDL_T_HYPO *)tHypoPtr.get();

        tHypo->makeDefaultChildren();

        LOOP_DLIST( reportPtr, m_newReportList )
        {
            report = (MDL_REPORT *)reportPtr.get();
            tHypo->makeChildrenFor( report );
        }
    }

    /* make a new track tree for each reported measurement */
    LOOP_DLIST( reportPtr, m_newReportList )
    {
        report = (MDL_REPORT *)reportPtr.get();

        root = new MDL_ROOT_T_HYPO( this );
        installTree( root, -1 );
        root->makeDefaultChildren();
        root->makeChildrenFor( report );
    }
}

/*-------------------------------------------------------------------*
 | MDL_ROOT_T_HYPO::makeDefaultChildren() -- make the default
 |                                           children of a ROOT node
 |
 | This makes the children that are not linked to a report.
 *-------------------------------------------------------------------*/

void MDL_ROOT_T_HYPO::makeDefaultChildren()
{


    installChild( new MDL_DUMMY_T_HYPO( m_mdlMht ) );
}

/*-------------------------------------------------------------------*
 | MDL_ROOT_T_HYPO::makeChildrenFor() -- make children of a ROOT node
 |
 | This makes the children that ARE linked to a report.
 *-------------------------------------------------------------------*/

void MDL_ROOT_T_HYPO::makeChildrenFor( MDL_REPORT *report )
{


    PTR_INTO_ptrDLIST_OF< MODEL > modelPtr;
    MODEL *mdl;
    MDL_STATE *state;
    int numStartStates;
    int i;

    installChild( new MDL_FALARM_T_HYPO( m_mdlMht, report ) );

    LOOP_DLIST( modelPtr, m_mdlMht->m_modelList )
    {
        mdl = modelPtr.get();

        numStartStates = mdl->beginNewStates( 0, report );

        for( i = 0; i < numStartStates; i++ )
        {
            state = mdl->getNewState( i, 0, report );
            if( state != 0 )
                installChild( new MDL_START_T_HYPO( m_mdlMht,
                                                    state,
                                                    report ) );
        }

        mdl->endNewStates();
    }
}

/*-------------------------------------------------------------------*
 | MDL_DUMMY_T_HYPO::makeDefaultChildren() -- make default children
 |                                            for a DUMMY node
 |
 | This makes the children that are not linked to a report.
 *-------------------------------------------------------------------*/

void MDL_DUMMY_T_HYPO::makeDefaultChildren()
{


    installChild( new MDL_DUMMY_T_HYPO( m_mdlMht, m_logLikelihood ) );
}

/*-------------------------------------------------------------------*
 | MDL_CONTINUE_T_HYPO::makeDefaultChildren() -- make default
 |                                               children for a
 |                                               CONTINUE node
 |
 | This makes the children that are not linked to a report.
 *-------------------------------------------------------------------*/

void MDL_CONTINUE_T_HYPO::makeDefaultChildren()
{


    MODEL *mdl = m_state->getMdl();
    double endLogLikelihood = mdl->getEndLogLikelihood( m_state );
    double continueLogLikelihood =
        mdl->getContinueLogLikelihood( m_state );
    double skipLogLikelihood =
        mdl->getSkipLogLikelihood( m_state );
    MDL_STATE *state;
    int numNewStates;
    int i;

    if( endLogLikelihood != -INFINITY )
        installChild( new MDL_END_T_HYPO( m_mdlMht,
                                          m_logLikelihood,
                                          skipLogLikelihood,
                                          endLogLikelihood ) );

    if( continueLogLikelihood != -INFINITY )
    {
        numNewStates = mdl->beginNewStates( m_state, 0 );

        for( i = 0; i < numNewStates; i++ )
        {
            state = mdl->getNewState( i, m_state, 0 );
            if( state != 0 )
                installChild( new MDL_SKIP_T_HYPO( m_mdlMht,
                                                   m_logLikelihood,
                                                   continueLogLikelihood,
                                                   skipLogLikelihood,
                                                   state ) );
        }

        mdl->endNewStates();
    }
}

/*-------------------------------------------------------------------*
 | MDL_CONTINUE_T_HYPO::makeChildrenFor() -- make children for a
 |                                           CONTINUE node
 |
 | This makes the children that ARE linked to a report.
 *-------------------------------------------------------------------*/

void MDL_CONTINUE_T_HYPO::makeChildrenFor( MDL_REPORT *report )
{


    MODEL *mdl = m_state->getMdl();
    double continueLogLikelihood =
        mdl->getContinueLogLikelihood( m_state );
    double detectLogLikelihood =
        mdl->getDetectLogLikelihood( m_state );
    MDL_STATE *state;
    int numNewStates;
    int i;

    numNewStates = mdl->beginNewStates( m_state, report );

    for( i = 0; i < numNewStates; i++ )
    {
        state = mdl->getNewState( i, m_state, report );
        if( state != 0 )
            installChild( new MDL_CONTINUE_T_HYPO( m_mdlMht,
                                                   m_logLikelihood,
                                                   continueLogLikelihood,
                                                   detectLogLikelihood,
                                                   state, report ) );
    }

    mdl->endNewStates();
}
