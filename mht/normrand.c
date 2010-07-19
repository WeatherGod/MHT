
/*********************************************************************
 * FILE: normRand.C                                                  *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    8 AUG 93 -- (mlm) commented                                    *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Routines for generating normally-distributed random numbers.    *
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

#include "normrand.h"

/*-------------------------------------------------------------------*
 | SNormRand() -- seed the random number generator
 *-------------------------------------------------------------------*/

void SNormRand( long seed )
{
    srand48( seed );
}

/*-------------------------------------------------------------------*
 | NormRand() -- generate a random number
 |
 | The method for this is from
 |
 |   Numerical Recipes
 |
 | I've changed the variable names from their (even more cryptic)
 | psuedocode.  Otherwise it's just a straight translation into C.
 *-------------------------------------------------------------------*/

double NormRand()
{
    static double extraDeviate;
    static char weHaveExtraDeviate;
    double fac, r, v1, v2;

    if( ! weHaveExtraDeviate )
    {
        do
        {
            v1 = 2. * drand48() - 1.;
            v2 = 2. * drand48() - 1.;
            r = v1 * v1 + v2 * v2;
        }
        while( r >= 1. || r == 0. );

        fac = sqrt( -2. * log( r ) / r );

        extraDeviate = v1 * fac;
        weHaveExtraDeviate = 1;

        return v2 * fac;
    }
    else
    {
        weHaveExtraDeviate = 0;

        return extraDeviate;
    }
}

